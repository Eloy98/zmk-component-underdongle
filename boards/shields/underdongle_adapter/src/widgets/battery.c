#include "battery.h"

#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/split_central_status_changed.h>

LV_FONT_DECLARE(vt323_16);

#define CRT_GREEN 0x33FF33
#define CRT_BACKGROUND 0x050F05

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct battery_state {
    uint8_t source;
    uint8_t level;
};

struct connection_status {
    uint8_t source;
    bool connected;
};

static void set_battery_state(struct zmk_widget_battery *widget, struct battery_state state,
                              bool is_initialized) {
    if (!is_initialized || state.source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        return;
    }

    lv_label_set_text_fmt(widget->battery_label, "[BAT:%d%%]", state.level);
}

static void set_connection_status(struct zmk_widget_battery *widget,
                                  struct connection_status status, bool is_initialized) {
    if (!is_initialized || status.source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        return;
    }

    lv_label_set_text_fmt(widget->ble_label, "[BLE:%d]", status.connected ? 1 : 0);
}

void battery_state_update_cb(struct battery_state state) {
    struct zmk_widget_battery *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_battery_state(widget, state, widget->initialized);
    }
}

static struct battery_state get_battery_state(const zmk_event_t *eh) {
    if (eh == NULL) {
        return (struct battery_state){.source = 0, .level = 0};
    }

    const struct zmk_peripheral_battery_state_changed *bat_ev =
        as_zmk_peripheral_battery_state_changed(eh);
    if (bat_ev == NULL) {
        return (struct battery_state){.source = 0, .level = 0};
    }

    return (struct battery_state){.source = bat_ev->source, .level = bat_ev->state_of_charge};
}

void connection_status_update_cb(struct connection_status status) {
    struct zmk_widget_battery *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_connection_status(widget, status, widget->initialized);
    }
}

static struct connection_status get_connection_status(const zmk_event_t *eh) {
    if (eh == NULL) {
        return (struct connection_status){.source = 0, .connected = false};
    }

    const struct zmk_split_central_status_changed *conn_ev =
        as_zmk_split_central_status_changed(eh);
    if (conn_ev == NULL) {
        return (struct connection_status){.source = 0, .connected = false};
    }

    return (struct connection_status){.source = conn_ev->slot, .connected = conn_ev->connected};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_state, struct battery_state, battery_state_update_cb,
                            get_battery_state);
ZMK_SUBSCRIPTION(widget_battery_state, zmk_peripheral_battery_state_changed);

ZMK_DISPLAY_WIDGET_LISTENER(widget_connection_status, struct connection_status,
                            connection_status_update_cb, get_connection_status);
ZMK_SUBSCRIPTION(widget_connection_status, zmk_split_central_status_changed);

int zmk_widget_battery_init(struct zmk_widget_battery *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 272, 18);
    lv_obj_set_style_bg_color(widget->obj, lv_color_hex(CRT_BACKGROUND), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

    widget->battery_label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->battery_label, &vt323_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->battery_label, lv_color_hex(CRT_GREEN), LV_PART_MAIN);
    lv_label_set_text(widget->battery_label, "[BAT:--%]");
    lv_obj_align(widget->battery_label, LV_ALIGN_LEFT_MID, 0, 0);

    widget->ble_label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->ble_label, &vt323_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->ble_label, lv_color_hex(CRT_GREEN), LV_PART_MAIN);
    lv_label_set_text(widget->ble_label, "[BLE:0]");
    lv_obj_align(widget->ble_label, LV_ALIGN_RIGHT_MID, 0, 0);

    sys_slist_append(&widgets, &widget->node);

    widget->initialized = true;
    widget_connection_status_init();
    widget_battery_state_init();

    return 0;
}

lv_obj_t *zmk_widget_battery_obj(struct zmk_widget_battery *widget) { return widget->obj; }
