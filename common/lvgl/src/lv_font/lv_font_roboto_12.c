/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --no-compress --no-prefilter --bpp 1 --size 12 --font Roboto-Regular.woff -r 0x41-0x5A,0x61-0x7A --font Roboto-Regular.woff -r 0x30-0x39 --font verdana.ttf -r 0x20-0x2F,0x3A-0x40,0x5B-0x60,0x7B-0x7F --font FontAwesome.ttf -r 61441,61448,61451,61452,61453,61457,61459,61460,61461,61465,61468,61473,61478,61479,61480,61502,61504,61512,61515,61516,61517,61521,61522,61523,61524,61536,61543,61544,61550,61552,61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61671,61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62099,62087,62189 --font fa.woff -r 62212,62218,62810,63426,63650 -o lv_font_roboto_12 --format lvgl --force-fast-kern-format
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_ROBOTO_12
#define LV_FONT_ROBOTO_12 1
#endif

#if LV_FONT_ROBOTO_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfc, 0x80,

    /* U+0022 "\"" */
    0xaa, 0xa0,

    /* U+0023 "#" */
    0x14, 0x14, 0x7f, 0x24, 0x28, 0x28, 0xfe, 0x48,
    0x48,

    /* U+0024 "$" */
    0x21, 0xfa, 0x68, 0xe0, 0xf2, 0x49, 0xf8, 0x82,
    0x0,

    /* U+0025 "%" */
    0x61, 0x12, 0x22, 0x48, 0x49, 0x6, 0x4c, 0x12,
    0x42, 0x48, 0x89, 0x10, 0xc0,

    /* U+0026 "&" */
    0x30, 0x24, 0x12, 0xe, 0x5, 0x24, 0x52, 0x19,
    0x8c, 0x79, 0x0,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x24, 0x48, 0x88, 0x88, 0x84, 0x42,

    /* U+0029 ")" */
    0x42, 0x21, 0x11, 0x11, 0x12, 0x24,

    /* U+002A "*" */
    0x25, 0x5d, 0xf2, 0x0,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0x7a,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x8, 0xc4, 0x22, 0x10, 0x88, 0x42, 0x21, 0x0,

    /* U+0030 "0" */
    0x74, 0xe3, 0x18, 0xc6, 0x3b, 0x70,

    /* U+0031 "1" */
    0x3c, 0x92, 0x49, 0x20,

    /* U+0032 "2" */
    0x74, 0x62, 0x11, 0x11, 0x88, 0xf8,

    /* U+0033 "3" */
    0x74, 0x42, 0x13, 0x4, 0x31, 0x70,

    /* U+0034 "4" */
    0x8, 0x62, 0x8a, 0x4b, 0x2f, 0xc2, 0x8,

    /* U+0035 "5" */
    0xfc, 0x21, 0xe1, 0x86, 0x33, 0x70,

    /* U+0036 "6" */
    0x32, 0x21, 0xe8, 0xc6, 0x39, 0x70,

    /* U+0037 "7" */
    0xfc, 0x10, 0x82, 0x10, 0x43, 0x8, 0x20,

    /* U+0038 "8" */
    0x74, 0x63, 0x17, 0x46, 0x31, 0x70,

    /* U+0039 "9" */
    0x74, 0xe3, 0x18, 0xbc, 0x22, 0x60,

    /* U+003A ":" */
    0x82,

    /* U+003B ";" */
    0x40, 0x15, 0x60,

    /* U+003C "<" */
    0x6, 0x33, 0x87, 0x3, 0x80, 0xc0, 0x0,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0xc0, 0x60, 0x38, 0x73, 0x98, 0x0, 0x0,

    /* U+003F "?" */
    0xf4, 0x42, 0x11, 0x10, 0x80, 0x20,

    /* U+0040 "@" */
    0x1e, 0x18, 0x65, 0xea, 0x89, 0xa2, 0x68, 0x9a,
    0x25, 0x7e, 0x60, 0x7, 0x80,

    /* U+0041 "A" */
    0x18, 0x18, 0x18, 0x24, 0x24, 0x64, 0x7e, 0x42,
    0xc2,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0xf8,

    /* U+0043 "C" */
    0x39, 0x18, 0x60, 0x82, 0x8, 0x51, 0x38,

    /* U+0044 "D" */
    0xf2, 0x28, 0x61, 0x86, 0x18, 0x62, 0xf0,

    /* U+0045 "E" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0xf8,

    /* U+0046 "F" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0x80,

    /* U+0047 "G" */
    0x3c, 0x8e, 0x4, 0x8, 0xf0, 0x60, 0xa1, 0x3c,

    /* U+0048 "H" */
    0x83, 0x6, 0xc, 0x1f, 0xf0, 0x60, 0xc1, 0x82,

    /* U+0049 "I" */
    0xff, 0x80,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0x86, 0x31, 0x70,

    /* U+004B "K" */
    0x8e, 0x29, 0x28, 0xe2, 0x49, 0xa2, 0x84,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0xf8,

    /* U+004D "M" */
    0xc3, 0xc3, 0xc3, 0xa5, 0xa5, 0xa5, 0x99, 0x99,
    0x99,

    /* U+004E "N" */
    0x83, 0x87, 0x8d, 0x99, 0x33, 0x63, 0xc3, 0x82,

    /* U+004F "O" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x8, 0x20, 0x80,

    /* U+0051 "Q" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x3c,
    0xc,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x68, 0xa1, 0x84,

    /* U+0053 "S" */
    0x7a, 0x38, 0x70, 0x30, 0x38, 0x61, 0x78,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x73, 0x78,

    /* U+0056 "V" */
    0xc2, 0x85, 0x1a, 0x22, 0x45, 0x8a, 0xc, 0x18,

    /* U+0057 "W" */
    0xc4, 0x53, 0x14, 0xa5, 0x2b, 0x4a, 0x8c, 0xa3,
    0x18, 0xc6, 0x31, 0x0,

    /* U+0058 "X" */
    0x42, 0xc8, 0xb0, 0xc1, 0x7, 0xb, 0x32, 0x42,

    /* U+0059 "Y" */
    0xc6, 0x89, 0xb1, 0x42, 0x82, 0x4, 0x8, 0x10,

    /* U+005A "Z" */
    0xfc, 0x30, 0x84, 0x30, 0x84, 0x30, 0xfc,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x70,

    /* U+005C "\\" */
    0x41, 0x4, 0x8, 0x20, 0x81, 0x4, 0x8, 0x20,
    0x81,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0xf0,

    /* U+005E "^" */
    0x18, 0x38, 0x24, 0x42, 0xc2,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0x94,

    /* U+0061 "a" */
    0x74, 0x42, 0xf8, 0xc5, 0xe0,

    /* U+0062 "b" */
    0x84, 0x21, 0xe9, 0xc6, 0x31, 0x9f, 0x80,

    /* U+0063 "c" */
    0x76, 0x61, 0x8, 0x65, 0xc0,

    /* U+0064 "d" */
    0x8, 0x42, 0xfc, 0xc6, 0x31, 0xcb, 0xc0,

    /* U+0065 "e" */
    0x73, 0x28, 0xbf, 0x83, 0x27, 0x80,

    /* U+0066 "f" */
    0x34, 0x4f, 0x44, 0x44, 0x44,

    /* U+0067 "g" */
    0x7e, 0x63, 0x18, 0xe5, 0xe1, 0x8b, 0x80,

    /* U+0068 "h" */
    0x84, 0x21, 0xe8, 0xc6, 0x31, 0x8c, 0x40,

    /* U+0069 "i" */
    0xbf, 0x80,

    /* U+006A "j" */
    0x45, 0x55, 0x57,

    /* U+006B "k" */
    0x84, 0x21, 0x2b, 0x73, 0x94, 0x94, 0x40,

    /* U+006C "l" */
    0xff, 0xc0,

    /* U+006D "m" */
    0xf7, 0x44, 0x62, 0x31, 0x18, 0x8c, 0x46, 0x22,

    /* U+006E "n" */
    0xf4, 0x63, 0x18, 0xc6, 0x20,

    /* U+006F "o" */
    0x7b, 0x38, 0x61, 0x87, 0x37, 0x80,

    /* U+0070 "p" */
    0xf4, 0xe3, 0x18, 0xcf, 0xd0, 0x84, 0x0,

    /* U+0071 "q" */
    0x7e, 0x63, 0x18, 0xe5, 0xe1, 0x8, 0x40,

    /* U+0072 "r" */
    0xf2, 0x49, 0x20,

    /* U+0073 "s" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x92, 0x60,

    /* U+0075 "u" */
    0x8c, 0x63, 0x18, 0xc5, 0xe0,

    /* U+0076 "v" */
    0x89, 0x24, 0x94, 0x30, 0xc2, 0x0,

    /* U+0077 "w" */
    0x88, 0xa4, 0x95, 0x4a, 0xa5, 0x51, 0x10, 0x88,

    /* U+0078 "x" */
    0x49, 0x23, 0xc, 0x31, 0x24, 0x80,

    /* U+0079 "y" */
    0x89, 0x24, 0x94, 0x30, 0xc3, 0x8, 0x23, 0x0,

    /* U+007A "z" */
    0xf8, 0xc4, 0x44, 0x63, 0xe0,

    /* U+007B "{" */
    0x19, 0x8, 0x42, 0x32, 0xc, 0x21, 0x8, 0x30,

    /* U+007C "|" */
    0xff, 0xf0,

    /* U+007D "}" */
    0xc1, 0x8, 0x42, 0x18, 0x26, 0x21, 0x9, 0x80,

    /* U+007E "~" */
    0x71, 0x99, 0x8e,

    /* U+F001 "" */
    0x0, 0x0, 0xf1, 0xfc, 0x7f, 0x1e, 0x44, 0x11,
    0x4, 0x47, 0x13, 0xfc, 0x6f, 0x0,

    /* U+F008 "" */
    0xff, 0xfc, 0x82, 0x64, 0x13, 0xe0, 0xf9, 0x4,
    0xc8, 0x27, 0xff, 0xf2, 0x9, 0xf0, 0x7c, 0x82,
    0x64, 0x13, 0xff, 0xf0,

    /* U+F00B "" */
    0xef, 0xfe, 0xff, 0x0, 0xe, 0xff, 0xef, 0xf0,
    0x0, 0x0, 0xe, 0xff, 0xef, 0xf0,

    /* U+F00C "" */
    0x0, 0x80, 0x74, 0x3b, 0x9c, 0xfe, 0x1f, 0x3,
    0x80, 0x40,

    /* U+F00D "" */
    0x42, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0x42,

    /* U+F011 "" */
    0xc, 0xb, 0x47, 0xfb, 0xb6, 0xcc, 0xf3, 0x3c,
    0xf, 0x87, 0x61, 0x8f, 0xc0,

    /* U+F013 "" */
    0xc, 0x1f, 0xe7, 0xf9, 0xce, 0xe1, 0xf8, 0x77,
    0x39, 0xfe, 0x7f, 0x83, 0x0,

    /* U+F014 "" */
    0x1c, 0x11, 0x3f, 0xe8, 0x14, 0xb, 0x55, 0xaa,
    0xd5, 0x6a, 0xa0, 0x5f, 0xc0,

    /* U+F015 "" */
    0xd, 0x82, 0x70, 0xb6, 0x7f, 0xef, 0xfa, 0xff,
    0x1c, 0xe3, 0x9c,

    /* U+F019 "" */
    0xe, 0x1, 0xc0, 0x38, 0x1f, 0xc1, 0xf0, 0x1c,
    0x1, 0x7, 0xdf, 0xff, 0xbf, 0xfc,

    /* U+F01C "" */
    0x3f, 0x18, 0x64, 0xb, 0x3, 0x80, 0x7c, 0xff,
    0xff, 0xff,

    /* U+F021 "" */
    0x3f, 0x5c, 0xf6, 0x1f, 0xf, 0x0, 0x0, 0xf,
    0xf, 0x86, 0xbe, 0x0,

    /* U+F026 "" */
    0x8, 0xff, 0xff, 0xfc, 0x61,

    /* U+F027 "" */
    0x8, 0x18, 0xfa, 0xf9, 0xf9, 0xfa, 0x18, 0x8,

    /* U+F028 "" */
    0x0, 0x81, 0x28, 0x62, 0xfd, 0xdf, 0x9b, 0xf7,
    0x46, 0x68, 0x42, 0x0, 0x80,

    /* U+F03E "" */
    0xff, 0xfc, 0x0, 0x60, 0x3, 0x60, 0x1b, 0x8,
    0xc0, 0xe6, 0x7f, 0xb7, 0xfd, 0xbf, 0xec, 0x0,
    0x7f, 0xfe,

    /* U+F040 "" */
    0x1, 0x80, 0x70, 0x6c, 0x3c, 0x1f, 0xf, 0x87,
    0xc2, 0xe0, 0xd0, 0x38, 0x0,

    /* U+F048 "" */
    0xc3, 0x8f, 0x3e, 0xff, 0xff, 0xf7, 0xe7, 0xc7,
    0x84,

    /* U+F04B "" */
    0x0, 0x60, 0x3c, 0x1f, 0xf, 0xe7, 0xff, 0xfd,
    0xf8, 0xf0, 0x70, 0x20, 0x0,

    /* U+F04C "" */
    0xf3, 0xfc, 0xff, 0x3f, 0xcf, 0xf3, 0xfc, 0xff,
    0x3f, 0xcf, 0xf3, 0xfc, 0xf0,

    /* U+F04D "" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xf0,

    /* U+F051 "" */
    0x87, 0x8f, 0x9f, 0xbf, 0xff, 0xfd, 0xf3, 0xc7,
    0xc,

    /* U+F052 "" */
    0xc, 0x7, 0x83, 0xf1, 0xfe, 0xff, 0xc0, 0x0,
    0x3, 0xff, 0xff, 0xc0,

    /* U+F053 "" */
    0x4, 0x1c, 0x71, 0xc7, 0x1c, 0x1c, 0x1c, 0x1c,
    0x1c, 0x10,

    /* U+F054 "" */
    0xc1, 0xc3, 0xc3, 0xc3, 0xc3, 0xcf, 0x3c, 0xf1,
    0xc3, 0x0,

    /* U+F060 "" */
    0xc, 0x7, 0x83, 0xc1, 0xe0, 0xff, 0xff, 0xf7,
    0x80, 0xf0, 0x1e, 0x3, 0x0,

    /* U+F067 "" */
    0x1c, 0xe, 0x7, 0x1f, 0xff, 0xff, 0xfc, 0x70,
    0x38, 0x1c, 0x0,

    /* U+F068 "" */
    0xff, 0xff, 0xc0,

    /* U+F06E "" */
    0xf, 0x3, 0xbc, 0x57, 0xa9, 0xf9, 0x9f, 0x94,
    0xf2, 0x30, 0xc0, 0xf0,

    /* U+F070 "" */
    0x7, 0x83, 0xb0, 0x57, 0x29, 0x69, 0x9c, 0x94,
    0xd2, 0x28, 0x41, 0xb0, 0x0, 0x0,

    /* U+F071 "" */
    0x6, 0x0, 0x60, 0xf, 0x1, 0x98, 0x19, 0x83,
    0x9c, 0x3f, 0xc7, 0x9e, 0xf9, 0xff, 0xff,

    /* U+F074 "" */
    0x0, 0x0, 0x6, 0xf3, 0xf1, 0x66, 0x1c, 0x0,
    0xc0, 0x18, 0x1, 0x66, 0xe3, 0xf0, 0x6,

    /* U+F077 "" */
    0x4, 0x1, 0xc0, 0x7c, 0x1d, 0xc7, 0x1d, 0xc1,
    0xd0, 0x10,

    /* U+F078 "" */
    0x40, 0x5c, 0x1f, 0xc7, 0x1d, 0xc1, 0xf0, 0x1c,
    0x1, 0x0,

    /* U+F079 "" */
    0x37, 0xf1, 0xe0, 0xcf, 0xc3, 0xc, 0xc, 0x30,
    0xfc, 0xc1, 0xe3, 0xfb, 0x0,

    /* U+F07B "" */
    0x70, 0x1f, 0x3, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xe0,

    /* U+F093 "" */
    0x4, 0x1, 0xc0, 0x7c, 0x1f, 0xc0, 0xe0, 0x1c,
    0x3, 0x87, 0x7, 0xff, 0xff, 0xf7, 0xff, 0x80,

    /* U+F095 "" */
    0x40, 0x70, 0x38, 0x18, 0x4, 0x3, 0x0, 0xcc,
    0x3f, 0x7, 0x0,

    /* U+F0C4 "" */
    0x60, 0x9, 0x3, 0x88, 0xdc, 0x92, 0x76, 0xc0,
    0xe8, 0x7f, 0x6c, 0x99, 0x98, 0x7f, 0x0,

    /* U+F0C5 "" */
    0x1f, 0x3, 0x10, 0x51, 0xff, 0x19, 0x86, 0x98,
    0xc9, 0x8f, 0x98, 0x81, 0xf8, 0x10, 0x81, 0x8,
    0x10, 0xff,

    /* U+F0C7 "" */
    0xff, 0x2c, 0xeb, 0x2e, 0xc9, 0xbe, 0x60, 0x1b,
    0xf6, 0x85, 0xa1, 0x68, 0x5f, 0xfc,

    /* U+F0E7 "" */
    0x71, 0xc6, 0x1f, 0xfb, 0x61, 0x4, 0x20, 0x82,
    0x0,

    /* U+F0EA "" */
    0xff, 0x8c, 0x18, 0xff, 0x8f, 0x8c, 0xf8, 0xaf,
    0x8b, 0xf8, 0xff, 0x81, 0xf8, 0x1f, 0x81, 0x8,
    0x10, 0xff,

    /* U+F0F3 "" */
    0x4, 0x3, 0xe0, 0xfe, 0x1f, 0xc3, 0xf8, 0x7f,
    0xf, 0xe3, 0xfe, 0x7f, 0xdf, 0xfc, 0x78, 0x7,
    0x0,

    /* U+F11C "" */
    0xff, 0xfc, 0x0, 0x75, 0xab, 0x0, 0x5a, 0xd6,
    0xc0, 0x6, 0x7e, 0xbf, 0xff,

    /* U+F124 "" */
    0x0, 0x0, 0xc1, 0xe3, 0xe7, 0xf7, 0xf0, 0x38,
    0x18, 0xc, 0x4, 0x0,

    /* U+F15B "" */
    0xfe, 0x3f, 0x4f, 0xdb, 0xf1, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,

    /* U+F1EB "" */
    0x6, 0x1, 0xff, 0x1c, 0x1f, 0x9f, 0x23, 0x8e,
    0x18, 0x30, 0x3c, 0x1, 0xb0, 0x0, 0x0, 0x10,
    0x0,

    /* U+F240 "" */
    0xff, 0xfe, 0x80, 0x2, 0xbf, 0xfa, 0xbf, 0xf9,
    0xbf, 0xf9, 0xbf, 0xf9, 0xbf, 0xfb, 0x80, 0x2,
    0xff, 0xfe,

    /* U+F241 "" */
    0xff, 0xfe, 0x80, 0x2, 0xbf, 0xe2, 0xbf, 0xe1,
    0xbf, 0xe1, 0xbf, 0xe1, 0xbf, 0xe3, 0x80, 0x2,
    0xff, 0xfe,

    /* U+F242 "" */
    0xff, 0xfe, 0x80, 0x2, 0xbf, 0x2, 0xbf, 0x1,
    0xbf, 0x1, 0xbf, 0x1, 0xbf, 0x3, 0x80, 0x2,
    0xff, 0xfe,

    /* U+F243 "" */
    0xff, 0xfe, 0x80, 0x2, 0xb8, 0x2, 0xb8, 0x1,
    0xb8, 0x1, 0xb8, 0x1, 0xb8, 0x3, 0x80, 0x2,
    0xff, 0xfe,

    /* U+F244 "" */
    0xff, 0xfe, 0x80, 0x2, 0x80, 0x2, 0x80, 0x1,
    0x80, 0x1, 0x80, 0x1, 0x80, 0x3, 0x80, 0x2,
    0xff, 0xfe,

    /* U+F287 "" */
    0x0, 0xc0, 0x7, 0x80, 0x10, 0x7, 0x20, 0x6f,
    0xff, 0xfc, 0x41, 0x80, 0x40, 0x0, 0xb8, 0x0,
    0xf0,

    /* U+F293 "" */
    0x3e, 0x3b, 0xbc, 0xdb, 0x3c, 0x1f, 0x1f, 0x8f,
    0x83, 0xd9, 0xf9, 0x9d, 0xc7, 0xc0,

    /* U+F2ED "" */
    0x0,

    /* U+F304 "" */
    0x0, 0x40, 0xe, 0x0, 0xf0, 0x37, 0x7, 0xa0,
    0xfc, 0x1f, 0x83, 0xf0, 0x7e, 0xf, 0xc0, 0xf8,
    0xf, 0x0, 0x80, 0x0,

    /* U+F30A "" */
    0x20, 0xc, 0x3, 0xff, 0xff, 0xf6, 0x0, 0x40,
    0x0,

    /* U+F55A "" */
    0xf, 0xfe, 0x3f, 0xfc, 0xfb, 0x3b, 0xf0, 0xff,
    0xf3, 0xef, 0xc3, 0xcf, 0xb7, 0x8f, 0xff, 0xf,
    0xfe,

    /* U+F7C2 "" */
    0x1f, 0x9a, 0xbe, 0xaf, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf8,

    /* U+F8A2 "" */
    0x0, 0x0, 0x3, 0x30, 0x37, 0x3, 0xff, 0xff,
    0xff, 0x70, 0x3, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 68, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 76, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 88, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 5, .adv_w = 157, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 122, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 23, .adv_w = 207, .box_w = 11, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 140, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 52, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 48, .adv_w = 87, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 54, .adv_w = 87, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 60, .adv_w = 122, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 64, .adv_w = 157, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 70, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 72, .adv_w = 87, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 73, .adv_w = 70, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 87, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 82, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 108, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 87, .box_w = 1, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 87, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 146, .adv_w = 157, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 157, .box_w = 7, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 157, .adv_w = 157, .box_w = 7, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 105, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 192, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 183, .adv_w = 125, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 125, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 126, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 109, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 131, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 52, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 256, .adv_w = 103, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 262, .adv_w = 168, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 121, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 294, .adv_w = 132, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 303, .adv_w = 118, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 310, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 325, .adv_w = 125, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 122, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 170, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 368, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 375, .adv_w = 87, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 380, .adv_w = 87, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 389, .adv_w = 87, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 394, .adv_w = 157, .box_w = 8, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 399, .adv_w = 122, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 400, .adv_w = 122, .box_w = 2, .box_h = 3, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 401, .adv_w = 104, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 101, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 418, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 102, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 431, .adv_w = 67, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 443, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 47, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 46, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 455, .adv_w = 97, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 462, .adv_w = 47, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 464, .adv_w = 168, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 472, .adv_w = 106, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 477, .adv_w = 110, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 490, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 497, .adv_w = 65, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 99, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 63, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 106, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 514, .adv_w = 93, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 520, .adv_w = 144, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 528, .adv_w = 95, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 534, .adv_w = 91, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 542, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 547, .adv_w = 122, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 555, .adv_w = 87, .box_w = 1, .box_h = 12, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 557, .adv_w = 122, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 565, .adv_w = 157, .box_w = 8, .box_h = 3, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 568, .adv_w = 165, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 582, .adv_w = 206, .box_w = 13, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 602, .adv_w = 192, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 616, .adv_w = 192, .box_w = 10, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 626, .adv_w = 151, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 634, .adv_w = 165, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 647, .adv_w = 165, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 660, .adv_w = 151, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 673, .adv_w = 178, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 684, .adv_w = 178, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 698, .adv_w = 165, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 165, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 82, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 725, .adv_w = 123, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 733, .adv_w = 178, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 746, .adv_w = 206, .box_w = 13, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 764, .adv_w = 165, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 777, .adv_w = 110, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 786, .adv_w = 151, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 799, .adv_w = 165, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 812, .adv_w = 165, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 825, .adv_w = 110, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 834, .adv_w = 165, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 846, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 856, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 866, .adv_w = 165, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 879, .adv_w = 151, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 890, .adv_w = 151, .box_w = 9, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 893, .adv_w = 192, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 192, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 919, .adv_w = 192, .box_w = 12, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 934, .adv_w = 192, .box_w = 12, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 949, .adv_w = 192, .box_w = 11, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 959, .adv_w = 192, .box_w = 11, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 969, .adv_w = 206, .box_w = 14, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 982, .adv_w = 178, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 995, .adv_w = 178, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1011, .adv_w = 151, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1022, .adv_w = 192, .box_w = 12, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1037, .adv_w = 192, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1055, .adv_w = 165, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1069, .adv_w = 96, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1078, .adv_w = 192, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1096, .adv_w = 192, .box_w = 11, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1113, .adv_w = 206, .box_w = 13, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1126, .adv_w = 151, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1138, .adv_w = 165, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1152, .adv_w = 219, .box_w = 13, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1169, .adv_w = 247, .box_w = 16, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1187, .adv_w = 247, .box_w = 16, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1205, .adv_w = 247, .box_w = 16, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1223, .adv_w = 247, .box_w = 16, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1241, .adv_w = 247, .box_w = 16, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1259, .adv_w = 247, .box_w = 15, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1276, .adv_w = 165, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1290, .adv_w = 192, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1291, .adv_w = 192, .box_w = 12, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1311, .adv_w = 168, .box_w = 11, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1320, .adv_w = 240, .box_w = 15, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1337, .adv_w = 144, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1351, .adv_w = 193, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x7, 0xa, 0xb, 0xc, 0x10, 0x12, 0x13,
    0x14, 0x18, 0x1b, 0x20, 0x25, 0x26, 0x27, 0x3d,
    0x3f, 0x47, 0x4a, 0x4b, 0x4c, 0x50, 0x51, 0x52,
    0x53, 0x5f, 0x66, 0x67, 0x6d, 0x6f, 0x70, 0x73,
    0x76, 0x77, 0x78, 0x7a, 0x92, 0x94, 0xc3, 0xc4,
    0xc6, 0xe6, 0xe9, 0xf2, 0x11b, 0x123, 0x15a, 0x1ea,
    0x23f, 0x240, 0x241, 0x242, 0x243, 0x286, 0x292, 0x2ec,
    0x303, 0x309, 0x559, 0x7c1, 0x8a1
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 61441, .range_length = 2210, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 61, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 3, 4, 5, 6, 7,
    0, 8, 8, 9, 10, 11, 8, 8,
    5, 12, 13, 14, 0, 15, 9, 16,
    17, 18, 19, 20, 0, 0, 0, 0,
    0, 0, 21, 22, 0, 0, 23, 24,
    0, 0, 0, 0, 25, 0, 0, 0,
    26, 22, 0, 27, 0, 28, 0, 29,
    0, 30, 29, 31, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 3, 0, 4, 0, 0, 0,
    4, 0, 0, 5, 0, 0, 0, 0,
    4, 0, 4, 0, 6, 7, 8, 9,
    10, 11, 12, 13, 0, 0, 0, 0,
    0, 0, 14, 0, 15, 15, 15, 16,
    15, 0, 0, 0, 0, 0, 17, 17,
    18, 17, 15, 19, 20, 21, 22, 23,
    24, 25, 23, 26, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    -12, -15, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, 0, 0,
    -12, -2, -8, -6, 0, -9, 0, 0,
    0, 0, 0, -1, 0, 0, -2, -1,
    -5, -3, 0, 1, 0, 0, 0, 0,
    0, 0, -3, 0, -2, 0, 0, -5,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, 0, 0, -3, 0,
    -2, 0, -2, -4, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0,
    -2, -2, 0, -2, 0, 0, 0, -2,
    -2, -2, 0, 0, 0, 0, -16, 0,
    -25, 0, 2, 0, 0, 0, 0, 0,
    0, -3, -2, 0, 0, -2, -2, 0,
    0, -2, -2, 0, 0, 0, 0, 0,
    2, 0, 0, 0, -3, 0, 0, 0,
    2, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, -2, -3, 0, 0, 0, -2,
    -4, -6, 0, 0, 0, 0, 2, -6,
    0, 0, -26, -5, -16, -13, 0, -22,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -4, -12, -9, 0, 0, 0, 0,
    -13, 0, -19, 0, 0, 0, 0, 0,
    -3, 0, -2, -1, -1, 0, 0, -1,
    0, 0, 1, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -4, 0,
    -3, -2, 0, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -7, 0, -2, 0, 0, -4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -7, -3,
    -22, -1, 2, 0, 2, 1, 0, 2,
    0, -11, -9, 0, -10, -9, -7, -11,
    0, -9, -7, -5, -7, -6, 0, 0,
    -7, -1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, -4, 0, 0, -4,
    -3, 0, 0, -3, -1, 0, 0, 0,
    0, 0, -4, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, -3, -3, 0,
    0, -3, -2, 0, 0, -2, 0, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0,
    -2, 0, 0, -2, 0, 0, 0, -2,
    -3, 0, 0, 0, 0, 0, -9, -3,
    -9, -1, 2, -9, 2, 2, 1, 2,
    0, -7, -6, -2, -4, -6, -4, -5,
    -2, -4, -2, 0, -2, -3, 0, 0,
    1, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, -2,
    0, 0, 0, -2, -3, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, -1, -1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, -2, -1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -4, -2, 1, 0, -2, 0, 0,
    5, 0, 2, 2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, -1, 1,
    0, -1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -1, 0, 0, -1, 0, 0,
    0, 0, 0, 0, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 31,
    .right_class_cnt     = 26,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t lv_font_roboto_12 = {
#else
lv_font_t lv_font_roboto_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if LV_FONT_ROBOTO_12*/

