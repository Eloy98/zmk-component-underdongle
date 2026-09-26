#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

LV_FONT_DECLARE(vt323_16);

#include "widgets/battery.h"
#include "widgets/layer.h"
#include "widgets/modifiers.h"
#include "widgets/time.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static struct zmk_widget_layer layer_widget;
static struct zmk_widget_battery battery_widget;
static struct zmk_widget_modifiers modifiers_widget;
static struct zmk_widget_time time_widget;

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, 284, 76);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x050F05), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(screen, lv_color_hex(0x33FF33), LV_PART_MAIN);
    lv_obj_set_style_border_width(screen, 2, LV_PART_MAIN);
    lv_obj_set_style_border_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(screen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(screen, 4, LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    zmk_widget_battery_init(&battery_widget, screen);
    lv_obj_align(zmk_widget_battery_obj(&battery_widget), LV_ALIGN_TOP_MID, 0, 0);

    zmk_widget_time_init(&time_widget, screen);
    lv_obj_align(zmk_widget_time_obj(&time_widget), LV_ALIGN_CENTER, 0, 0);

    zmk_widget_modifiers_init(&modifiers_widget, screen);
    lv_obj_align(zmk_widget_modifiers_obj(&modifiers_widget), LV_ALIGN_BOTTOM_LEFT, 0, 0);

    zmk_widget_layer_init(&layer_widget, screen);
    lv_obj_align(zmk_widget_layer_obj(&layer_widget), LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    return screen;
}
