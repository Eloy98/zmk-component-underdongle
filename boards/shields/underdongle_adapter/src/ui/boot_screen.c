/*
 * SCADA / CRT dongle UI — boot splash.
 * Full-screen opaque overlay drawn on top of the (already built) main screen.
 * Runs a non-blocking 0->100% progress sequence via lv_timer, then fades out
 * and deletes itself, revealing the main UI underneath.
 */
#include "dongle_ui.h"

LV_FONT_DECLARE(vt323_16);

#define PROG_SEGS 12
#define STEP_MS 180
#define STEP_PCT 20
#define HOLD_DELAY_MS 120
#define FADE_MS 300

static lv_obj_t *boot_overlay;
static lv_obj_t *boot_prog_segs[PROG_SEGS];
static lv_obj_t *boot_percent;
static lv_timer_t *boot_timer;
static int boot_pct;

static void seg_fill(int filled) {
    for (int i = 0; i < PROG_SEGS; i++) {
        lv_obj_set_style_bg_color(boot_prog_segs[i],
                                  lv_color_hex(i < filled ? UI_COLOR_FG : UI_COLOR_DIM), 0);
    }
}

static void overlay_opa_cb(void *var, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t *)var, v, 0);
}

static void overlay_del_cb(lv_anim_t *a) {
    if (boot_timer) {
        lv_timer_del(boot_timer);
        boot_timer = NULL;
    }
    lv_obj_del((lv_obj_t *)a->var);
    boot_overlay = NULL;
}

static void start_fade_out(void) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, boot_overlay);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&a, FADE_MS);
    lv_anim_set_delay(&a, HOLD_DELAY_MS);
    lv_anim_set_exec_cb(&a, overlay_opa_cb);
    lv_anim_set_ready_cb(&a, overlay_del_cb);
    lv_anim_start(&a);
}

static void boot_step_cb(lv_timer_t *t) {
    boot_pct += STEP_PCT;
    if (boot_pct >= 100) {
        boot_pct = 100;
    }
    seg_fill((boot_pct * PROG_SEGS + 50) / 100);
    lv_label_set_text_fmt(boot_percent, "%d%%", boot_pct);
    if (boot_pct >= 100) {
        lv_timer_pause(t); /* stop stepping; freed later in overlay_del_cb */
        start_fade_out();
    }
}

static lv_obj_t *plain_rect(lv_obj_t *parent, int w, int h, uint32_t color, lv_opa_t opa) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(o, opa, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_radius(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    return o;
}

void dongle_boot_screen_start(lv_obj_t *screen) {
    boot_overlay = lv_obj_create(screen);
    lv_obj_set_size(boot_overlay, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_set_pos(boot_overlay, 0, 0);
    lv_obj_set_style_bg_color(boot_overlay, lv_color_hex(UI_COLOR_BG), 0);
    lv_obj_set_style_bg_opa(boot_overlay, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(boot_overlay, lv_color_hex(UI_COLOR_FG), 0);
    lv_obj_set_style_border_width(boot_overlay, 1, 0);
    lv_obj_set_style_radius(boot_overlay, 0, 0);
    lv_obj_set_style_pad_all(boot_overlay, 0, 0);
    lv_obj_clear_flag(boot_overlay, LV_OBJ_FLAG_SCROLLABLE);

    /* dongle pixel icon (left) */
    lv_obj_t *body = lv_obj_create(boot_overlay);
    lv_obj_set_size(body, 20, 34);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(body, lv_color_hex(UI_COLOR_FG), 0);
    lv_obj_set_style_border_width(body, 2, 0);
    lv_obj_set_style_radius(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0);
    lv_obj_clear_flag(body, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(body, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_t *eye = plain_rect(body, 10, 4, UI_COLOR_FG, LV_OPA_COVER);
    lv_obj_align(eye, LV_ALIGN_TOP_MID, 0, 6);

    /* title */
    lv_obj_t *title = lv_label_create(boot_overlay);
    lv_obj_set_style_text_font(title, &vt323_16, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(UI_COLOR_FG), 0);
    lv_label_set_text(title, "Dongle Booting...");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 24, 10);

    /* progress bar (discrete blocks) + percent */
    lv_obj_t *bar = lv_obj_create(boot_overlay);
    lv_obj_set_size(bar, PROG_SEGS * 11 + (PROG_SEGS - 1) * 2, 14);
    lv_obj_set_style_bg_opa(bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(UI_COLOR_FG), 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 1, 0);
    lv_obj_set_style_pad_column(bar, 2, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(bar, LV_ALIGN_CENTER, 8, 4);
    for (int i = 0; i < PROG_SEGS; i++) {
        boot_prog_segs[i] = plain_rect(bar, 11, 10, UI_COLOR_DIM, LV_OPA_COVER);
    }

    boot_percent = lv_label_create(boot_overlay);
    lv_obj_set_style_text_font(boot_percent, &vt323_16, 0);
    lv_obj_set_style_text_color(boot_percent, lv_color_hex(UI_COLOR_FG), 0);
    lv_label_set_text(boot_percent, "0%");
    lv_obj_align_to(boot_percent, bar, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    /* loading line */
    lv_obj_t *loading = lv_label_create(boot_overlay);
    lv_obj_set_style_text_font(loading, &vt323_16, 0);
    lv_obj_set_style_text_color(loading, lv_color_hex(UI_COLOR_DIM), 0);
    lv_label_set_text(loading, "loading...");
    lv_obj_align(loading, LV_ALIGN_BOTTOM_MID, 24, -6);

    boot_pct = 0;
    seg_fill(0);
    boot_timer = lv_timer_create(boot_step_cb, STEP_MS, NULL);
}
