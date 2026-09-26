你现在正在修改一个已经可以工作的 ZMK dongle 屏幕项目。

目标：将当前 dongle UI 完整改造成一个 **90 年代 DOS / CRT / SCADA 工业终端风格**的 UI。

硬件屏幕：

* ST7789 TFT
* 分辨率：284 × 76
* 横向显示
* RGB565
* 屏幕比例约 3.74:1
* 黑绿色背景：#050F05
* 主色：#33FF33
* 不使用现代圆角卡片
* 不使用渐变
* 不使用抗锯齿
* 尽量使用 1px / 2px 像素级几何图形
* 使用像素字体 / 等宽字体
* 整体视觉类似 DOS Terminal + CRT + 工业 SCADA

==================================================
一、首先不要破坏现有硬件驱动
==============

先分析当前仓库：

* ST7789 driver
* display initialization
* LVGL 初始化
* ZMK display status screen
* battery 数据来源
* layer 数据来源
* modifier 数据来源
* HID volume callback
* BLE / USB 状态来源
* 当前 UI 创建和刷新代码

禁止为了修改 UI 而修改：

* ST7789 SPI 驱动
* ST7789 initialization
* MADCTL / rotation
* display flush
* SPI DMA
* framebuffer
* ZMK BLE / HID 数据逻辑

如果当前项目已经使用 LVGL，则继续使用 LVGL。

优先采用：

CONFIG_ZMK_DISPLAY=y
CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y

把 UI 与数据逻辑分离。

==================================================
二、最终 UI
=======

屏幕尺寸：

284 × 76

整个 UI 使用一个 1px 或 2px 的双线像素边框。

整体：

+----------------------------------------------------------+
| L:[██████░░] 80%              VOL:65% BLE.1              |
| R:[███████░] 90%                                            |
|                                                            |
|                 +----------------+                         |
|                 |     16:33      |                         |
|                 +----------------+                         |
|                                                            |
| [GUI] #ALT #CTRL #SHFT                 > #03 NAV_MEDIA < |
+----------------------------------------------------------+

注意：

这是视觉结构，不要求严格照 ASCII 的比例。

必须针对 284×76 原生像素重新布局。

不要从 240×280 UI 缩放。

==================================================
三、中央核心区
=======

中央区域是整个 UI 的视觉核心。

创建一个居中的双线框。

框的中心位置必须严格位于：

X = 142
Y = 38

默认显示：

16:33

使用最大、最醒目的像素字体。

建议中央区域大约：

X = 90~100
Y = 15~60

实际坐标根据字体实际尺寸调整。

时间：

* 居中
* 等宽
* 像素字体
* #33FF33
* 不使用 anti-aliasing

==================================================
四、Volume HUD
============

当收到：

on_hid_volume_changed(volume)

立即将中央时间区域切换为：

VOL: 65%

下一行：

[████████░░]

注意：

Volume HUD 只替换中央核心区域。

左上/右上常驻状态不能被覆盖。

Volume HUD 持续：

1500 ms

无新的音量事件后：

1500ms

恢复：

16:33

不要重新创建 screen。

只更新现有 LVGL objects。

推荐对象：

ui_center_time
ui_center_volume
ui_center_volume_bar

状态：

enum center_mode {
CENTER_TIME,
CENTER_VOLUME
};

使用 timer/work 延迟恢复。

不要使用 blocking sleep。

==================================================
五、Volume Progress Bar
=====================

不要直接依赖 Unicode：

████████░░

如果当前 LVGL 字体无法可靠显示 block 字符，则不要强行使用 Unicode。

优先使用：

LVGL lv_bar

或者自己绘制像素矩形。

视觉必须类似：

[████████░░]

左右有明确边框。

volume = 65：

大约：

[██████░░░░]

volume = 80：

[████████░░]

volume = 100：

[██████████]

==================================================
六、右上角常驻状态
=========

右上角始终显示：

VOL:65% BLE.1

这个区域不能因为中央 Volume HUD 而消失。

对象：

ui_status_volume
ui_status_ble

位置：

屏幕右上。

字体较小。

VOL 数值和 BLE 状态需要动态更新。

例如：

VOL:65% BLE.1

BLE profile 改变：

BLE.2
BLE.3

USB 状态可以根据现有项目的数据结构显示 USB。

不要自己重新实现 BLE 状态检测。

使用项目现有的数据来源。

==================================================
七、左右键盘电池
========

左上区域显示：

L:[██████░░] 80%
R:[███████░] 90%

这是两个独立 widget。

对象：

ui_left_battery
ui_right_battery

每个 widget 至少包含：

label
battery outline
battery fill
percentage

不要使用 bitmap。

优先使用：

LVGL bar

或者：

LVGL rectangle objects

电池百分比必须动态更新。

如果现有项目已经有左右键盘 battery widget：

复用数据获取逻辑。

只重做视觉。

不要重复实现 battery BLE 数据读取。

==================================================
八、Modifier
==========

左下显示：

[GUI] #ALT #CTRL #SHFT

按下：

[GUI]

使用：

#33FF33 实心背景
黑色文字

未按下：

#ALT

使用：

黑绿色背景
绿色文字
1px / 虚线风格边框

Modifier 状态：

GUI
ALT
CTRL
SHIFT

必须动态更新。

复用现有 ZMK modifier 状态。

不要重新实现 modifier event system。

==================================================
九、Layer
=======

右下是动态 layer 区域。

例如：

> #03 NAV_MEDIA <

设计成：

亮绿色背景
黑色像素字体

这是整个 UI 第二个视觉重点。

结构：

+--------------------------------+
| > #03 NAV_MEDIA <              |
+--------------------------------+

必须支持任意 layer：

#00 BASE
#01 LOWER
#02 RAISE
#03 NAV_MEDIA
#04 GAME
etc.

不要把：

NAV_MEDIA

写死。

调用：

on_zmk_layer_changed(id, name)

时：

1. 更新 layer id
2. 更新 layer name
3. 触发 150ms 高亮动画

动画要求：

0ms：
正常亮度

0~75ms：
亮度/透明度增强

75~150ms：
恢复正常

不要使用复杂动画。

不要造成整个屏幕闪烁。

只影响 layer widget。

如果 LVGL 不适合 opacity：

可以通过背景色/边框颜色切换实现。

==================================================
十、Boot Screen
=============

启动时不要直接进入主 UI。

第一个页面必须是完整铺满：

284 × 76

的：

DONGLE BOOTING

Boot screen 不要显示主 UI 的边框和 widgets。

完整屏幕作为一个独立 boot screen。

视觉：

+----------------------------------------------------------+
|                                                          |
|                    DONGLE BOOTING...                     |
|                                                          |
|                    [████████████████] 100%               |
|                                                          |
|                       loading...                         |
|                                                          |
+----------------------------------------------------------+

建议加入一个简单的 dongle pixel icon。

Boot screen：

start_boot_sequence()

流程：

0%
↓
20%
↓
40%
↓
60%
↓
80%
↓
100%

总时间可以约 800~1500ms。

不要 blocking sleep。

使用 LVGL timer / Zephyr work queue。

达到 100%：

保持约 100ms

然后：

Fade Out
↓
切换主 screen
↓
Fade In

如果当前 LVGL/版本不适合真正 opacity animation：

使用简单的逐步隐藏/显示实现。

==================================================
十一、像素风格
=======

所有 UI 必须：

* #050F05 background
* #33FF33 foreground
* 允许使用更暗的绿色作为 inactive state
* RGB565
* pixel font
* monospace
* sharp edges
* no rounded corners
* no gradient
* no shadow
* no blur
* no anti-aliasing

可以增加非常轻微的 CRT scanline。

但是：

scanline 不能覆盖文字。

不要每帧重新绘制整个 scanline。

如果性能成本过高，可以省略 scanline。

==================================================
十二、LVGL 对象结构
============

建议：

ui_screen
├── ui_background
├── ui_border_outer
├── ui_border_inner
│
├── ui_left_battery
│   ├── ui_left_battery_label
│   ├── ui_left_battery_bar
│   └── ui_left_battery_percent
│
├── ui_right_battery
│   ├── ui_right_battery_label
│   ├── ui_right_battery_bar
│   └── ui_right_battery_percent
│
├── ui_status
│   ├── ui_status_volume
│   └── ui_status_ble
│
├── ui_center
│   ├── ui_center_border
│   ├── ui_center_time
│   ├── ui_center_volume
│   └── ui_center_volume_bar
│
├── ui_modifiers
│   ├── ui_mod_gui
│   ├── ui_mod_alt
│   ├── ui_mod_ctrl
│   └── ui_mod_shift
│
└── ui_layer
├── ui_layer_bg
└── ui_layer_label

Boot：

boot_screen
├── boot_border
├── boot_icon
├── boot_title
├── boot_progress
├── boot_percent
└── boot_loading

==================================================
十三、代码架构
=======

不要把所有代码塞进一个巨大 C 文件。

建议：

src/
├── ui/
│   ├── dongle_ui.c
│   ├── dongle_ui.h
│   ├── boot_screen.c
│   ├── boot_screen.h
│   ├── main_screen.c
│   └── main_screen.h
│
├── widgets/
│   ├── battery_widget.c
│   ├── modifier_widget.c
│   ├── layer_widget.c
│   └── volume_widget.c
│
└── existing ZMK data/event code

如果仓库已经存在类似文件结构：

优先沿用现有架构，不要为了形式重构整个项目。

==================================================
十四、数据层与 UI 层严格分离
================

UI 不负责获取 ZMK 数据。

例如：

错误：

ui.c 自己读取 BLE battery。

正确：

ZMK data/event
↓
dongle_ui_update_battery(left, right)
↓
LVGL widget

接口类似：

dongle_ui_update_left_battery(80);
dongle_ui_update_right_battery(90);

dongle_ui_update_layer(3, "NAV_MEDIA");

dongle_ui_update_volume(65);

dongle_ui_update_ble(1);

dongle_ui_update_modifiers(gui, alt, ctrl, shift);

dongle_ui_update_time("16:33");

这样以后更换 UI 不需要修改 ZMK 数据逻辑。

==================================================
十五、线程安全
=======

这是 ZMK/Zephyr 项目。

不要从任意 input/event thread 直接修改 LVGL。

检查当前项目的 LVGL/ZMK display work queue 架构。

所有 LVGL UI 更新应该通过当前项目已经使用的：

* LVGL task
* work queue
* display update callback
* ZMK display mechanism

进行。

如果当前项目已有 UI update queue：

复用它。

不要自己创建第二套线程同步机制。

禁止：

sleep()
k_sleep()
阻塞 LVGL UI thread

来实现动画。

使用：

lv_timer
k_work_delayable
或者现有项目的异步机制。

==================================================
十六、性能约束
=======

屏幕：

284 × 76 = 21584 pixels

这是小屏，不需要高刷新率。

避免：

* 每个 tick 重建 widget
* 删除/创建 LVGL object
* 全屏动画
* 高频 printf
* 高频日志
* 每帧更新不变的 border/background

UI 初始化时创建一次。

运行时只更新：

text
bar value
state
visibility
style

==================================================
十七、重要：先分析再修改
============

在真正修改代码之前：

1. 找出当前 ST7789 driver
2. 找出 LVGL 初始化位置
3. 找出当前 UI screen
4. 找出 battery 数据来源
5. 找出 layer 数据来源
6. 找出 modifier 数据来源
7. 找出 volume HID callback
8. 找出 BLE/output 状态来源
9. 找出当前 build/Kconfig
10. 确认当前 LVGL major version

先输出：

* 文件路径
* 函数名
* 当前 UI 架构
* 哪些代码可以复用
* 哪些代码需要修改
* 哪些代码绝对不能修改

然后再实施。

不要猜文件位置。

不要因为找不到 callback 就自行创建重复的数据系统。

==================================================
十八、验收标准
=======

编译成功：

west build ...

启动：

必须先显示：

DONGLE BOOTING...

然后 progress：

0 → 100%

之后：

Fade Out
→
主 UI Fade In

正常状态：

中央：

16:33

音量：

触发 on_hid_volume_changed(65)

中央立即变成：

VOL:65%

[██████░░░░]

1500ms 无新事件：

恢复：

16:33

Layer：

on_zmk_layer_changed(3, "NAV_MEDIA")

显示：

> #03 NAV_MEDIA <

并触发：

150ms highlight

Battery：

L 80%
R 90%

Modifier：

GUI pressed：

[GUI]

其它：

#ALT #CTRL #SHFT

右上：

VOL:65% BLE.1

所有动态数据都不能覆盖其它区域。

最终 UI 必须适配：

284 × 76

原生像素。

不要以 320×240、240×280 或其它分辨率作为设计基准。

==================================================
十九、不要过度重构
=========

这是一个已有的 ZMK dongle 项目。

目标不是重写项目。

目标：

“保留硬件和数据层，只重做 UI。”

优先级：

1. 保证 ST7789 正常工作
2. 保证 ZMK 正常工作
3. 保证现有 battery/layer/modifier/BLE 数据正常
4. 替换 UI
5. 增加 boot screen
6. 增加 volume HUD
7. 增加 layer highlight
8. 最后再优化 CRT/pixel visual effects

如果某个视觉效果会明显增加 RAM/Flash/CPU 占用，优先放弃视觉效果而不是破坏 ZMK 稳定性。
