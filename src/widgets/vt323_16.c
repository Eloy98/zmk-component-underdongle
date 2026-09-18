/*******************************************************************************
 * Size: 15 px
 * Bpp: 1
 * Opts: --bpp 1 --size 15 --no-compress --stride 1 --align 1 --font VT323 Regular 400.ttf --range 32-127 --format lvgl -o vt323_16.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef VT323_16
#define VT323_16 1
#endif

#if VT323_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf3,

    /* U+0022 "\"" */
    0x99,

    /* U+0023 "#" */
    0xff, 0xff, 0xff, 0xf0,

    /* U+0024 "$" */
    0x6, 0xfd, 0xcf, 0x77, 0xdf, 0x40,

    /* U+0025 "%" */
    0x9d, 0xa4, 0x59, 0xb9,

    /* U+0026 "&" */
    0x6a, 0x5d, 0xbb, 0xbd,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x46, 0xaa, 0x94,

    /* U+0029 ")" */
    0x89, 0x55, 0x68,

    /* U+002A "*" */
    0x4d, 0x6d, 0x66,

    /* U+002B "+" */
    0x66, 0x6f, 0x66,

    /* U+002C "," */
    0xf6,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x11, 0x13, 0x26, 0x44, 0xc8, 0x80,

    /* U+0030 "0" */
    0x69, 0x99, 0x99, 0x96,

    /* U+0031 "1" */
    0x6e, 0x66, 0x66, 0x6f,

    /* U+0032 "2" */
    0xf9, 0x12, 0x64, 0x4f,

    /* U+0033 "3" */
    0xf9, 0x17, 0x11, 0x9f,

    /* U+0034 "4" */
    0x35, 0x99, 0xf1, 0x11,

    /* U+0035 "5" */
    0xf8, 0xed, 0x11, 0x9e,

    /* U+0036 "6" */
    0x7c, 0x8f, 0x9d, 0xd6,

    /* U+0037 "7" */
    0xf1, 0x32, 0x24, 0x44,

    /* U+0038 "8" */
    0x69, 0x96, 0xa9, 0x96,

    /* U+0039 "9" */
    0x6b, 0xbf, 0x11, 0x3e,

    /* U+003A ":" */
    0xf0, 0x3c,

    /* U+003B ";" */
    0xf0, 0x3d, 0x80,

    /* U+003C "<" */
    0x32, 0x4c, 0x44, 0x23,

    /* U+003D "=" */
    0xf0, 0xf0,

    /* U+003E ">" */
    0xc4, 0x23, 0x24, 0x4c,

    /* U+003F "?" */
    0xf9, 0x12, 0x60, 0x6,

    /* U+0040 "@" */
    0x6a, 0x9b, 0xdd, 0xb8, 0x87,

    /* U+0041 "A" */
    0x66, 0xf9, 0xf9, 0x99,

    /* U+0042 "B" */
    0xf9, 0x9f, 0x99, 0x9f,

    /* U+0043 "C" */
    0x79, 0x88, 0x88, 0x97,

    /* U+0044 "D" */
    0xe9, 0x99, 0x99, 0x9e,

    /* U+0045 "E" */
    0xf8, 0x8f, 0x88, 0x8f,

    /* U+0046 "F" */
    0xf8, 0x8f, 0x88, 0x88,

    /* U+0047 "G" */
    0x79, 0x8b, 0x99, 0x97,

    /* U+0048 "H" */
    0x99, 0x9f, 0x99, 0x99,

    /* U+0049 "I" */
    0xf6, 0x66, 0x66, 0x6f,

    /* U+004A "J" */
    0x72, 0x22, 0x22, 0xae,

    /* U+004B "K" */
    0x99, 0xac, 0xaa, 0x99,

    /* U+004C "L" */
    0x88, 0x88, 0x88, 0x8f,

    /* U+004D "M" */
    0x9f, 0xff, 0xdd, 0x99,

    /* U+004E "N" */
    0x9d, 0xdf, 0xbb, 0xb9,

    /* U+004F "O" */
    0x69, 0x99, 0x99, 0x96,

    /* U+0050 "P" */
    0xf9, 0x9f, 0x88, 0x88,

    /* U+0051 "Q" */
    0xf9, 0x99, 0x99, 0xa6, 0x11,

    /* U+0052 "R" */
    0xf9, 0x9e, 0xb9, 0x99,

    /* U+0053 "S" */
    0xf9, 0x8f, 0x11, 0x9f,

    /* U+0054 "T" */
    0xf6, 0x66, 0x66, 0x66,

    /* U+0055 "U" */
    0x99, 0x99, 0x99, 0x9f,

    /* U+0056 "V" */
    0x99, 0x99, 0x9f, 0x66,

    /* U+0057 "W" */
    0xfd, 0xdb, 0xff, 0xf9,

    /* U+0058 "X" */
    0x99, 0x66, 0x6a, 0x99,

    /* U+0059 "Y" */
    0x99, 0x96, 0x66, 0x66,

    /* U+005A "Z" */
    0xf1, 0x36, 0x48, 0x8f,

    /* U+005B "[" */
    0xea, 0xaa, 0xac,

    /* U+005C "\\" */
    0x88, 0x8c, 0x46, 0x22, 0x31, 0x10,

    /* U+005D "]" */
    0xd5, 0x55, 0x5c,

    /* U+005E "^" */
    0x66, 0xa9, 0x90,

    /* U+005F "_" */
    0xf0,

    /* U+0060 "`" */
    0xb0,

    /* U+0061 "a" */
    0xe1, 0xf9, 0x9f,

    /* U+0062 "b" */
    0x88, 0xe9, 0x99, 0x9e,

    /* U+0063 "c" */
    0x79, 0x88, 0x97,

    /* U+0064 "d" */
    0x11, 0x79, 0x99, 0x97,

    /* U+0065 "e" */
    0x79, 0xf8, 0x87,

    /* U+0066 "f" */
    0x74, 0xf4, 0x44, 0x4e,

    /* U+0067 "g" */
    0x79, 0x99, 0x97, 0x16,

    /* U+0068 "h" */
    0x88, 0xf9, 0x99, 0x99,

    /* U+0069 "i" */
    0x60, 0xe6, 0x66, 0x6f,

    /* U+006A "j" */
    0x21, 0x92, 0x49, 0x78,

    /* U+006B "k" */
    0x88, 0xba, 0xcc, 0xab,

    /* U+006C "l" */
    0xe6, 0x66, 0x66, 0x6f,

    /* U+006D "m" */
    0xff, 0xdd, 0xbf,

    /* U+006E "n" */
    0xf9, 0x99, 0x99,

    /* U+006F "o" */
    0x69, 0x99, 0xa6,

    /* U+0070 "p" */
    0xe9, 0x99, 0xae, 0x88,

    /* U+0071 "q" */
    0x79, 0x99, 0x97, 0x11,

    /* U+0072 "r" */
    0xbd, 0xcc, 0xce,

    /* U+0073 "s" */
    0xf8, 0xf1, 0x1f,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x47,

    /* U+0075 "u" */
    0x99, 0x99, 0x9f,

    /* U+0076 "v" */
    0x99, 0x9f, 0x66,

    /* U+0077 "w" */
    0xdf, 0xff, 0xf9,

    /* U+0078 "x" */
    0x9f, 0x66, 0x99,

    /* U+0079 "y" */
    0xca, 0x52, 0xf3, 0x18, 0xcc,

    /* U+007A "z" */
    0xf3, 0x40, 0x8f,

    /* U+007B "{" */
    0x32, 0x66, 0x6c, 0x66, 0x66, 0x30,

    /* U+007C "|" */
    0xff, 0xff, 0xfc,

    /* U+007D "}" */
    0xc4, 0x66, 0x63, 0x66, 0x66, 0xc0,

    /* U+007E "~" */
    0xdd, 0xdb, 0xb0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 96, .box_w = 2, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 4, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 4, .adv_w = 96, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 8, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 14, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 96, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 23, .adv_w = 96, .box_w = 2, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 26, .adv_w = 96, .box_w = 2, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 29, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 32, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 35, .adv_w = 96, .box_w = 2, .box_h = 4, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 36, .adv_w = 96, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 37, .adv_w = 96, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 44, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 48, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 72, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 96, .box_w = 2, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 96, .box_w = 2, .box_h = 9, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 89, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 96, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 95, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 120, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 168, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 177, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 189, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 205, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 96, .box_w = 2, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 216, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 222, .adv_w = 96, .box_w = 2, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 225, .adv_w = 96, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 228, .adv_w = 96, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 96, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 230, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 255, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 96, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 267, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 278, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 288, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 292, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 96, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 302, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 319, .adv_w = 96, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 328, .adv_w = 96, .box_w = 2, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 331, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 337, .adv_w = 96, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t vt323_16 = {
#else
lv_font_t vt323_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if VT323_16*/
