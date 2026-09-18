#include "time.h"

#include <hid.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
LV_FONT_DECLARE(vt323_16);

#define CRT_GREEN 0x33FF33
#define CRT_BACKGROUND 0x050F05

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static struct time_notification get_time(const zmk_event_t *eh) {
    struct time_notification *notification = as_time_notification(eh);
    if (notification) {
        return *notification;
    }

    return (struct time_notification){.hour = 0, .minute = 0};
}

static void time_update_cb(struct time_notification time) {
    struct zmk_widget_time *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        lv_label_set_text_fmt(widget->label, "> %02i:%02i", time.hour, time.minute);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_time, struct time_notification, time_update_cb, get_time)
ZMK_SUBSCRIPTION(widget_time, time_notification);

static void cursor_timer_cb(lv_timer_t *timer) {
    lv_obj_t *cursor = timer->user_data;
    lv_opa_t opa = lv_obj_get_style_bg_opa(cursor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cursor, opa == LV_OPA_TRANSP ? LV_OPA_COVER : LV_OPA_TRANSP,
                            LV_PART_MAIN);
}

int zmk_widget_time_init(struct zmk_widget_time *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, 20);
    lv_obj_set_style_bg_color(widget->obj, lv_color_hex(CRT_BACKGROUND), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(widget->obj, 3, LV_PART_MAIN);
    lv_obj_set_flex_flow(widget->obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(widget->obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

    widget->label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->label, &vt323_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->label, lv_color_hex(CRT_GREEN), LV_PART_MAIN);
    lv_label_set_text(widget->label, "> 00:00");

    widget->cursor = lv_obj_create(widget->obj);
    lv_obj_set_size(widget->cursor, 6, 12);
    lv_obj_set_style_bg_color(widget->cursor, lv_color_hex(CRT_GREEN), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->cursor, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->cursor, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(widget->cursor, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->cursor, 0, LV_PART_MAIN);
    lv_obj_clear_flag(widget->cursor, LV_OBJ_FLAG_SCROLLABLE);

    widget->cursor_timer = lv_timer_create(cursor_timer_cb, 500, widget->cursor);

    sys_slist_append(&widgets, &widget->node);

    widget_time_init();

    return 0;
}

lv_obj_t *zmk_widget_time_obj(struct zmk_widget_time *widget) { return widget->obj; }
