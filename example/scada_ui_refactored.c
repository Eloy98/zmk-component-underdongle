/*
 * ====================================================================
 * LVGL SCADA Industrial UI - 284x76 Green Monochrome Display
 * Pixel-Perfect Layout with Enhanced Animations & Interactions
 * ====================================================================
 */

#include <lvgl.h>
#include <stdio.h>
#include <string.h>

/* ================= 1. CONFIGURATION & CONSTANTS ================= */
#define SCREEN_WIDTH        284
#define SCREEN_HEIGHT       76

#define COLOR_BG            0x000000  // Pure black
#define COLOR_FG            0x00FF66  // Bright green (SCADA style)
#define COLOR_DIM           0x008844  // Dimmed green

// Layout coordinates (pixel-perfect alignment)
#define BATTERY_X           4
#define BATTERY_Y_L         3
#define BATTERY_Y_R         17

#define MODS_X              4
#define MODS_Y              -3

#define TOPRIGHT_X          -4
#define TOPRIGHT_Y          3

#define CENTER_BOX_W        120
#define CENTER_BOX_H        48
#define CENTER_Y_OFFSET     -2

#define LAYER_BADGE_X       -4
#define LAYER_BADGE_Y       -3
#define LAYER_BADGE_H       18
#define LAYER_BADGE_PAD     3

// Animation timings
#define VOL_POPUP_TIMEOUT   1500   // ms
#define LAYER_BLINK_TIME    150    // ms
#define BOOT_TICK_INTERVAL  35     // ms
#define BOOT_PROGRESS_STEP  3      // per tick

/* ================= 2. GLOBAL HANDLES & STATE ================= */
static lv_obj_t *main_screen = NULL;

// Central multipurpose area
static lv_obj_t *center_box = NULL;
static lv_obj_t *clock_label = NULL;
static lv_obj_t *vol_popup_container = NULL;
static lv_obj_t *vol_popup_text = NULL;
static lv_obj_t *vol_popup_bar_label = NULL;
static lv_timer_t *vol_timer = NULL;

// Top-right persistent status
static lv_obj_t *top_right_label = NULL;

// Bottom-right dynamic layer badge (inverted colors)
static lv_obj_t *layer_badge = NULL;
static lv_obj_t *layer_label = NULL;

// Left side status widgets
static lv_obj_t *bat_l_label = NULL;
static lv_obj_t *bat_r_label = NULL;
static lv_obj_t *mods_label = NULL;

// Boot sequence widgets
static lv_obj_t *boot_screen = NULL;
static lv_obj_t *boot_bar = NULL;
static lv_obj_t *boot_label = NULL;
static lv_timer_t *boot_timer = NULL;
static int boot_progress = 0;

// State tracking
static uint8_t current_volume = 65;

/* ================= 3. UTILITY FUNCTIONS ================= */

/**
 * Generate pixel-block progress bar string
 * Example: [████████░░] for 80%
 */
static void generate_pixel_bar(int percent, int total_blocks, char *out_buf) {
    int filled = (percent * total_blocks + 50) / 100; // Round properly
    if (filled > total_blocks) filled = total_blocks;

    strcpy(out_buf, "[");
    for (int i = 0; i < total_blocks; i++) {
        strcat(out_buf, (i < filled) ? "█" : "░");
    }
    strcat(out_buf, "]");
}

/**
 * Create monospace styled label (for consistent SCADA look)
 */
static lv_obj_t *create_scada_label(lv_obj_t *parent, uint32_t color) {
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    return label;
}

/* ================= 4. EVENT CALLBACKS & UI LOGIC ================= */

/**
 * Central volume popup timeout - restore clock display
 */
static void vol_popup_timeout_cb(lv_timer_t *timer) {
    if (vol_popup_container && clock_label) {
        lv_obj_add_flag(vol_popup_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(clock_label, LV_OBJ_FLAG_HIDDEN);
    }
    vol_timer = NULL;
}

/**
 * EVENT A: HID Volume Changed
 * Triggers central HUD popup with volume bar, auto-hides after 1.5s
 */
void on_hid_volume_changed(uint8_t volume_percent) {
    if (!main_screen || volume_percent > 100) return;

    current_volume = volume_percent;
    char bar_str[16], text_buf[32], full_buf[64];

    // Update top-right persistent status
    snprintf(text_buf, sizeof(text_buf), "VOL:%2d%% BLE.1", volume_percent);
    lv_label_set_text(top_right_label, text_buf);

    // Update central popup content
    snprintf(text_buf, sizeof(text_buf), "VOL: %d%%", volume_percent);
    lv_label_set_text(vol_popup_text, text_buf);

    generate_pixel_bar(volume_percent, 10, bar_str);
    lv_label_set_text(vol_popup_bar_label, bar_str);

    // Switch central display mode: hide clock, show volume
    lv_obj_add_flag(clock_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(vol_popup_container, LV_OBJ_FLAG_HIDDEN);

    // Reset or create auto-hide timer
    if (vol_timer) {
        lv_timer_reset(vol_timer);
    } else {
        vol_timer = lv_timer_create(vol_popup_timeout_cb, VOL_POPUP_TIMEOUT, NULL);
        lv_timer_set_repeat_count(vol_timer, 1);
    }
}

/**
 * Layer badge blink animation callback (opacity pulse)
 */
static void layer_blink_exec_cb(void *var, int32_t value) {
    lv_obj_set_style_bg_opa((lv_obj_t *)var, value, 0);
}

/**
 * EVENT B: ZMK Layer Changed (supports arbitrary layer IDs)
 * Triggers industrial console highlight flash effect
 */
void on_zmk_layer_changed(uint8_t layer_id, const char *layer_name) {
    if (!layer_label || !layer_badge) return;

    char buf[40];
    snprintf(buf, sizeof(buf), "[ #%02d %s ]", layer_id, layer_name ? layer_name : "???");
    lv_label_set_text(layer_label, buf);

    // Industrial feedback: quick opacity flash (dim -> bright -> stable)
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, layer_badge);
    lv_anim_set_values(&anim, LV_OPA_50, LV_OPA_COVER);
    lv_anim_set_time(&anim, LAYER_BLINK_TIME);
    lv_anim_set_exec_cb(&anim, layer_blink_exec_cb);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
}

/**
 * EVENT C: Battery Status Update (left & right split keyboard)
 */
void on_zmk_battery_changed(uint8_t left_pct, uint8_t right_pct) {
    if (!bat_l_label || !bat_r_label) return;

    char bar[16], buf[40];

    // Left battery with pixel bar
    generate_pixel_bar(left_pct, 8, bar);
    snprintf(buf, sizeof(buf), "L:%s %d%%", bar, left_pct);
    lv_label_set_text(bat_l_label, buf);

    // Right battery with pixel bar
    generate_pixel_bar(right_pct, 8, bar);
    snprintf(buf, sizeof(buf), "R:%s %d%%", bar, right_pct);
    lv_label_set_text(bat_r_label, buf);
}

/**
 * EVENT D: Modifier Keys Status Update
 * Bitmask: bit0=GUI, bit1=ALT, bit2=CTRL, bit3=SHIFT
 * Pressed keys shown as [KEY], unpressed as #KEY
 */
void on_zmk_mods_changed(uint8_t mods_mask) {
    if (!mods_label) return;

    char buf[80];
    snprintf(buf, sizeof(buf), "%s %s %s %s",
             (mods_mask & 0x01) ? "[GUI]"  : "#GUI",
             (mods_mask & 0x02) ? "[ALT]"  : "#ALT",
             (mods_mask & 0x04) ? "[CTRL]" : "#CTRL",
             (mods_mask & 0x08) ? "[SHFT]" : "#SHFT");
    lv_label_set_text(mods_label, buf);
}

/**
 * Update clock display (call this every minute or second)
 */
void update_clock_display(uint8_t hour, uint8_t minute) {
    if (!clock_label) return;

    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
    lv_label_set_text(clock_label, buf);
}

/* ================= 5. MAIN UI CONSTRUCTION ================= */

/**
 * Build main SCADA UI (284x76 industrial CRT/terminal style)
 */
void create_main_scada_ui(void) {
    main_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_bg_opa(main_screen, LV_OPA_COVER, 0);

    /* ========== LEFT SIDE STATUS AREA ========== */

    // Left battery (top-left position)
    bat_l_label = create_scada_label(main_screen, COLOR_FG);
    lv_obj_set_style_text_font(bat_l_label, &lv_font_montserrat_10, 0);
    lv_obj_align(bat_l_label, LV_ALIGN_TOP_LEFT, BATTERY_X, BATTERY_Y_L);

    // Right battery (second line)
    bat_r_label = create_scada_label(main_screen, COLOR_FG);
    lv_obj_set_style_text_font(bat_r_label, &lv_font_montserrat_10, 0);
    lv_obj_align(bat_r_label, LV_ALIGN_TOP_LEFT, BATTERY_X, BATTERY_Y_R);

    // Initialize with demo values
    on_zmk_battery_changed(85, 92);

    // Modifiers status (bottom-left corner)
    mods_label = create_scada_label(main_screen, COLOR_FG);
    lv_obj_set_style_text_font(mods_label, &lv_font_montserrat_10, 0);
    lv_obj_align(mods_label, LV_ALIGN_BOTTOM_LEFT, MODS_X, MODS_Y);
    on_zmk_mods_changed(0x00); // All released initially

    /* ========== TOP-RIGHT PERSISTENT STATUS ========== */

    top_right_label = create_scada_label(main_screen, COLOR_FG);
    lv_obj_set_style_text_font(top_right_label, &lv_font_montserrat_10, 0);
    lv_obj_align(top_right_label, LV_ALIGN_TOP_RIGHT, TOPRIGHT_X, TOPRIGHT_Y);
    lv_label_set_text(top_right_label, "VOL:65% BLE.1");

    /* ========== BOTTOM-RIGHT DYNAMIC LAYER BADGE ========== */

    // Inverted color card (bright green background, black text)
    layer_badge = lv_obj_create(main_screen);
    lv_obj_set_size(layer_badge, LV_SIZE_CONTENT, LAYER_BADGE_H);
    lv_obj_set_style_bg_color(layer_badge, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_bg_opa(layer_badge, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(layer_badge, 0, 0);
    lv_obj_set_style_radius(layer_badge, 0, 0); // Sharp industrial corners
    lv_obj_set_style_pad_left(layer_badge, LAYER_BADGE_PAD, 0);
    lv_obj_set_style_pad_right(layer_badge, LAYER_BADGE_PAD, 0);
    lv_obj_set_style_pad_top(layer_badge, 2, 0);
    lv_obj_set_style_pad_bottom(layer_badge, 2, 0);
    lv_obj_align(layer_badge, LV_ALIGN_BOTTOM_RIGHT, LAYER_BADGE_X, LAYER_BADGE_Y);

    layer_label = lv_label_create(layer_badge);
    lv_obj_set_style_text_color(layer_label, lv_color_hex(COLOR_BG), 0); // Black text
    lv_obj_set_style_text_font(layer_label, &lv_font_montserrat_10, 0);
    lv_obj_center(layer_label);

    // Initialize with demo layer
    on_zmk_layer_changed(3, "NAV_MEDIA");

    /* ========== CENTER CORE AREA (TIME / VOLUME HUD) ========== */

    // Double-line bordered box at screen center
    center_box = lv_obj_create(main_screen);
    lv_obj_set_size(center_box, CENTER_BOX_W, CENTER_BOX_H);
    lv_obj_center(center_box);
    lv_obj_set_y(center_box, CENTER_Y_OFFSET);
    lv_obj_set_style_bg_color(center_box, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_bg_opa(center_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(center_box, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_border_width(center_box, 2, 0); // Double-line effect
    lv_obj_set_style_border_opa(center_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(center_box, 0, 0);
    lv_obj_set_style_pad_all(center_box, 0, 0);

    // Default mode: Large pixel clock display
    clock_label = lv_label_create(center_box);
    lv_obj_set_style_text_color(clock_label, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_text_font(clock_label, &lv_font_montserrat_28, 0); // Large font
    lv_obj_center(clock_label);
    update_clock_display(16, 33); // Demo time

    // Volume HUD mode (hidden by default, shown on volume change)
    vol_popup_container = lv_obj_create(center_box);
    lv_obj_set_size(vol_popup_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(vol_popup_container);
    lv_obj_set_style_bg_opa(vol_popup_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vol_popup_container, 0, 0);
    lv_obj_set_style_pad_all(vol_popup_container, 0, 0);
    lv_obj_set_flex_flow(vol_popup_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(vol_popup_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    vol_popup_text = lv_label_create(vol_popup_container);
    lv_obj_set_style_text_color(vol_popup_text, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_text_font(vol_popup_text, &lv_font_montserrat_16, 0);
    lv_label_set_text(vol_popup_text, "VOL: 65%");

    vol_popup_bar_label = lv_label_create(vol_popup_container);
    lv_obj_set_style_text_color(vol_popup_bar_label, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_text_font(vol_popup_bar_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(vol_popup_bar_label, "[██████░░░░]");
    lv_obj_set_style_pad_top(vol_popup_bar_label, 4, 0);

    // Hide volume popup initially
    lv_obj_add_flag(vol_popup_container, LV_OBJ_FLAG_HIDDEN);

    // Load main screen with smooth fade-in transition
    lv_scr_load_anim(main_screen, LV_SCR_LOAD_ANIM_FADE_IN, 400, 0, true);
}

/* ================= 6. BOOT SEQUENCE ANIMATION ================= */

/**
 * Boot progress timer callback
 * Increments loading bar and animates dot sequence
 */
static void boot_progress_cb(lv_timer_t *timer) {
    boot_progress += BOOT_PROGRESS_STEP;

    if (boot_progress <= 100) {
        // Update progress bar
        lv_bar_set_value(boot_bar, boot_progress, LV_ANIM_ON);

        // Animate loading text with cycling dots
        char dots[8] = "";
        int dot_count = (boot_progress / 8) % 4;
        for (int i = 0; i < dot_count; i++) {
            strcat(dots, ".");
        }

        char buf[32];
        snprintf(buf, sizeof(buf), "loading%s", dots);
        lv_label_set_text(boot_label, buf);

    } else {
        // Boot complete - clean up and transition to main UI
        lv_timer_del(boot_timer);
        boot_timer = NULL;

        // Smooth transition with fade out -> fade in
        create_main_scada_ui();
    }
}

/**
 * Start boot loading sequence
 * Shows industrial-style loading screen with progress bar
 */
void start_boot_sequence(void) {
    boot_screen = lv_scr_act();
    lv_obj_set_style_bg_color(boot_screen, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_bg_opa(boot_screen, LV_OPA_COVER, 0);

    // Loading text with retro terminal style
    boot_label = lv_label_create(boot_screen);
    lv_obj_set_style_text_color(boot_label, lv_color_hex(COLOR_FG), 0);
    lv_obj_set_style_text_font(boot_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(boot_label, "loading...");
    lv_obj_align(boot_label, LV_ALIGN_CENTER, 0, -16);

    // Pixel-style progress bar
    boot_bar = lv_bar_create(boot_screen);
    lv_obj_set_size(boot_bar, 200, 16);
    lv_obj_align(boot_bar, LV_ALIGN_CENTER, 0, 12);

    // Bar background style
    lv_obj_set_style_bg_color(boot_bar, lv_color_hex(COLOR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(boot_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(boot_bar, lv_color_hex(COLOR_FG), LV_PART_MAIN);
    lv_obj_set_style_border_width(boot_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_opa(boot_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(boot_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(boot_bar, 1, LV_PART_MAIN);

    // Bar indicator (filled part) style
    lv_obj_set_style_bg_color(boot_bar, lv_color_hex(COLOR_FG), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(boot_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(boot_bar, 0, LV_PART_INDICATOR);

    // Initialize bar
    lv_bar_set_range(boot_bar, 0, 100);
    lv_bar_set_value(boot_bar, 0, LV_ANIM_OFF);

    // Start boot timer
    boot_progress = 0;
    boot_timer = lv_timer_create(boot_progress_cb, BOOT_TICK_INTERVAL, NULL);
}

/* ================= 7. ADDITIONAL UTILITY FUNCTIONS ================= */

/**
 * Force refresh volume display (useful after resume from sleep)
 */
void refresh_volume_display(void) {
    on_hid_volume_changed(current_volume);
}

/**
 * Get current main screen object (for external management)
 */
lv_obj_t *get_main_screen(void) {
    return main_screen;
}

/**
 * Clean up all timers and animations (call before screen switch)
 */
void cleanup_scada_ui(void) {
    if (vol_timer) {
        lv_timer_del(vol_timer);
        vol_timer = NULL;
    }
    if (boot_timer) {
        lv_timer_del(boot_timer);
        boot_timer = NULL;
    }
}

/* ================= 8. DEMO / TEST FUNCTIONS ================= */

#ifdef SCADA_UI_DEMO_MODE

/**
 * Demo sequence to test all UI features
 * Call this to see animations and state changes
 */
void run_scada_demo_sequence(void) {
    // Simulate layer changes
    on_zmk_layer_changed(0, "DEFAULT");
    lv_task_handler();
    lv_delay_ms(1000);

    on_zmk_layer_changed(1, "LOWER");
    lv_task_handler();
    lv_delay_ms(1000);

    on_zmk_layer_changed(2, "RAISE");
    lv_task_handler();
    lv_delay_ms(1000);

    // Simulate volume changes
    for (int vol = 50; vol <= 100; vol += 10) {
        on_hid_volume_changed(vol);
        lv_task_handler();
        lv_delay_ms(2000); // Wait for auto-hide
    }

    // Simulate modifier key presses
    on_zmk_mods_changed(0x01); // GUI pressed
    lv_task_handler();
    lv_delay_ms(500);

    on_zmk_mods_changed(0x05); // GUI + CTRL pressed
    lv_task_handler();
    lv_delay_ms(500);

    on_zmk_mods_changed(0x0F); // All modifiers pressed
    lv_task_handler();
    lv_delay_ms(500);

    on_zmk_mods_changed(0x00); // All released
    lv_task_handler();

    // Simulate battery drain
    for (int bat = 100; bat >= 20; bat -= 20) {
        on_zmk_battery_changed(bat, bat - 5);
        lv_task_handler();
        lv_delay_ms(1000);
    }
}

#endif // SCADA_UI_DEMO_MODE

/* ================= END OF FILE ================= */
