#include "modifiers.h"

#include <dt-bindings/zmk/hid_usage.h>
#include <dt-bindings/zmk/hid_usage_pages.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

LV_FONT_DECLARE(vt323_16);

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

#define MOD_ACTIVE "#33FF33"
#define MOD_INACTIVE "#1A4D1A"

struct modifier_state {
    bool gui;
    bool alt;
    bool ctrl;
    bool shift;
};

static struct modifier_state current_state = {
    .gui = false,
    .alt = false,
    .ctrl = false,
    .shift = false,
};

static void modifier_update_cb(struct modifier_state state) {
    struct zmk_widget_modifiers *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        lv_label_set_text_fmt(
            widget->obj, "%s [GUI]#%s [ALT]#%s [CTRL]#%s [SHFT]#",
            state.gui ? MOD_ACTIVE : MOD_INACTIVE, state.alt ? MOD_ACTIVE : MOD_INACTIVE,
            state.ctrl ? MOD_ACTIVE : MOD_INACTIVE, state.shift ? MOD_ACTIVE : MOD_INACTIVE);
    }
}

static void update_modifier_state(uint8_t keycode, bool pressed) {
    bool changed = false;

    switch (keycode) {
    case HID_USAGE_KEY_KEYBOARD_LEFTSHIFT:
    case HID_USAGE_KEY_KEYBOARD_RIGHTSHIFT:
        if (current_state.shift != pressed) {
            current_state.shift = pressed;
            changed = true;
        }
        break;
    case HID_USAGE_KEY_KEYBOARD_LEFTCONTROL:
    case HID_USAGE_KEY_KEYBOARD_RIGHTCONTROL:
        if (current_state.ctrl != pressed) {
            current_state.ctrl = pressed;
            changed = true;
        }
        break;
    case HID_USAGE_KEY_KEYBOARD_LEFTALT:
    case HID_USAGE_KEY_KEYBOARD_RIGHTALT:
        if (current_state.alt != pressed) {
            current_state.alt = pressed;
            changed = true;
        }
        break;
    case HID_USAGE_KEY_KEYBOARD_LEFT_GUI:
    case HID_USAGE_KEY_KEYBOARD_RIGHT_GUI:
        if (current_state.gui != pressed) {
            current_state.gui = pressed;
            changed = true;
        }
        break;
    }

    if (changed) {
        modifier_update_cb(current_state);
    }
}

static int keycode_state_changed_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *event = as_zmk_keycode_state_changed(eh);
    if (event && is_mod(event->usage_page, event->keycode)) {
        update_modifier_state(event->keycode, event->state);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_modifiers, keycode_state_changed_listener);
ZMK_SUBSCRIPTION(widget_modifiers, zmk_keycode_state_changed);

// ZMK_DISPLAY_WIDGET_LISTENER(widget_modifiers, struct modifier_state,
// modifier_update_cb,
//                             modifiers_get_caps_state)

int zmk_widget_modifiers_init(struct zmk_widget_modifiers *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    lv_obj_set_style_text_font(widget->obj, &vt323_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->obj, lv_color_hex(0x33FF33), LV_PART_MAIN);
    lv_label_set_recolor(widget->obj, true);

    sys_slist_append(&widgets, &widget->node);

    // Initialize modifier state from current HID state
    zmk_mod_flags_t mods = zmk_hid_get_explicit_mods();
    current_state.shift = (mods & (MOD_LSFT | MOD_RSFT)) != 0;
    current_state.ctrl = (mods & (MOD_LCTL | MOD_RCTL)) != 0;
    current_state.alt = (mods & (MOD_LALT | MOD_RALT)) != 0;
    current_state.gui = (mods & (MOD_LGUI | MOD_RGUI)) != 0;

    // Apply initial state to display
    modifier_update_cb(current_state);
    return 0;
}

lv_obj_t *zmk_widget_modifiers_obj(struct zmk_widget_modifiers *widget) { return widget->obj; }
