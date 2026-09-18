/*
 * ====================================================================
 * ZMK SCADA Industrial Status Screen - 284x76 Green Monochrome
 * Integrated with ZMK Event System
 * ====================================================================
 */

#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/ble.h>
#include <zmk/usb.h>

#include <hid.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

LV_FONT_DECLARE(vt323_16);

/* ================= CONFIGURATION ================= */
#define SCREEN_WIDTH        284
#define SCREEN_HEIGHT       76

#define COLOR_BG            0x000000
#define COLOR_FG            0x33FF33  // Matching existing CRT green
#define COLOR_BG_DIM        0x050F05

#define VOL_POPUP_TIMEOUT   1500
#define LAYER_BLINK_TIME    150

/* ================= GLOBAL STATE ================= */
static lv_obj_t *scada_screen = NULL;

// Central multipurpose area
static lv_obj_t *center_box = NULL;
static lv_obj_t *clock_label = NULL;
static lv_obj_t *clock_cursor = NULL;
static lv_obj_t *vol_popup_container = NULL;
static lv_obj_t *vol_popup_text = NULL;
static lv_obj_t *vol_popup_bar = NULL;
static lv_timer_t *vol_timer = NULL;
static lv_timer_t *cursor_timer = NULL;

// Status widgets
static lv_obj_t *top_right_label = NULL;
static lv_obj_t *layer_badge = NULL;
static lv_obj_t *layer_label = NULL;
static lv_obj_t *bat_left_label = NULL;
static lv_obj_t *bat_right_label = NULL;
static lv_obj_t *mods_label = NULL;

// State tracking
static uint8_t current_volume = 50;
static uint8_t current_hour = 0;
static uint8_t current_minute = 0;
static uint8_t current_layer = 0;
static uint8_t current_mods = 0;
static uint8_t bat_left_pct = 100;
static uint8_t bat_right_pct = 100;

/* ================= UTILITY FUNCTIONS ================= */

static void generate_pixel_bar(int percent, int total_blocks, char *out_buf, size_t buf_size) {
    if (percent > 100) percent = 100;
    if (percent < 0) percent = 0;

    int filled = (percent * total_blocks + 50) / 100;
    if (filled > total_blocks) filled = total_blocks;

    snprintf(out_buf, buf_size, "[");
    size_t len = strlen(out_buf);

    for (int i = 0; i < total_blocks && len < buf_size - 2; i++) {
        strcat(out_buf, (i < filled) ? "█" : "░");
        len++;
    }
    strcat(out_buf, "]");
}

/* ================= CURSOR ANIMATION ================= */

static void cursor_blink_cb(lv_timer_t *timer) {
    if (!clock_cursor) return;
    lv_opa_t opa = lv_obj_get_style_bg_opa(clock_cursor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(clock_cursor,
                            opa == LV_OPA_TRANSP ? LV_OPA_COVER : LV_OPA_TRANSP,
                            LV_PART_MAIN);
}

/* ================= VOLUME POPUP LOGIC ================= */

static void vol_popup_timeout_cb(lv_timer_t *timer) {
    if (vol_popup_container && clock_label && clock_cursor) {
        lv_obj_add_flag(vol_popup_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(clock_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(clock_cursor, LV_OBJ_FLAG_HIDDEN);
    }
    vol_timer = NULL;
}

static void show_volume_popup(uint8_t volume_percent) {
    if (!scada_screen || volume_percent > 100) return;

    current_volume = volume_percent;
    char bar_str[20], text_buf[32];

    // Update top-right status
    snprintf(text_buf, sizeof(text_buf), "VOL:%2d%% BLE.%d",
             volume_percent, zmk_ble_active_profile_index() + 1);
    lv_label_set_text(top_right_label, text_buf);

    // Update central popup
    snprintf(text_buf, sizeof(text_buf), "VOL: %d%%", volume_percent);
    lv_label_set_text(vol_popup_text, text_buf);

    generate_pixel_bar(volume_percent, 10, bar_str, sizeof(bar_str));
    lv_label_set_text(vol_popup_bar, bar_str);

    // Switch display mode
    lv_obj_add_flag(clock_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(clock_cursor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(vol_popup_container, LV_OBJ_FLAG_HIDDEN);

    // Reset auto-hide timer
    if (vol_timer) {
        lv_timer_reset(vol_timer);
    } else {
        vol_timer = lv_timer_create(vol_popup_timeout_cb, VOL_POPUP_TIMEOUT, NULL);
        lv_timer_set_repeat_count(vol_timer, 1);
    }
}

/* ================= LAYER ANIMATION ================= */

static void layer_blink_exec_cb(void *var, int32_t value) {
    lv_obj_set_style_bg_opa((lv_obj_t *)var, value, 0);
}

static void update_layer_display(uint8_t layer_index) {
    if (!layer_label || !layer_badge) return;

    current_layer = layer_index;
    const char *layer_name = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(layer_index));

    char buf[40];
    snprintf(buf, sizeof(buf), "[ #%02d %s ]",
             layer_index, layer_name ? layer_name : "???");
    lv_label_set_text(layer_label, buf);

    // Trigger blink animation
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, layer_badge);
    lv_anim_set_values(&anim, LV_OPA_50, LV_OPA_COVER);
    lv_anim_set_time(&anim, LAYER_BLINK_TIME);
    lv_anim_set_exec_cb(&anim, layer_blink_exec_cb);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
}

/* ================= BATTERY DISPLAY ================= */

static void update_battery_display(void) {
    if (!bat_left_label || !bat_right_label) return;

    char bar[20], buf[48];

    // Left battery
    generate_pixel_bar(bat_left_pct, 8, bar, sizeof(bar));
    snprintf(buf, sizeof(buf), "L:%s %d%%", bar, bat_left_pct);
    lv_label_set_text(bat_left_label, buf);

    // Right battery
    generate_pixel_bar(bat_right_pct, 8, bar, sizeof(bar));
    snprintf(buf, sizeof(buf), "R:%s %d%%", bar, bat_right_pct);
    lv_label_set_text(bat_right_label, buf);
}

/* ================= MODIFIERS DISPLAY ================= */

static void update_modifiers_display(uint8_t mods_mask) {
    if (!mods_label) return;

    current_mods = mods_mask;
    char buf[80];
    snprintf(buf, sizeof(buf), "%s %s %s %s",
             (mods_mask & 0x01) ? "[GUI]"  : "#GUI",
             (mods_mask & 0x02) ? "[ALT]"  : "#ALT",
             (mods_mask & 0x04) ? "[CTRL]" : "#CTRL",
             (mods_mask & 0x08) ? "[SHFT]" : "#SHFT");
    lv_label_set_text(mods_label, buf);
}

/* ================= TIME DISPLAY ================= */

static void update_time_display(uint8_t hour, uint8_t minute) {
    if (!clock_label) return;

    current_hour = hour;
    current_minute = minute;

    char buf[16];
    snprintf(buf, sizeof(buf), "> %02d:%02d", hour, minute);
    lv_label_set_text(clock_label, buf);
}

/* ================= ZMK EVENT HANDLERS ================= */

// Volume notification handler
static struct volume_notification get_volume(const zmk_event_t *eh) {
    struct volume_notification *notification = as_volume_notification(eh);
    if (notification) {
        return *notification;
    }
    return (struct volume_notification){.value = current_volume};
}

static void volume_update_cb(struct volume_notification volume) {
    show_volume_popup(volume.value);
}

ZMK_DISPLAY_WIDGET_LISTENER(scada_volume, struct volume_notification,
                            volume_update_cb, get_volume)
ZMK_SUBSCRIPTION(scada_volume, volume_notification);

// Time notification handler
static struct time_notification get_time(const zmk_event_t *eh) {
    struct time_notification *notification = as_time_notification(eh);
    if (notification) {
        return *notification;
    }
    return (struct time_notification){.hour = current_hour, .minute = current_minute};
}

static void time_update_cb(struct time_notification time) {
    update_time_display(time.hour, time.minute);
}

ZMK_DISPLAY_WIDGET_LISTENER(scada_time, struct time_notification,
                            time_update_cb, get_time)
ZMK_SUBSCRIPTION(scada_time, time_notification);

// Layer state handler
static struct layer_state {
    uint8_t index;
} layer_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_state){.index = index};
}

static void layer_update_cb(struct layer_state state) {
    update_layer_display(state.index);
}

ZMK_DISPLAY_WIDGET_LISTENER(scada_layer, struct layer_state,
                            layer_update_cb, layer_get_state)
ZMK_SUBSCRIPTION(scada_layer, zmk_layer_state_changed);

// Modifier state tracking
#include <dt-bindings/zmk/hid_usage.h>
#include <dt-bindings/zmk/hid_usage_pages.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

struct modifier_state {
    bool gui;
    bool alt;
    bool ctrl;
    bool shift;
};

static struct modifier_state mod_state = {false, false, false, false};

static void update_modifier_from_state(void) {
    uint8_t mask = 0;
    if (mod_state.gui)   mask |= 0x01;
    if (mod_state.alt)   mask |= 0x02;
    if (mod_state.ctrl)  mask |= 0x04;
    if (mod_state.shift) mask |= 0x08;
    update_modifiers_display(mask);
}

static void update_modifier_key_state(uint8_t keycode, bool pressed) {
    bool changed = false;

    switch (keycode) {
    case HID_USAGE_KEY_KEYBOARD_LEFTSHIFT:
    case HID_USAGE_KEY_KEYBOARD_RIGHTSHIFT:
        if (mod_state.shift != pressed) {
            mod_state.shift = pressed;
            changed = true;
        }
        break;
    case HID_USAGE_KEY_KEYBOARD_LEFTCONTROL:
    case HID_USAGE_KEY_KEYBOARD_RIGHTCONTROL:
        if (mod_state.ctrl != pressed) {
            mod_state.ctrl = pressed;
            changed = true;
        }
        break;
    case HID_USAGE_KEY_KEYBOARD_LEFTALT:
    case HID_USAGE_KEY_KEYBOARD_RIGHTALT:
        if (mod_state.alt != pressed) {
            mod_state.alt = pressed;
            changed = true;
        }
        break;
    case HID_USAGE_KEY_KEYBOARD_LEFT_GUI:
    case HID_USAGE_KEY_KEYBOARD_RIGHT_GUI:
        if (mod_state.gui != pressed) {
            mod_state.gui = pressed;
            changed = true;
        }
        break;
    }

    if (changed) {
        update_modifier_from_state();
    }
}

static int scada_keycode_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *event = as_zmk_keycode_state_changed(eh);
    if (event && is_mod(event->usage_page, event->keycode)) {
        update_modifier_key_state(event->keycode, event->state);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(scada_mods, scada_keycode_listener)
ZMK_SUBSCRIPTION(scada_mods, zmk_keycode_state_changed);

// Battery state handler
static int battery_status_update_cb(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    if (ev) {
        // Assume split keyboard - alternate updates for left/right
        // You may need to adjust this based on your specific setup
        static bool is_left = true;

        if (is_left) {
            bat_left_pct = ev->state_of_charge;
        } else {
            bat_right_pct = ev->state_of_charge;
        }
        is_left = !is_left;

        update_battery_display();
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(scada_battery, battery_status_update_cb)
ZMK_SUBSCRIPTION(scada_battery, zmk_battery_state_changed);

/* ================= UI CONSTRUCTION ================= */

lv_obj_t *zmk_display_status_screen(void) {
    // Create main screen
    scada_screen = lv_obj_create(NULL);
    lv_obj_set_size(scada_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(scada_screen, lv_color_hex(COLOR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scada_screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(scada_screen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scada_screen, 0, LV_PART_MAIN);
    lv_obj_clear_flag(scada_screen, LV_OBJ_FLAG_SCROLLABLE);

    /* ========== LEFT SIDE STATUS ========== */

    // Left battery (top-left)
    bat_left_label = lv_label_create(scada_screen);
    lv_obj_set_style_text_font(bat_left_label, &vt323_16, 0);
    lv_obj_set_style_text_color(bat_left_label, lv_color_hex(COLOR_FG), 0);
    lv_obj_align(bat_left_label, LV_ALIGN_TOP_LEFT, 4, 3);

    // Right battery
    bat_right_label = lv_label_create(scada_screen);
    lv_obj_set_style_text_font(bat_right_label, &vt323_16, 0);
    lv_obj_set_style_text_color(bat_right_label, lv_color_hex(COLOR_FG), 0);
    lv_obj_align(bat_right_label, LV_ALIGN_TOP_LEFT, 4, 17);

    update_battery_display();

    // Modifiers (bottom-left)
    mods_label = lv_label_create(scada_screen);
    lv_obj_set_style_text_font(mods_label, &vt323_16, 0);
    lv_obj_set_style_text_color(mods_label, lv_color_hex(COLOR_FG), 0);
    lv_obj_align(mods_label, LV_ALIGN_BOTTOM_LEFT, 4, -3);
    update_modifiers_display(0);

    /* ========== TOP-RIGHT STATUS ========== */

    top_right_label = lv_label_create(scada_screen);
    lv_obj_set_style_text_font(top_right_label, &vt323_16, 0);
    lv_obj_set_style_text_color(top_right_label, lv_color_hex(COLOR_FG), 0);
    lv_obj_align(top_right_label, LV_ALIGN_TOP_RIGHT, -4, 3);
    lv_label_set_text(top_right_label, "VOL:50% BLE.1");

    /* ========== BOTTOM-RIGHT LAYER BADGE ========== */

    layer_badge = lv_obj_create(scada_screen);
    lv_obj_set_size(layer_badge, LV_SIZE_CONTENT, 18);
    lv_obj_set_style_bg_color(layer_badge, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_bg_opa(layer_badge, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(layer_badge, 0, 0);
    lv_obj_set_style_radius(layer_badge, 0, 0);
    lv_obj_set_style_pad_hor(layer_badge, 3, 0);
    lv_obj_set_style_pad_ver(layer_badge, 2, 0);
    lv_obj_align(layer_badge, LV_ALIGN_BOTTOM_RIGHT, -4, -3);
    lv_obj_clear_flag(layer_badge, LV_OBJ_FLAG_SCROLLABLE);

    layer_label = lv_label_create(layer_badge);
    lv_obj_set_style_text_color(layer_label, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_text_font(layer_label, &vt323_16, 0);
    lv_obj_center(layer_label);

    update_layer_display(0);

    /* ========== CENTER CORE AREA ========== */

    // Double-line bordered box
    center_box = lv_obj_create(scada_screen);
    lv_obj_set_size(center_box, 120, 48);
    lv_obj_center(center_box);
    lv_obj_set_y(center_box, -2);
    lv_obj_set_style_bg_color(center_box, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_bg_opa(center_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(center_box, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_border_width(center_box, 2, 0);
    lv_obj_set_style_radius(center_box, 0, 0);
    lv_obj_set_style_pad_all(center_box, 0, 0);
    lv_obj_clear_flag(center_box, LV_OBJ_FLAG_SCROLLABLE);

    // Time display with cursor
    lv_obj_t *time_container = lv_obj_create(center_box);
    lv_obj_set_size(time_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(time_container);
    lv_obj_set_style_bg_opa(time_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(time_container, 0, 0);
    lv_obj_set_style_pad_all(time_container, 0, 0);
    lv_obj_set_style_pad_column(time_container, 3, 0);
    lv_obj_set_flex_flow(time_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(time_container, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(time_container, LV_OBJ_FLAG_SCROLLABLE);

    clock_label = lv_label_create(time_container);
    lv_obj_set_style_text_color(clock_label, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_text_font(clock_label, &vt323_16, 0);
    lv_label_set_text(clock_label, "> 00:00");

    clock_cursor = lv_obj_create(time_container);
    lv_obj_set_size(clock_cursor, 6, 12);
    lv_obj_set_style_bg_color(clock_cursor, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_bg_opa(clock_cursor, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(clock_cursor, 0, 0);
    lv_obj_set_style_radius(clock_cursor, 0, 0);
    lv_obj_clear_flag(clock_cursor, LV_OBJ_FLAG_SCROLLABLE);

    cursor_timer = lv_timer_create(cursor_blink_cb, 500, NULL);

    // Volume popup (hidden initially)
    vol_popup_container = lv_obj_create(center_box);
    lv_obj_set_size(vol_popup_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(vol_popup_container);
    lv_obj_set_style_bg_opa(vol_popup_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vol_popup_container, 0, 0);
    lv_obj_set_style_pad_all(vol_popup_container, 0, 0);
    lv_obj_set_flex_flow(vol_popup_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(vol_popup_container, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(vol_popup_container, LV_OBJ_FLAG_SCROLLABLE);

    vol_popup_text = lv_label_create(vol_popup_container);
    lv_obj_set_style_text_color(vol_popup_text, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_text_font(vol_popup_text, &vt323_16, 0);
    lv_label_set_text(vol_popup_text, "VOL: 50%");

    vol_popup_bar = lv_label_create(vol_popup_container);
    lv_obj_set_style_text_color(vol_popup_bar, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_text_font(vol_popup_bar, &vt323_16, 0);
    lv_label_set_text(vol_popup_bar, "[█████░░░░░]");
    lv_obj_set_style_pad_top(vol_popup_bar, 2, 0);

    lv_obj_add_flag(vol_popup_container, LV_OBJ_FLAG_HIDDEN);

    // Initialize all event listeners
    scada_volume_init();
    scada_time_init();
    scada_layer_init();
    scada_mods_init();
    scada_battery_init();

    // Initialize modifier state from current HID state
    zmk_mod_flags_t mods = zmk_hid_get_explicit_mods();
    mod_state.shift = (mods & (MOD_LSFT | MOD_RSFT)) != 0;
    mod_state.ctrl = (mods & (MOD_LCTL | MOD_RCTL)) != 0;
    mod_state.alt = (mods & (MOD_LALT | MOD_RALT)) != 0;
    mod_state.gui = (mods & (MOD_LGUI | MOD_RGUI)) != 0;
    update_modifier_from_state();

    LOG_INF("SCADA status screen initialized");

    return scada_screen;
}
