/*
 * SCADA / CRT dongle UI — ZMK data glue (the ONLY file that touches ZMK APIs).
 *
 * Every listener uses ZMK_DISPLAY_WIDGET_LISTENER so its callback runs on the
 * ZMK display work queue — the one context where touching LVGL is safe. The UI
 * is updated exclusively through the dongle_ui_update_* API (dongle_ui.h); this
 * file never creates or reaches into LVGL objects directly.
 *
 * Also provides zmk_display_status_screen(): the single ZMK display entry point.
 */
#include "dongle_ui.h"

#include <zephyr/kernel.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/split_central_status_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/hid.h>

#if IS_ENABLED(CONFIG_ZMK_BLE)
#include <zmk/ble.h>
#include <zmk/events/ble_active_profile_changed.h>
#endif

#ifdef CONFIG_RAW_HID
#include <hid.h>
#endif

/* ================= L / R peripheral batteries ================= *
 * source/slot 0 -> left half, 1 -> right half. Battery level and connection
 * arrive as two separate events; we cache both and re-render together. */
static uint8_t s_pct[2];
static bool s_conn[2];

static void render_batteries(void) {
    dongle_ui_update_left_battery(s_pct[0], s_conn[0]);
    dongle_ui_update_right_battery(s_pct[1], s_conn[1]);
}

struct batt_evt {
    uint8_t source;
    uint8_t level;
    bool valid;
};

static struct batt_evt batt_get(const zmk_event_t *eh) {
    if (eh == NULL) {
        return (struct batt_evt){.valid = false}; /* init prime: no-op */
    }
    const struct zmk_peripheral_battery_state_changed *ev =
        as_zmk_peripheral_battery_state_changed(eh);
    if (ev == NULL) {
        return (struct batt_evt){.valid = false};
    }
    return (struct batt_evt){.source = ev->source, .level = ev->state_of_charge, .valid = true};
}

static void batt_cb(struct batt_evt e) {
    if (!e.valid || e.source >= 2) {
        return;
    }
    s_pct[e.source] = e.level;
    s_conn[e.source] = true; /* a fresh battery report implies the half is connected */
    render_batteries();
}

ZMK_DISPLAY_WIDGET_LISTENER(du_batt, struct batt_evt, batt_cb, batt_get)
ZMK_SUBSCRIPTION(du_batt, zmk_peripheral_battery_state_changed);

struct conn_evt {
    uint8_t slot;
    bool connected;
    bool valid;
};

static struct conn_evt conn_get(const zmk_event_t *eh) {
    if (eh == NULL) {
        return (struct conn_evt){.valid = false};
    }
    const struct zmk_split_central_status_changed *ev =
        as_zmk_split_central_status_changed(eh);
    if (ev == NULL) {
        return (struct conn_evt){.valid = false};
    }
    return (struct conn_evt){.slot = ev->slot, .connected = ev->connected, .valid = true};
}

static void conn_cb(struct conn_evt e) {
    if (!e.valid || e.slot >= 2) {
        return;
    }
    s_conn[e.slot] = e.connected;
    render_batteries();
}

ZMK_DISPLAY_WIDGET_LISTENER(du_conn, struct conn_evt, conn_cb, conn_get)
ZMK_SUBSCRIPTION(du_conn, zmk_split_central_status_changed);

/* ================= layer ================= */
struct layer_evt {
    uint8_t index;
};

static struct layer_evt layer_get(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    return (struct layer_evt){.index = zmk_keymap_highest_layer_active()};
}

static void layer_cb(struct layer_evt e) {
    const char *name = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(e.index));
    dongle_ui_update_layer(e.index, name);
}

ZMK_DISPLAY_WIDGET_LISTENER(du_layer, struct layer_evt, layer_cb, layer_get)
ZMK_SUBSCRIPTION(du_layer, zmk_layer_state_changed);

/* ================= modifiers ================= *
 * Fires on every keycode event; we read the aggregate explicit-mods bitmap and
 * skip redundant redraws. This routes through the display queue (LVGL-safe),
 * unlike a plain ZMK_LISTENER. */
struct mods_evt {
    zmk_mod_flags_t mods;
};

static struct mods_evt mods_get(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    return (struct mods_evt){.mods = zmk_hid_get_explicit_mods()};
}

static void mods_cb(struct mods_evt e) {
    static int last = -1;
    if ((int)e.mods == last) {
        return;
    }
    last = e.mods;
    dongle_ui_update_modifiers((e.mods & (MOD_LGUI | MOD_RGUI)) != 0,
                               (e.mods & (MOD_LALT | MOD_RALT)) != 0,
                               (e.mods & (MOD_LCTL | MOD_RCTL)) != 0,
                               (e.mods & (MOD_LSFT | MOD_RSFT)) != 0);
}

ZMK_DISPLAY_WIDGET_LISTENER(du_mods, struct mods_evt, mods_cb, mods_get)
ZMK_SUBSCRIPTION(du_mods, zmk_keycode_state_changed);

/* ================= BLE active profile ================= */
#if IS_ENABLED(CONFIG_ZMK_BLE)
struct ble_evt {
    uint8_t index;
};

static struct ble_evt ble_get(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    return (struct ble_evt){.index = zmk_ble_active_profile_index()};
}

static void ble_cb(struct ble_evt e) {
    dongle_ui_update_ble(e.index);
}

ZMK_DISPLAY_WIDGET_LISTENER(du_ble, struct ble_evt, ble_cb, ble_get)
ZMK_SUBSCRIPTION(du_ble, zmk_ble_active_profile_changed);
#endif /* CONFIG_ZMK_BLE */

/* ================= volume + time (raw HID from companion app) ================= *
 * value/hour == 0xFF is a sentinel meaning "init prime" -> ignored, so boot
 * shows the default clock and never a spurious volume HUD. */
#ifdef CONFIG_RAW_HID
static struct volume_notification vol_get(const zmk_event_t *eh) {
    struct volume_notification *n = as_volume_notification(eh);
    if (n) {
        return *n;
    }
    return (struct volume_notification){.value = 0xFF};
}

static void vol_cb(struct volume_notification v) {
    if (v.value == 0xFF) {
        return;
    }
    dongle_ui_update_volume(v.value);
}

ZMK_DISPLAY_WIDGET_LISTENER(du_vol, struct volume_notification, vol_cb, vol_get)
ZMK_SUBSCRIPTION(du_vol, volume_notification);

static struct time_notification time_get(const zmk_event_t *eh) {
    struct time_notification *n = as_time_notification(eh);
    if (n) {
        return *n;
    }
    return (struct time_notification){.hour = 0xFF, .minute = 0xFF};
}

static void time_cb(struct time_notification t) {
    if (t.hour == 0xFF) {
        return;
    }
    dongle_ui_update_time(t.hour, t.minute);
}

ZMK_DISPLAY_WIDGET_LISTENER(du_time, struct time_notification, time_cb, time_get)
ZMK_SUBSCRIPTION(du_time, time_notification);
#endif /* CONFIG_RAW_HID */

/* ================= ZMK entry point ================= */
lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);

    dongle_main_screen_create(screen);

    /* Register + prime listeners. All getters are NULL-safe; batt/conn/vol/time
     * treat the init prime as a no-op, while layer/mods/ble prime real state. */
    du_batt_init();
    du_conn_init();
    du_layer_init();
    du_mods_init();
#if IS_ENABLED(CONFIG_ZMK_BLE)
    du_ble_init();
#endif
#ifdef CONFIG_RAW_HID
    du_vol_init();
    du_time_init();
#endif

    /* Opaque boot splash on top; it self-animates 0->100%, fades, then deletes
     * itself to reveal the main UI underneath. */
    dongle_boot_screen_start(screen);

    return screen;
}
