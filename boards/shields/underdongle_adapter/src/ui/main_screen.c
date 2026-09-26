/*
 * SCADA / CRT dongle UI — main status screen (UI layer ONLY).
 * No ZMK data access here; everything arrives via dongle_ui_update_*().
 * Objects are created once; runtime updates only touch text/color/visibility.
 */
#include "dongle_ui.h"

LV_FONT_DECLARE(vt323_16);
LV_FONT_DECLARE(cascadia_digits_28);

#define BAT_SEGS 5
#define VOL_SEGS 8
#define VOL_HUD_MS 1500

enum center_mode { CENTER_TIME, CENTER_VOLUME };

/* ---- objects created once ---- */
static lv_obj_t *bat_l_segs[BAT_SEGS];
static lv_obj_t *bat_r_segs[BAT_SEGS];
static lv_obj_t *bat_l_pct;
static lv_obj_t *bat_r_pct;
static lv_obj_t *status_label; /* "VOL:65% BLE.1" (top-right) */
static lv_obj_t *center_time;  /* big clock */
static lv_obj_t *center_vol;   /* "VOL: 65%" (HUD) */
static lv_obj_t *center_vol_bar;
static lv_obj_t *center_vol_segs[VOL_SEGS];
static lv_obj_t *mod_lbl[4]; /* GUI ALT CTRL SHFT */
static lv_obj_t *layer_badge;

static lv_timer_t *vol_timer;
static enum center_mode center_mode = CENTER_TIME;
static uint8_t st_volume = 50;
static uint8_t st_ble;

/* ---------- helpers ---------- */
static void fill_segments(lv_obj_t **segs, int n, int filled) {
    for (int i = 0; i < n; i++) {
        lv_obj_set_style_bg_color(segs[i],
                                  lv_color_hex(i < filled ? UI_COLOR_FG : UI_COLOR_DIM), 0);
    }
}

static lv_obj_t *make_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color) {
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    return l;
}

/* A row of discrete pixel blocks (no Unicode); segs[] filled with children. */
static lv_obj_t *make_seg_bar(lv_obj_t *parent, lv_obj_t **segs, int n, int seg_w, int seg_h,
                              int gap) {
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, n * seg_w + (n - 1) * gap, seg_h);
    lv_obj_set_style_bg_opa(bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_set_style_pad_column(bar, gap, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    for (int i = 0; i < n; i++) {
        lv_obj_t *s = lv_obj_create(bar);
        lv_obj_set_size(s, seg_w, seg_h);
        lv_obj_set_style_bg_color(s, lv_color_hex(UI_COLOR_DIM), 0);
        lv_obj_set_style_bg_opa(s, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(s, 0, 0);
        lv_obj_set_style_radius(s, 0, 0);
        lv_obj_clear_flag(s, LV_OBJ_FLAG_SCROLLABLE);
        segs[i] = s;
    }
    return bar;
}

/* "L:" + outlined battery box (segments) + terminal nub + percent label. */
static void make_battery_row(lv_obj_t *screen, const char *tag, lv_obj_t **segs,
                             lv_obj_t **pct_out, int y) {
    lv_obj_t *tag_lbl = make_label(screen, &vt323_16, UI_COLOR_FG);
    lv_label_set_text(tag_lbl, tag);
    lv_obj_align(tag_lbl, LV_ALIGN_TOP_LEFT, 6, y);

    lv_obj_t *box = lv_obj_create(screen);
    lv_obj_set_size(box, BAT_SEGS * 6 + (BAT_SEGS - 1) * 1 + 2, 13);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(box, lv_color_hex(UI_COLOR_FG), 0);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_radius(box, 0, 0);
    lv_obj_set_style_pad_all(box, 1, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(box, LV_ALIGN_TOP_LEFT, 22, y);

    lv_obj_t *bar = make_seg_bar(box, segs, BAT_SEGS, 6, 9, 1);
    lv_obj_center(bar);

    lv_obj_t *nub = lv_obj_create(screen);
    lv_obj_set_size(nub, 2, 6);
    lv_obj_set_style_bg_color(nub, lv_color_hex(UI_COLOR_FG), 0);
    lv_obj_set_style_bg_opa(nub, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(nub, 0, 0);
    lv_obj_set_style_radius(nub, 0, 0);
    lv_obj_align_to(nub, box, LV_ALIGN_OUT_RIGHT_MID, 1, 0);

    lv_obj_t *pct = make_label(screen, &vt323_16, UI_COLOR_FG);
    lv_label_set_text(pct, "--%");
    lv_obj_align(pct, LV_ALIGN_TOP_LEFT, 64, y);
    *pct_out = pct;
}

/* transparent box with a 1px pixel border (no radius). */
static lv_obj_t *make_box(lv_obj_t *parent, int w, int h) {
    lv_obj_t *b = lv_obj_create(parent);
    lv_obj_set_size(b, w, h);
    lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(UI_COLOR_FG), 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_radius(b, 0, 0);
    lv_obj_set_style_pad_all(b, 0, 0);
    lv_obj_clear_flag(b, LV_OBJ_FLAG_SCROLLABLE);
    return b;
}

/* ---------- build (once) ---------- */
void dongle_main_screen_create(lv_obj_t *screen) {
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BG), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    /* double-line frame */
    lv_obj_t *frame_o = make_box(screen, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_set_pos(frame_o, 0, 0);
    lv_obj_t *frame_i = make_box(screen, UI_SCREEN_W - 6, UI_SCREEN_H - 6);
    lv_obj_center(frame_i);

    /* left: L/R battery gauges */
    make_battery_row(screen, "L:", bat_l_segs, &bat_l_pct, 5);
    make_battery_row(screen, "R:", bat_r_segs, &bat_r_pct, 23);

    /* top-right persistent status */
    status_label = make_label(screen, &vt323_16, UI_COLOR_FG);
    lv_label_set_text(status_label, "VOL:50% BLE.1");
    lv_obj_align(status_label, LV_ALIGN_TOP_RIGHT, -8, 5);

    /* center double box @ (142, 38) */
    lv_obj_t *cbox_o = make_box(screen, 96, 46);
    lv_obj_center(cbox_o);
    lv_obj_t *cbox_i = make_box(screen, 88, 38);
    lv_obj_center(cbox_i);

    center_time = make_label(cbox_i, &cascadia_digits_28, UI_COLOR_FG);
    lv_label_set_text(center_time, "16:33");
    lv_obj_center(center_time);

    center_vol = make_label(cbox_i, &vt323_16, UI_COLOR_FG);
    lv_label_set_text(center_vol, "VOL: 50%");
    lv_obj_align(center_vol, LV_ALIGN_TOP_MID, 0, 3);
    lv_obj_add_flag(center_vol, LV_OBJ_FLAG_HIDDEN);

    center_vol_bar = make_seg_bar(cbox_i, center_vol_segs, VOL_SEGS, 7, 8, 1);
    lv_obj_align(center_vol_bar, LV_ALIGN_BOTTOM_MID, 0, -3);
    lv_obj_add_flag(center_vol_bar, LV_OBJ_FLAG_HIDDEN);

    /* bottom-left modifiers */
    lv_obj_t *mods = lv_obj_create(screen);
    lv_obj_set_size(mods, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(mods, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(mods, 0, 0);
    lv_obj_set_style_radius(mods, 0, 0);
    lv_obj_set_style_pad_all(mods, 0, 0);
    lv_obj_set_style_pad_column(mods, 4, 0);
    lv_obj_set_flex_flow(mods, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mods, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(mods, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(mods, LV_ALIGN_BOTTOM_LEFT, 5, -3);

    static const char *const names[4] = {"GUI", "ALT", "CTRL", "SHFT"};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *l = make_label(mods, &vt323_16, UI_COLOR_DIM);
        lv_obj_set_style_pad_hor(l, 2, 0);
        lv_obj_set_style_radius(l, 0, 0);
        lv_label_set_text_fmt(l, "#%s", names[i]);
        mod_lbl[i] = l;
    }

    /* bottom-right layer badge (inverted) */
    layer_badge = lv_label_create(screen);
    lv_obj_set_style_text_font(layer_badge, &vt323_16, 0);
    lv_obj_set_style_text_color(layer_badge, lv_color_hex(UI_COLOR_BLACK), 0);
    lv_obj_set_style_bg_color(layer_badge, lv_color_hex(UI_COLOR_FG), 0);
    lv_obj_set_style_bg_opa(layer_badge, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(layer_badge, 0, 0);
    lv_obj_set_style_pad_hor(layer_badge, 4, 0);
    lv_obj_set_style_pad_ver(layer_badge, 1, 0);
    lv_label_set_text(layer_badge, "> #00 BASE <");
    lv_obj_align(layer_badge, LV_ALIGN_BOTTOM_RIGHT, -6, -3);
}

/* ---------- runtime updates ---------- */
static void set_battery(lv_obj_t **segs, lv_obj_t *pct, uint8_t percent, bool connected) {
    int filled = connected ? (percent * BAT_SEGS + 50) / 100 : 0;
    fill_segments(segs, BAT_SEGS, filled);
    if (connected) {
        lv_label_set_text_fmt(pct, "%d%%", percent);
    } else {
        lv_label_set_text(pct, "--%");
    }
}

void dongle_ui_update_left_battery(uint8_t percent, bool connected) {
    set_battery(bat_l_segs, bat_l_pct, percent, connected);
}

void dongle_ui_update_right_battery(uint8_t percent, bool connected) {
    set_battery(bat_r_segs, bat_r_pct, percent, connected);
}

static void refresh_status(void) {
    lv_label_set_text_fmt(status_label, "VOL:%d%% BLE.%d", st_volume, st_ble + 1);
}

void dongle_ui_update_ble(uint8_t profile_index) {
    st_ble = profile_index;
    refresh_status();
}

static void vol_timeout_cb(lv_timer_t *t) {
    lv_obj_add_flag(center_vol, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(center_vol_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(center_time, LV_OBJ_FLAG_HIDDEN);
    center_mode = CENTER_TIME;
    vol_timer = NULL; /* one-shot: LVGL frees it after this callback */
}

void dongle_ui_update_volume(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }
    st_volume = percent;
    refresh_status();

    lv_label_set_text_fmt(center_vol, "VOL: %d%%", percent);
    fill_segments(center_vol_segs, VOL_SEGS, (percent * VOL_SEGS + 50) / 100);

    /* HUD replaces ONLY the center area; corners stay put */
    lv_obj_add_flag(center_time, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(center_vol, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(center_vol_bar, LV_OBJ_FLAG_HIDDEN);
    center_mode = CENTER_VOLUME;

    if (vol_timer) {
        lv_timer_reset(vol_timer);
    } else {
        vol_timer = lv_timer_create(vol_timeout_cb, VOL_HUD_MS, NULL);
        lv_timer_set_repeat_count(vol_timer, 1);
    }
}

void dongle_ui_update_time(uint8_t hour, uint8_t minute) {
    /* cascadia_digits_28 only carries glyphs 0-9 and ':' */
    lv_label_set_text_fmt(center_time, "%02d:%02d", hour, minute);
}

static void layer_flash_cb(void *var, int32_t v) {
    lv_obj_set_style_bg_opa((lv_obj_t *)var, v, 0);
}

void dongle_ui_update_layer(uint8_t id, const char *name) {
    lv_label_set_text_fmt(layer_badge, "> #%02d %s <", id, name ? name : "BASE");

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, layer_badge);
    lv_anim_set_values(&a, LV_OPA_50, LV_OPA_COVER);
    lv_anim_set_time(&a, 150);
    lv_anim_set_exec_cb(&a, layer_flash_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}

static void set_mod(lv_obj_t *l, const char *name, bool active) {
    if (active) {
        lv_obj_set_style_bg_color(l, lv_color_hex(UI_COLOR_FG), 0);
        lv_obj_set_style_bg_opa(l, LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(l, lv_color_hex(UI_COLOR_BLACK), 0);
        lv_label_set_text_fmt(l, "[%s]", name);
    } else {
        lv_obj_set_style_bg_opa(l, LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(l, lv_color_hex(UI_COLOR_DIM), 0);
        lv_label_set_text_fmt(l, "#%s", name);
    }
}

void dongle_ui_update_modifiers(bool gui, bool alt, bool ctrl, bool shift) {
    set_mod(mod_lbl[0], "GUI", gui);
    set_mod(mod_lbl[1], "ALT", alt);
    set_mod(mod_lbl[2], "CTRL", ctrl);
    set_mod(mod_lbl[3], "SHFT", shift);
}



