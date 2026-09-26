#pragma once

#include <lvgl.h>
#include <stdbool.h>
#include <stdint.h>

/* ===== SCADA / CRT palette (RGB565 friendly) ===== */
#define UI_COLOR_BG 0x050F05
#define UI_COLOR_FG 0x33FF33
#define UI_COLOR_DIM 0x1A4D1A
#define UI_COLOR_BLACK 0x000000

#define UI_SCREEN_W 284
#define UI_SCREEN_H 76

/* ===== Main screen (pure UI, no ZMK data access) ===== *
 * Builds all objects once on `screen`; runtime changes only text/state.   */
void dongle_main_screen_create(lv_obj_t *screen);

/* UI update API - the ONLY way data reaches the screen (called by data layer). */
void dongle_ui_update_left_battery(uint8_t percent, bool connected);
void dongle_ui_update_right_battery(uint8_t percent, bool connected);
void dongle_ui_update_volume(uint8_t percent); /* also raises the center HUD */
void dongle_ui_update_ble(uint8_t profile_index);
void dongle_ui_update_layer(uint8_t id, const char *name);
void dongle_ui_update_modifiers(bool gui, bool alt, bool ctrl, bool shift);
void dongle_ui_update_time(uint8_t hour, uint8_t minute);

/* ===== Boot splash =====
 * Opaque full-screen overlay on `screen`; self-animates 0->100% then fades
 * out and deletes itself, revealing the main UI. Non-blocking (LVGL timer). */
void dongle_boot_screen_start(lv_obj_t *screen);
