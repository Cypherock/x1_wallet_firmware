#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifndef LV_FONT_ROBOTO_12
#define LV_FONT_ROBOTO_12 1
#endif

#if LV_FONT_ROBOTO_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */
    0x0,

    /* U+21 "!" */
    0xfc, 0x80,

    /* U+22 "\"" */
    0xfc,

    /* U+23 "#" */
    0x28, 0xaf, 0xd2, 0x4b, 0xf5, 0x14, 0x50,

    /* U+24 "$" */
    0x23, 0xa7, 0x18, 0x30, 0xc1, 0x8c, 0x5c, 0x40,

    /* U+25 "%" */
    0x60, 0x94, 0x94, 0x68, 0x10, 0x16, 0x29, 0x29,
    0x6,

    /* U+26 "&" */
    0x30, 0x91, 0x23, 0x86, 0x1a, 0xa3, 0x44, 0x7c,

    /* U+27 "'" */
    0xe0,

    /* U+28 "(" */
    0x29, 0x49, 0x24, 0x91, 0x22,

    /* U+29 ")" */
    0x89, 0x12, 0x49, 0x25, 0x28,

    /* U+2A "*" */
    0x21, 0x3e, 0xc5, 0x0,

    /* U+2B "+" */
    0x20, 0x82, 0x3f, 0x20, 0x82, 0x0,

    /* U+2C "," */
    0x54,

    /* U+2D "-" */
    0xe0,

    /* U+2E "." */
    0x80,

    /* U+2F "/" */
    0x10, 0x84, 0x42, 0x11, 0x8, 0x44, 0x0,

    /* U+30 "0" */
    0x74, 0xe3, 0x18, 0xc6, 0x3b, 0x70,

    /* U+31 "1" */
    0x3c, 0x92, 0x49, 0x20,

    /* U+32 "2" */
    0x74, 0x62, 0x11, 0x11, 0x88, 0xf8,

    /* U+33 "3" */
    0x74, 0x42, 0x13, 0x4, 0x31, 0x70,

    /* U+34 "4" */
    0x8, 0x62, 0x8a, 0x4b, 0x2f, 0xc2, 0x8,

    /* U+35 "5" */
    0xfc, 0x21, 0xe1, 0x86, 0x31, 0x70,

    /* U+36 "6" */
    0x32, 0x21, 0xe8, 0xc6, 0x39, 0x70,

    /* U+37 "7" */
    0xfc, 0x10, 0x82, 0x10, 0x43, 0x8, 0x20,

    /* U+38 "8" */
    0x74, 0x63, 0x17, 0x46, 0x31, 0x70,

    /* U+39 "9" */
    0x74, 0xe3, 0x18, 0xbc, 0x22, 0x60,

    /* U+3A ":" */
    0x82,

    /* U+3B ";" */
    0x87,

    /* U+3C "<" */
    0x9, 0xb1, 0xc3, 0x84,

    /* U+3D "=" */
    0xf8, 0x1, 0xf0,

    /* U+3E ">" */
    0x83, 0x6, 0x7e, 0x40,

    /* U+3F "?" */
    0x72, 0x42, 0x11, 0x10, 0x80, 0x20,

    /* U+40 "@" */
    0x1e, 0x8, 0x64, 0xa, 0x39, 0x92, 0x64, 0x99,
    0x26, 0x5b, 0x9b, 0x90, 0x6, 0x0, 0xf8,

    /* U+41 "A" */
    0x18, 0x18, 0x18, 0x24, 0x24, 0x64, 0x7e, 0x42,
    0xc2,

    /* U+42 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0xf8,

    /* U+43 "C" */
    0x39, 0x18, 0x60, 0x82, 0x8, 0x51, 0x38,

    /* U+44 "D" */
    0xf2, 0x28, 0x61, 0x86, 0x18, 0x62, 0xf0,

    /* U+45 "E" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0xf8,

    /* U+46 "F" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0x80,

    /* U+47 "G" */
    0x3c, 0x8e, 0x4, 0x8, 0xf0, 0x60, 0xa1, 0x3c,

    /* U+48 "H" */
    0x83, 0x6, 0xc, 0x1f, 0xf0, 0x60, 0xc1, 0x82,

    /* U+49 "I" */
    0xff, 0x80,

    /* U+4A "J" */
    0x8, 0x42, 0x10, 0x86, 0x31, 0x70,

    /* U+4B "K" */
    0x8e, 0x29, 0x28, 0xe2, 0x49, 0xa2, 0x84,

    /* U+4C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0xf8,

    /* U+4D "M" */
    0xc3, 0xc3, 0xc3, 0xa5, 0xa5, 0xa5, 0x99, 0x99,
    0x99,

    /* U+4E "N" */
    0x83, 0x87, 0x8d, 0x99, 0x33, 0x63, 0xc3, 0x82,

    /* U+4F "O" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+50 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x8, 0x20, 0x80,

    /* U+51 "Q" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x3c,
    0xc,

    /* U+52 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x28, 0xa1, 0x84,

    /* U+53 "S" */
    0x7a, 0x38, 0x70, 0x30, 0x38, 0x71, 0x78,

    /* U+54 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,

    /* U+55 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x73, 0x78,

    /* U+56 "V" */
    0xc2, 0x85, 0x1a, 0x22, 0x45, 0x8a, 0xc, 0x18,

    /* U+57 "W" */
    0xc4, 0x53, 0x14, 0xa5, 0x2b, 0x4a, 0x8c, 0xa3,
    0x18, 0xc6, 0x31, 0x0,

    /* U+58 "X" */
    0x42, 0xc8, 0xb0, 0xc1, 0x7, 0xb, 0x32, 0x42,

    /* U+59 "Y" */
    0xc6, 0x89, 0xb1, 0x42, 0x82, 0x4, 0x8, 0x10,

    /* U+5A "Z" */
    0xfc, 0x30, 0x84, 0x30, 0x84, 0x30, 0xfc,

    /* U+5B "[" */
    0xea, 0xaa, 0xaa, 0xc0,

    /* U+5C "\\" */
    0x82, 0x10, 0xc2, 0x10, 0x42, 0x10, 0x40,

    /* U+5D "]" */
    0xd5, 0x55, 0x55, 0xc0,

    /* U+5E "^" */
    0x21, 0x14, 0xa5, 0x0,

    /* U+5F "_" */
    0xf8,

    /* U+60 "`" */
    0x4c,

    /* U+61 "a" */
    0x74, 0x42, 0xf8, 0xc5, 0xe0,

    /* U+62 "b" */
    0x84, 0x21, 0xe9, 0xc6, 0x31, 0x9f, 0x80,

    /* U+63 "c" */
    0x7b, 0x28, 0x20, 0x83, 0x27, 0x0,

    /* U+64 "d" */
    0x8, 0x42, 0xfc, 0xc6, 0x31, 0xcb, 0xc0,

    /* U+65 "e" */
    0x76, 0x63, 0xf8, 0x65, 0xc0,

    /* U+66 "f" */
    0x34, 0x4f, 0x44, 0x44, 0x44,

    /* U+67 "g" */
    0x7e, 0x63, 0x18, 0xe5, 0xe1, 0x9b, 0x80,

    /* U+68 "h" */
    0x84, 0x21, 0xe8, 0xc6, 0x31, 0x8c, 0x40,

    /* U+69 "i" */
    0xbf, 0x80,

    /* U+6A "j" */
    0x45, 0x55, 0x57,

    /* U+6B "k" */
    0x84, 0x21, 0x2b, 0x73, 0x94, 0x94, 0x40,

    /* U+6C "l" */
    0xff, 0xc0,

    /* U+6D "m" */
    0xf7, 0x44, 0x62, 0x31, 0x18, 0x8c, 0x46, 0x22,

    /* U+6E "n" */
    0xf4, 0x63, 0x18, 0xc6, 0x20,

    /* U+6F "o" */
    0x7b, 0x38, 0x61, 0x87, 0x37, 0x80,

    /* U+70 "p" */
    0xf4, 0xe3, 0x18, 0xcf, 0xd0, 0x84, 0x0,

    /* U+71 "q" */
    0x7e, 0x63, 0x18, 0xe5, 0xe1, 0x8, 0x40,

    /* U+72 "r" */
    0xf2, 0x49, 0x20,

    /* U+73 "s" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+74 "t" */
    0x4b, 0xa4, 0x92, 0x60,

    /* U+75 "u" */
    0x8c, 0x63, 0x18, 0xc5, 0xe0,

    /* U+76 "v" */
    0x89, 0x24, 0x94, 0x30, 0xc2, 0x0,

    /* U+77 "w" */
    0x88, 0xa4, 0x95, 0x4a, 0xa5, 0x51, 0x10, 0x88,

    /* U+78 "x" */
    0x49, 0x23, 0xc, 0x31, 0x24, 0x80,

    /* U+79 "y" */
    0x89, 0x24, 0x94, 0x30, 0xc3, 0x8, 0x23, 0x0,

    /* U+7A "z" */
    0xf8, 0xc4, 0x44, 0x63, 0xe0,

    /* U+7B "{" */
    0x29, 0x24, 0xa2, 0x49, 0x22,

    /* U+7C "|" */
    0xff, 0xe0,

    /* U+7D "}" */
    0x89, 0x24, 0x8a, 0x49, 0x28,

    /* U+7E "~" */
    0x65, 0x2a, 0x70,

    /* U+F001 "" */
    0x0, 0x70, 0x3f, 0x1f, 0xf1, 0xfb, 0x1c, 0x31,
    0x83, 0x18, 0x31, 0x83, 0x19, 0xf7, 0x9f, 0xf8,
    0x47, 0x0,

    /* U+F008 "" */
    0xbf, 0xde, 0x7, 0xa0, 0x5e, 0x7, 0xbf, 0xde,
    0x7, 0xa0, 0x5e, 0x7, 0xbf, 0xd0,

    /* U+F00B "" */
    0xf7, 0xf7, 0xbf, 0xfd, 0xfe, 0x0, 0xf, 0x7f,
    0x7b, 0xff, 0xdf, 0xc0, 0x0, 0xf7, 0xf7, 0xbf,
    0xfd, 0xfc,

    /* U+F00C "" */
    0x0, 0x20, 0x7, 0x0, 0xe4, 0x1c, 0xe3, 0x87,
    0x70, 0x3e, 0x1, 0xc0, 0x8, 0x0,

    /* U+F00D "" */
    0xc3, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0xc3,

    /* U+F011 "" */
    0x6, 0x2, 0x64, 0x76, 0xe6, 0x66, 0xc6, 0x3c,
    0x63, 0xc6, 0x3c, 0x3, 0x60, 0x67, 0xe, 0x3f,
    0xc0, 0xf0,

    /* U+F013 "" */
    0xe, 0x4, 0xf0, 0x7f, 0xef, 0xfe, 0x71, 0xe7,
    0xc, 0x71, 0xef, 0xfe, 0x7f, 0xe4, 0xf0, 0xe,
    0x0,

    /* U+F015 "" */
    0x3, 0x30, 0x1e, 0xc1, 0xcf, 0xc, 0xcc, 0x6f,
    0xdb, 0x7f, 0xb3, 0xff, 0xf, 0x3c, 0x3c, 0xf0,
    0xf3, 0xc0,

    /* U+F019 "" */
    0xe, 0x0, 0xe0, 0xe, 0x0, 0xe0, 0x3f, 0xc3,
    0xf8, 0x1f, 0x0, 0xe0, 0xf5, 0xff, 0xff, 0xff,
    0x5f, 0xff,

    /* U+F01C "" */
    0x1f, 0xe0, 0xc0, 0xc6, 0x1, 0x90, 0x2, 0xf8,
    0xf, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,

    /* U+F021 "" */
    0x0, 0x31, 0xf3, 0x71, 0xfc, 0x7, 0xc3, 0xf0,
    0x0, 0x0, 0x0, 0x0, 0xfc, 0x3e, 0x3, 0xf8,
    0xec, 0xf8, 0xc0, 0x0,

    /* U+F026 "" */
    0xc, 0x7f, 0xff, 0xff, 0xf1, 0xc3,

    /* U+F027 "" */
    0xc, 0xe, 0x3f, 0x7f, 0x9f, 0xdf, 0xe0, 0x70,
    0x18,

    /* U+F028 "" */
    0x0, 0x60, 0x1, 0x83, 0x34, 0x38, 0xdf, 0xda,
    0xfe, 0x57, 0xf6, 0xbf, 0x8d, 0x1c, 0xd0, 0x61,
    0x80, 0x18,

    /* U+F03E "" */
    0xff, 0xf9, 0xff, 0x9f, 0xf9, 0xef, 0xfc, 0x7d,
    0x83, 0xc0, 0x38, 0x3, 0xff, 0xf0,

    /* U+F048 "" */
    0xc3, 0xc7, 0xcf, 0xdf, 0xff, 0xff, 0xdf, 0xcf,
    0xc7, 0xc3,

    /* U+F04B "" */
    0x0, 0x1c, 0x3, 0xe0, 0x7f, 0xf, 0xf9, 0xff,
    0xbf, 0xff, 0xfe, 0xff, 0x9f, 0xc3, 0xe0, 0x70,
    0x0, 0x0,

    /* U+F04C "" */
    0xfb, 0xff, 0x7f, 0xef, 0xfd, 0xff, 0xbf, 0xf7,
    0xfe, 0xff, 0xdf, 0xfb, 0xff, 0x7c,

    /* U+F04D "" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,

    /* U+F051 "" */
    0xc3, 0xe3, 0xf3, 0xfb, 0xff, 0xff, 0xfb, 0xf3,
    0xe3, 0xc3,

    /* U+F052 "" */
    0xc, 0x3, 0xc0, 0x7c, 0x1f, 0xc7, 0xfd, 0xff,
    0xbf, 0xf0, 0x0, 0xff, 0xff, 0xff, 0xff, 0x80,

    /* U+F053 "" */
    0xc, 0x73, 0x9c, 0xe3, 0x87, 0xe, 0x1c, 0x30,

    /* U+F054 "" */
    0x83, 0x87, 0xe, 0x1c, 0x73, 0x9c, 0xe2, 0x0,

    /* U+F060 "" */
    0xc, 0x3, 0x80, 0xe0, 0x38, 0xf, 0xff, 0xff,
    0xdc, 0x1, 0xc0, 0x1c, 0x1, 0x80,

    /* U+F067 "" */
    0xe, 0x1, 0xc0, 0x38, 0x7, 0xf, 0xff, 0xff,
    0xc3, 0x80, 0x70, 0xe, 0x1, 0xc0,

    /* U+F068 "" */
    0xff, 0xff, 0xfc,

    /* U+F06E "" */
    0xf, 0x81, 0xc7, 0x1c, 0x1d, 0xc6, 0x7e, 0xfb,
    0xf7, 0xdd, 0xdd, 0xc7, 0x1c, 0xf, 0x80,

    /* U+F070 "" */
    0x0, 0x1, 0xc0, 0x1, 0xdf, 0x0, 0xe3, 0x80,
    0xd3, 0x84, 0xfb, 0x9c, 0x77, 0x3c, 0x6e, 0x38,
    0x78, 0x38, 0x70, 0x1e, 0x30, 0x0, 0x30, 0x0,
    0x0,

    /* U+F071 "" */
    0x3, 0x0, 0x1c, 0x0, 0xf8, 0x3, 0xf0, 0x1c,
    0xc0, 0x73, 0x83, 0xcf, 0x1f, 0xfc, 0x7c, 0xfb,
    0xf3, 0xef, 0xff, 0x80,

    /* U+F074 "" */
    0x0, 0x0, 0x6, 0xe1, 0xff, 0x3f, 0x17, 0x60,
    0xe4, 0x1f, 0x6f, 0xbf, 0xf1, 0xf0, 0x6, 0x0,
    0x40,

    /* U+F077 "" */
    0x0, 0x3, 0x1, 0xe0, 0xcc, 0x61, 0xb0, 0x30,
    0x0,

    /* U+F078 "" */
    0x0, 0x30, 0x36, 0x18, 0xcc, 0x1e, 0x3, 0x0,
    0x0,

    /* U+F079 "" */
    0x30, 0x0, 0xf7, 0xf3, 0xf0, 0x65, 0xa0, 0xc3,
    0x1, 0x86, 0xb, 0x4c, 0x1f, 0x9f, 0xde, 0x0,
    0x18,

    /* U+F07B "" */
    0x78, 0xf, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,

    /* U+F093 "" */
    0x6, 0x0, 0xf0, 0x1f, 0x83, 0xfc, 0x7, 0x0,
    0x70, 0x7, 0x0, 0x70, 0xf7, 0xff, 0xff, 0xff,
    0x5f, 0xff,

    /* U+F095 "" */
    0x0, 0x0, 0xf, 0x0, 0xf0, 0x1f, 0x0, 0xf0,
    0x6, 0x0, 0xe0, 0x1c, 0x73, 0xcf, 0xf8, 0xfe,
    0xf, 0xc0, 0x0, 0x0,

    /* U+F0C4 "" */
    0x70, 0x5b, 0x3f, 0x6f, 0x3f, 0xc1, 0xf0, 0x3e,
    0x1f, 0xe6, 0xde, 0xd9, 0xee, 0x8,

    /* U+F0C5 "" */
    0x1f, 0x43, 0xef, 0x7f, 0xef, 0xfd, 0xff, 0xbf,
    0xf7, 0xfe, 0xff, 0xdf, 0xf8, 0x3, 0xfc, 0x0,

    /* U+F0C7 "" */
    0xff, 0x98, 0x1b, 0x3, 0xe0, 0x7c, 0xf, 0xff,
    0xfe, 0x7f, 0x8f, 0xf9, 0xff, 0xfc,

    /* U+F0E7 "" */
    0x78, 0x78, 0xf8, 0xf0, 0xff, 0xfe, 0xfc, 0x1c,
    0x18, 0x18, 0x10, 0x30,

    /* U+F0EA "" */
    0x18, 0x3b, 0x8e, 0xe3, 0xf8, 0xe0, 0x3b, 0xae,
    0xe7, 0xbf, 0xef, 0xfb, 0xf0, 0xfc, 0x3f,

    /* U+F0F3 "" */
    0x4, 0x0, 0x80, 0x7c, 0x1f, 0xc3, 0xf8, 0x7f,
    0x1f, 0xf3, 0xfe, 0x7f, 0xdf, 0xfc, 0x0, 0x7,
    0x0,

    /* U+F11C "" */
    0xff, 0xff, 0x52, 0xbd, 0x4a, 0xff, 0xff, 0xeb,
    0x5f, 0xff, 0xfd, 0x2, 0xf4, 0xb, 0xff, 0xfc,

    /* U+F124 "" */
    0x0, 0x0, 0xf, 0x3, 0xf0, 0xfe, 0x3f, 0xef,
    0xfc, 0xff, 0xc0, 0x78, 0x7, 0x80, 0x78, 0x7,
    0x0, 0x70, 0x2, 0x0,

    /* U+F15B "" */
    0xfa, 0x7d, 0xbe, 0xff, 0xf, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,

    /* U+F1EB "" */
    0x7, 0xc0, 0x7f, 0xf1, 0xe0, 0xf7, 0x0, 0x70,
    0x7c, 0x3, 0xfe, 0x6, 0xc, 0x0, 0x0, 0x3,
    0x80, 0x7, 0x0, 0xe, 0x0,

    /* U+F240 "" */
    0xff, 0xff, 0x80, 0x1f, 0x7f, 0xfe, 0xff, 0xbd,
    0xff, 0x78, 0x1, 0xff, 0xff, 0x80,

    /* U+F241 "" */
    0xff, 0xff, 0x80, 0x1f, 0x7f, 0x3e, 0xfe, 0x3d,
    0xfc, 0x78, 0x1, 0xff, 0xff, 0x80,

    /* U+F242 "" */
    0xff, 0xff, 0x80, 0x1f, 0x78, 0x3e, 0xf0, 0x3d,
    0xe0, 0x78, 0x1, 0xff, 0xff, 0x80,

    /* U+F243 "" */
    0xff, 0xff, 0x80, 0x1f, 0x60, 0x3e, 0xc0, 0x3d,
    0x80, 0x78, 0x1, 0xff, 0xff, 0x80,

    /* U+F244 "" */
    0xff, 0xff, 0x80, 0x1f, 0x0, 0x3e, 0x0, 0x3c,
    0x0, 0x78, 0x1, 0xff, 0xff, 0x80,

    /* U+F287 "" */
    0x0, 0xc0, 0x7, 0x80, 0x10, 0x7, 0x20, 0x6f,
    0xff, 0xfc, 0x41, 0x80, 0x40, 0x0, 0xb8, 0x0,
    0xf0,

    /* U+F293 "" */
    0x3e, 0x3b, 0x9c, 0xdb, 0x7c, 0xbf, 0x1f, 0x9f,
    0x87, 0xd5, 0xf9, 0x9d, 0xc7, 0xc0,

    /* U+F2ED "" */
    0xe, 0x1f, 0xfc, 0x0, 0x0, 0x7, 0xfc, 0xd5,
    0x9a, 0xb3, 0x56, 0x6a, 0xcd, 0x59, 0xab, 0x3f,
    0xe0,

    /* U+F304 "" */
    0x0, 0x0, 0xe, 0x0, 0xf0, 0x37, 0x7, 0xa0,
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
    {.bitmap_index = 0, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 49, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 61, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 4, .adv_w = 118, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 11, .adv_w = 108, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 19, .adv_w = 141, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 119, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 33, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 37, .adv_w = 66, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 42, .adv_w = 67, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 47, .adv_w = 83, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 51, .adv_w = 109, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 57, .adv_w = 38, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 58, .adv_w = 53, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 59, .adv_w = 51, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 79, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 67, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 108, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 89, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 108, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 47, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 41, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 129, .adv_w = 98, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 133, .adv_w = 105, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 136, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 140, .adv_w = 91, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 172, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 161, .adv_w = 125, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 125, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 126, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 109, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 131, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 52, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 106, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 103, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 168, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 121, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 132, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 281, .adv_w = 118, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 125, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 310, .adv_w = 122, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 170, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 330, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 51, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 357, .adv_w = 79, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 364, .adv_w = 51, .box_w = 2, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 368, .adv_w = 80, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 372, .adv_w = 87, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 373, .adv_w = 59, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 374, .adv_w = 104, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 101, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 399, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 404, .adv_w = 67, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 416, .adv_w = 106, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 47, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 46, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 428, .adv_w = 97, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 47, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 437, .adv_w = 168, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 445, .adv_w = 106, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 110, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 463, .adv_w = 109, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 470, .adv_w = 65, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 99, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 63, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 482, .adv_w = 106, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 487, .adv_w = 93, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 493, .adv_w = 144, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 501, .adv_w = 95, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 507, .adv_w = 91, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 515, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 520, .adv_w = 65, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 525, .adv_w = 47, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 527, .adv_w = 65, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 532, .adv_w = 131, .box_w = 7, .box_h = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 535, .adv_w = 192, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 553, .adv_w = 192, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 192, .box_w = 13, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 585, .adv_w = 192, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 599, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 607, .adv_w = 192, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 625, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 642, .adv_w = 216, .box_w = 14, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 660, .adv_w = 192, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 678, .adv_w = 216, .box_w = 14, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 694, .adv_w = 192, .box_w = 12, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 714, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 144, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 729, .adv_w = 216, .box_w = 13, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 747, .adv_w = 192, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 761, .adv_w = 168, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 771, .adv_w = 168, .box_w = 11, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 789, .adv_w = 168, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 803, .adv_w = 168, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 817, .adv_w = 168, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 827, .adv_w = 168, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 843, .adv_w = 120, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 851, .adv_w = 120, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 859, .adv_w = 168, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 873, .adv_w = 168, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 887, .adv_w = 168, .box_w = 11, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 890, .adv_w = 216, .box_w = 13, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 240, .box_w = 15, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 930, .adv_w = 216, .box_w = 14, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 950, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 967, .adv_w = 168, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 976, .adv_w = 168, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 985, .adv_w = 240, .box_w = 15, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1002, .adv_w = 192, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1016, .adv_w = 192, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1034, .adv_w = 192, .box_w = 12, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1054, .adv_w = 168, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1068, .adv_w = 168, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1084, .adv_w = 168, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1098, .adv_w = 120, .box_w = 8, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1110, .adv_w = 168, .box_w = 10, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1125, .adv_w = 168, .box_w = 11, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1142, .adv_w = 216, .box_w = 14, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1158, .adv_w = 192, .box_w = 12, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1178, .adv_w = 144, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1192, .adv_w = 240, .box_w = 15, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1213, .adv_w = 240, .box_w = 15, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1227, .adv_w = 240, .box_w = 15, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1241, .adv_w = 240, .box_w = 15, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1255, .adv_w = 240, .box_w = 15, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1269, .adv_w = 240, .box_w = 15, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1283, .adv_w = 240, .box_w = 15, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1300, .adv_w = 168, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1314, .adv_w = 168, .box_w = 11, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1331, .adv_w = 192, .box_w = 12, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1351, .adv_w = 168, .box_w = 11, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1360, .adv_w = 240, .box_w = 15, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1377, .adv_w = 144, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1391, .adv_w = 193, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x7, 0xa, 0xb, 0xc, 0x10, 0x12, 0x14,
    0x18, 0x1b, 0x20, 0x25, 0x26, 0x27, 0x3d, 0x47,
    0x4a, 0x4b, 0x4c, 0x50, 0x51, 0x52, 0x53, 0x5f,
    0x66, 0x67, 0x6d, 0x6f, 0x70, 0x73, 0x76, 0x77,
    0x78, 0x7a, 0x92, 0x94, 0xc3, 0xc4, 0xc6, 0xe6,
    0xe9, 0xf2, 0x11b, 0x123, 0x15a, 0x1ea, 0x23f, 0x240,
    0x241, 0x242, 0x243, 0x286, 0x292, 0x2ec, 0x303, 0x309,
    0x559, 0x7c1, 0x8a1
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
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 59, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    1, 53,
    3, 3,
    3, 8,
    3, 34,
    3, 66,
    3, 68,
    3, 69,
    3, 70,
    3, 72,
    3, 78,
    3, 79,
    3, 80,
    3, 81,
    3, 82,
    3, 84,
    3, 88,
    8, 3,
    8, 8,
    8, 34,
    8, 66,
    8, 68,
    8, 69,
    8, 70,
    8, 72,
    8, 78,
    8, 79,
    8, 80,
    8, 81,
    8, 82,
    8, 84,
    8, 88,
    9, 55,
    9, 56,
    9, 58,
    13, 3,
    13, 8,
    15, 3,
    15, 8,
    16, 16,
    34, 3,
    34, 8,
    34, 32,
    34, 36,
    34, 40,
    34, 48,
    34, 50,
    34, 53,
    34, 54,
    34, 55,
    34, 56,
    34, 58,
    34, 80,
    34, 85,
    34, 86,
    34, 87,
    34, 88,
    34, 90,
    34, 91,
    35, 53,
    35, 55,
    35, 58,
    36, 10,
    36, 53,
    36, 62,
    36, 94,
    37, 13,
    37, 15,
    37, 34,
    37, 53,
    37, 55,
    37, 57,
    37, 58,
    37, 59,
    38, 53,
    38, 68,
    38, 69,
    38, 70,
    38, 71,
    38, 72,
    38, 80,
    38, 82,
    38, 86,
    38, 87,
    38, 88,
    38, 90,
    39, 13,
    39, 15,
    39, 34,
    39, 43,
    39, 53,
    39, 66,
    39, 68,
    39, 69,
    39, 70,
    39, 72,
    39, 80,
    39, 82,
    39, 83,
    39, 86,
    39, 87,
    39, 90,
    41, 34,
    41, 53,
    41, 57,
    41, 58,
    42, 34,
    42, 53,
    42, 57,
    42, 58,
    43, 34,
    44, 14,
    44, 36,
    44, 40,
    44, 48,
    44, 50,
    44, 68,
    44, 69,
    44, 70,
    44, 72,
    44, 78,
    44, 79,
    44, 80,
    44, 81,
    44, 82,
    44, 86,
    44, 87,
    44, 88,
    44, 90,
    45, 3,
    45, 8,
    45, 34,
    45, 36,
    45, 40,
    45, 48,
    45, 50,
    45, 53,
    45, 54,
    45, 55,
    45, 56,
    45, 58,
    45, 86,
    45, 87,
    45, 88,
    45, 90,
    46, 34,
    46, 53,
    46, 57,
    46, 58,
    47, 34,
    47, 53,
    47, 57,
    47, 58,
    48, 13,
    48, 15,
    48, 34,
    48, 53,
    48, 55,
    48, 57,
    48, 58,
    48, 59,
    49, 13,
    49, 15,
    49, 34,
    49, 43,
    49, 57,
    49, 59,
    49, 66,
    49, 68,
    49, 69,
    49, 70,
    49, 72,
    49, 80,
    49, 82,
    49, 85,
    49, 87,
    49, 90,
    50, 53,
    50, 55,
    50, 56,
    50, 58,
    51, 53,
    51, 55,
    51, 58,
    53, 1,
    53, 13,
    53, 14,
    53, 15,
    53, 34,
    53, 36,
    53, 40,
    53, 43,
    53, 48,
    53, 50,
    53, 52,
    53, 53,
    53, 55,
    53, 56,
    53, 58,
    53, 66,
    53, 68,
    53, 69,
    53, 70,
    53, 72,
    53, 78,
    53, 79,
    53, 80,
    53, 81,
    53, 82,
    53, 83,
    53, 84,
    53, 86,
    53, 87,
    53, 88,
    53, 89,
    53, 90,
    53, 91,
    54, 34,
    55, 10,
    55, 13,
    55, 14,
    55, 15,
    55, 34,
    55, 36,
    55, 40,
    55, 48,
    55, 50,
    55, 62,
    55, 66,
    55, 68,
    55, 69,
    55, 70,
    55, 72,
    55, 80,
    55, 82,
    55, 83,
    55, 86,
    55, 87,
    55, 90,
    55, 94,
    56, 10,
    56, 13,
    56, 14,
    56, 15,
    56, 34,
    56, 53,
    56, 62,
    56, 66,
    56, 68,
    56, 69,
    56, 70,
    56, 72,
    56, 80,
    56, 82,
    56, 83,
    56, 86,
    56, 94,
    57, 14,
    57, 36,
    57, 40,
    57, 48,
    57, 50,
    57, 55,
    57, 68,
    57, 69,
    57, 70,
    57, 72,
    57, 80,
    57, 82,
    57, 86,
    57, 87,
    57, 90,
    58, 7,
    58, 10,
    58, 11,
    58, 13,
    58, 14,
    58, 15,
    58, 34,
    58, 36,
    58, 40,
    58, 43,
    58, 48,
    58, 50,
    58, 52,
    58, 53,
    58, 54,
    58, 55,
    58, 56,
    58, 57,
    58, 58,
    58, 62,
    58, 66,
    58, 68,
    58, 69,
    58, 70,
    58, 71,
    58, 72,
    58, 78,
    58, 79,
    58, 80,
    58, 81,
    58, 82,
    58, 83,
    58, 84,
    58, 85,
    58, 86,
    58, 87,
    58, 89,
    58, 90,
    58, 91,
    58, 94,
    59, 34,
    59, 36,
    59, 40,
    59, 48,
    59, 50,
    59, 68,
    59, 69,
    59, 70,
    59, 72,
    59, 80,
    59, 82,
    59, 86,
    59, 87,
    59, 88,
    59, 90,
    60, 43,
    60, 54,
    66, 3,
    66, 8,
    66, 87,
    66, 90,
    67, 3,
    67, 8,
    67, 87,
    67, 89,
    67, 90,
    67, 91,
    68, 3,
    68, 8,
    70, 3,
    70, 8,
    70, 87,
    70, 90,
    71, 3,
    71, 8,
    71, 10,
    71, 62,
    71, 68,
    71, 69,
    71, 70,
    71, 72,
    71, 82,
    71, 94,
    73, 3,
    73, 8,
    76, 68,
    76, 69,
    76, 70,
    76, 72,
    76, 82,
    78, 3,
    78, 8,
    79, 3,
    79, 8,
    80, 3,
    80, 8,
    80, 87,
    80, 89,
    80, 90,
    80, 91,
    81, 3,
    81, 8,
    81, 87,
    81, 89,
    81, 90,
    81, 91,
    83, 3,
    83, 8,
    83, 13,
    83, 15,
    83, 66,
    83, 68,
    83, 69,
    83, 70,
    83, 71,
    83, 72,
    83, 80,
    83, 82,
    83, 85,
    83, 87,
    83, 88,
    83, 90,
    85, 80,
    87, 3,
    87, 8,
    87, 13,
    87, 15,
    87, 66,
    87, 68,
    87, 69,
    87, 70,
    87, 71,
    87, 72,
    87, 80,
    87, 82,
    88, 13,
    88, 15,
    89, 68,
    89, 69,
    89, 70,
    89, 72,
    89, 80,
    89, 82,
    90, 3,
    90, 8,
    90, 13,
    90, 15,
    90, 66,
    90, 68,
    90, 69,
    90, 70,
    90, 71,
    90, 72,
    90, 80,
    90, 82,
    91, 68,
    91, 69,
    91, 70,
    91, 72,
    91, 80,
    91, 82,
    92, 43,
    92, 54
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -4, -10, -10, -11, -5, -6, -6, -6,
    -6, -2, -2, -6, -2, -6, -7, 1,
    -10, -10, -11, -5, -6, -6, -6, -6,
    -2, -2, -6, -2, -6, -7, 1, 2,
    2, 2, -16, -16, -16, -16, -21, -11,
    -11, -6, -1, -1, -1, -1, -12, -2,
    -8, -6, -9, -1, -2, -1, -5, -3,
    -5, 1, -3, -2, -5, -2, -3, -1,
    -2, -10, -10, -2, -3, -2, -2, -4,
    -2, 2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -22, -22, -16,
    -25, 2, -3, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, 2, -3, 2,
    -3, 2, -3, 2, -3, -2, -6, -3,
    -3, -3, -3, -2, -2, -2, -2, -2,
    -2, -3, -2, -2, -2, -4, -6, -4,
    -31, -31, 2, -6, -6, -6, -6, -26,
    -5, -16, -13, -22, -4, -12, -9, -12,
    2, -3, 2, -3, 2, -3, 2, -3,
    -10, -10, -2, -3, -2, -2, -4, -2,
    -30, -30, -13, -19, -3, -2, -1, -1,
    -1, -1, -1, -1, -1, 1, 1, 1,
    -4, -3, -2, -3, -7, -2, -4, -4,
    -20, -22, -20, -7, -3, -3, -22, -3,
    -3, -1, 2, 2, 1, 2, -11, -9,
    -9, -9, -9, -10, -10, -9, -10, -9,
    -7, -11, -9, -7, -5, -7, -7, -6,
    -2, 2, -21, -3, -21, -7, -1, -1,
    -1, -1, 2, -4, -4, -4, -4, -4,
    -4, -4, -3, -3, -1, -1, 2, 1,
    -12, -6, -12, -4, 1, 1, -3, -3,
    -3, -3, -3, -3, -3, -2, -2, 1,
    -4, -2, -2, -2, -2, 1, -2, -2,
    -2, -2, -2, -2, -2, -3, -3, -3,
    2, -5, -20, -5, -20, -9, -3, -3,
    -9, -3, -3, -1, 2, -9, 2, 2,
    1, 2, 2, -7, -6, -6, -6, -2,
    -6, -4, -4, -6, -4, -6, -4, -5,
    -2, -4, -2, -2, -2, -3, 2, 1,
    -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -3, -3, -3, -2, -2,
    -6, -6, -1, -1, -3, -3, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    2, 2, 2, 2, -2, -2, -2, -2,
    -2, 2, -10, -10, -2, -2, -2, -2,
    -2, -10, -10, -10, -10, -13, -13, -1,
    -2, -1, -1, -3, -3, -1, -1, -1,
    -1, 2, 2, -12, -12, -4, -2, -2,
    -2, 1, -2, -2, -2, 5, 2, 2,
    2, -2, 1, 1, -10, -10, -1, -1,
    -1, -1, 1, -1, -1, -1, -12, -12,
    -2, -2, -2, -2, -2, -2, 1, 1,
    -10, -10, -1, -1, -1, -1, 1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -2, -2
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 434,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
    .glyph_bitmap = gylph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t lv_font_roboto_12 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};

#endif /*#if LV_FONT_ROBOTO_12*/

