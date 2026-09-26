#include <lvgl.h>
#include <stdio.h>
#include <string.h>

/* ================= 1. 全局句柄与状态变量 ================= */
static lv_obj_t *main_screen = NULL;

// 中央复用区域控件
static lv_obj_t *center_box = NULL;
static lv_obj_t *clock_label = NULL;
static lv_obj_t *vol_popup_label = NULL;
static lv_timer_t *vol_timer = NULL;

// 右上常显音量与蓝牙
static lv_obj_t *top_right_label = NULL;

// 右下动态层级卡片
static lv_obj_t *layer_badge = NULL;
static lv_obj_t *layer_label = NULL;
static lv_anim_t layer_blink_anim;

// 左侧状态控件
static lv_obj_t *bat_l_label = NULL;
static lv_obj_t *bat_r_label = NULL;
static lv_obj_t *mods_label = NULL;

// 开机 Loading 控件
static lv_obj_t *boot_screen = NULL;
static lv_obj_t *boot_bar = NULL;
static lv_obj_t *boot_label = NULL;
static lv_timer_t *boot_timer = NULL;
static int boot_progress = 0;

/* ================= 2. 辅助工具函数 ================= */
// 生成像素块进度条字符串，如 [██████░░░░]
static void generate_pixel_bar(int percent, int total_blocks, char *out_buf) {
    int filled = (percent * total_blocks) / 100;
    strcpy(out_buf, "[");
    for (int i = 0; i < total_blocks; i++) {
        strcat(out_buf, (i < filled) ? "█" : "░");
    }
    strcat(out_buf, "]");
}

/* ================= 3. 事件回调与 UI 逻辑 ================= */

// 中央音量弹窗超时：恢复显示时间
static void vol_popup_timeout_cb(lv_timer_t *timer) {
    if (vol_popup_label && clock_label) {
        lv_obj_add_flag(vol_popup_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(clock_label, LV_OBJ_FLAG_HIDDEN);
    }
    vol_timer = NULL;
}

// 事件 A：当接收到 HID 音量改变时触发
void on_hid_volume_changed(uint8_t volume_percent) {
    if (!main_screen) return;

    char bar_str[16], buf[64];
    generate_pixel_bar(volume_percent, 8, bar_str);

    // 1. 更新右上角常驻音量小字
    snprintf(buf, sizeof(buf), "VOL:%2d%% BLE.1", volume_percent);
    lv_label_set_text(top_right_label, buf);

    // 2. 更新中央大弹窗
    snprintf(buf, sizeof(buf), "VOL: %d%%\n%s", volume_percent, bar_str);
    lv_label_set_text(vol_popup_label, buf);

    // 切换中央区域显示
    lv_obj_add_flag(clock_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(vol_popup_label, LV_OBJ_FLAG_HIDDEN);

    // 重置或创建 1.5 秒自动恢复定时器
    if (vol_timer) {
        lv_timer_reset(vol_timer);
    } else {
        vol_timer = lv_timer_create(vol_popup_timeout_cb, 1500, NULL);
        lv_timer_set_repeat_count(vol_timer, 1);
    }
}

// 层级卡片高亮闪烁动画回调
static void layer_blink_anim_cb(void *var, int32_t v) {
    lv_obj_set_style_bg_opa((lv_obj_t *)var, v, 0);
}

// 事件 B：当 ZMK 层级切换时触发 (支持任意层数)
void on_zmk_layer_changed(uint8_t layer_id, const char *layer_name) {
    if (!layer_label) return;

    char buf[32];
    snprintf(buf, sizeof(buf), "[ #%02d %s ]", layer_id, layer_name);
    lv_label_set_text(layer_label, buf);

    // 触发工业控制台高亮闪烁效果 (瞬间反色亮起再稳定)
    lv_anim_init(&layer_blink_anim);
    lv_anim_set_var(&layer_blink_anim, layer_badge);
    lv_anim_set_values(&layer_blink_anim, LV_OPA_20, LV_OPA_COVER);
    lv_anim_set_time(&layer_blink_anim, 150);
    lv_anim_set_exec_cb(&layer_blink_anim, layer_blink_anim_cb);
    lv_anim_start(&layer_blink_anim);
}

// 事件 C：更新左右电量
void on_zmk_battery_changed(uint8_t left_pct, uint8_t right_pct) {
    if (!bat_l_label) return;
    char bar[16], buf[32];

    generate_pixel_bar(left_pct, 6, bar);
    snprintf(buf, sizeof(buf), "L:%s %d%%", bar, left_pct);
    lv_label_set_text(bat_l_label, buf);

    generate_pixel_bar(right_pct, 6, bar);
    snprintf(buf, sizeof(buf), "R:%s %d%%", bar, right_pct);
    lv_label_set_text(bat_r_label, buf);
}

// 事件 D：更新修饰键状态 (bits: 0:GUI, 1:ALT, 2:CTRL, 3:SHFT)
void on_zmk_mods_changed(uint8_t mods_mask) {
    if (!mods_label) return;
    char buf[64];
    snprintf(buf, sizeof(buf), "%s %s %s %s",
             (mods_mask & 0x01) ? "[GUI]"  : "#GUI",
             (mods_mask & 0x02) ? "[ALT]"  : "#ALT",
             (mods_mask & 0x04) ? "[CTRL]" : "#CTRL",
             (mods_mask & 0x08) ? "[SHFT]" : "#SHFT");
    lv_label_set_text(mods_label, buf);
}

/* ================= 4. UI 界面构建 ================= */

// 构建主 UI (284x76 工业 SCADA / CRT 风格)
void create_main_scada_ui(void) {
    main_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x000000), 0);

    // --- 左上：左右独立电量 ---
    bat_l_label = lv_label_create(main_screen);
    lv_obj_set_style_text_color(bat_l_label, lv_color_hex(0x00FF66), 0);
    lv_obj_align(bat_l_label, LV_ALIGN_TOP_LEFT, 4, 4);

    bat_r_label = lv_label_create(main_screen);
    lv_obj_set_style_text_color(bat_r_label, lv_color_hex(0x00FF66), 0);
    lv_obj_align(bat_r_label, LV_ALIGN_TOP_LEFT, 4, 20);
    
    on_zmk_battery_changed(80, 90); // 初始值

    // --- 左下：Modifiers 状态 ---
    mods_label = lv_label_create(main_screen);
    lv_obj_set_style_text_color(mods_label, lv_color_hex(0x00FF66), 0);
    lv_obj_align(mods_label, LV_ALIGN_BOTTOM_LEFT, 4, -4);
    on_zmk_mods_changed(0x01); // 默认按下 GUI

    // --- 右上：常驻音量与蓝牙 ---
    top_right_label = lv_label_create(main_screen);
    lv_obj_set_style_text_color(top_right_label, lv_color_hex(0x00FF66), 0);
    lv_obj_align(top_right_label, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_label_set_text(top_right_label, "VOL:65% BLE.1");

    // --- 右下：动态 Layer 反色长条卡片 ---
    layer_badge = lv_obj_create(main_screen);
    lv_obj_set_size(layer_badge, LV_SIZE_CONTENT, 20);
    lv_obj_set_style_bg_color(layer_badge, lv_color_hex(0x00FF66), 0); // 亮绿底
    lv_obj_set_style_bg_opa(layer_badge, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(layer_badge, 0, 0); // 工业直角
    lv_obj_set_style_pad_all(layer_badge, 2, 0);
    lv_obj_align(layer_badge, LV_ALIGN_BOTTOM_RIGHT, -4, -4);

    layer_label = lv_label_create(layer_badge);
    lv_obj_set_style_text_color(layer_label, lv_color_hex(0x000000), 0); // 黑字
    lv_obj_center(layer_label);
    on_zmk_layer_changed(3, "NAV_MEDIA");

    // --- 中央核心复用区 (像素双线框) ---
    center_box = lv_obj_create(main_screen);
    lv_obj_set_size(center_box, 116, 46);
    lv_obj_align(center_box, LV_ALIGN_CENTER, 0, -2);
    lv_obj_set_style_bg_color(center_box, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_color(center_box, lv_color_hex(0x00FF66), 0);
    lv_obj_set_style_border_width(center_box, 1, 0);
    lv_obj_set_style_radius(center_box, 0, 0);

    // 平时：复古时间
    clock_label = lv_label_create(center_box);
    lv_obj_set_style_text_color(clock_label, lv_color_hex(0x00FF66), 0);
    lv_label_set_text(clock_label, "16:33");
    lv_obj_center(clock_label);

    // 调节音量时：HUD 弹窗 (默认隐藏)
    vol_popup_label = lv_label_create(center_box);
    lv_obj_set_style_text_color(vol_popup_label, lv_color_hex(0x00FF66), 0);
    lv_obj_center(vol_popup_label);
    lv_obj_add_flag(vol_popup_label, LV_OBJ_FLAG_HIDDEN);

    // 淡入加载主界面
    lv_scr_load_anim(main_screen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, true);
}

// --- 开机 Loading 动画逻辑 ---
static void boot_progress_cb(lv_timer_t *timer) {
    boot_progress += 4;
    if (boot_progress <= 100) {
        lv_bar_set_value(boot_bar, boot_progress, LV_ANIM_ON);
        char dots[8] = "";
        for (int i = 0; i < (boot_progress / 12) % 4; i++) strcat(dots, ".");
        char buf[32];
        snprintf(buf, sizeof(buf), "loading%s", dots);
        lv_label_set_text(boot_label, buf);
    } else {
        lv_timer_del(boot_timer);
        create_main_scada_ui(); // 切换至主界面
    }
}

void start_boot_sequence(void) {
    boot_screen = lv_scr_act();
    lv_obj_set_style_bg_color(boot_screen, lv_color_hex(0x000000), 0);

    boot_label = lv_label_create(boot_screen);
    lv_obj_set_style_text_color(boot_label, lv_color_hex(0x00FF66), 0);
    lv_label_set_text(boot_label, "loading...");
    lv_obj_align(boot_label, LV_ALIGN_CENTER, 0, -12);

    boot_bar = lv_bar_create(boot_screen);
    lv_obj_set_size(boot_bar, 180, 14);
    lv_obj_align(boot_bar, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_color(boot_bar, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_color(boot_bar, lv_color_hex(0x00FF66), LV_PART_MAIN);
    lv_obj_set_style_border_width(boot_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_color(boot_bar, lv_color_hex(0x00FF66), LV_PART_INDICATOR);

    boot_timer = lv_timer_create(boot_progress_cb, 40, NULL);
}