/*
 * Copyright (c) 2006 Luc Verhaegen (quirks list)
 * Copyright (c) 2007-2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 * Copyright 2010 Red Hat, Inc.
 *
 * DDC probing routines (drm_ddc_read & drm_do_probe_ddc_edid) originally from
 * FB layer.
 *   Copyright (C) 2006 Dennis Munsie <dmunsie@cecropia.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <linux/hdmi.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vga_switcheroo.h>

#include <drm/drm_displayid.h>
#include <drm/drm_drv.h>
#include <drm/drm_edid.h>
#include <drm/drm_encoder.h>
#include <drm/drm_print.h>
#include <drm/drm_scdc_helper.h>

#include "drm_crtc_internal.h"

#define version_greater(edid, maj, min)                                                                                \
    (((edid)->version > (maj)) || ((edid)->version == (maj) && (edid)->revision > (min)))

#define EDID_EST_TIMINGS 0x10
#define EDID_STD_TIMINGS 0x8
#define EDID_DETAILED_TIMINGS 0x4

/*
 * EDID blocks out in the wild have a variety of bugs, try to collect
 * them here (note that userspace may work around broken monitors first,
 * but fixes should make their way here so that the kernel "just works"
 * on as many displays as possible).
 */

/* First detailed mode wrong, use largest 60Hz mode */
#define EDID_QUIRK_PREFER_LARGE_60 (1 << 0)
/* Reported 135MHz pixel clock is too high, needs adjustment */
#define EDID_QUIRK_135_CLOCK_TOO_HIGH (1 << 1)
/* Prefer the largest mode at 75 Hz */
#define EDID_QUIRK_PREFER_LARGE_75 (1 << 2)
/* Detail timing is in cm not mm */
#define EDID_QUIRK_DETAILED_IN_CM (1 << 3)
/* Detailed timing descriptors have bogus size values, so just take the
 * maximum size and use that.
 */
#define EDID_QUIRK_DETAILED_USE_MAXIMUM_SIZE (1 << 4)
/* use +hsync +vsync for detailed mode */
#define EDID_QUIRK_DETAILED_SYNC_PP (1 << 6)
/* Force reduced-blanking timings for detailed modes */
#define EDID_QUIRK_FORCE_REDUCED_BLANKING (1 << 7)
/* Force 8bpc */
#define EDID_QUIRK_FORCE_8BPC (1 << 8)
/* Force 12bpc */
#define EDID_QUIRK_FORCE_12BPC (1 << 9)
/* Force 6bpc */
#define EDID_QUIRK_FORCE_6BPC (1 << 10)
/* Force 10bpc */
#define EDID_QUIRK_FORCE_10BPC (1 << 11)
/* Non desktop display (i.e. HMD) */
#define EDID_QUIRK_NON_DESKTOP (1 << 12)

struct detailed_mode_closure {
    struct drm_connector *connector;
    struct edid *edid;
    bool preferred;
    u32 quirks;
    int modes;
};

#define LEVEL_DMT 0
#define LEVEL_GTF 1
#define LEVEL_GTF2 2
#define LEVEL_CVT 3

static const struct edid_quirk {
    char vendor[4];
    int product_id;
    u32 quirks;
} edid_quirk_list[] = {
    /* Acer AL1706 */
    {"ACR", 44358, EDID_QUIRK_PREFER_LARGE_60},
    /* Acer F51 */
    {"API", 0x7602, EDID_QUIRK_PREFER_LARGE_60},

    /* AEO model 0 reports 8 bpc, but is a 6 bpc panel */
    {"AEO", 0, EDID_QUIRK_FORCE_6BPC},

    /* BOE model on HP Pavilion 15-n233sl reports 8 bpc, but is a 6 bpc panel */
    {"BOE", 0x78b, EDID_QUIRK_FORCE_6BPC},

    /* CPT panel of Asus UX303LA reports 8 bpc, but is a 6 bpc panel */
    {"CPT", 0x17df, EDID_QUIRK_FORCE_6BPC},

    /* SDC panel of Lenovo B50-80 reports 8 bpc, but is a 6 bpc panel */
    {"SDC", 0x3652, EDID_QUIRK_FORCE_6BPC},

    /* BOE model 0x0771 reports 8 bpc, but is a 6 bpc panel */
    {"BOE", 0x0771, EDID_QUIRK_FORCE_6BPC},

    /* Belinea 10 15 55 */
    {"MAX", 1516, EDID_QUIRK_PREFER_LARGE_60},
    {"MAX", 0x77e, EDID_QUIRK_PREFER_LARGE_60},

    /* Envision Peripherals, Inc. EN-7100e */
    {"EPI", 59264, EDID_QUIRK_135_CLOCK_TOO_HIGH},
    /* Envision EN2028 */
    {"EPI", 8232, EDID_QUIRK_PREFER_LARGE_60},

    /* Funai Electronics PM36B */
    {"FCM", 13600, EDID_QUIRK_PREFER_LARGE_75 | EDID_QUIRK_DETAILED_IN_CM},

    /* LGD panel of HP zBook 17 G2, eDP 10 bpc, but reports unknown bpc */
    {"LGD", 764, EDID_QUIRK_FORCE_10BPC},

    /* LG Philips LCD LP154W01-A5 */
    {"LPL", 0, EDID_QUIRK_DETAILED_USE_MAXIMUM_SIZE},
    {"LPL", 0x2a00, EDID_QUIRK_DETAILED_USE_MAXIMUM_SIZE},

    /* Samsung SyncMaster 205BW.  Note: irony */
    {"SAM", 541, EDID_QUIRK_DETAILED_SYNC_PP},
    /* Samsung SyncMaster 22[5-6]BW */
    {"SAM", 596, EDID_QUIRK_PREFER_LARGE_60},
    {"SAM", 638, EDID_QUIRK_PREFER_LARGE_60},

    /* Sony PVM-2541A does up to 12 bpc, but only reports max 8 bpc */
    {"SNY", 0x2541, EDID_QUIRK_FORCE_12BPC},

    /* ViewSonic VA2026w */
    {"VSC", 5020, EDID_QUIRK_FORCE_REDUCED_BLANKING},

    /* Medion MD 30217 PG */
    {"MED", 0x7b8, EDID_QUIRK_PREFER_LARGE_75},

    /* Lenovo G50 */
    {"SDC", 18514, EDID_QUIRK_FORCE_6BPC},

    /* Panel in Samsung NP700G7A-S01PL notebook reports 6bpc */
    {"SEC", 0xd033, EDID_QUIRK_FORCE_8BPC},

    /* Rotel RSX-1058 forwards sink's EDID but only does HDMI 1.1 */
    {"ETR", 13896, EDID_QUIRK_FORCE_8BPC},

    /* Valve Index Headset */
    {"VLV", 0x91a8, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b0, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b1, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b2, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b3, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b4, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b5, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b6, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b7, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b8, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91b9, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91ba, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91bb, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91bc, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91bd, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91be, EDID_QUIRK_NON_DESKTOP},
    {"VLV", 0x91bf, EDID_QUIRK_NON_DESKTOP},

    /* HTC Vive and Vive Pro VR Headsets */
    {"HVR", 0xaa01, EDID_QUIRK_NON_DESKTOP},
    {"HVR", 0xaa02, EDID_QUIRK_NON_DESKTOP},

    /* Oculus Rift DK1, DK2, CV1 and Rift S VR Headsets */
    {"OVR", 0x0001, EDID_QUIRK_NON_DESKTOP},
    {"OVR", 0x0003, EDID_QUIRK_NON_DESKTOP},
    {"OVR", 0x0004, EDID_QUIRK_NON_DESKTOP},
    {"OVR", 0x0012, EDID_QUIRK_NON_DESKTOP},

    /* Windows Mixed Reality Headsets */
    {"ACR", 0x7fce, EDID_QUIRK_NON_DESKTOP},
    {"HPN", 0x3515, EDID_QUIRK_NON_DESKTOP},
    {"LEN", 0x0408, EDID_QUIRK_NON_DESKTOP},
    {"LEN", 0xb800, EDID_QUIRK_NON_DESKTOP},
    {"FUJ", 0x1970, EDID_QUIRK_NON_DESKTOP},
    {"DEL", 0x7fce, EDID_QUIRK_NON_DESKTOP},
    {"SEC", 0x144a, EDID_QUIRK_NON_DESKTOP},
    {"AUS", 0xc102, EDID_QUIRK_NON_DESKTOP},

    /* Sony PlayStation VR Headset */
    {"SNY", 0x0704, EDID_QUIRK_NON_DESKTOP},

    /* Sensics VR Headsets */
    {"SEN", 0x1019, EDID_QUIRK_NON_DESKTOP},

    /* OSVR HDK and HDK2 VR Headsets */
    {"SVR", 0x1019, EDID_QUIRK_NON_DESKTOP},
};

/*
 * Autogenerated from the DMT spec.
 * This table is copied from xfree86/modes/xf86EdidModes.c.
 */
static const struct drm_display_mode drm_dmt_modes[] = {
    /* 0x01 - 640x350@85Hz */
    {DRM_MODE("640x350", DRM_MODE_TYPE_DRIVER, 31500, 640, 672, 736, 832, 0, 350, 382, 385, 445, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x02 - 640x400@85Hz */
    {DRM_MODE("640x400", DRM_MODE_TYPE_DRIVER, 31500, 640, 672, 736, 832, 0, 400, 401, 404, 445, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x03 - 720x400@85Hz */
    {DRM_MODE("720x400", DRM_MODE_TYPE_DRIVER, 35500, 720, 756, 828, 936, 0, 400, 401, 404, 446, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x04 - 640x480@60Hz */
    {DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 25175, 640, 656, 752, 800, 0, 480, 490, 492, 525, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x05 - 640x480@72Hz */
    {DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 31500, 640, 664, 704, 832, 0, 480, 489, 492, 520, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x06 - 640x480@75Hz */
    {DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 31500, 640, 656, 720, 840, 0, 480, 481, 484, 500, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x07 - 640x480@85Hz */
    {DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 36000, 640, 696, 752, 832, 0, 480, 481, 484, 509, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x08 - 800x600@56Hz */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 36000, 800, 824, 896, 1024, 0, 600, 601, 603, 625, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x09 - 800x600@60Hz */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 40000, 800, 840, 968, 1056, 0, 600, 601, 605, 628, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x0a - 800x600@72Hz */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 50000, 800, 856, 976, 1040, 0, 600, 637, 643, 666, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x0b - 800x600@75Hz */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 49500, 800, 816, 896, 1056, 0, 600, 601, 604, 625, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x0c - 800x600@85Hz */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 56250, 800, 832, 896, 1048, 0, 600, 601, 604, 631, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x0d - 800x600@120Hz RB */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 73250, 800, 848, 880, 960, 0, 600, 603, 607, 636, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x0e - 848x480@60Hz */
    {DRM_MODE("848x480", DRM_MODE_TYPE_DRIVER, 33750, 848, 864, 976, 1088, 0, 480, 486, 494, 517, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x0f - 1024x768@43Hz, interlace */
    {DRM_MODE("1024x768i", DRM_MODE_TYPE_DRIVER, 44900, 1024, 1032, 1208, 1264, 0, 768, 768, 776, 817, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC | DRM_MODE_FLAG_INTERLACE)},
    /* 0x10 - 1024x768@60Hz */
    {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 65000, 1024, 1048, 1184, 1344, 0, 768, 771, 777, 806, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x11 - 1024x768@70Hz */
    {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 75000, 1024, 1048, 1184, 1328, 0, 768, 771, 777, 806, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x12 - 1024x768@75Hz */
    {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 78750, 1024, 1040, 1136, 1312, 0, 768, 769, 772, 800, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x13 - 1024x768@85Hz */
    {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 94500, 1024, 1072, 1168, 1376, 0, 768, 769, 772, 808, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x14 - 1024x768@120Hz RB */
    {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 115500, 1024, 1072, 1104, 1184, 0, 768, 771, 775, 813, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x15 - 1152x864@75Hz */
    {DRM_MODE("1152x864", DRM_MODE_TYPE_DRIVER, 108000, 1152, 1216, 1344, 1600, 0, 864, 865, 868, 900, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x55 - 1280x720@60Hz */
    {DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 1390, 1430, 1650, 0, 720, 725, 730, 750, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x16 - 1280x768@60Hz RB */
    {DRM_MODE("1280x768", DRM_MODE_TYPE_DRIVER, 68250, 1280, 1328, 1360, 1440, 0, 768, 771, 778, 790, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x17 - 1280x768@60Hz */
    {DRM_MODE("1280x768", DRM_MODE_TYPE_DRIVER, 79500, 1280, 1344, 1472, 1664, 0, 768, 771, 778, 798, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x18 - 1280x768@75Hz */
    {DRM_MODE("1280x768", DRM_MODE_TYPE_DRIVER, 102250, 1280, 1360, 1488, 1696, 0, 768, 771, 778, 805, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x19 - 1280x768@85Hz */
    {DRM_MODE("1280x768", DRM_MODE_TYPE_DRIVER, 117500, 1280, 1360, 1496, 1712, 0, 768, 771, 778, 809, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x1a - 1280x768@120Hz RB */
    {DRM_MODE("1280x768", DRM_MODE_TYPE_DRIVER, 140250, 1280, 1328, 1360, 1440, 0, 768, 771, 778, 813, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x1b - 1280x800@60Hz RB */
    {DRM_MODE("1280x800", DRM_MODE_TYPE_DRIVER, 71000, 1280, 1328, 1360, 1440, 0, 800, 803, 809, 823, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x1c - 1280x800@60Hz */
    {DRM_MODE("1280x800", DRM_MODE_TYPE_DRIVER, 83500, 1280, 1352, 1480, 1680, 0, 800, 803, 809, 831, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x1d - 1280x800@75Hz */
    {DRM_MODE("1280x800", DRM_MODE_TYPE_DRIVER, 106500, 1280, 1360, 1488, 1696, 0, 800, 803, 809, 838, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x1e - 1280x800@85Hz */
    {DRM_MODE("1280x800", DRM_MODE_TYPE_DRIVER, 122500, 1280, 1360, 1496, 1712, 0, 800, 803, 809, 843, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x1f - 1280x800@120Hz RB */
    {DRM_MODE("1280x800", DRM_MODE_TYPE_DRIVER, 146250, 1280, 1328, 1360, 1440, 0, 800, 803, 809, 847, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x20 - 1280x960@60Hz */
    {DRM_MODE("1280x960", DRM_MODE_TYPE_DRIVER, 108000, 1280, 1376, 1488, 1800, 0, 960, 961, 964, 1000, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x21 - 1280x960@85Hz */
    {DRM_MODE("1280x960", DRM_MODE_TYPE_DRIVER, 148500, 1280, 1344, 1504, 1728, 0, 960, 961, 964, 1011, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x22 - 1280x960@120Hz RB */
    {DRM_MODE("1280x960", DRM_MODE_TYPE_DRIVER, 175500, 1280, 1328, 1360, 1440, 0, 960, 963, 967, 1017, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x23 - 1280x1024@60Hz */
    {DRM_MODE("1280x1024", DRM_MODE_TYPE_DRIVER, 108000, 1280, 1328, 1440, 1688, 0, 1024, 1025, 1028, 1066, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x24 - 1280x1024@75Hz */
    {DRM_MODE("1280x1024", DRM_MODE_TYPE_DRIVER, 135000, 1280, 1296, 1440, 1688, 0, 1024, 1025, 1028, 1066, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x25 - 1280x1024@85Hz */
    {DRM_MODE("1280x1024", DRM_MODE_TYPE_DRIVER, 157500, 1280, 1344, 1504, 1728, 0, 1024, 1025, 1028, 1072, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x26 - 1280x1024@120Hz RB */
    {DRM_MODE("1280x1024", DRM_MODE_TYPE_DRIVER, 187250, 1280, 1328, 1360, 1440, 0, 1024, 1027, 1034, 1084, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x27 - 1360x768@60Hz */
    {DRM_MODE("1360x768", DRM_MODE_TYPE_DRIVER, 85500, 1360, 1424, 1536, 1792, 0, 768, 771, 777, 795, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x28 - 1360x768@120Hz RB */
    {DRM_MODE("1360x768", DRM_MODE_TYPE_DRIVER, 148250, 1360, 1408, 1440, 1520, 0, 768, 771, 776, 813, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x51 - 1366x768@60Hz */
    {DRM_MODE("1366x768", DRM_MODE_TYPE_DRIVER, 85500, 1366, 1436, 1579, 1792, 0, 768, 771, 774, 798, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x56 - 1366x768@60Hz */
    {DRM_MODE("1366x768", DRM_MODE_TYPE_DRIVER, 72000, 1366, 1380, 1436, 1500, 0, 768, 769, 772, 800, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x29 - 1400x1050@60Hz RB */
    {DRM_MODE("1400x1050", DRM_MODE_TYPE_DRIVER, 101000, 1400, 1448, 1480, 1560, 0, 1050, 1053, 1057, 1080, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x2a - 1400x1050@60Hz */
    {DRM_MODE("1400x1050", DRM_MODE_TYPE_DRIVER, 121750, 1400, 1488, 1632, 1864, 0, 1050, 1053, 1057, 1089, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x2b - 1400x1050@75Hz */
    {DRM_MODE("1400x1050", DRM_MODE_TYPE_DRIVER, 156000, 1400, 1504, 1648, 1896, 0, 1050, 1053, 1057, 1099, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x2c - 1400x1050@85Hz */
    {DRM_MODE("1400x1050", DRM_MODE_TYPE_DRIVER, 179500, 1400, 1504, 1656, 1912, 0, 1050, 1053, 1057, 1105, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x2d - 1400x1050@120Hz RB */
    {DRM_MODE("1400x1050", DRM_MODE_TYPE_DRIVER, 208000, 1400, 1448, 1480, 1560, 0, 1050, 1053, 1057, 1112, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x2e - 1440x900@60Hz RB */
    {DRM_MODE("1440x900", DRM_MODE_TYPE_DRIVER, 88750, 1440, 1488, 1520, 1600, 0, 900, 903, 909, 926, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x2f - 1440x900@60Hz */
    {DRM_MODE("1440x900", DRM_MODE_TYPE_DRIVER, 106500, 1440, 1520, 1672, 1904, 0, 900, 903, 909, 934, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x30 - 1440x900@75Hz */
    {DRM_MODE("1440x900", DRM_MODE_TYPE_DRIVER, 136750, 1440, 1536, 1688, 1936, 0, 900, 903, 909, 942, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x31 - 1440x900@85Hz */
    {DRM_MODE("1440x900", DRM_MODE_TYPE_DRIVER, 157000, 1440, 1544, 1696, 1952, 0, 900, 903, 909, 948, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x32 - 1440x900@120Hz RB */
    {DRM_MODE("1440x900", DRM_MODE_TYPE_DRIVER, 182750, 1440, 1488, 1520, 1600, 0, 900, 903, 909, 953, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x53 - 1600x900@60Hz */
    {DRM_MODE("1600x900", DRM_MODE_TYPE_DRIVER, 108000, 1600, 1624, 1704, 1800, 0, 900, 901, 904, 1000, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x33 - 1600x1200@60Hz */
    {DRM_MODE("1600x1200", DRM_MODE_TYPE_DRIVER, 162000, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x34 - 1600x1200@65Hz */
    {DRM_MODE("1600x1200", DRM_MODE_TYPE_DRIVER, 175500, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x35 - 1600x1200@70Hz */
    {DRM_MODE("1600x1200", DRM_MODE_TYPE_DRIVER, 189000, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x36 - 1600x1200@75Hz */
    {DRM_MODE("1600x1200", DRM_MODE_TYPE_DRIVER, 202500, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x37 - 1600x1200@85Hz */
    {DRM_MODE("1600x1200", DRM_MODE_TYPE_DRIVER, 229500, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x38 - 1600x1200@120Hz RB */
    {DRM_MODE("1600x1200", DRM_MODE_TYPE_DRIVER, 268250, 1600, 1648, 1680, 1760, 0, 1200, 1203, 1207, 1271, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x39 - 1680x1050@60Hz RB */
    {DRM_MODE("1680x1050", DRM_MODE_TYPE_DRIVER, 119000, 1680, 1728, 1760, 1840, 0, 1050, 1053, 1059, 1080, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x3a - 1680x1050@60Hz */
    {DRM_MODE("1680x1050", DRM_MODE_TYPE_DRIVER, 146250, 1680, 1784, 1960, 2240, 0, 1050, 1053, 1059, 1089, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x3b - 1680x1050@75Hz */
    {DRM_MODE("1680x1050", DRM_MODE_TYPE_DRIVER, 187000, 1680, 1800, 1976, 2272, 0, 1050, 1053, 1059, 1099, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x3c - 1680x1050@85Hz */
    {DRM_MODE("1680x1050", DRM_MODE_TYPE_DRIVER, 214750, 1680, 1808, 1984, 2288, 0, 1050, 1053, 1059, 1105, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x3d - 1680x1050@120Hz RB */
    {DRM_MODE("1680x1050", DRM_MODE_TYPE_DRIVER, 245500, 1680, 1728, 1760, 1840, 0, 1050, 1053, 1059, 1112, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x3e - 1792x1344@60Hz */
    {DRM_MODE("1792x1344", DRM_MODE_TYPE_DRIVER, 204750, 1792, 1920, 2120, 2448, 0, 1344, 1345, 1348, 1394, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x3f - 1792x1344@75Hz */
    {DRM_MODE("1792x1344", DRM_MODE_TYPE_DRIVER, 261000, 1792, 1888, 2104, 2456, 0, 1344, 1345, 1348, 1417, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x40 - 1792x1344@120Hz RB */
    {DRM_MODE("1792x1344", DRM_MODE_TYPE_DRIVER, 333250, 1792, 1840, 1872, 1952, 0, 1344, 1347, 1351, 1423, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x41 - 1856x1392@60Hz */
    {DRM_MODE("1856x1392", DRM_MODE_TYPE_DRIVER, 218250, 1856, 1952, 2176, 2528, 0, 1392, 1393, 1396, 1439, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x42 - 1856x1392@75Hz */
    {DRM_MODE("1856x1392", DRM_MODE_TYPE_DRIVER, 288000, 1856, 1984, 2208, 2560, 0, 1392, 1393, 1396, 1500, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x43 - 1856x1392@120Hz RB */
    {DRM_MODE("1856x1392", DRM_MODE_TYPE_DRIVER, 356500, 1856, 1904, 1936, 2016, 0, 1392, 1395, 1399, 1474, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x52 - 1920x1080@60Hz */
    {DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x44 - 1920x1200@60Hz RB */
    {DRM_MODE("1920x1200", DRM_MODE_TYPE_DRIVER, 154000, 1920, 1968, 2000, 2080, 0, 1200, 1203, 1209, 1235, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x45 - 1920x1200@60Hz */
    {DRM_MODE("1920x1200", DRM_MODE_TYPE_DRIVER, 193250, 1920, 2056, 2256, 2592, 0, 1200, 1203, 1209, 1245, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x46 - 1920x1200@75Hz */
    {DRM_MODE("1920x1200", DRM_MODE_TYPE_DRIVER, 245250, 1920, 2056, 2264, 2608, 0, 1200, 1203, 1209, 1255, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x47 - 1920x1200@85Hz */
    {DRM_MODE("1920x1200", DRM_MODE_TYPE_DRIVER, 281250, 1920, 2064, 2272, 2624, 0, 1200, 1203, 1209, 1262, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x48 - 1920x1200@120Hz RB */
    {DRM_MODE("1920x1200", DRM_MODE_TYPE_DRIVER, 317000, 1920, 1968, 2000, 2080, 0, 1200, 1203, 1209, 1271, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x49 - 1920x1440@60Hz */
    {DRM_MODE("1920x1440", DRM_MODE_TYPE_DRIVER, 234000, 1920, 2048, 2256, 2600, 0, 1440, 1441, 1444, 1500, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x4a - 1920x1440@75Hz */
    {DRM_MODE("1920x1440", DRM_MODE_TYPE_DRIVER, 297000, 1920, 2064, 2288, 2640, 0, 1440, 1441, 1444, 1500, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x4b - 1920x1440@120Hz RB */
    {DRM_MODE("1920x1440", DRM_MODE_TYPE_DRIVER, 380500, 1920, 1968, 2000, 2080, 0, 1440, 1443, 1447, 1525, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x54 - 2048x1152@60Hz */
    {DRM_MODE("2048x1152", DRM_MODE_TYPE_DRIVER, 162000, 2048, 2074, 2154, 2250, 0, 1152, 1153, 1156, 1200, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x4c - 2560x1600@60Hz RB */
    {DRM_MODE("2560x1600", DRM_MODE_TYPE_DRIVER, 268500, 2560, 2608, 2640, 2720, 0, 1600, 1603, 1609, 1646, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x4d - 2560x1600@60Hz */
    {DRM_MODE("2560x1600", DRM_MODE_TYPE_DRIVER, 348500, 2560, 2752, 3032, 3504, 0, 1600, 1603, 1609, 1658, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x4e - 2560x1600@75Hz */
    {DRM_MODE("2560x1600", DRM_MODE_TYPE_DRIVER, 443250, 2560, 2768, 3048, 3536, 0, 1600, 1603, 1609, 1672, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x4f - 2560x1600@85Hz */
    {DRM_MODE("2560x1600", DRM_MODE_TYPE_DRIVER, 505250, 2560, 2768, 3048, 3536, 0, 1600, 1603, 1609, 1682, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)},
    /* 0x50 - 2560x1600@120Hz RB */
    {DRM_MODE("2560x1600", DRM_MODE_TYPE_DRIVER, 552750, 2560, 2608, 2640, 2720, 0, 1600, 1603, 1609, 1694, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x57 - 4096x2160@60Hz RB */
    {DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 556744, 4096, 4104, 4136, 4176, 0, 2160, 2208, 2216, 2222, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
    /* 0x58 - 4096x2160@59.94Hz RB */
    {DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 556188, 4096, 4104, 4136, 4176, 0, 2160, 2208, 2216, 2222, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC)},
};

/*
 * These more or less come from the DMT spec.  The 720x400 modes are
 * inferred from historical 80x25 practice.  The 640x480@67 and 832x624@75
 * modes are old-school Mac modes.  The EDID spec says the 1152x864@75 mode
 * should be 1152x870, again for the Mac, but instead we use the x864 DMT
 * mode.
 *
 * The DMT modes have been fact-checked; the rest are mild guesses.
 */
static const struct drm_display_mode edid_est_modes[] = {
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 40000, 800, 840, 968, 1056, 0, 600, 601, 605, 628, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)}, /* 800x600@60Hz */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 36000, 800, 824, 896, 1024, 0, 600, 601, 603, 625, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)}, /* 800x600@56Hz */
    {DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 31500, 640, 656, 720, 840, 0, 480, 481, 484, 500, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)}, /* 640x480@75Hz */
    {DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 31500, 640, 664, 704, 832, 0, 480, 489, 492, 520, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)}, /* 640x480@72Hz */
    {DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 30240, 640, 704, 768, 864, 0, 480, 483, 486, 525, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)}, /* 640x480@67Hz */
    {DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 25175, 640, 656, 752, 800, 0, 480, 490, 492, 525, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)}, /* 640x480@60Hz */
    {DRM_MODE("720x400", DRM_MODE_TYPE_DRIVER, 35500, 720, 738, 846, 900, 0, 400, 421, 423, 449, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)}, /* 720x400@88Hz */
    {DRM_MODE("720x400", DRM_MODE_TYPE_DRIVER, 28320, 720, 738, 846, 900, 0, 400, 412, 414, 449, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC)}, /* 720x400@70Hz */
    {DRM_MODE("1280x1024", DRM_MODE_TYPE_DRIVER, 135000, 1280, 1296, 1440, 1688, 0, 1024, 1025, 1028, 1066, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)}, /* 1280x1024@75Hz */
    {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 78750, 1024, 1040, 1136, 1312, 0, 768, 769, 772, 800, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)}, /* 1024x768@75Hz */
    {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 75000, 1024, 1048, 1184, 1328, 0, 768, 771, 777, 806, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)}, /* 1024x768@70Hz */
    {DRM_MODE("1024x768", DRM_MODE_TYPE_DRIVER, 65000, 1024, 1048, 1184, 1344, 0, 768, 771, 777, 806, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)}, /* 1024x768@60Hz */
    {DRM_MODE("1024x768i", DRM_MODE_TYPE_DRIVER, 44900, 1024, 1032, 1208, 1264, 0, 768, 768, 776, 817, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC | DRM_MODE_FLAG_INTERLACE)}, /* 1024x768@43Hz */
    {DRM_MODE("832x624", DRM_MODE_TYPE_DRIVER, 57284, 832, 864, 928, 1152, 0, 624, 625, 628, 667, 0,
              DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC)}, /* 832x624@75Hz */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 49500, 800, 816, 896, 1056, 0, 600, 601, 604, 625, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)}, /* 800x600@75Hz */
    {DRM_MODE("800x600", DRM_MODE_TYPE_DRIVER, 50000, 800, 856, 976, 1040, 0, 600, 637, 643, 666, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)}, /* 800x600@72Hz */
    {DRM_MODE("1152x864", DRM_MODE_TYPE_DRIVER, 108000, 1152, 1216, 1344, 1600, 0, 864, 865, 868, 900, 0,
              DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC)}, /* 1152x864@75Hz */
};

struct minimode {
    short w;
    short h;
    short r;
    short rb;
};

static const struct minimode est3_modes[] = {
    /* byte 6 */
    {640, 350, 85, 0},
    {640, 400, 85, 0},
    {720, 400, 85, 0},
    {640, 480, 85, 0},
    {848, 480, 60, 0},
    {800, 600, 85, 0},
    {1024, 768, 85, 0},
    {1152, 864, 75, 0},
    /* byte 7 */
    {1280, 768, 60, 1},
    {1280, 768, 60, 0},
    {1280, 768, 75, 0},
    {1280, 768, 85, 0},
    {1280, 960, 60, 0},
    {1280, 960, 85, 0},
    {1280, 1024, 60, 0},
    {1280, 1024, 85, 0},
    /* byte 8 */
    {1360, 768, 60, 0},
    {1440, 900, 60, 1},
    {1440, 900, 60, 0},
    {1440, 900, 75, 0},
    {1440, 900, 85, 0},
    {1400, 1050, 60, 1},
    {1400, 1050, 60, 0},
    {1400, 1050, 75, 0},
    /* byte 9 */
    {1400, 1050, 85, 0},
    {1680, 1050, 60, 1},
    {1680, 1050, 60, 0},
    {1680, 1050, 75, 0},
    {1680, 1050, 85, 0},
    {1600, 1200, 60, 0},
    {1600, 1200, 65, 0},
    {1600, 1200, 70, 0},
    /* byte 10 */
    {1600, 1200, 75, 0},
    {1600, 1200, 85, 0},
    {1792, 1344, 60, 0},
    {1792, 1344, 75, 0},
    {1856, 1392, 60, 0},
    {1856, 1392, 75, 0},
    {1920, 1200, 60, 1},
    {1920, 1200, 60, 0},
    /* byte 11 */
    {1920, 1200, 75, 0},
    {1920, 1200, 85, 0},
    {1920, 1440, 60, 0},
    {1920, 1440, 75, 0},
};

static const struct minimode extra_modes[] = {
    {1024, 576, 60, 0},  {1366, 768, 60, 0},  {1600, 900, 60, 0},  {1680, 945, 60, 0},
    {1920, 1080, 60, 0}, {2048, 1152, 60, 0}, {2048, 1536, 60, 0},
};

/*
 * From CEA/CTA-861 spec.
 *
 * Do not access directly, instead always use cea_mode_for_vic().
 */
static const struct drm_display_mode edid_cea_modes_1[] = {
    /* 1 - 640x480@60Hz 4:3 */
    {
        DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 25175, 640, 656, 752, 800, 0, 480, 490, 492, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 2 - 720x480@60Hz 4:3 */
    {
        DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 27000, 720, 736, 798, 858, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 3 - 720x480@60Hz 16:9 */
    {
        DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 27000, 720, 736, 798, 858, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 4 - 1280x720@60Hz 16:9 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 1390, 1430, 1650, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 5 - 1920x1080i@60Hz 16:9 */
    {
        DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1094, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 6 - 720(1440)x480i@60Hz 4:3 */
    {
        DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 13500, 720, 739, 801, 858, 0, 480, 488, 494, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 7 - 720(1440)x480i@60Hz 16:9 */
    {
        DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 13500, 720, 739, 801, 858, 0, 480, 488, 494, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 8 - 720(1440)x240@60Hz 4:3 */
    {
        DRM_MODE("720x240", DRM_MODE_TYPE_DRIVER, 13500, 720, 739, 801, 858, 0, 240, 244, 247, 262, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 9 - 720(1440)x240@60Hz 16:9 */
    {
        DRM_MODE("720x240", DRM_MODE_TYPE_DRIVER, 13500, 720, 739, 801, 858, 0, 240, 244, 247, 262, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 10 - 2880x480i@60Hz 4:3 */
    {
        DRM_MODE("2880x480i", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2956, 3204, 3432, 0, 480, 488, 494, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 11 - 2880x480i@60Hz 16:9 */
    {
        DRM_MODE("2880x480i", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2956, 3204, 3432, 0, 480, 488, 494, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 12 - 2880x240@60Hz 4:3 */
    {
        DRM_MODE("2880x240", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2956, 3204, 3432, 0, 240, 244, 247, 262, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 13 - 2880x240@60Hz 16:9 */
    {
        DRM_MODE("2880x240", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2956, 3204, 3432, 0, 240, 244, 247, 262, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 14 - 1440x480@60Hz 4:3 */
    {
        DRM_MODE("1440x480", DRM_MODE_TYPE_DRIVER, 54000, 1440, 1472, 1596, 1716, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 15 - 1440x480@60Hz 16:9 */
    {
        DRM_MODE("1440x480", DRM_MODE_TYPE_DRIVER, 54000, 1440, 1472, 1596, 1716, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 16 - 1920x1080@60Hz 16:9 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 17 - 720x576@50Hz 4:3 */
    {
        DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 27000, 720, 732, 796, 864, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 18 - 720x576@50Hz 16:9 */
    {
        DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 27000, 720, 732, 796, 864, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 19 - 1280x720@50Hz 16:9 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 1720, 1760, 1980, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 20 - 1920x1080i@50Hz 16:9 */
    {
        DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2448, 2492, 2640, 0, 1080, 1084, 1094, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 21 - 720(1440)x576i@50Hz 4:3 */
    {
        DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 13500, 720, 732, 795, 864, 0, 576, 580, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 22 - 720(1440)x576i@50Hz 16:9 */
    {
        DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 13500, 720, 732, 795, 864, 0, 576, 580, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 23 - 720(1440)x288@50Hz 4:3 */
    {
        DRM_MODE("720x288", DRM_MODE_TYPE_DRIVER, 13500, 720, 732, 795, 864, 0, 288, 290, 293, 312, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 24 - 720(1440)x288@50Hz 16:9 */
    {
        DRM_MODE("720x288", DRM_MODE_TYPE_DRIVER, 13500, 720, 732, 795, 864, 0, 288, 290, 293, 312, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 25 - 2880x576i@50Hz 4:3 */
    {
        DRM_MODE("2880x576i", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2928, 3180, 3456, 0, 576, 580, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 26 - 2880x576i@50Hz 16:9 */
    {
        DRM_MODE("2880x576i", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2928, 3180, 3456, 0, 576, 580, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 27 - 2880x288@50Hz 4:3 */
    {
        DRM_MODE("2880x288", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2928, 3180, 3456, 0, 288, 290, 293, 312, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 28 - 2880x288@50Hz 16:9 */
    {
        DRM_MODE("2880x288", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2928, 3180, 3456, 0, 288, 290, 293, 312, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 29 - 1440x576@50Hz 4:3 */
    {
        DRM_MODE("1440x576", DRM_MODE_TYPE_DRIVER, 54000, 1440, 1464, 1592, 1728, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 30 - 1440x576@50Hz 16:9 */
    {
        DRM_MODE("1440x576", DRM_MODE_TYPE_DRIVER, 54000, 1440, 1464, 1592, 1728, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 31 - 1920x1080@50Hz 16:9 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2448, 2492, 2640, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 32 - 1920x1080@24Hz 16:9 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2558, 2602, 2750, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 33 - 1920x1080@25Hz 16:9 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2448, 2492, 2640, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 34 - 1920x1080@30Hz 16:9 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 35 - 2880x480@60Hz 4:3 */
    {
        DRM_MODE("2880x480", DRM_MODE_TYPE_DRIVER, 108000, 2880, 2944, 3192, 3432, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 36 - 2880x480@60Hz 16:9 */
    {
        DRM_MODE("2880x480", DRM_MODE_TYPE_DRIVER, 108000, 2880, 2944, 3192, 3432, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 37 - 2880x576@50Hz 4:3 */
    {
        DRM_MODE("2880x576", DRM_MODE_TYPE_DRIVER, 108000, 2880, 2928, 3184, 3456, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 38 - 2880x576@50Hz 16:9 */
    {
        DRM_MODE("2880x576", DRM_MODE_TYPE_DRIVER, 108000, 2880, 2928, 3184, 3456, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 39 - 1920x1080i@50Hz 16:9 */
    {
        DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 72000, 1920, 1952, 2120, 2304, 0, 1080, 1126, 1136, 1250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 40 - 1920x1080i@100Hz 16:9 */
    {
        DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2448, 2492, 2640, 0, 1080, 1084, 1094, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 41 - 1280x720@100Hz 16:9 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 148500, 1280, 1720, 1760, 1980, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 42 - 720x576@100Hz 4:3 */
    {
        DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 54000, 720, 732, 796, 864, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 43 - 720x576@100Hz 16:9 */
    {
        DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 54000, 720, 732, 796, 864, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 44 - 720(1440)x576i@100Hz 4:3 */
    {
        DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 27000, 720, 732, 795, 864, 0, 576, 580, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 45 - 720(1440)x576i@100Hz 16:9 */
    {
        DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 27000, 720, 732, 795, 864, 0, 576, 580, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 46 - 1920x1080i@120Hz 16:9 */
    {
        DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1094, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC | DRM_MODE_FLAG_INTERLACE),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 47 - 1280x720@120Hz 16:9 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 148500, 1280, 1390, 1430, 1650, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 48 - 720x480@120Hz 4:3 */
    {
        DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 54000, 720, 736, 798, 858, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 49 - 720x480@120Hz 16:9 */
    {
        DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 54000, 720, 736, 798, 858, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 50 - 720(1440)x480i@120Hz 4:3 */
    {
        DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 27000, 720, 739, 801, 858, 0, 480, 488, 494, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 51 - 720(1440)x480i@120Hz 16:9 */
    {
        DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 27000, 720, 739, 801, 858, 0, 480, 488, 494, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 52 - 720x576@200Hz 4:3 */
    {
        DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 108000, 720, 732, 796, 864, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 53 - 720x576@200Hz 16:9 */
    {
        DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 108000, 720, 732, 796, 864, 0, 576, 581, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 54 - 720(1440)x576i@200Hz 4:3 */
    {
        DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 54000, 720, 732, 795, 864, 0, 576, 580, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 55 - 720(1440)x576i@200Hz 16:9 */
    {
        DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 54000, 720, 732, 795, 864, 0, 576, 580, 586, 625, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 56 - 720x480@240Hz 4:3 */
    {
        DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 108000, 720, 736, 798, 858, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 57 - 720x480@240Hz 16:9 */
    {
        DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 108000, 720, 736, 798, 858, 0, 480, 489, 495, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 58 - 720(1440)x480i@240Hz 4:3 */
    {
        DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 54000, 720, 739, 801, 858, 0, 480, 488, 494, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3,
    },
    /* 59 - 720(1440)x480i@240Hz 16:9 */
    {
        DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 54000, 720, 739, 801, 858, 0, 480, 488, 494, 525, 0,
                 DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC | DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 60 - 1280x720@24Hz 16:9 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 59400, 1280, 3040, 3080, 3300, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 61 - 1280x720@25Hz 16:9 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 3700, 3740, 3960, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 62 - 1280x720@30Hz 16:9 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 3040, 3080, 3300, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 63 - 1920x1080@120Hz 16:9 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 297000, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 64 - 1920x1080@100Hz 16:9 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 297000, 1920, 2448, 2492, 2640, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 65 - 1280x720@24Hz 64:27 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 59400, 1280, 3040, 3080, 3300, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 66 - 1280x720@25Hz 64:27 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 3700, 3740, 3960, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 67 - 1280x720@30Hz 64:27 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 3040, 3080, 3300, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 68 - 1280x720@50Hz 64:27 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 1720, 1760, 1980, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 69 - 1280x720@60Hz 64:27 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 1390, 1430, 1650, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 70 - 1280x720@100Hz 64:27 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 148500, 1280, 1720, 1760, 1980, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 71 - 1280x720@120Hz 64:27 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 148500, 1280, 1390, 1430, 1650, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 72 - 1920x1080@24Hz 64:27 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2558, 2602, 2750, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 73 - 1920x1080@25Hz 64:27 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2448, 2492, 2640, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 74 - 1920x1080@30Hz 64:27 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 75 - 1920x1080@50Hz 64:27 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2448, 2492, 2640, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 76 - 1920x1080@60Hz 64:27 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 77 - 1920x1080@100Hz 64:27 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 297000, 1920, 2448, 2492, 2640, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 78 - 1920x1080@120Hz 64:27 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 297000, 1920, 2008, 2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 79 - 1680x720@24Hz 64:27 */
    {
        DRM_MODE("1680x720", DRM_MODE_TYPE_DRIVER, 59400, 1680, 3040, 3080, 3300, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 80 - 1680x720@25Hz 64:27 */
    {
        DRM_MODE("1680x720", DRM_MODE_TYPE_DRIVER, 59400, 1680, 2908, 2948, 3168, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 81 - 1680x720@30Hz 64:27 */
    {
        DRM_MODE("1680x720", DRM_MODE_TYPE_DRIVER, 59400, 1680, 2380, 2420, 2640, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 82 - 1680x720@50Hz 64:27 */
    {
        DRM_MODE("1680x720", DRM_MODE_TYPE_DRIVER, 82500, 1680, 1940, 1980, 2200, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 83 - 1680x720@60Hz 64:27 */
    {
        DRM_MODE("1680x720", DRM_MODE_TYPE_DRIVER, 99000, 1680, 1940, 1980, 2200, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 84 - 1680x720@100Hz 64:27 */
    {
        DRM_MODE("1680x720", DRM_MODE_TYPE_DRIVER, 165000, 1680, 1740, 1780, 2000, 0, 720, 725, 730, 825, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 85 - 1680x720@120Hz 64:27 */
    {
        DRM_MODE("1680x720", DRM_MODE_TYPE_DRIVER, 198000, 1680, 1740, 1780, 2000, 0, 720, 725, 730, 825, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 86 - 2560x1080@24Hz 64:27 */
    {
        DRM_MODE("2560x1080", DRM_MODE_TYPE_DRIVER, 99000, 2560, 3558, 3602, 3750, 0, 1080, 1084, 1089, 1100, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 87 - 2560x1080@25Hz 64:27 */
    {
        DRM_MODE("2560x1080", DRM_MODE_TYPE_DRIVER, 90000, 2560, 3008, 3052, 3200, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 88 - 2560x1080@30Hz 64:27 */
    {
        DRM_MODE("2560x1080", DRM_MODE_TYPE_DRIVER, 118800, 2560, 3328, 3372, 3520, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 89 - 2560x1080@50Hz 64:27 */
    {
        DRM_MODE("2560x1080", DRM_MODE_TYPE_DRIVER, 185625, 2560, 3108, 3152, 3300, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 90 - 2560x1080@60Hz 64:27 */
    {
        DRM_MODE("2560x1080", DRM_MODE_TYPE_DRIVER, 198000, 2560, 2808, 2852, 3000, 0, 1080, 1084, 1089, 1100, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 91 - 2560x1080@100Hz 64:27 */
    {
        DRM_MODE("2560x1080", DRM_MODE_TYPE_DRIVER, 371250, 2560, 2778, 2822, 2970, 0, 1080, 1084, 1089, 1250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 92 - 2560x1080@120Hz 64:27 */
    {
        DRM_MODE("2560x1080", DRM_MODE_TYPE_DRIVER, 495000, 2560, 3108, 3152, 3300, 0, 1080, 1084, 1089, 1250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 93 - 3840x2160@24Hz 16:9 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 5116, 5204, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 94 - 3840x2160@25Hz 16:9 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 4896, 4984, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 95 - 3840x2160@30Hz 16:9 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 4016, 4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 96 - 3840x2160@50Hz 16:9 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 594000, 3840, 4896, 4984, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 97 - 3840x2160@60Hz 16:9 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 594000, 3840, 4016, 4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 98 - 4096x2160@24Hz 256:135 */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 297000, 4096, 5116, 5204, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
    /* 99 - 4096x2160@25Hz 256:135 */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 297000, 4096, 5064, 5152, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
    /* 100 - 4096x2160@30Hz 256:135 */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 297000, 4096, 4184, 4272, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
    /* 101 - 4096x2160@50Hz 256:135 */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 594000, 4096, 5064, 5152, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
    /* 102 - 4096x2160@60Hz 256:135 */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 594000, 4096, 4184, 4272, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
    /* 103 - 3840x2160@24Hz 64:27 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 5116, 5204, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 104 - 3840x2160@25Hz 64:27 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 4896, 4984, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 105 - 3840x2160@30Hz 64:27 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 4016, 4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 106 - 3840x2160@50Hz 64:27 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 594000, 3840, 4896, 4984, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 107 - 3840x2160@60Hz 64:27 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 594000, 3840, 4016, 4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 108 - 1280x720@48Hz 16:9 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 90000, 1280, 2240, 2280, 2500, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 109 - 1280x720@48Hz 64:27 */
    {
        DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 90000, 1280, 2240, 2280, 2500, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 110 - 1680x720@48Hz 64:27 */
    {
        DRM_MODE("1680x720", DRM_MODE_TYPE_DRIVER, 99000, 1680, 2490, 2530, 2750, 0, 720, 725, 730, 750, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 111 - 1920x1080@48Hz 16:9 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2558, 2602, 2750, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 112 - 1920x1080@48Hz 64:27 */
    {
        DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2558, 2602, 2750, 0, 1080, 1084, 1089, 1125, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 113 - 2560x1080@48Hz 64:27 */
    {
        DRM_MODE("2560x1080", DRM_MODE_TYPE_DRIVER, 198000, 2560, 3558, 3602, 3750, 0, 1080, 1084, 1089, 1100, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 114 - 3840x2160@48Hz 16:9 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 594000, 3840, 5116, 5204, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 115 - 4096x2160@48Hz 256:135 */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 594000, 4096, 5116, 5204, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
    /* 116 - 3840x2160@48Hz 64:27 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 594000, 3840, 5116, 5204, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 117 - 3840x2160@100Hz 16:9 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 1188000, 3840, 4896, 4984, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 118 - 3840x2160@120Hz 16:9 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 1188000, 3840, 4016, 4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 119 - 3840x2160@100Hz 64:27 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 1188000, 3840, 4896, 4984, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 120 - 3840x2160@120Hz 64:27 */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 1188000, 3840, 4016, 4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 121 - 5120x2160@24Hz 64:27 */
    {
        DRM_MODE("5120x2160", DRM_MODE_TYPE_DRIVER, 396000, 5120, 7116, 7204, 7500, 0, 2160, 2168, 2178, 2200, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 122 - 5120x2160@25Hz 64:27 */
    {
        DRM_MODE("5120x2160", DRM_MODE_TYPE_DRIVER, 396000, 5120, 6816, 6904, 7200, 0, 2160, 2168, 2178, 2200, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 123 - 5120x2160@30Hz 64:27 */
    {
        DRM_MODE("5120x2160", DRM_MODE_TYPE_DRIVER, 396000, 5120, 5784, 5872, 6000, 0, 2160, 2168, 2178, 2200, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 124 - 5120x2160@48Hz 64:27 */
    {
        DRM_MODE("5120x2160", DRM_MODE_TYPE_DRIVER, 742500, 5120, 5866, 5954, 6250, 0, 2160, 2168, 2178, 2475, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 125 - 5120x2160@50Hz 64:27 */
    {
        DRM_MODE("5120x2160", DRM_MODE_TYPE_DRIVER, 742500, 5120, 6216, 6304, 6600, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 126 - 5120x2160@60Hz 64:27 */
    {
        DRM_MODE("5120x2160", DRM_MODE_TYPE_DRIVER, 742500, 5120, 5284, 5372, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 127 - 5120x2160@100Hz 64:27 */
    {
        DRM_MODE("5120x2160", DRM_MODE_TYPE_DRIVER, 1485000, 5120, 6216, 6304, 6600, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
};

/*
 * From CEA/CTA-861 spec.
 *
 * Do not access directly, instead always use cea_mode_for_vic().
 */
static const struct drm_display_mode edid_cea_modes_193[] = {
    /* 193 - 5120x2160@120Hz 64:27 */
    {
        DRM_MODE("5120x2160", DRM_MODE_TYPE_DRIVER, 1485000, 5120, 5284, 5372, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 194 - 7680x4320@24Hz 16:9 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 1188000, 7680, 10232, 10408, 11000, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 195 - 7680x4320@25Hz 16:9 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 1188000, 7680, 10032, 10208, 10800, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 196 - 7680x4320@30Hz 16:9 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 1188000, 7680, 8232, 8408, 9000, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 197 - 7680x4320@48Hz 16:9 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 2376000, 7680, 10232, 10408, 11000, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 198 - 7680x4320@50Hz 16:9 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 2376000, 7680, 10032, 10208, 10800, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 199 - 7680x4320@60Hz 16:9 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 2376000, 7680, 8232, 8408, 9000, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 200 - 7680x4320@100Hz 16:9 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 4752000, 7680, 9792, 9968, 10560, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 201 - 7680x4320@120Hz 16:9 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 4752000, 7680, 8032, 8208, 8800, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 202 - 7680x4320@24Hz 64:27 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 1188000, 7680, 10232, 10408, 11000, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 203 - 7680x4320@25Hz 64:27 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 1188000, 7680, 10032, 10208, 10800, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 204 - 7680x4320@30Hz 64:27 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 1188000, 7680, 8232, 8408, 9000, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 205 - 7680x4320@48Hz 64:27 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 2376000, 7680, 10232, 10408, 11000, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 206 - 7680x4320@50Hz 64:27 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 2376000, 7680, 10032, 10208, 10800, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 207 - 7680x4320@60Hz 64:27 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 2376000, 7680, 8232, 8408, 9000, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 208 - 7680x4320@100Hz 64:27 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 4752000, 7680, 9792, 9968, 10560, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 209 - 7680x4320@120Hz 64:27 */
    {
        DRM_MODE("7680x4320", DRM_MODE_TYPE_DRIVER, 4752000, 7680, 8032, 8208, 8800, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 210 - 10240x4320@24Hz 64:27 */
    {
        DRM_MODE("10240x4320", DRM_MODE_TYPE_DRIVER, 1485000, 10240, 11732, 11908, 12500, 0, 4320, 4336, 4356, 4950, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 211 - 10240x4320@25Hz 64:27 */
    {
        DRM_MODE("10240x4320", DRM_MODE_TYPE_DRIVER, 1485000, 10240, 12732, 12908, 13500, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 212 - 10240x4320@30Hz 64:27 */
    {
        DRM_MODE("10240x4320", DRM_MODE_TYPE_DRIVER, 1485000, 10240, 10528, 10704, 11000, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 213 - 10240x4320@48Hz 64:27 */
    {
        DRM_MODE("10240x4320", DRM_MODE_TYPE_DRIVER, 2970000, 10240, 11732, 11908, 12500, 0, 4320, 4336, 4356, 4950, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 214 - 10240x4320@50Hz 64:27 */
    {
        DRM_MODE("10240x4320", DRM_MODE_TYPE_DRIVER, 2970000, 10240, 12732, 12908, 13500, 0, 4320, 4336, 4356, 4400, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 215 - 10240x4320@60Hz 64:27 */
    {
        DRM_MODE("10240x4320", DRM_MODE_TYPE_DRIVER, 2970000, 10240, 10528, 10704, 11000, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 216 - 10240x4320@100Hz 64:27 */
    {
        DRM_MODE("10240x4320", DRM_MODE_TYPE_DRIVER, 5940000, 10240, 12432, 12608, 13200, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 217 - 10240x4320@120Hz 64:27 */
    {
        DRM_MODE("10240x4320", DRM_MODE_TYPE_DRIVER, 5940000, 10240, 10528, 10704, 11000, 0, 4320, 4336, 4356, 4500, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_64_27,
    },
    /* 218 - 4096x2160@100Hz 256:135 */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 1188000, 4096, 4896, 4984, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
    /* 219 - 4096x2160@120Hz 256:135 */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 1188000, 4096, 4184, 4272, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
};

/*
 * HDMI 1.4 4k modes. Index using the VIC.
 */
static const struct drm_display_mode edid_4k_modes[] = {
    /* 0 - dummy, VICs start at 1 */
    {},
    /* 1 - 3840x2160@30Hz */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 4016, 4104, 4400, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 2 - 3840x2160@25Hz */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 4896, 4984, 5280, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 3 - 3840x2160@24Hz */
    {
        DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000, 3840, 5116, 5204, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9,
    },
    /* 4 - 4096x2160@24Hz (SMPTE) */
    {
        DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 297000, 4096, 5116, 5204, 5500, 0, 2160, 2168, 2178, 2250, 0,
                 DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
        .picture_aspect_ratio = HDMI_PICTURE_ASPECT_256_135,
    },
};

/*** DDC fetch and block validation ***/

static const u8 edid_header[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};

/**
 * drm_edid_header_is_valid - sanity check the header of the base EDID block
 * @raw_edid: pointer to raw base EDID block
 *
 * Sanity check the header of the base EDID block.
 *
 * Return: 8 if the header is perfect, down to 0 if it's totally wrong.
 */
int drm_edid_header_is_valid(const u8 *raw_edid)
{
    int i, score = 0;

    for (i = 0; i < sizeof(edid_header); i++) {
        if (raw_edid[i] == edid_header[i]) {
            score++;
        }
    }

    return score;
}
EXPORT_SYMBOL(drm_edid_header_is_valid);

static int edid_fixup __read_mostly = 0x6;
module_param_named(edid_fixup, edid_fixup, int, 0x100);
MODULE_PARM_DESC(edid_fixup, "Minimum number of valid EDID header bytes (0-8, default 6)");

static int validate_displayid(u8 *displayid, int length, int idx);

static int drm_edid_block_checksum(const u8 *raw_edid)
{
    int i;
    u8 csum = 0, crc = 0;

    for (i = 0; i < EDID_LENGTH - 1; i++) {
        csum += raw_edid[i];
    }

    crc = 0x100 - csum;

    return crc;
}

static bool drm_edid_block_checksum_diff(const u8 *raw_edid, u8 real_checksum)
{
    if (raw_edid[EDID_LENGTH - 1] != real_checksum) {
        return true;
    } else {
        return false;
    }
}

static bool drm_edid_is_zero(const u8 *in_edid, int length)
{
    if (memchr_inv(in_edid, 0, length)) {
        return false;
    }

    return true;
}

/**
 * drm_edid_are_equal - compare two edid blobs.
 * @edid1: pointer to first blob
 * @edid2: pointer to second blob
 * This helper can be used during probing to determine if
 * edid had changed.
 */
bool drm_edid_are_equal(const struct edid *edid1, const struct edid *edid2)
{
    int edid1_len, edid2_len;
    bool edid1_present = edid1 != NULL;
    bool edid2_present = edid2 != NULL;

    if (edid1_present != edid2_present) {
        return false;
    }

    if (edid1) {
        edid1_len = EDID_LENGTH * (1 + edid1->extensions);
        edid2_len = EDID_LENGTH * (1 + edid2->extensions);

        if (edid1_len != edid2_len) {
            return false;
        }

        if (memcmp(edid1, edid2, edid1_len)) {
            return false;
        }
    }

    return true;
}
EXPORT_SYMBOL(drm_edid_are_equal);

/**
 * drm_edid_block_valid - Sanity check the EDID block (base or extension)
 * @raw_edid: pointer to raw EDID block
 * @block: type of block to validate (0 for base, extension otherwise)
 * @print_bad_edid: if true, dump bad EDID blocks to the console
 * @edid_corrupt: if true, the header or checksum is invalid
 *
 * Validate a base or extension EDID block and optionally dump bad blocks to
 * the console.
 *
 * Return: True if the block is valid, false otherwise.
 */
bool drm_edid_block_valid(u8 *raw_edid, int block, bool print_bad_edid, bool *edid_corrupt)
{
    u8 csum;
    struct edid *edid = (struct edid *)raw_edid;

    if (WARN_ON(!raw_edid)) {
        return false;
    }

    if (edid_fixup > 0x8 || edid_fixup < 0) {
        edid_fixup = 0x6;
    }

    if (block == 0) {
        int score = drm_edid_header_is_valid(raw_edid);
        if (score == 0x8) {
            if (edid_corrupt) {
                *edid_corrupt = false;
            }
        } else if (score >= edid_fixup) {
            /* Displayport Link CTS Core 1.2 rev1.1 test 4.2.2.6
             * The corrupt flag needs to be set here otherwise, the
             * fix-up code here will correct the problem, the
             * checksum is correct and the test fails
             */
            if (edid_corrupt) {
                *edid_corrupt = true;
            }
            DRM_DEBUG("Fixing EDID header, your hardware may be failing\n");
            memcpy(raw_edid, edid_header, sizeof(edid_header));
        } else {
            if (edid_corrupt) {
                *edid_corrupt = true;
            }
            goto bad;
        }
    }

    csum = drm_edid_block_checksum(raw_edid);
    if (drm_edid_block_checksum_diff(raw_edid, csum)) {
        if (edid_corrupt) {
            *edid_corrupt = true;
        }

        /* allow CEA to slide through, switches mangle this */
        if (raw_edid[0] == CEA_EXT) {
            DRM_DEBUG("EDID checksum is invalid, remainder is %d\n", csum);
            DRM_DEBUG("Assuming a KVM switch modified the CEA block but left the original checksum\n");
        } else {
            if (print_bad_edid) {
                DRM_NOTE("EDID checksum is invalid, remainder is %d\n", csum);
            }

            goto bad;
        }
    }

    /* per-block-type checks */
    switch (raw_edid[0]) {
        case 0: /* base */
            if (edid->version != 1) {
                DRM_NOTE("EDID has major version %d, instead of 1\n", edid->version);
                goto bad;
            }

            if (edid->revision > 0x4) {
                DRM_DEBUG("EDID minor > 4, assuming backward compatibility\n");
            }
            break;

        default:
            break;
    }

    return true;

bad:
    if (print_bad_edid) {
        if (drm_edid_is_zero(raw_edid, EDID_LENGTH)) {
            pr_notice("EDID block is all zeroes\n");
        } else {
            pr_notice("Raw EDID:\n");
            print_hex_dump(KERN_NOTICE, " \t", DUMP_PREFIX_NONE, 0x10, 1, raw_edid, EDID_LENGTH, false);
        }
    }
    return false;
}
EXPORT_SYMBOL(drm_edid_block_valid);

/**
 * drm_edid_is_valid - sanity check EDID data
 * @edid: EDID data
 *
 * Sanity-check an entire EDID record (including extensions)
 *
 * Return: True if the EDID data is valid, false otherwise.
 */
bool drm_edid_is_valid(struct edid *edid)
{
    int i;
    u8 *raw = (u8 *)edid;

    if (!edid) {
        return false;
    }

    for (i = 0; i <= edid->extensions; i++) {
        if (!drm_edid_block_valid(raw + i * EDID_LENGTH, i, true, NULL)) {
            return false;
        }
    }

    return true;
}
EXPORT_SYMBOL(drm_edid_is_valid);

#define DDC_SEGMENT_ADDR 0x30
/**
 * drm_do_probe_ddc_edid() - get EDID information via I2C
 * @data: I2C device adapter
 * @buf: EDID data buffer to be filled
 * @block: 128 byte EDID block to start fetching from
 * @len: EDID data buffer length to fetch
 *
 * Try to fetch EDID information by calling I2C driver functions.
 *
 * Return: 0 on success or -1 on failure.
 */
static int drm_do_probe_ddc_edid(void *data, u8 *buf, unsigned int block, size_t len)
{
    struct i2c_adapter *adapter = data;
    unsigned char start = block * EDID_LENGTH;
    unsigned char segment = block >> 1;
    unsigned char xfers = segment ? 0x3 : 0x2;
    int ret, retries = 0x5;

    /*
     * The core I2C driver will automatically retry the transfer if the
     * adapter reports EAGAIN. However, we find that bit-banging transfers
     * are susceptible to errors under a heavily loaded machine and
     * generate spurious NAKs and timeouts. Retrying the transfer
     * of the individual block a few times seems to overcome this.
     */
    do {
        struct i2c_msg msgs[] = {
            {
                .addr = DDC_SEGMENT_ADDR,
                .flags = 0,
                .len = 1,
                .buf = &segment,
            },
            {
                .addr = DDC_ADDR,
                .flags = 0,
                .len = 1,
                .buf = &start,
            },
            {
                .addr = DDC_ADDR,
                .flags = I2C_M_RD,
                .len = len,
                .buf = buf,
            }
        };

        /*
         * Avoid sending the segment addr to not upset non-compliant
         * DDC monitors.
         */
        ret = i2c_transfer(adapter, &msgs[0x3 - xfers], xfers);
        if (ret == -ENXIO) {
            DRM_DEBUG_KMS("drm: skipping non-existent adapter %s\n", adapter->name);
            break;
        }
    } while (ret != xfers && --retries);

    return ret == xfers ? 0 : -1;
}

static void connector_bad_edid(struct drm_connector *connector, u8 *edid, int num_blocks)
{
    int i;
    u8 num_of_ext = edid[0x7e];

    /* Calculate real checksum for the last edid extension block data */
    connector->real_edid_checksum = drm_edid_block_checksum(edid + num_of_ext * EDID_LENGTH);

    if (connector->bad_edid_counter++ && !drm_debug_enabled(DRM_UT_KMS)) {
        return;
    }

    drm_warn(connector->dev, "%s: EDID is invalid:\n", connector->name);
    for (i = 0; i < num_blocks; i++) {
        u8 *block = edid + i * EDID_LENGTH;
        char prefix[0x14];

        if (drm_edid_is_zero(block, EDID_LENGTH)) {
            sprintf(prefix, "\t[%02x] ZERO ", i);
        } else if (!drm_edid_block_valid(block, i, false, NULL)) {
            sprintf(prefix, "\t[%02x] BAD  ", i);
        } else {
            sprintf(prefix, "\t[%02x] GOOD ", i);
        }

        print_hex_dump(KERN_WARNING, prefix, DUMP_PREFIX_NONE, 0x10, 1, block, EDID_LENGTH, false);
    }
}

/* Get override or firmware EDID */
static struct edid *drm_get_override_edid(struct drm_connector *connector)
{
    struct edid *override = NULL;

    if (connector->override_edid) {
        override = drm_edid_duplicate(connector->edid_blob_ptr->data);
    }

    if (!override) {
        override = drm_load_edid_firmware(connector);
    }

    return IS_ERR(override) ? NULL : override;
}

/**
 * drm_do_get_edid - get EDID data using a custom EDID block read function
 * @connector: connector we're probing
 * @get_edid_block: EDID block read function
 * @data: private data passed to the block read function
 *
 * When the I2C adapter connected to the DDC bus is hidden behind a device that
 * exposes a different interface to read EDID blocks this function can be used
 * to get EDID data using a custom block read function.
 *
 * As in the general case the DDC bus is accessible by the kernel at the I2C
 * level, drivers must make all reasonable efforts to expose it as an I2C
 * adapter and use drm_get_edid() instead of abusing this function.
 *
 * The EDID may be overridden using debugfs override_edid or firmare EDID
 * (drm_load_edid_firmware() and drm.edid_firmware parameter), in this priority
 * order. Having either of them bypasses actual EDID reads.
 *
 * Return: Pointer to valid EDID or NULL if we couldn't find any.
 */
struct edid *drm_do_get_edid(struct drm_connector *connector,
                             int (*get_edid_block)(void *data, u8 *buf, unsigned int block, size_t len), void *data)
{
    int i, j = 0, valid_extensions = 0;
    u8 *edid, *new;
    struct edid *override;

    override = drm_get_override_edid(connector);
    if (override) {
        return override;
    }

    if ((edid = kmalloc(EDID_LENGTH, GFP_KERNEL)) == NULL) {
        return NULL;
    }

    /* base block fetch */
    for (i = 0; i < 0x4; i++) {
        if (get_edid_block(data, edid, 0, EDID_LENGTH)) {
            goto out;
        }
        if (drm_edid_block_valid(edid, 0, false, &connector->edid_corrupt)) {
            break;
        }
        if (i == 0 && drm_edid_is_zero(edid, EDID_LENGTH)) {
            connector->null_edid_counter++;
            goto carp;
        }
    }
    if (i == 0x4) {
        goto carp;
    }

    /* if there's no extensions, we're done */
    valid_extensions = edid[0x7e];
    if (valid_extensions == 0) {
        return (struct edid *)edid;
    }

    new = krealloc(edid, (valid_extensions + 1) * EDID_LENGTH, GFP_KERNEL);
    if (!new) {
        goto out;
    }
    edid = new;

    for (j = 1; j <= edid[0x7e]; j++) {
        u8 *block = edid + j * EDID_LENGTH;

        for (i = 0; i < 0x4; i++) {
            if (get_edid_block(data, block, j, EDID_LENGTH)) {
                goto out;
            }
            if (drm_edid_block_valid(block, j, false, NULL)) {
                break;
            }
        }

        if (i == 0x4) {
            valid_extensions--;
        }
    }

    if (valid_extensions != edid[0x7e]) {
        u8 *base;

        connector_bad_edid(connector, edid, edid[0x7e] + 1);

        new = kmalloc_array(valid_extensions + 1, EDID_LENGTH, GFP_KERNEL);
        if (!new) {
            goto out;
        }

        base = new;
        for (i = 0; i <= edid[0x7e]; i++) {
            u8 *block = edid + i * EDID_LENGTH;

            if (!drm_edid_block_valid(block, i, false, NULL)) {
                continue;
            }

            memcpy(base, block, EDID_LENGTH);
            base += EDID_LENGTH;
        }

		new[EDID_LENGTH - 1] += new[0x7e] - valid_extensions;
		new[0x7e] = valid_extensions;

        kfree(edid);
        edid = new;
    }

    return (struct edid *)edid;

carp:
    connector_bad_edid(connector, edid, 1);
out:
    kfree(edid);
    return NULL;
}
EXPORT_SYMBOL_GPL(drm_do_get_edid);

/**
 * drm_probe_ddc() - probe DDC presence
 * @adapter: I2C adapter to probe
 *
 * Return: True on success, false on failure.
 */
bool drm_probe_ddc(struct i2c_adapter *adapter)
{
    unsigned char out;

    return (drm_do_probe_ddc_edid(adapter, &out, 0, 1) == 0);
}
EXPORT_SYMBOL(drm_probe_ddc);

/**
 * drm_get_edid - get EDID data, if available
 * @connector: connector we're probing
 * @adapter: I2C adapter to use for DDC
 *
 * Poke the given I2C channel to grab EDID data if possible.  If found,
 * attach it to the connector.
 *
 * Return: Pointer to valid EDID or NULL if we couldn't find any.
 */
struct edid *drm_get_edid(struct drm_connector *connector, struct i2c_adapter *adapter)
{
    struct edid *edid;

    if (connector->force == DRM_FORCE_OFF) {
        return NULL;
    }

    if (connector->force == DRM_FORCE_UNSPECIFIED && !drm_probe_ddc(adapter)) {
        return NULL;
    }

    edid = drm_do_get_edid(connector, drm_do_probe_ddc_edid, adapter);
    drm_connector_update_edid_property(connector, edid);
    return edid;
}
EXPORT_SYMBOL(drm_get_edid);

/**
 * drm_get_edid_switcheroo - get EDID data for a vga_switcheroo output
 * @connector: connector we're probing
 * @adapter: I2C adapter to use for DDC
 *
 * Wrapper around drm_get_edid() for laptops with dual GPUs using one set of
 * outputs. The wrapper adds the requisite vga_switcheroo calls to temporarily
 * switch DDC to the GPU which is retrieving EDID.
 *
 * Return: Pointer to valid EDID or %NULL if we couldn't find any.
 */
struct edid *drm_get_edid_switcheroo(struct drm_connector *connector, struct i2c_adapter *adapter)
{
    struct pci_dev *pdev = connector->dev->pdev;
    struct edid *edid;

    vga_switcheroo_lock_ddc(pdev);
    edid = drm_get_edid(connector, adapter);
    vga_switcheroo_unlock_ddc(pdev);

    return edid;
}
EXPORT_SYMBOL(drm_get_edid_switcheroo);

/**
 * drm_edid_duplicate - duplicate an EDID and the extensions
 * @edid: EDID to duplicate
 *
 * Return: Pointer to duplicated EDID or NULL on allocation failure.
 */
struct edid *drm_edid_duplicate(const struct edid *edid)
{
    return kmemdup(edid, (edid->extensions + 1) * EDID_LENGTH, GFP_KERNEL);
}
EXPORT_SYMBOL(drm_edid_duplicate);

/*** EDID parsing ***/

/**
 * edid_vendor - match a string against EDID's obfuscated vendor field
 * @edid: EDID to match
 * @vendor: vendor string
 *
 * Returns true if @vendor is in @edid, false otherwise
 */
static bool edid_vendor(const struct edid *edid, const char *vendor)
{
    char edid_vendor[3];

    edid_vendor[0] = ((edid->mfg_id[0] & 0x7c) >> 0x2) + '@';
    edid_vendor[1] = (((edid->mfg_id[0] & 0x3) << 0x3) | ((edid->mfg_id[1] & 0xe0) >> 0x5)) + '@';
    edid_vendor[0x2] = (edid->mfg_id[1] & 0x1f) + '@';

    return !strncmp(edid_vendor, vendor, 0x3);
}

/**
 * edid_get_quirks - return quirk flags for a given EDID
 * @edid: EDID to process
 *
 * This tells subsequent routines what fixes they need to apply.
 */
static u32 edid_get_quirks(const struct edid *edid)
{
    const struct edid_quirk *quirk;
    int i;

    for (i = 0; i < ARRAY_SIZE(edid_quirk_list); i++) {
        quirk = &edid_quirk_list[i];

        if (edid_vendor(edid, quirk->vendor) && (EDID_PRODUCT_ID(edid) == quirk->product_id)) {
            return quirk->quirks;
        }
    }

    return 0;
}

#define MODE_SIZE(m) ((m)->hdisplay * (m)->vdisplay)
#define MODE_REFRESH_DIFF(c, t) (abs((c) - (t)))

/**
 * edid_fixup_preferred - set preferred modes based on quirk list
 * @connector: has mode list to fix up
 * @quirks: quirks list
 *
 * Walk the mode list for @connector, clearing the preferred status
 * on existing modes and setting it anew for the right mode ala @quirks.
 */
static void edid_fixup_preferred(struct drm_connector *connector, u32 quirks)
{
    struct drm_display_mode *t, *cur_mode, *preferred_mode;
    int target_refresh = 0;
    int cur_vrefresh, preferred_vrefresh;

    if (list_empty(&connector->probed_modes)) {
        return;
    }

    if (quirks & EDID_QUIRK_PREFER_LARGE_60) {
        target_refresh = 0x40;
    }
    if (quirks & EDID_QUIRK_PREFER_LARGE_75) {
        target_refresh = 0x4b;
    }

    preferred_mode = list_first_entry(&connector->probed_modes, struct drm_display_mode, head);

    list_for_each_entry_safe(cur_mode, t, &connector->probed_modes, head)
    {
        cur_mode->type &= ~DRM_MODE_TYPE_PREFERRED;

        if (cur_mode == preferred_mode) {
            continue;
        }

        /* Largest mode is preferred */
        if (MODE_SIZE(cur_mode) > MODE_SIZE(preferred_mode)) {
            preferred_mode = cur_mode;
        }

        cur_vrefresh = drm_mode_vrefresh(cur_mode);
        preferred_vrefresh = drm_mode_vrefresh(preferred_mode);
        /* At a given size, try to get closest to target refresh */
        if ((MODE_SIZE(cur_mode) == MODE_SIZE(preferred_mode)) &&
            MODE_REFRESH_DIFF(cur_vrefresh, target_refresh) < MODE_REFRESH_DIFF(preferred_vrefresh, target_refresh)) {
            preferred_mode = cur_mode;
        }
    }

    preferred_mode->type |= DRM_MODE_TYPE_PREFERRED;
}

static bool mode_is_rb(const struct drm_display_mode *mode)
{
    return (mode->htotal - mode->hdisplay == 0xa0) && (mode->hsync_end - mode->hdisplay == 0x50) &&
           (mode->hsync_end - mode->hsync_start == 0x20) && (mode->vsync_start - mode->vdisplay == 0x3);
}

/*
 * drm_mode_find_dmt - Create a copy of a mode if present in DMT
 * @dev: Device to duplicate against
 * @hsize: Mode width
 * @vsize: Mode height
 * @fresh: Mode refresh rate
 * @rb: Mode reduced-blanking-ness
 *
 * Walk the DMT mode list looking for a match for the given parameters.
 *
 * Return: A newly allocated copy of the mode, or NULL if not found.
 */
struct drm_display_mode *drm_mode_find_dmt(struct drm_device *dev, int hsize, int vsize, int fresh, bool rb)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(drm_dmt_modes); i++) {
        const struct drm_display_mode *ptr = &drm_dmt_modes[i];

        if (hsize != ptr->hdisplay) {
            continue;
        }
        if (vsize != ptr->vdisplay) {
            continue;
        }
        if (fresh != drm_mode_vrefresh(ptr)) {
            continue;
        }
        if (rb != mode_is_rb(ptr)) {
            continue;
        }

        return drm_mode_duplicate(dev, ptr);
    }

    return NULL;
}
EXPORT_SYMBOL(drm_mode_find_dmt);

static bool is_display_descriptor(const u8 d[0x12], u8 tag)
{
    return d[0] == 0x00 && d[1] == 0x00 && d[0x2] == 0x00 && d[0x3] == tag;
}

static bool is_detailed_timing_descriptor(const u8 d[0x12])
{
    return d[0] != 0x00 || d[1] != 0x00;
}

typedef void detailed_cb(struct detailed_timing *timing, void *closure);

static void cea_for_each_detailed_block(u8 *ext, detailed_cb *cb, void *closure)
{
    int i, n;
    u8 d = ext[0x02];
    u8 *det_base = ext + d;

    if (d < 0x4 || d > 0x7f) {
        return;
    }

    n = (0x7f - d) / 0x12;
    for (i = 0; i < n; i++) {
        cb((struct detailed_timing *)(det_base + 0x12 * i), closure);
    }
}

static void vtb_for_each_detailed_block(u8 *ext, detailed_cb *cb, void *closure)
{
    unsigned int i, n = min((int)ext[0x02], 0x6);
    u8 *det_base = ext + 0x5;

    if (ext[0x01] != 1) {
        return; /* unknown version */
    }

    for (i = 0; i < n; i++) {
        cb((struct detailed_timing *)(det_base + 0x12 * i), closure);
    }
}

static void drm_for_each_detailed_block(u8 *raw_edid, detailed_cb *cb, void *closure)
{
    int i;
    struct edid *edid = (struct edid *)raw_edid;

    if (edid == NULL) {
        return;
    }

    for (i = 0; i < EDID_DETAILED_TIMINGS; i++) {
        cb(&(edid->detailed_timings[i]), closure);
    }

    for (i = 1; i <= raw_edid[0x7e]; i++) {
        u8 *ext = raw_edid + (i * EDID_LENGTH);

        switch (*ext) {
            case CEA_EXT:
                cea_for_each_detailed_block(ext, cb, closure);
                break;
            case VTB_EXT:
                vtb_for_each_detailed_block(ext, cb, closure);
                break;
            default:
                break;
        }
    }
}

static void is_rb(struct detailed_timing *t, void *data)
{
    u8 *r = (u8 *)t;

    if (!is_display_descriptor(r, EDID_DETAIL_MONITOR_RANGE)) {
        return;
    }

    if (r[0xf] & 0x10) {
        *(bool *)data = true;
    }
}

/* EDID 1.4 defines this explicitly.  For EDID 1.3, we guess, badly. */
static bool drm_monitor_supports_rb(struct edid *edid)
{
    if (edid->revision >= 0x4) {
        bool ret = false;

        drm_for_each_detailed_block((u8 *)edid, is_rb, &ret);
        return ret;
    }

    return ((edid->input & DRM_EDID_INPUT_DIGITAL) != 0);
}

static void find_gtf2(struct detailed_timing *t, void *data)
{
    u8 *r = (u8 *)t;

    if (!is_display_descriptor(r, EDID_DETAIL_MONITOR_RANGE)) {
        return;
    }

    if (r[0xa] == 0x02) {
        *(u8 **)data = r;
    }
}

/* Secondary GTF curve kicks in above some break frequency */
static int drm_gtf2_hbreak(struct edid *edid)
{
    u8 *r = NULL;

    drm_for_each_detailed_block((u8 *)edid, find_gtf2, &r);
    return r ? (r[0xc] * 0x2) : 0;
}

static int drm_gtf2_2c(struct edid *edid)
{
    u8 *r = NULL;

    drm_for_each_detailed_block((u8 *)edid, find_gtf2, &r);
    return r ? r[0xd] : 0;
}

static int drm_gtf2_m(struct edid *edid)
{
    u8 *r = NULL;

    drm_for_each_detailed_block((u8 *)edid, find_gtf2, &r);
    return r ? (r[0xf] << 0x8) + r[0xe] : 0;
}

static int drm_gtf2_k(struct edid *edid)
{
    u8 *r = NULL;

    drm_for_each_detailed_block((u8 *)edid, find_gtf2, &r);
    return r ? r[0x10] : 0;
}

static int drm_gtf2_2j(struct edid *edid)
{
    u8 *r = NULL;

    drm_for_each_detailed_block((u8 *)edid, find_gtf2, &r);
    return r ? r[0x11] : 0;
}

/**
 * standard_timing_level - get std. timing level(CVT/GTF/DMT)
 * @edid: EDID block to scan
 */
static int standard_timing_level(struct edid *edid)
{
    if (edid->revision >= 0x2) {
        if (edid->revision >= 0x4 && (edid->features & DRM_EDID_FEATURE_DEFAULT_GTF)) {
            return LEVEL_CVT;
        }
        if (drm_gtf2_hbreak(edid)) {
            return LEVEL_GTF2;
        }
        if (edid->features & DRM_EDID_FEATURE_DEFAULT_GTF) {
            return LEVEL_GTF;
        }
    }
    return LEVEL_DMT;
}

/*
 * 0 is reserved.  The spec says 0x01 fill for unused timings.  Some old
 * monitors fill with ascii space (0x20) instead.
 */
static int bad_std_timing(u8 a, u8 b)
{
    return (a == 0x00 && b == 0x00) || (a == 0x01 && b == 0x01) || (a == 0x20 && b == 0x20);
}

static int drm_mode_hsync(const struct drm_display_mode *mode)
{
    if (mode->htotal <= 0) {
        return 0;
    }

    return DIV_ROUND_CLOSEST(mode->clock, mode->htotal);
}

/**
 * drm_mode_std - convert standard mode info (width, height, refresh) into mode
 * @connector: connector of for the EDID block
 * @edid: EDID block to scan
 * @t: standard timing params
 *
 * Take the standard timing params (in this case width, aspect, and refresh)
 * and convert them into a real mode using CVT/GTF/DMT.
 */
static struct drm_display_mode *drm_mode_std(struct drm_connector *connector, struct edid *edid, struct std_timing *t)
{
    struct drm_device *dev = connector->dev;
    struct drm_display_mode *m, *mode = NULL;
    int hsize, vsize;
    int vrefresh_rate;
    unsigned aspect_ratio = (t->vfreq_aspect & EDID_TIMING_ASPECT_MASK) >> EDID_TIMING_ASPECT_SHIFT;
    unsigned vfreq = (t->vfreq_aspect & EDID_TIMING_VFREQ_MASK) >> EDID_TIMING_VFREQ_SHIFT;
    int timing_level = standard_timing_level(edid);

    if (bad_std_timing(t->hsize, t->vfreq_aspect)) {
        return NULL;
    }

    /* According to the EDID spec, the hdisplay = hsize * 8 + 248 */
    hsize = t->hsize * 0x8 + 0xf8;
    /* vrefresh_rate = vfreq + 60 */
    vrefresh_rate = vfreq + 0x3c;
    /* the vdisplay is calculated based on the aspect ratio */
    if (aspect_ratio == 0) {
        if (edid->revision < 0x3) {
            vsize = hsize;
        } else {
            vsize = (hsize * 0xa) / 0x10;
        }
    } else if (aspect_ratio == 0x1) {
        vsize = (hsize * 0x3) / 0x4;
    } else if (aspect_ratio == 0x2) {
        vsize = (hsize * 0x4) / 0x5;
    } else {
        vsize = (hsize * 0x9) / 0x10;
    }

    /* HDTV hack, part 1 */
    if (vrefresh_rate == 0x3c && ((hsize == 0x550 && vsize == 0x2fd) || (hsize == 0x558 && vsize == 0x301))) {
        hsize = 0x556;
        vsize = 0x300;
    }

    /*
     * If this connector already has a mode for this size and refresh
     * rate (because it came from detailed or CVT info), use that
     * instead.  This way we don't have to guess at interlace or
     * reduced blanking.
     */
    list_for_each_entry(m, &connector->probed_modes, head) if (m->hdisplay == hsize && m->vdisplay == vsize &&
                                                               drm_mode_vrefresh(m) == vrefresh_rate) return NULL;

    /* HDTV hack, part 2 */
    if (hsize == 0x556 && vsize == 0x300 && vrefresh_rate == 0x3c) {
        mode = drm_cvt_mode(dev, 0x556, 0x300, vrefresh_rate, 0, 0, false);
        if (!mode) {
            return NULL;
        }
        mode->hdisplay = 0x556;
        mode->hsync_start = mode->hsync_start - 1;
        mode->hsync_end = mode->hsync_end - 1;
        return mode;
    }

    /* check whether it can be found in default mode table */
    if (drm_monitor_supports_rb(edid)) {
        mode = drm_mode_find_dmt(dev, hsize, vsize, vrefresh_rate, true);
        if (mode) {
            return mode;
        }
    }
    mode = drm_mode_find_dmt(dev, hsize, vsize, vrefresh_rate, false);
    if (mode) {
        return mode;
    }

    /* okay, generate it */
    switch (timing_level) {
        case LEVEL_DMT:
            break;
        case LEVEL_GTF:
            mode = drm_gtf_mode(dev, hsize, vsize, vrefresh_rate, 0, 0);
            break;
        case LEVEL_GTF2:
            /*
             * This is potentially wrong if there's ever a monitor with
             * more than one ranges section, each claiming a different
             * secondary GTF curve.  Please don't do that.
             */
            mode = drm_gtf_mode(dev, hsize, vsize, vrefresh_rate, 0, 0);
            if (!mode) {
                return NULL;
            }
            if (drm_mode_hsync(mode) > drm_gtf2_hbreak(edid)) {
                drm_mode_destroy(dev, mode);
                mode = drm_gtf_mode_complex(dev, hsize, vsize, vrefresh_rate, 0, 0, drm_gtf2_m(edid), drm_gtf2_2c(edid),
                                            drm_gtf2_k(edid), drm_gtf2_2j(edid));
            }
            break;
        case LEVEL_CVT:
            mode = drm_cvt_mode(dev, hsize, vsize, vrefresh_rate, 0, 0, false);
            break;
        default:
            break;
    }
    return mode;
}

/*
 * EDID is delightfully ambiguous about how interlaced modes are to be
 * encoded.  Our internal representation is of frame height, but some
 * HDTV detailed timings are encoded as field height.
 *
 * The format list here is from CEA, in frame size.  Technically we
 * should be checking refresh rate too.  Whatever.
 */
static void drm_mode_do_interlace_quirk(struct drm_display_mode *mode, struct detailed_pixel_timing *pt)
{
    int i;
    static const struct {
        int w, h;
    } cea_interlaced[] = {
        {1920, 1080}, {720, 480}, {1440, 480}, {2880, 480}, {720, 576}, {1440, 576}, {2880, 576},
    };

    if (!(pt->misc & DRM_EDID_PT_INTERLACED)) {
        return;
    }

    for (i = 0; i < ARRAY_SIZE(cea_interlaced); i++) {
        if ((mode->hdisplay == cea_interlaced[i].w) && (mode->vdisplay == cea_interlaced[i].h / 0x2)) {
            mode->vdisplay *= 0x2;
            mode->vsync_start *= 0x2;
            mode->vsync_end *= 0x2;
            mode->vtotal *= 0x2;
            mode->vtotal |= 0x1;
        }
    }

    mode->flags |= DRM_MODE_FLAG_INTERLACE;
}

/**
 * drm_mode_detailed - create a new mode from an EDID detailed timing section
 * @dev: DRM device (needed to create new mode)
 * @edid: EDID block
 * @timing: EDID detailed timing info
 * @quirks: quirks to apply
 *
 * An EDID detailed timing block contains enough info for us to create and
 * return a new struct drm_display_mode.
 */
static struct drm_display_mode *drm_mode_detailed(struct drm_device *dev, struct edid *edid,
                                                  struct detailed_timing *timing, u32 quirks)
{
    struct drm_display_mode *mode;
    struct detailed_pixel_timing *pt = &timing->data.pixel_data;
    unsigned hactive = (pt->hactive_hblank_hi & 0xf0) << 0x4 | pt->hactive_lo;
    unsigned vactive = (pt->vactive_vblank_hi & 0xf0) << 0x4 | pt->vactive_lo;
    unsigned hblank = (pt->hactive_hblank_hi & 0xf) << 0x8 | pt->hblank_lo;
    unsigned vblank = (pt->vactive_vblank_hi & 0xf) << 0x8 | pt->vblank_lo;
    unsigned hsync_offset = (pt->hsync_vsync_offset_pulse_width_hi & 0xc0) << 0x2 | pt->hsync_offset_lo;
    unsigned hsync_pulse_width = (pt->hsync_vsync_offset_pulse_width_hi & 0x30) << 0x4 | pt->hsync_pulse_width_lo;
    unsigned vsync_offset = (pt->hsync_vsync_offset_pulse_width_hi & 0xc) << 0x2 | pt->vsync_offset_pulse_width_lo >> 4;
    unsigned vsync_pulse_width =
        (pt->hsync_vsync_offset_pulse_width_hi & 0x3) << 0x4 | (pt->vsync_offset_pulse_width_lo & 0xf);

    /* ignore tiny modes */
    if (hactive < 0x40 || vactive < 0x40) {
        return NULL;
    }

    if (pt->misc & DRM_EDID_PT_STEREO) {
        DRM_DEBUG_KMS("stereo mode not supported\n");
        return NULL;
    }
    if (!(pt->misc & DRM_EDID_PT_SEPARATE_SYNC)) {
        DRM_DEBUG_KMS("composite sync not supported\n");
    }

    /* it is incorrect if hsync/vsync width is zero */
    if (!hsync_pulse_width || !vsync_pulse_width) {
        DRM_DEBUG_KMS("Incorrect Detailed timing. "
                      "Wrong Hsync/Vsync pulse width\n");
        return NULL;
    }

    if (quirks & EDID_QUIRK_FORCE_REDUCED_BLANKING) {
        mode = drm_cvt_mode(dev, hactive, vactive, 0x3c, true, false, false);
        if (!mode) {
            return NULL;
        }

        goto set_size;
    }

    mode = drm_mode_create(dev);
    if (!mode) {
        return NULL;
    }

    if (quirks & EDID_QUIRK_135_CLOCK_TOO_HIGH) {
        timing->pixel_clock = cpu_to_le16(0x440);
    }

    mode->clock = le16_to_cpu(timing->pixel_clock) * 0xa;

    mode->hdisplay = hactive;
    mode->hsync_start = mode->hdisplay + hsync_offset;
    mode->hsync_end = mode->hsync_start + hsync_pulse_width;
    mode->htotal = mode->hdisplay + hblank;

    mode->vdisplay = vactive;
    mode->vsync_start = mode->vdisplay + vsync_offset;
    mode->vsync_end = mode->vsync_start + vsync_pulse_width;
    mode->vtotal = mode->vdisplay + vblank;

    /* Some EDIDs have bogus h/vtotal values */
    if (mode->hsync_end > mode->htotal) {
        mode->htotal = mode->hsync_end + 1;
    }
    if (mode->vsync_end > mode->vtotal) {
        mode->vtotal = mode->vsync_end + 1;
    }

    drm_mode_do_interlace_quirk(mode, pt);

    if (quirks & EDID_QUIRK_DETAILED_SYNC_PP) {
        pt->misc |= DRM_EDID_PT_HSYNC_POSITIVE | DRM_EDID_PT_VSYNC_POSITIVE;
    }

    mode->flags |= (pt->misc & DRM_EDID_PT_HSYNC_POSITIVE) ? DRM_MODE_FLAG_PHSYNC : DRM_MODE_FLAG_NHSYNC;
    mode->flags |= (pt->misc & DRM_EDID_PT_VSYNC_POSITIVE) ? DRM_MODE_FLAG_PVSYNC : DRM_MODE_FLAG_NVSYNC;

set_size:
    mode->width_mm = pt->width_mm_lo | (pt->width_height_mm_hi & 0xf0) << 0x4;
    mode->height_mm = pt->height_mm_lo | (pt->width_height_mm_hi & 0xf) << 0x8;

    if (quirks & EDID_QUIRK_DETAILED_IN_CM) {
        mode->width_mm *= 0xa;
        mode->height_mm *= 0xa;
    }

    if (quirks & EDID_QUIRK_DETAILED_USE_MAXIMUM_SIZE) {
        mode->width_mm = edid->width_cm * 0xa;
        mode->height_mm = edid->height_cm * 0xa;
    }

    mode->type = DRM_MODE_TYPE_DRIVER;
    drm_mode_set_name(mode);

    return mode;
}

static bool mode_in_hsync_range(const struct drm_display_mode *mode, struct edid *edid, u8 *t)
{
    int hsync, hmin, hmax;

    hmin = t[0x7];
    if (edid->revision >= 0x4) {
        hmin += ((t[0x4] & 0x04) ? 0xff : 0);
    }
    hmax = t[0x8];
    if (edid->revision >= 0x4) {
        hmax += ((t[0x4] & 0x08) ? 0xff : 0);
    }
    hsync = drm_mode_hsync(mode);

    return (hsync <= hmax && hsync >= hmin);
}

static bool mode_in_vsync_range(const struct drm_display_mode *mode, struct edid *edid, u8 *t)
{
    int vsync, vmin, vmax;

    vmin = t[0x5];
    if (edid->revision >= 0x4) {
        vmin += ((t[0x4] & 0x01) ? 0xff : 0);
    }
    vmax = t[0x6];
    if (edid->revision >= 0x4) {
        vmax += ((t[0x4] & 0x02) ? 0xff : 0);
    }
    vsync = drm_mode_vrefresh(mode);

    return (vsync <= vmax && vsync >= vmin);
}

static u32 range_pixel_clock(struct edid *edid, u8 *t)
{
    /* unspecified */
    if (t[0x9] == 0x0 || t[0x9] == 0xff) {
        return 0x0;
    }

    /* 1.4 with CVT support gives us real precision, yay */
    if (edid->revision >= 0x4 && t[0xa] == 0x04) {
        return (t[0x9] * 0x2710) - ((t[0xc] >> 0x2) * 0xfa);
    }

    /* 1.3 is pathetic, so fuzz up a bit */
    return t[0x9] * 0x2710 + 0x1389;
}

static bool mode_in_range(const struct drm_display_mode *mode, struct edid *edid, struct detailed_timing *timing)
{
    u32 max_clock;
    u8 *t = (u8 *)timing;

    if (!mode_in_hsync_range(mode, edid, t)) {
        return false;
    }

    if (!mode_in_vsync_range(mode, edid, t)) {
        return false;
    }

    if ((max_clock = range_pixel_clock(edid, t))) {
        if (mode->clock > max_clock) {
            return false;
        }
    }

    /* 1.4 max horizontal check */
    if (edid->revision >= 0x4 && t[0xa] == 0x04) {
        if (t[0xd] && mode->hdisplay > 0x8 * (t[0xd] + (0x100 * (t[0xc] & 0x3)))) {
            return false;
        }
    }

    if (mode_is_rb(mode) && !drm_monitor_supports_rb(edid)) {
        return false;
    }

    return true;
}

static bool valid_inferred_mode(const struct drm_connector *connector, const struct drm_display_mode *mode)
{
    const struct drm_display_mode *m;
    bool ok = false;

    list_for_each_entry(m, &connector->probed_modes, head)
    {
        if (mode->hdisplay == m->hdisplay && mode->vdisplay == m->vdisplay &&
            drm_mode_vrefresh(mode) == drm_mode_vrefresh(m)) {
            return false; /* duplicated */
        }
        if (mode->hdisplay <= m->hdisplay && mode->vdisplay <= m->vdisplay) {
            ok = true;
        }
    }
    return ok;
}

static int drm_dmt_modes_for_range(struct drm_connector *connector, struct edid *edid, struct detailed_timing *timing)
{
    int i, modes = 0;
    struct drm_display_mode *newmode;
    struct drm_device *dev = connector->dev;

    for (i = 0; i < ARRAY_SIZE(drm_dmt_modes); i++) {
        if (mode_in_range(drm_dmt_modes + i, edid, timing) && valid_inferred_mode(connector, drm_dmt_modes + i)) {
            newmode = drm_mode_duplicate(dev, &drm_dmt_modes[i]);
            if (newmode) {
                drm_mode_probed_add(connector, newmode);
                modes++;
            }
        }
    }

    return modes;
}

/* fix up 1366x768 mode from 1368x768;
 * GFT/CVT can't express 1366 width which isn't dividable by 8
 */
void drm_mode_fixup_1366x768(struct drm_display_mode *mode)
{
    if (mode->hdisplay == 0x558 && mode->vdisplay == 0x300) {
        mode->hdisplay = 0x556;
        mode->hsync_start--;
        mode->hsync_end--;
        drm_mode_set_name(mode);
    }
}

static int drm_gtf_modes_for_range(struct drm_connector *connector, struct edid *edid, struct detailed_timing *timing)
{
    int i, modes = 0;
    struct drm_display_mode *newmode;
    struct drm_device *dev = connector->dev;

    for (i = 0; i < ARRAY_SIZE(extra_modes); i++) {
        const struct minimode *m = &extra_modes[i];

        newmode = drm_gtf_mode(dev, m->w, m->h, m->r, 0, 0);
        if (!newmode) {
            return modes;
        }

        drm_mode_fixup_1366x768(newmode);
        if (!mode_in_range(newmode, edid, timing) || !valid_inferred_mode(connector, newmode)) {
            drm_mode_destroy(dev, newmode);
            continue;
        }

        drm_mode_probed_add(connector, newmode);
        modes++;
    }

    return modes;
}

static int drm_cvt_modes_for_range(struct drm_connector *connector, struct edid *edid, struct detailed_timing *timing)
{
    int i, modes = 0;
    struct drm_display_mode *newmode;
    struct drm_device *dev = connector->dev;
    bool rb = drm_monitor_supports_rb(edid);

    for (i = 0; i < ARRAY_SIZE(extra_modes); i++) {
        const struct minimode *m = &extra_modes[i];

        newmode = drm_cvt_mode(dev, m->w, m->h, m->r, rb, 0, 0);
        if (!newmode) {
            return modes;
        }

        drm_mode_fixup_1366x768(newmode);
        if (!mode_in_range(newmode, edid, timing) || !valid_inferred_mode(connector, newmode)) {
            drm_mode_destroy(dev, newmode);
            continue;
        }

        drm_mode_probed_add(connector, newmode);
        modes++;
    }

    return modes;
}

static void do_inferred_modes(struct detailed_timing *timing, void *c)
{
    struct detailed_mode_closure *closure = c;
    struct detailed_non_pixel *data = &timing->data.other_data;
    struct detailed_data_monitor_range *range = &data->data.range;

    if (!is_display_descriptor((const u8 *)timing, EDID_DETAIL_MONITOR_RANGE)) {
        return;
    }

    closure->modes += drm_dmt_modes_for_range(closure->connector, closure->edid, timing);

    if (!version_greater(closure->edid, 1, 1)) {
        return; /* GTF not defined yet */
    }

    switch (range->flags) {
        case 0x02: /* secondary gtf, XXX could do more */
        case 0x00: /* default gtf */
            closure->modes += drm_gtf_modes_for_range(closure->connector, closure->edid, timing);
            break;
        case 0x04: /* cvt, only in 1.4+ */
            if (!version_greater(closure->edid, 1, 0x3)) {
                break;
            }

            closure->modes += drm_cvt_modes_for_range(closure->connector, closure->edid, timing);
            break;
        case 0x01: /* just the ranges, no formula */
        default:
            break;
    }
}

static int add_inferred_modes(struct drm_connector *connector, struct edid *edid)
{
    struct detailed_mode_closure closure = {
        .connector = connector,
        .edid = edid,
    };

    if (version_greater(edid, 1, 0)) {
        drm_for_each_detailed_block((u8 *)edid, do_inferred_modes, &closure);
    }

    return closure.modes;
}

static int drm_est3_modes(struct drm_connector *connector, struct detailed_timing *timing)
{
    int i, j, m, modes = 0x0;
    struct drm_display_mode *mode;
    u8 *est = ((u8 *)timing) + 0x6;

    for (i = 0x0; i < 0x6; i++) {
        for (j = 0x7; j >= 0x0; j--) {
            m = (i * 0x8) + (0x7 - j);
            if (m >= ARRAY_SIZE(est3_modes)) {
                break;
            }
            if (est[i] & (0x1 << j)) {
                mode = drm_mode_find_dmt(connector->dev, est3_modes[m].w, est3_modes[m].h, est3_modes[m].r,
                                         est3_modes[m].rb);
                if (mode) {
                    drm_mode_probed_add(connector, mode);
                    modes++;
                }
            }
        }
    }

    return modes;
}

static void do_established_modes(struct detailed_timing *timing, void *c)
{
    struct detailed_mode_closure *closure = c;

    if (!is_display_descriptor((const u8 *)timing, EDID_DETAIL_EST_TIMINGS)) {
        return;
    }

    closure->modes += drm_est3_modes(closure->connector, timing);
}

/**
 * add_established_modes - get est. modes from EDID and add them
 * @connector: connector to add mode(s) to
 * @edid: EDID block to scan
 *
 * Each EDID block contains a bitmap of the supported "established modes" list
 * (defined above).  Tease them out and add them to the global modes list.
 */
static int add_established_modes(struct drm_connector *connector, struct edid *edid)
{
    struct drm_device *dev = connector->dev;
    unsigned long est_bits = edid->established_timings.t1 | (edid->established_timings.t2 << 0x8) |
                             ((edid->established_timings.mfg_rsvd & 0x80) << 0x9);
    int i, modes = 0x0;
    struct detailed_mode_closure closure = {
        .connector = connector,
        .edid = edid,
    };

    for (i = 0x0; i <= EDID_EST_TIMINGS; i++) {
        if (est_bits & (0x1 << i)) {
            struct drm_display_mode *newmode;

            newmode = drm_mode_duplicate(dev, &edid_est_modes[i]);
            if (newmode) {
                drm_mode_probed_add(connector, newmode);
                modes++;
            }
        }
    }

    if (version_greater(edid, 0x1, 0x0)) {
        drm_for_each_detailed_block((u8 *)edid, do_established_modes, &closure);
    }

    return modes + closure.modes;
}

static void do_standard_modes(struct detailed_timing *timing, void *c)
{
    struct detailed_mode_closure *closure = c;
    struct detailed_non_pixel *data = &timing->data.other_data;
    struct drm_connector *connector = closure->connector;
    struct edid *edid = closure->edid;
    int i;

    if (!is_display_descriptor((const u8 *)timing, EDID_DETAIL_STD_MODES)) {
        return;
    }

    for (i = 0; i < 0x6; i++) {
        struct std_timing *std = &data->data.timings[i];
        struct drm_display_mode *newmode;

        newmode = drm_mode_std(connector, edid, std);
        if (newmode) {
            drm_mode_probed_add(connector, newmode);
            closure->modes++;
        }
    }
}

/**
 * add_standard_modes - get std. modes from EDID and add them
 * @connector: connector to add mode(s) to
 * @edid: EDID block to scan
 *
 * Standard modes can be calculated using the appropriate standard (DMT,
 * GTF or CVT. Grab them from @edid and add them to the list.
 */
static int add_standard_modes(struct drm_connector *connector, struct edid *edid)
{
    int i, modes = 0;
    struct detailed_mode_closure closure = {
        .connector = connector,
        .edid = edid,
    };

    for (i = 0; i < EDID_STD_TIMINGS; i++) {
        struct drm_display_mode *newmode;

        newmode = drm_mode_std(connector, edid, &edid->standard_timings[i]);
        if (newmode) {
            drm_mode_probed_add(connector, newmode);
            modes++;
        }
    }

    if (version_greater(edid, 1, 0)) {
        drm_for_each_detailed_block((u8 *)edid, do_standard_modes, &closure);
    }

    /* XXX should also look for standard codes in VTB blocks */

    return modes + closure.modes;
}

static int drm_cvt_modes(struct drm_connector *connector, struct detailed_timing *timing)
{
    int i, j, modes = 0;
    struct drm_display_mode *newmode;
    struct drm_device *dev = connector->dev;
    struct cvt_timing *cvt;
    const int rates[] = {60, 85, 75, 60, 50};
    const u8 empty[3] = {0, 0, 0};

    for (i = 0; i < 0x4; i++) {
        int width, height;

        cvt = &(timing->data.other_data.data.cvt[i]);

        if (!memcmp(cvt->code, empty, 0x3)) {
            continue;
        }

        height = (cvt->code[0] + ((cvt->code[1] & 0xf0) << 0x4) + 1) * 0x2;
        switch (cvt->code[1] & 0x0c) {
            /* default - because compiler doesn't see that we've enumerated all cases */
            default:
            case 0x00:
                width = height * 0x4 / 0x3;
                break;
            case 0x04:
                width = height * 0x10 / 0x9;
                break;
            case 0x08:
                width = height * 0x10 / 0xa;
                break;
            case 0x0c:
                width = height * 0xf / 0x9;
                break;
        }

        for (j = 1; j < 0x5; j++) {
            if (cvt->code[0x2] & (1 << j)) {
                newmode = drm_cvt_mode(dev, width, height, rates[j], j == 0, false, false);
                if (newmode) {
                    drm_mode_probed_add(connector, newmode);
                    modes++;
                }
            }
        }
    }

    return modes;
}

static void do_cvt_mode(struct detailed_timing *timing, void *c)
{
    struct detailed_mode_closure *closure = c;

    if (!is_display_descriptor((const u8 *)timing, EDID_DETAIL_CVT_3BYTE)) {
        return;
    }

    closure->modes += drm_cvt_modes(closure->connector, timing);
}

static int add_cvt_modes(struct drm_connector *connector, struct edid *edid)
{
    struct detailed_mode_closure closure = {
        .connector = connector,
        .edid = edid,
    };

    if (version_greater(edid, 1, 0x2)) {
        drm_for_each_detailed_block((u8 *)edid, do_cvt_mode, &closure);
    }

    /* XXX should also look for CVT codes in VTB blocks */

    return closure.modes;
}

static void fixup_detailed_cea_mode_clock(struct drm_display_mode *mode);

static void do_detailed_mode(struct detailed_timing *timing, void *c)
{
    struct detailed_mode_closure *closure = c;
    struct drm_display_mode *newmode;

    if (!is_detailed_timing_descriptor((const u8 *)timing)) {
        return;
    }

    newmode = drm_mode_detailed(closure->connector->dev, closure->edid, timing, closure->quirks);
    if (!newmode) {
        return;
    }

    if (closure->preferred) {
        newmode->type |= DRM_MODE_TYPE_PREFERRED;
    }

    /*
     * Detailed modes are limited to 10kHz pixel clock resolution,
     * so fix up anything that looks like CEA/HDMI mode, but the clock
     * is just slightly off.
     */
    fixup_detailed_cea_mode_clock(newmode);

    drm_mode_probed_add(closure->connector, newmode);
    closure->modes++;
    closure->preferred = false;
}

/*
 * add_detailed_modes - Add modes from detailed timings
 * @connector: attached connector
 * @edid: EDID block to scan
 * @quirks: quirks to apply
 */
static int add_detailed_modes(struct drm_connector *connector, struct edid *edid, u32 quirks)
{
    struct detailed_mode_closure closure = {
        .connector = connector,
        .edid = edid,
        .preferred = true,
        .quirks = quirks,
    };

    if (closure.preferred && !version_greater(edid, 1, 0x3)) {
        closure.preferred = (edid->features & DRM_EDID_FEATURE_PREFERRED_TIMING);
    }

    drm_for_each_detailed_block((u8 *)edid, do_detailed_mode, &closure);

    return closure.modes;
}

#define AUDIO_BLOCK 0x01
#define VIDEO_BLOCK 0x02
#define VENDOR_BLOCK 0x03
#define SPEAKER_BLOCK 0x04
#define HDR_STATIC_METADATA_BLOCK 0x6
#define USE_EXTENDED_TAG 0x07
#define EXT_VIDEO_CAPABILITY_BLOCK 0x00
#define EXT_VIDEO_DATA_BLOCK_420 0x0E
#define EXT_VIDEO_CAP_BLOCK_Y420CMDB 0x0F
#define EDID_BASIC_AUDIO (1 << 6)
#define EDID_CEA_YCRCB444 (1 << 5)
#define EDID_CEA_YCRCB422 (1 << 4)
#define EDID_CEA_VCDB_QS (1 << 6)

/*
 * Search EDID for CEA extension block.
 */
static u8 *drm_find_edid_extension(const struct edid *edid, int ext_id, int *ext_index)
{
    u8 *edid_ext = NULL;
    int i;

    /* No EDID or EDID extensions */
    if (edid == NULL || edid->extensions == 0) {
        return NULL;
    }

    /* Find CEA extension */
    for (i = *ext_index; i < edid->extensions; i++) {
        edid_ext = (u8 *)edid + EDID_LENGTH * (i + 1);
        if (edid_ext[0] == ext_id) {
            break;
        }
    }

    if (i >= edid->extensions) {
        return NULL;
    }

    *ext_index = i + 1;

    return edid_ext;
}

static u8 *drm_find_displayid_extension(const struct edid *edid, int *length, int *idx, int *ext_index)
{
    u8 *displayid = drm_find_edid_extension(edid, DISPLAYID_EXT, ext_index);
    struct displayid_hdr *base;
    int ret;

    if (!displayid) {
        return NULL;
    }

    /* EDID extensions block checksum isn't for us */
    *length = EDID_LENGTH - 1;
    *idx = 1;

    ret = validate_displayid(displayid, *length, *idx);
    if (ret) {
        return NULL;
    }

    base = (struct displayid_hdr *)&displayid[*idx];
    *length = *idx + sizeof(*base) + base->bytes;

    return displayid;
}

static u8 *drm_find_cea_extension(const struct edid *edid)
{
    int length, idx;
    struct displayid_block *block;
    u8 *cea;
    u8 *displayid;
    int ext_index;

    /* Look for a top level CEA extension block */
    /* make callers iterate through multiple CEA ext blocks? */
    ext_index = 0;
    cea = drm_find_edid_extension(edid, CEA_EXT, &ext_index);
    if (cea) {
        return cea;
    }

    /* CEA blocks can also be found embedded in a DisplayID block */
    ext_index = 0;
    for (;;) {
        displayid = drm_find_displayid_extension(edid, &length, &idx, &ext_index);
        if (!displayid) {
            return NULL;
        }

        idx += sizeof(struct displayid_hdr);
        for_each_displayid_db(displayid, block, idx, length)
        {
            if (block->tag == DATA_BLOCK_CTA) {
                return (u8 *)block;
            }
        }
    }

    return NULL;
}

static __always_inline const struct drm_display_mode *cea_mode_for_vic(u8 vic)
{
    BUILD_BUG_ON(1 + ARRAY_SIZE(edid_cea_modes_1) - 1 != 0x7f);
    BUILD_BUG_ON(0xc1 + ARRAY_SIZE(edid_cea_modes_193) - 1 != 0xdb);

    if (vic >= 1 && vic < 1 + ARRAY_SIZE(edid_cea_modes_1)) {
        return &edid_cea_modes_1[vic - 1];
    }
    if (vic >= 0xc1 && vic < 0xc1 + ARRAY_SIZE(edid_cea_modes_193)) {
        return &edid_cea_modes_193[vic - 0xc1];
    }
    return NULL;
}

static u8 cea_num_vics(void)
{
    return 0xc1 + ARRAY_SIZE(edid_cea_modes_193);
}

static u8 cea_next_vic(u8 vic)
{
    if (++vic == 1 + ARRAY_SIZE(edid_cea_modes_1)) {
        vic = 0xc1;
    }
    return vic;
}

/*
 * Calculate the alternate clock for the CEA mode
 * (60Hz vs. 59.94Hz etc.)
 */
static unsigned int cea_mode_alternate_clock(const struct drm_display_mode *cea_mode)
{
    unsigned int clock = cea_mode->clock;

    if (drm_mode_vrefresh(cea_mode) % 0x6 != 0) {
        return clock;
    }

    /*
     * edid_cea_modes contains the 59.94Hz
     * variant for 240 and 480 line modes,
     * and the 60Hz variant otherwise.
     */
    if (cea_mode->vdisplay == 0xf0 || cea_mode->vdisplay == 0x1e0) {
        clock = DIV_ROUND_CLOSEST(clock * 0x3e9, 0x3e8);
    } else {
        clock = DIV_ROUND_CLOSEST(clock * 0x3e8, 0x3e9);
    }

    return clock;
}

static bool cea_mode_alternate_timings(u8 vic, struct drm_display_mode *mode)
{
    /*
     * For certain VICs the spec allows the vertical
     * front porch to vary by one or two lines.
     *
     * cea_modes[] stores the variant with the shortest
     * vertical front porch. We can adjust the mode to
     * get the other variants by simply increasing the
     * vertical front porch length.
     */
    BUILD_BUG_ON(cea_mode_for_vic(0x8)->vtotal != 0x106 || cea_mode_for_vic(0x9)->vtotal != 0x106 ||
                 cea_mode_for_vic(0xc)->vtotal != 0x106 || cea_mode_for_vic(0xd)->vtotal != 0x106 ||
                 cea_mode_for_vic(0x17)->vtotal != 0x138 || cea_mode_for_vic(0x18)->vtotal != 0x138 ||
                 cea_mode_for_vic(0x1b)->vtotal != 0x138 || cea_mode_for_vic(0x1c)->vtotal != 0x138);

    if (((vic == 0x8 || vic == 0x9 || vic == 0xc || vic == 0xd) && mode->vtotal < 0x107) ||
        ((vic == 0x17 || vic == 0x18 || vic == 0x1b || vic == 0x1c) && mode->vtotal < 0x13a)) {
        mode->vsync_start++;
        mode->vsync_end++;
        mode->vtotal++;

        return true;
    }

    return false;
}

static u8 drm_match_cea_mode_clock_tolerance(const struct drm_display_mode *to_match, unsigned int clock_tolerance)
{
    unsigned int match_flags = DRM_MODE_MATCH_TIMINGS | DRM_MODE_MATCH_FLAGS;
    u8 vic;

    if (!to_match->clock) {
        return 0;
    }

    if (to_match->picture_aspect_ratio) {
        match_flags |= DRM_MODE_MATCH_ASPECT_RATIO;
    }

    for (vic = 1; vic < cea_num_vics(); vic = cea_next_vic(vic)) {
        struct drm_display_mode cea_mode = *cea_mode_for_vic(vic);
        unsigned int clock1, clock2;

        /* Check both 60Hz and 59.94Hz */
        clock1 = cea_mode.clock;
        clock2 = cea_mode_alternate_clock(&cea_mode);
        if (abs(to_match->clock - clock1) > clock_tolerance && abs(to_match->clock - clock2) > clock_tolerance) {
            continue;
        }

        do {
            if (drm_mode_match(to_match, &cea_mode, match_flags)) {
                return vic;
            }
        } while (cea_mode_alternate_timings(vic, &cea_mode));
    }

    return 0;
}

/**
 * drm_match_cea_mode - look for a CEA mode matching given mode
 * @to_match: display mode
 *
 * Return: The CEA Video ID (VIC) of the mode or 0 if it isn't a CEA-861
 * mode.
 */
u8 drm_match_cea_mode(const struct drm_display_mode *to_match)
{
    unsigned int match_flags = DRM_MODE_MATCH_TIMINGS | DRM_MODE_MATCH_FLAGS;
    u8 vic;

    if (!to_match->clock) {
        return 0;
    }

    if (to_match->picture_aspect_ratio) {
        match_flags |= DRM_MODE_MATCH_ASPECT_RATIO;
    }

    for (vic = 1; vic < cea_num_vics(); vic = cea_next_vic(vic)) {
        struct drm_display_mode cea_mode = *cea_mode_for_vic(vic);
        unsigned int clock1, clock2;

        /* Check both 60Hz and 59.94Hz */
        clock1 = cea_mode.clock;
        clock2 = cea_mode_alternate_clock(&cea_mode);
        if (KHZ2PICOS(to_match->clock) != KHZ2PICOS(clock1) && KHZ2PICOS(to_match->clock) != KHZ2PICOS(clock2)) {
            continue;
        }

        do {
            if (drm_mode_match(to_match, &cea_mode, match_flags)) {
                return vic;
            }
        } while (cea_mode_alternate_timings(vic, &cea_mode));
    }

    return 0;
}
EXPORT_SYMBOL(drm_match_cea_mode);

static bool drm_valid_cea_vic(u8 vic)
{
    return cea_mode_for_vic(vic) != NULL;
}

static enum hdmi_picture_aspect drm_get_cea_aspect_ratio(const u8 video_code)
{
    const struct drm_display_mode *mode = cea_mode_for_vic(video_code);

    if (mode) {
        return mode->picture_aspect_ratio;
    }

    return HDMI_PICTURE_ASPECT_NONE;
}

static enum hdmi_picture_aspect drm_get_hdmi_aspect_ratio(const u8 video_code)
{
    return edid_4k_modes[video_code].picture_aspect_ratio;
}

/*
 * Calculate the alternate clock for HDMI modes (those from the HDMI vendor
 * specific block).
 */
static unsigned int hdmi_mode_alternate_clock(const struct drm_display_mode *hdmi_mode)
{
    return cea_mode_alternate_clock(hdmi_mode);
}

static u8 drm_match_hdmi_mode_clock_tolerance(const struct drm_display_mode *to_match, unsigned int clock_tolerance)
{
    unsigned int match_flags = DRM_MODE_MATCH_TIMINGS | DRM_MODE_MATCH_FLAGS;
    u8 vic;

    if (!to_match->clock) {
        return 0;
    }

    if (to_match->picture_aspect_ratio) {
        match_flags |= DRM_MODE_MATCH_ASPECT_RATIO;
    }

    for (vic = 1; vic < ARRAY_SIZE(edid_4k_modes); vic++) {
        const struct drm_display_mode *hdmi_mode = &edid_4k_modes[vic];
        unsigned int clock1, clock2;

        /* Make sure to also match alternate clocks */
        clock1 = hdmi_mode->clock;
        clock2 = hdmi_mode_alternate_clock(hdmi_mode);
        if (abs(to_match->clock - clock1) > clock_tolerance && abs(to_match->clock - clock2) > clock_tolerance) {
            continue;
        }

        if (drm_mode_match(to_match, hdmi_mode, match_flags)) {
            return vic;
        }
    }

    return 0;
}

/*
 * drm_match_hdmi_mode - look for a HDMI mode matching given mode
 * @to_match: display mode
 *
 * An HDMI mode is one defined in the HDMI vendor specific block.
 *
 * Returns the HDMI Video ID (VIC) of the mode or 0 if it isn't one.
 */
static u8 drm_match_hdmi_mode(const struct drm_display_mode *to_match)
{
    unsigned int match_flags = DRM_MODE_MATCH_TIMINGS | DRM_MODE_MATCH_FLAGS;
    u8 vic;

    if (!to_match->clock) {
        return 0;
    }

    if (to_match->picture_aspect_ratio) {
        match_flags |= DRM_MODE_MATCH_ASPECT_RATIO;
    }

    for (vic = 1; vic < ARRAY_SIZE(edid_4k_modes); vic++) {
        const struct drm_display_mode *hdmi_mode = &edid_4k_modes[vic];
        unsigned int clock1, clock2;

        /* Make sure to also match alternate clocks */
        clock1 = hdmi_mode->clock;
        clock2 = hdmi_mode_alternate_clock(hdmi_mode);
        if ((KHZ2PICOS(to_match->clock) == KHZ2PICOS(clock1) || KHZ2PICOS(to_match->clock) == KHZ2PICOS(clock2)) &&
            drm_mode_match(to_match, hdmi_mode, match_flags)) {
            return vic;
        }
    }
    return 0;
}

static bool drm_valid_hdmi_vic(u8 vic)
{
    return vic > 0 && vic < ARRAY_SIZE(edid_4k_modes);
}

static int add_alternate_cea_modes(struct drm_connector *connector, struct edid *edid)
{
    struct drm_device *dev = connector->dev;
    struct drm_display_mode *mode, *tmp;
    LIST_HEAD(list);
    int modes = 0;

    /* Don't add CEA modes if the CEA extension block is missing */
    if (!drm_find_cea_extension(edid)) {
        return 0;
    }

    /*
     * Go through all probed modes and create a new mode
     * with the alternate clock for certain CEA modes.
     */
    list_for_each_entry(mode, &connector->probed_modes, head)
    {
        const struct drm_display_mode *cea_mode = NULL;
        struct drm_display_mode *newmode;
        u8 vic = drm_match_cea_mode(mode);
        unsigned int clock1, clock2;

        if (drm_valid_cea_vic(vic)) {
            cea_mode = cea_mode_for_vic(vic);
            clock2 = cea_mode_alternate_clock(cea_mode);
        } else {
            vic = drm_match_hdmi_mode(mode);
            if (drm_valid_hdmi_vic(vic)) {
                cea_mode = &edid_4k_modes[vic];
                clock2 = hdmi_mode_alternate_clock(cea_mode);
            }
        }

        if (!cea_mode) {
            continue;
        }

        clock1 = cea_mode->clock;

        if (clock1 == clock2) {
            continue;
        }

        if (mode->clock != clock1 && mode->clock != clock2) {
            continue;
        }

        newmode = drm_mode_duplicate(dev, cea_mode);
        if (!newmode) {
            continue;
        }

        /* Carry over the stereo flags */
        newmode->flags |= mode->flags & DRM_MODE_FLAG_3D_MASK;

        /*
         * The current mode could be either variant. Make
         * sure to pick the "other" clock for the new mode.
         */
        if (mode->clock != clock1) {
            newmode->clock = clock1;
        } else {
            newmode->clock = clock2;
        }

        list_add_tail(&newmode->head, &list);
    }

    list_for_each_entry_safe(mode, tmp, &list, head)
    {
        list_del(&mode->head);
        drm_mode_probed_add(connector, mode);
        modes++;
    }

    return modes;
}

static u8 svd_to_vic(u8 svd)
{
    /* 0-6 bit vic, 7th bit native mode indicator */
    if ((svd >= 0x1 && svd <= 0x40) || (svd >= 0x81 && svd <= 0xc0)) {
        return svd & 0x7f;
    }

    return svd;
}

static struct drm_display_mode *drm_display_mode_from_vic_index(struct drm_connector *connector, const u8 *video_db,
                                                                u8 video_len, u8 video_index)
{
    struct drm_device *dev = connector->dev;
    struct drm_display_mode *newmode;
    u8 vic;

    if (video_db == NULL || video_index >= video_len) {
        return NULL;
    }

    /* CEA modes are numbered 1..127 */
    vic = svd_to_vic(video_db[video_index]);
    if (!drm_valid_cea_vic(vic)) {
        return NULL;
    }

    newmode = drm_mode_duplicate(dev, cea_mode_for_vic(vic));
    if (!newmode) {
        return NULL;
    }

    return newmode;
}

/*
 * do_y420vdb_modes - Parse YCBCR 420 only modes
 * @connector: connector corresponding to the HDMI sink
 * @svds: start of the data block of CEA YCBCR 420 VDB
 * @len: length of the CEA YCBCR 420 VDB
 *
 * Parse the CEA-861-F YCBCR 420 Video Data Block (Y420VDB)
 * which contains modes which can be supported in YCBCR 420
 * output format only.
 */
static int do_y420vdb_modes(struct drm_connector *connector, const u8 *svds, u8 svds_len)
{
    int modes = 0, i;
    struct drm_device *dev = connector->dev;
    struct drm_display_info *info = &connector->display_info;
    struct drm_hdmi_info *hdmi = &info->hdmi;

    for (i = 0; i < svds_len; i++) {
        u8 vic = svd_to_vic(svds[i]);
        struct drm_display_mode *newmode;

        if (!drm_valid_cea_vic(vic)) {
            continue;
        }

        newmode = drm_mode_duplicate(dev, cea_mode_for_vic(vic));
        if (!newmode) {
            break;
        }
        bitmap_set(hdmi->y420_vdb_modes, vic, 1);
        drm_mode_probed_add(connector, newmode);
        modes++;
    }

    if (modes > 0) {
        info->color_formats |= DRM_COLOR_FORMAT_YCRCB420;
    }
    return modes;
}

/*
 * drm_add_cmdb_modes - Add a YCBCR 420 mode into bitmap
 * @connector: connector corresponding to the HDMI sink
 * @vic: CEA vic for the video mode to be added in the map
 *
 * Makes an entry for a videomode in the YCBCR 420 bitmap
 */
static void drm_add_cmdb_modes(struct drm_connector *connector, u8 svd)
{
    u8 vic = svd_to_vic(svd);
    struct drm_hdmi_info *hdmi = &connector->display_info.hdmi;

    if (!drm_valid_cea_vic(vic)) {
        return;
    }

    bitmap_set(hdmi->y420_cmdb_modes, vic, 1);
}

/**
 * drm_display_mode_from_cea_vic() - return a mode for CEA VIC
 * @dev: DRM device
 * @video_code: CEA VIC of the mode
 *
 * Creates a new mode matching the specified CEA VIC.
 *
 * Returns: A new drm_display_mode on success or NULL on failure
 */
struct drm_display_mode *drm_display_mode_from_cea_vic(struct drm_device *dev, u8 video_code)
{
    const struct drm_display_mode *cea_mode;
    struct drm_display_mode *newmode;

    cea_mode = cea_mode_for_vic(video_code);
    if (!cea_mode) {
        return NULL;
    }

    newmode = drm_mode_duplicate(dev, cea_mode);
    if (!newmode) {
        return NULL;
    }

    return newmode;
}
EXPORT_SYMBOL(drm_display_mode_from_cea_vic);

static int do_cea_modes(struct drm_connector *connector, const u8 *db, u8 len)
{
    int i, modes = 0;
    struct drm_hdmi_info *hdmi = &connector->display_info.hdmi;

    for (i = 0; i < len; i++) {
        struct drm_display_mode *mode;

        mode = drm_display_mode_from_vic_index(connector, db, len, i);
        if (mode) {
            /*
             * YCBCR420 capability block contains a bitmap which
             * gives the index of CEA modes from CEA VDB, which
             * can support YCBCR 420 sampling output also (apart
             * from RGB/YCBCR444 etc).
             * For example, if the bit 0 in bitmap is set,
             * first mode in VDB can support YCBCR420 output too.
             * Add YCBCR420 modes only if sink is HDMI 2.0 capable.
             */
            if (i < 0x40 && hdmi->y420_cmdb_map & (1ULL << i)) {
                drm_add_cmdb_modes(connector, db[i]);
            }

            drm_mode_probed_add(connector, mode);
            modes++;
        }
    }

    return modes;
}

struct stereo_mandatory_mode {
    int width, height, vrefresh;
    unsigned int flags;
};

static const struct stereo_mandatory_mode stereo_mandatory_modes[] = {
    {1920, 1080, 24, DRM_MODE_FLAG_3D_TOP_AND_BOTTOM},
    {1920, 1080, 24, DRM_MODE_FLAG_3D_FRAME_PACKING},
    {1920, 1080, 50, DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF},
    {1920, 1080, 60, DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF},
    {1280, 720, 50, DRM_MODE_FLAG_3D_TOP_AND_BOTTOM},
    {1280, 720, 50, DRM_MODE_FLAG_3D_FRAME_PACKING},
    {1280, 720, 60, DRM_MODE_FLAG_3D_TOP_AND_BOTTOM},
    {1280, 720, 60, DRM_MODE_FLAG_3D_FRAME_PACKING}};

static bool stereo_match_mandatory(const struct drm_display_mode *mode, const struct stereo_mandatory_mode *stereo_mode)
{
    unsigned int interlaced = mode->flags & DRM_MODE_FLAG_INTERLACE;

    return mode->hdisplay == stereo_mode->width && mode->vdisplay == stereo_mode->height &&
           interlaced == (stereo_mode->flags & DRM_MODE_FLAG_INTERLACE) &&
           drm_mode_vrefresh(mode) == stereo_mode->vrefresh;
}

static int add_hdmi_mandatory_stereo_modes(struct drm_connector *connector)
{
    struct drm_device *dev = connector->dev;
    const struct drm_display_mode *mode;
    struct list_head stereo_modes;
    int modes = 0, i;

    INIT_LIST_HEAD(&stereo_modes);

    list_for_each_entry(mode, &connector->probed_modes, head)
    {
        for (i = 0; i < ARRAY_SIZE(stereo_mandatory_modes); i++) {
            const struct stereo_mandatory_mode *mandatory;
            struct drm_display_mode *new_mode;

            if (!stereo_match_mandatory(mode, &stereo_mandatory_modes[i])) {
                continue;
            }

            mandatory = &stereo_mandatory_modes[i];
            new_mode = drm_mode_duplicate(dev, mode);
            if (!new_mode) {
                continue;
            }

            new_mode->flags |= mandatory->flags;
            list_add_tail(&new_mode->head, &stereo_modes);
            modes++;
        }
    }

    list_splice_tail(&stereo_modes, &connector->probed_modes);

    return modes;
}

static int add_hdmi_mode(struct drm_connector *connector, u8 vic)
{
    struct drm_device *dev = connector->dev;
    struct drm_display_mode *newmode;

    if (!drm_valid_hdmi_vic(vic)) {
        DRM_ERROR("Unknown HDMI VIC: %d\n", vic);
        return 0;
    }

    newmode = drm_mode_duplicate(dev, &edid_4k_modes[vic]);
    if (!newmode) {
        return 0;
    }

    drm_mode_probed_add(connector, newmode);

    return 1;
}

static int add_3d_struct_modes(struct drm_connector *connector, u16 structure, const u8 *video_db, u8 video_len,
                               u8 video_index)
{
    struct drm_display_mode *newmode;
    int modes = 0;

    if (structure & (1 << 0)) {
        newmode = drm_display_mode_from_vic_index(connector, video_db, video_len, video_index);
        if (newmode) {
            newmode->flags |= DRM_MODE_FLAG_3D_FRAME_PACKING;
            drm_mode_probed_add(connector, newmode);
            modes++;
        }
    }
    if (structure & (1 << 0x6)) {
        newmode = drm_display_mode_from_vic_index(connector, video_db, video_len, video_index);
        if (newmode) {
            newmode->flags |= DRM_MODE_FLAG_3D_TOP_AND_BOTTOM;
            drm_mode_probed_add(connector, newmode);
            modes++;
        }
    }
    if (structure & (1 << 0x8)) {
        newmode = drm_display_mode_from_vic_index(connector, video_db, video_len, video_index);
        if (newmode) {
            newmode->flags |= DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF;
            drm_mode_probed_add(connector, newmode);
            modes++;
        }
    }

    return modes;
}

/*
 * do_hdmi_vsdb_modes - Parse the HDMI Vendor Specific data block
 * @connector: connector corresponding to the HDMI sink
 * @db: start of the CEA vendor specific block
 * @len: length of the CEA block payload, ie. one can access up to db[len]
 *
 * Parses the HDMI VSDB looking for modes to add to @connector. This function
 * also adds the stereo 3d modes when applicable.
 */
static int do_hdmi_vsdb_modes(struct drm_connector *connector, const u8 *db, u8 len, const u8 *video_db, u8 video_len)
{
    struct drm_display_info *info = &connector->display_info;
    int modes = 0, offset = 0, i, multi_present = 0, multi_len;
    u8 vic_len, hdmi_3d_len = 0;
    u16 mask;
    u16 structure_all;

    if (len < 0x8) {
        goto out;
    }

    /* no HDMI_Video_Present */
    if (!(db[0x8] & (0x1 << 0x5))) {
        goto out;
    }

    /* Latency_Fields_Present */
    if (db[0x8] & (0x1 << 0x7)) {
        offset += 0x2;
    }

    /* I_Latency_Fields_Present */
    if (db[0x8] & (0x1 << 0x6)) {
        offset += 0x2;
    }

    /* the declared length is not long enough for the 2 first bytes
     * of additional video format capabilities */
    if (len < (0x8 + offset + 0x2)) {
        goto out;
    }

    /* 3D_Present */
    offset++;
    if (db[0x8 + offset] & (1 << 0x7)) {
        modes += add_hdmi_mandatory_stereo_modes(connector);

        /* 3D_Multi_present */
        multi_present = (db[0x8 + offset] & 0x60) >> 0x5;
    }

    offset++;
    vic_len = db[0x8 + offset] >> 0x5;
    hdmi_3d_len = db[0x8 + offset] & 0x1f;

    for (i = 0x0; i < vic_len && len >= (0x9 + offset + i); i++) {
        u8 vic;

        vic = db[0x9 + offset + i];
        modes += add_hdmi_mode(connector, vic);
    }
    offset += 0x1 + vic_len;

    if (multi_present == 1) {
        multi_len = 0x2;
    } else if (multi_present == 0x2) {
        multi_len = 0x4;
    } else {
        multi_len = 0x0;
    }

    if (len < (0x8 + offset + hdmi_3d_len - 0x1)) {
        goto out;
    }

    if (hdmi_3d_len < multi_len) {
        goto out;
    }

    if (multi_present == 0x1 || multi_present == 0x2) {
        /* 3D_Structure_ALL */
        structure_all = (db[0x8 + offset] << 0x8) | db[0x9 + offset];

        /* check if 3D_MASK is present */
        if (multi_present == 0x2) {
            mask = (db[0xa + offset] << 0x8) | db[0xb + offset];
        } else {
            mask = 0xffff;
        }

        for (i = 0; i < 0x10; i++) {
            if (mask & (1 << i)) {
                modes += add_3d_struct_modes(connector, structure_all, video_db, video_len, i);
            }
        }
    }

    offset += multi_len;

    for (i = 0; i < (hdmi_3d_len - multi_len); i++) {
        int vic_index;
        struct drm_display_mode *newmode = NULL;
        unsigned int newflag = 0;
        bool detail_present;

        detail_present = ((db[0x8 + offset + i] & 0x0f) > 0x7);

        if (detail_present && (i + 1 == hdmi_3d_len - multi_len)) {
            break;
        }

        /* 2D_VIC_order_X */
        vic_index = db[0x8 + offset + i] >> 0x4;

        /* 3D_Structure_X */
        switch (db[0x8 + offset + i] & 0x0f) {
            case 0x0:
                newflag = DRM_MODE_FLAG_3D_FRAME_PACKING;
                break;
            case 0x6:
                newflag = DRM_MODE_FLAG_3D_TOP_AND_BOTTOM;
                break;
            case 0x8:
                /* 3D_Detail_X */
                if ((db[0x9 + offset + i] >> 0x4) == 1) {
                    newflag = DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF;
                }
                break;
        }

        if (newflag != 0) {
            newmode = drm_display_mode_from_vic_index(connector, video_db, video_len, vic_index);
            if (newmode) {
                newmode->flags |= newflag;
                drm_mode_probed_add(connector, newmode);
                modes++;
            }
        }

        if (detail_present) {
            i++;
        }
    }

out:
    if (modes > 0) {
        info->has_hdmi_infoframe = true;
    }
    return modes;
}

static int cea_db_payload_len(const u8 *db)
{
    return db[0] & 0x1f;
}

static int cea_db_extended_tag(const u8 *db)
{
    return db[1];
}

static int cea_db_tag(const u8 *db)
{
    return db[0] >> 0x5;
}

static int cea_revision(const u8 *cea)
{
    /*
     * this correct for the DispID variant?
     * The DispID spec doesn't really specify whether
     * this is the revision of the CEA extension or
     * the DispID CEA data block. And the only value
     * given as an example is 0.
     */
    return cea[1];
}

static int cea_db_offsets(const u8 *cea, int *start, int *end)
{
    /* DisplayID CTA extension blocks and top-level CEA EDID
     * block header definitions differ in the following bytes:
     *   1) Byte 2 of the header specifies length differently,
     *   2) Byte 3 is only present in the CEA top level block.
     *
     * The different definitions for byte 2 follow.
     *
     * DisplayID CTA extension block defines byte 2 as:
     *   Number of payload bytes
     *
     * CEA EDID block defines byte 2 as:
     *   Byte number (decimal) within this block where the 18-byte
     *   DTDs begin. If no non-DTD data is present in this extension
     *   block, the value should be set to 04h (the byte after next).
     *   If set to 00h, there are no DTDs present in this block and
     *   no non-DTD data.
     */
    if (cea[0] == DATA_BLOCK_CTA) {
        /*
         * for_each_displayid_db() has already verified
         * that these stay within expected bounds.
         */
        *start = 0x3;
        *end = *start + cea[0x2];
    } else if (cea[0] == CEA_EXT) {
        /* Data block offset in CEA extension block */
        *start = 0x4;
        *end = cea[0x2];
        if (*end == 0) {
            *end = 0x7f;
        }
        if (*end < 0x4 || *end > 0x7f) {
            return -ERANGE;
        }
    } else {
        return -EOPNOTSUPP;
    }

    return 0;
}

static bool cea_db_is_hdmi_vsdb(const u8 *db)
{
    int hdmi_id;

    if (cea_db_tag(db) != VENDOR_BLOCK) {
        return false;
    }

    if (cea_db_payload_len(db) < 0x5) {
        return false;
    }

    hdmi_id = db[0x1] | (db[0x2] << 0x8) | (db[0x3] << 0x10);

    return hdmi_id == HDMI_IEEE_OUI;
}

static bool cea_db_is_hdmi_forum_vsdb(const u8 *db)
{
    unsigned int oui;

    if (cea_db_tag(db) != VENDOR_BLOCK) {
        return false;
    }

    if (cea_db_payload_len(db) < 0x7) {
        return false;
    }

    oui = (db[0x3] << 0x10) | (db[0x2] << 0x8) | db[0x1];

    return oui == HDMI_FORUM_IEEE_OUI;
}

static bool cea_db_is_vcdb(const u8 *db)
{
    if (cea_db_tag(db) != USE_EXTENDED_TAG) {
        return false;
    }

    if (cea_db_payload_len(db) != 0x2) {
        return false;
    }

    if (cea_db_extended_tag(db) != EXT_VIDEO_CAPABILITY_BLOCK) {
        return false;
    }

    return true;
}

static bool cea_db_is_y420cmdb(const u8 *db)
{
    if (cea_db_tag(db) != USE_EXTENDED_TAG) {
        return false;
    }

    if (!cea_db_payload_len(db)) {
        return false;
    }

    if (cea_db_extended_tag(db) != EXT_VIDEO_CAP_BLOCK_Y420CMDB) {
        return false;
    }

    return true;
}

static bool cea_db_is_y420vdb(const u8 *db)
{
    if (cea_db_tag(db) != USE_EXTENDED_TAG) {
        return false;
    }

    if (!cea_db_payload_len(db)) {
        return false;
    }

    if (cea_db_extended_tag(db) != EXT_VIDEO_DATA_BLOCK_420) {
        return false;
    }

    return true;
}

#define for_each_cea_db(cea, i, start, end)                                           \
    for ((i) = (start); (i) < (end) && (i) + cea_db_payload_len(&(cea)[(i)]) < (end); \
        (i) += cea_db_payload_len(&(cea)[(i)]) + 1)

static void drm_parse_y420cmdb_bitmap(struct drm_connector *connector, const u8 *db)
{
    struct drm_display_info *info = &connector->display_info;
    struct drm_hdmi_info *hdmi = &info->hdmi;
    u8 map_len = cea_db_payload_len(db) - 1;
    u8 count;
    u64 map = 0;

    if (map_len == 0) {
        /* All CEA modes support ycbcr420 sampling also. */
        hdmi->y420_cmdb_map = U64_MAX;
        info->color_formats |= DRM_COLOR_FORMAT_YCRCB420;
        return;
    }

    /*
     * This map indicates which of the existing CEA block modes
     * from VDB can support YCBCR420 output too. So if bit=0 is
     * set, first mode from VDB can support YCBCR420 output too.
     * We will parse and keep this map, before parsing VDB itself
     * to avoid going through the same block again and again.
     *
     * Spec is not clear about max possible size of this block.
     * Clamping max bitmap block size at 8 bytes. Every byte can
     * address 8 CEA modes, in this way this map can address
     * 8*8 = first 64 SVDs.
     */
    if (WARN_ON_ONCE(map_len > 0x8)) {
        map_len = 0x8;
    }

    for (count = 0; count < map_len; count++) {
        map |= (u64)db[0x2 + count] << (0x8 * count);
    }

    if (map) {
        info->color_formats |= DRM_COLOR_FORMAT_YCRCB420;
    }

    hdmi->y420_cmdb_map = map;
}

static int add_cea_modes(struct drm_connector *connector, struct edid *edid)
{
    const u8 *cea = drm_find_cea_extension(edid);
    const u8 *db, *hdmi = NULL, *video = NULL;
    u8 dbl, hdmi_len, video_len = 0;
    int modes = 0;

    if (cea && cea_revision(cea) >= 0x3) {
        int i, start, end;

        if (cea_db_offsets(cea, &start, &end)) {
            return 0;
        }

        for ((i) = (start); (i) < (end) && (i) + cea_db_payload_len(&(cea)[(i)]) < (end); \
            (i) += cea_db_payload_len(&(cea)[(i)]) + 1) {
            db = &cea[i];
            dbl = cea_db_payload_len(db);

            if (cea_db_tag(db) == VIDEO_BLOCK) {
                video = db + 1;
                video_len = dbl;
                modes += do_cea_modes(connector, video, dbl);
            } else if (cea_db_is_hdmi_vsdb(db)) {
                hdmi = db;
                hdmi_len = dbl;
            } else if (cea_db_is_y420vdb(db)) {
                const u8 *vdb420 = &db[0x2];

                /* Add 4:2:0(only) modes present in EDID */
                modes += do_y420vdb_modes(connector, vdb420, dbl - 1);
            }
        }
    }

    /*
     * We parse the HDMI VSDB after having added the cea modes as we will
     * be patching their flags when the sink supports stereo 3D.
     */
    if (hdmi) {
        modes += do_hdmi_vsdb_modes(connector, hdmi, hdmi_len, video, video_len);
    }

    return modes;
}

static void fixup_detailed_cea_mode_clock(struct drm_display_mode *mode)
{
    const struct drm_display_mode *cea_mode;
    int clock1, clock2, clock;
    u8 vic;
    const char *type;

    /*
     * allow 5kHz clock difference either way to account for
     * the 10kHz clock resolution limit of detailed timings.
     */
    vic = drm_match_cea_mode_clock_tolerance(mode, 0x5);
    if (drm_valid_cea_vic(vic)) {
        type = "CEA";
        cea_mode = cea_mode_for_vic(vic);
        clock1 = cea_mode->clock;
        clock2 = cea_mode_alternate_clock(cea_mode);
    } else {
        vic = drm_match_hdmi_mode_clock_tolerance(mode, 0x5);
        if (drm_valid_hdmi_vic(vic)) {
            type = "HDMI";
            cea_mode = &edid_4k_modes[vic];
            clock1 = cea_mode->clock;
            clock2 = hdmi_mode_alternate_clock(cea_mode);
        } else {
            return;
        }
    }

    /* pick whichever is closest */
    if (abs(mode->clock - clock1) < abs(mode->clock - clock2)) {
        clock = clock1;
    } else {
        clock = clock2;
    }

    if (mode->clock == clock) {
        return;
    }

    DRM_DEBUG("detailed mode matches %s VIC %d, adjusting clock %d -> %d\n", type, vic, mode->clock, clock);
    mode->clock = clock;
}

static bool cea_db_is_hdmi_hdr_metadata_block(const u8 *db)
{
    if (cea_db_tag(db) != USE_EXTENDED_TAG) {
        return false;
    }

    if (db[1] != HDR_STATIC_METADATA_BLOCK) {
        return false;
    }

    if (cea_db_payload_len(db) < 0x3) {
        return false;
    }

    return true;
}

static uint8_t eotf_supported(const u8 *edid_ext)
{
    return edid_ext[2] & (BIT(HDMI_EOTF_TRADITIONAL_GAMMA_SDR) | BIT(HDMI_EOTF_TRADITIONAL_GAMMA_HDR) |
                          BIT(HDMI_EOTF_SMPTE_ST2084) | BIT(HDMI_EOTF_BT_2100_HLG));
}

static uint8_t hdr_metadata_type(const u8 *edid_ext)
{
    return edid_ext[3] & BIT(HDMI_STATIC_METADATA_TYPE1);
}

static void drm_parse_hdr_metadata_block(struct drm_connector *connector, const u8 *db)
{
    u16 len;

    len = cea_db_payload_len(db);

    connector->hdr_sink_metadata.hdmi_type1.eotf = eotf_supported(db);
    connector->hdr_sink_metadata.hdmi_type1.metadata_type = hdr_metadata_type(db);

    if (len >= 0x4) {
        connector->hdr_sink_metadata.hdmi_type1.max_cll = db[0x4];
    }
    if (len >= 0x5) {
        connector->hdr_sink_metadata.hdmi_type1.max_fall = db[0x5];
    }
    if (len >= 0x6) {
        connector->hdr_sink_metadata.hdmi_type1.min_cll = db[0x6];
    }
}

static void drm_parse_hdmi_vsdb_audio(struct drm_connector *connector, const u8 *db)
{
    u8 len = cea_db_payload_len(db);
    if (len >= 0x6 && (db[0x6] & (1 << 0x7))) {
        connector->eld[DRM_ELD_SAD_COUNT_CONN_TYPE] |= DRM_ELD_SUPPORTS_AI;
    }
    if (len >= 0x8) {
        connector->latency_present[0] = db[0x8] >> 0x7;
        connector->latency_present[1] = (db[0x8] >> 0x6) & 1;
    }
    if (len >= 0x9) {
        connector->video_latency[0] = db[0x9];
    }
    if (len >= 0xa) {
        connector->audio_latency[0] = db[0xa];
    }
    if (len >= 0xb) {
        connector->video_latency[1] = db[0xb];
    }
    if (len >= 0xc) {
        connector->audio_latency[1] = db[0xc];
    }

    DRM_DEBUG_KMS("HDMI: latency present %d %d, "
                  "video latency %d %d, "
                  "audio latency %d %d\n",
                  connector->latency_present[0], connector->latency_present[1], connector->video_latency[0],
                  connector->video_latency[1], connector->audio_latency[0], connector->audio_latency[1]);
}

static void monitor_name(struct detailed_timing *t, void *data)
{
    if (!is_display_descriptor((const u8 *)t, EDID_DETAIL_MONITOR_NAME)) {
        return;
    }

    *(u8 **)data = t->data.other_data.data.str.str;
}

static int get_monitor_name(struct edid *edid, char name[13])
{
    char *edid_name = NULL;
    int mnl;

    if (!edid || !name) {
        return 0;
    }

    drm_for_each_detailed_block((u8 *)edid, monitor_name, &edid_name);
    for (mnl = 0; edid_name && mnl < 0xd; mnl++) {
        if (edid_name[mnl] == 0x0a) {
            break;
        }

        name[mnl] = edid_name[mnl];
    }

    return mnl;
}

/**
 * drm_edid_get_monitor_name - fetch the monitor name from the edid
 * @edid: monitor EDID information
 * @name: pointer to a character array to hold the name of the monitor
 * @bufsize: The size of the name buffer (should be at least 14 chars.)
 *
 */
void drm_edid_get_monitor_name(struct edid *edid, char *name, int bufsize)
{
    int name_length;
    char buf[0xd];

    if (bufsize <= 0) {
        return;
    }

    name_length = min(get_monitor_name(edid, buf), bufsize - 1);
    memcpy(name, buf, name_length);
    name[name_length] = '\0';
}
EXPORT_SYMBOL(drm_edid_get_monitor_name);

static void clear_eld(struct drm_connector *connector)
{
    memset(connector->eld, 0, sizeof(connector->eld));

    connector->latency_present[0] = false;
    connector->latency_present[1] = false;
    connector->video_latency[0] = 0;
    connector->audio_latency[0] = 0;
    connector->video_latency[1] = 0;
    connector->audio_latency[1] = 0;
}

/*
 * drm_edid_to_eld - build ELD from EDID
 * @connector: connector corresponding to the HDMI/DP sink
 * @edid: EDID to parse
 *
 * Fill the ELD (EDID-Like Data) buffer for passing to the audio driver. The
 * HDCP and Port_ID ELD fields are left for the graphics driver to fill in.
 */
static void drm_edid_to_eld(struct drm_connector *connector, struct edid *edid)
{
    uint8_t *eld = connector->eld;
    u8 *cea;
    u8 *db;
    int total_sad_count = 0;
    int mnl;
    int dbl;

    clear_eld(connector);

    if (!edid) {
        return;
    }

    cea = drm_find_cea_extension(edid);
    if (!cea) {
        DRM_DEBUG_KMS("ELD: no CEA Extension found\n");
        return;
    }

    mnl = get_monitor_name(edid, &eld[DRM_ELD_MONITOR_NAME_STRING]);
    DRM_DEBUG_KMS("ELD monitor %s\n", &eld[DRM_ELD_MONITOR_NAME_STRING]);

    eld[DRM_ELD_CEA_EDID_VER_MNL] = cea[1] << DRM_ELD_CEA_EDID_VER_SHIFT;
    eld[DRM_ELD_CEA_EDID_VER_MNL] |= mnl;

    eld[DRM_ELD_VER] = DRM_ELD_VER_CEA861D;

    eld[DRM_ELD_MANUFACTURER_NAME0] = edid->mfg_id[0];
    eld[DRM_ELD_MANUFACTURER_NAME1] = edid->mfg_id[1];
    eld[DRM_ELD_PRODUCT_CODE0] = edid->prod_code[0];
    eld[DRM_ELD_PRODUCT_CODE1] = edid->prod_code[1];

    if (cea_revision(cea) >= 0x3) {
        int i, start, end;
        int sad_count;

        if (cea_db_offsets(cea, &start, &end)) {
            start = 0;
            end = 0;
        }

        for ((i) = (start); (i) < (end) && (i) + cea_db_payload_len(&(cea)[(i)]) < (end); \
            (i) += cea_db_payload_len(&(cea)[(i)]) + 1) {
            db = &cea[i] ;
            dbl = cea_db_payload_len(db);

            switch (cea_db_tag(db)) {
                case AUDIO_BLOCK:
                    /* Audio Data Block, contains SADs */
                    sad_count = min(dbl / 0x3, 0xf - total_sad_count);
                    if (sad_count >= 1) {
                        memcpy(&eld[DRM_ELD_CEA_SAD(mnl, total_sad_count)], &db[1], sad_count * 0x3);
                    }
                    total_sad_count += sad_count;
                    break;
                case SPEAKER_BLOCK:
                    /* Speaker Allocation Data Block */
                    if (dbl >= 1) {
                        eld[DRM_ELD_SPEAKER] = db[1];
                    }
                    break;
                case VENDOR_BLOCK:
                    /* HDMI Vendor-Specific Data Block */
                    if (cea_db_is_hdmi_vsdb(db)) {
                        drm_parse_hdmi_vsdb_audio(connector, db);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    eld[DRM_ELD_SAD_COUNT_CONN_TYPE] |= total_sad_count << DRM_ELD_SAD_COUNT_SHIFT;

    if (connector->connector_type == DRM_MODE_CONNECTOR_DisplayPort ||
        connector->connector_type == DRM_MODE_CONNECTOR_eDP) {
        eld[DRM_ELD_SAD_COUNT_CONN_TYPE] |= DRM_ELD_CONN_TYPE_DP;
    } else {
        eld[DRM_ELD_SAD_COUNT_CONN_TYPE] |= DRM_ELD_CONN_TYPE_HDMI;
    }

    eld[DRM_ELD_BASELINE_ELD_LEN] = DIV_ROUND_UP(drm_eld_calc_baseline_block_size(eld), 0x4);

    DRM_DEBUG_KMS("ELD size %d, SAD count %d\n", drm_eld_size(eld), total_sad_count);
}

/**
 * drm_edid_to_sad - extracts SADs from EDID
 * @edid: EDID to parse
 * @sads: pointer that will be set to the extracted SADs
 *
 * Looks for CEA EDID block and extracts SADs (Short Audio Descriptors) from it.
 *
 * Note: The returned pointer needs to be freed using kfree().
 *
 * Return: The number of found SADs or negative number on error.
 */
int drm_edid_to_sad(struct edid *edid, struct cea_sad **sads)
{
    int count = 0;
    int i, start, end, dbl;
    u8 *cea;

    cea = drm_find_cea_extension(edid);
    if (!cea) {
        DRM_DEBUG_KMS("SAD: no CEA Extension found\n");
        return 0;
    }

    if (cea_revision(cea) < 0x3) {
        DRM_DEBUG_KMS("SAD: wrong CEA revision\n");
        return 0;
    }

    if (cea_db_offsets(cea, &start, &end)) {
        DRM_DEBUG_KMS("SAD: invalid data block offsets\n");
        return -EPROTO;
    }

    for ((i) = (start); (i) < (end) && (i) + cea_db_payload_len(&(cea)[(i)]) < (end); \
        (i) += cea_db_payload_len(&(cea)[(i)]) + 1) {
        u8 *db = &cea[i];
        if (cea_db_tag(db) == AUDIO_BLOCK) {
            int j;

            dbl = cea_db_payload_len(db);

            count = dbl / 0x3; /* SAD is 3B */
            *sads = kcalloc(count, sizeof(**sads), GFP_KERNEL);
            if (!*sads) {
                return -ENOMEM;
            }
            for (j = 0; j < count; j++) {
                u8 *sad = &db[1 + j * 0x3];

                (*sads)[j].format = (sad[0] & 0x78) >> 0x3;
                (*sads)[j].channels = sad[0] & 0x7;
                (*sads)[j].freq = sad[0x1] & 0x7F;
                (*sads)[j].byte2 = sad[0x2];
            }
            break;
        }
    }

    return count;
}
EXPORT_SYMBOL(drm_edid_to_sad);

/**
 * drm_edid_to_speaker_allocation - extracts Speaker Allocation Data Blocks from EDID
 * @edid: EDID to parse
 * @sadb: pointer to the speaker block
 *
 * Looks for CEA EDID block and extracts the Speaker Allocation Data Block from it.
 *
 * Note: The returned pointer needs to be freed using kfree().
 *
 * Return: The number of found Speaker Allocation Blocks or negative number on
 * error.
 */
int drm_edid_to_speaker_allocation(struct edid *edid, u8 **sadb)
{
    int count = 0;
    int i, start, end, dbl;
    const u8 *cea;

    cea = drm_find_cea_extension(edid);
    if (!cea) {
        DRM_DEBUG_KMS("SAD: no CEA Extension found\n");
        return 0;
    }

    if (cea_revision(cea) < 0x3) {
        DRM_DEBUG_KMS("SAD: wrong CEA revision\n");
        return 0;
    }

    if (cea_db_offsets(cea, &start, &end)) {
        DRM_DEBUG_KMS("SAD: invalid data block offsets\n");
        return -EPROTO;
    }

    for ((i) = (start); (i) < (end) && (i) + cea_db_payload_len(&(cea)[(i)]) < (end); \
        (i) += cea_db_payload_len(&(cea)[(i)]) + 1) {
        const u8 *db = &cea[i];

        if (cea_db_tag(db) == SPEAKER_BLOCK) {
            dbl = cea_db_payload_len(db);
            /* Speaker Allocation Data Block */
            if (dbl == 0x3) {
                *sadb = kmemdup(&db[1], dbl, GFP_KERNEL);
                if (!*sadb) {
                    return -ENOMEM;
                }
                count = dbl;
                break;
            }
        }
    }

    return count;
}
EXPORT_SYMBOL(drm_edid_to_speaker_allocation);

/**
 * drm_av_sync_delay - compute the HDMI/DP sink audio-video sync delay
 * @connector: connector associated with the HDMI/DP sink
 * @mode: the display mode
 *
 * Return: The HDMI/DP sink's audio-video sync delay in milliseconds or 0 if
 * the sink doesn't support audio or video.
 */
int drm_av_sync_delay(struct drm_connector *connector, const struct drm_display_mode *mode)
{
    int i = !!(mode->flags & DRM_MODE_FLAG_INTERLACE);
    int a, v;

    if (!connector->latency_present[0]) {
        return 0;
    }
    if (!connector->latency_present[1]) {
        i = 0;
    }

    a = connector->audio_latency[i];
    v = connector->video_latency[i];

    /*
     * HDMI/DP sink doesn't support audio or video?
     */
    if (a == 0xff || v == 0xff) {
        return 0;
    }

    /*
     * Convert raw EDID values to millisecond.
     * Treat unknown latency as 0ms.
     */
    if (a) {
        a = min(0x2 * (a - 1), 0x1f4);
    }
    if (v) {
        v = min(0x2 * (v - 1), 0x1f4);
    }

    return max(v - a, 0);
}
EXPORT_SYMBOL(drm_av_sync_delay);

/**
 * drm_detect_hdmi_monitor - detect whether monitor is HDMI
 * @edid: monitor EDID information
 *
 * Parse the CEA extension according to CEA-861-B.
 *
 * Drivers that have added the modes parsed from EDID to drm_display_info
 * should use &drm_display_info.is_hdmi instead of calling this function.
 *
 * Return: True if the monitor is HDMI, false if not or unknown.
 */
bool drm_detect_hdmi_monitor(struct edid *edid)
{
    u8 *edid_ext;
    int i;
    int start_offset, end_offset;

    edid_ext = drm_find_cea_extension(edid);
    if (!edid_ext) {
        return false;
    }

    if (cea_db_offsets(edid_ext, &start_offset, &end_offset)) {
        return false;
    }

    /*
     * Because HDMI identifier is in Vendor Specific Block,
     * search it from all data blocks of CEA extension.
     */
    for ((i) = (start_offset); (i) < (end_offset) && (i) + cea_db_payload_len(&(edid_ext)[(i)]) < (end_offset); \
        (i) += cea_db_payload_len(&(edid_ext)[(i)]) + 1) {
        if (cea_db_is_hdmi_vsdb(&edid_ext[i])) {
            return true;
        }
    }

    return false;
}
EXPORT_SYMBOL(drm_detect_hdmi_monitor);

/**
 * drm_detect_monitor_audio - check monitor audio capability
 * @edid: EDID block to scan
 *
 * Monitor should have CEA extension block.
 * If monitor has 'basic audio', but no CEA audio blocks, it's 'basic
 * audio' only. If there is any audio extension block and supported
 * audio format, assume at least 'basic audio' support, even if 'basic
 * audio' is not defined in EDID.
 *
 * Return: True if the monitor supports audio, false otherwise.
 */
bool drm_detect_monitor_audio(struct edid *edid)
{
    u8 *edid_ext;
    int i, j;
    bool has_audio = false;
    int start_offset, end_offset;

    edid_ext = drm_find_cea_extension(edid);
    if (!edid_ext) {
        goto end;
    }

	has_audio = (edid_ext[0] == CEA_EXT &&
		    (edid_ext[3] & EDID_BASIC_AUDIO) != 0);

    if (has_audio) {
        DRM_DEBUG_KMS("Monitor has basic audio support\n");
        goto end;
    }

    if (cea_db_offsets(edid_ext, &start_offset, &end_offset)) {
        goto end;
    }

    for ((i) = (start_offset); (i) < (end_offset) && (i) + cea_db_payload_len(&(edid_ext)[(i)]) < (end_offset); \
        (i) += cea_db_payload_len(&(edid_ext)[(i)]) + 1) {
        if (cea_db_tag(&edid_ext[i]) == AUDIO_BLOCK) {
            has_audio = true;
            for (j = 1; j < cea_db_payload_len(&edid_ext[i]) + 1; j += 0x3) {
                DRM_DEBUG_KMS("CEA audio format %d\n", (edid_ext[i + j] >> 0x3) & 0xf);
            }
            goto end;
        }
    }
end:
    return has_audio;
}
EXPORT_SYMBOL(drm_detect_monitor_audio);

/**
 * drm_default_rgb_quant_range - default RGB quantization range
 * @mode: display mode
 *
 * Determine the default RGB quantization range for the mode,
 * as specified in CEA-861.
 *
 * Return: The default RGB quantization range for the mode
 */
enum hdmi_quantization_range drm_default_rgb_quant_range(const struct drm_display_mode *mode)
{
    /* All CEA modes other than VIC 1 use limited quantization range. */
    return drm_match_cea_mode(mode) > 1 ? HDMI_QUANTIZATION_RANGE_LIMITED : HDMI_QUANTIZATION_RANGE_FULL;
}
EXPORT_SYMBOL(drm_default_rgb_quant_range);

static void drm_parse_vcdb(struct drm_connector *connector, const u8 *db)
{
    struct drm_display_info *info = &connector->display_info;

    DRM_DEBUG_KMS("CEA VCDB 0x%02x\n", db[0x2]);

    if (db[0x2] & EDID_CEA_VCDB_QS) {
        info->rgb_quant_range_selectable = true;
    }
}

#ifdef CONFIG_NO_GKI
static void drm_get_max_frl_rate(int max_frl_rate, u8 *max_lanes, u8 *max_rate_per_lane)
{
    switch (max_frl_rate) {
        case 1:
            *max_lanes = 0x3;
            *max_rate_per_lane = 0x3;
            break;
        case 0x2:
            *max_lanes = 0x3;
            *max_rate_per_lane = 0x6;
            break;
        case 0x3:
            *max_lanes = 0x4;
            *max_rate_per_lane = 0x6;
            break;
        case 0x4:
            *max_lanes = 0x4;
            *max_rate_per_lane = 0x8;
            break;
        case 0x5:
            *max_lanes = 0x4;
            *max_rate_per_lane = 0xa;
            break;
        case 0x6:
            *max_lanes = 0x4;
            *max_rate_per_lane = 0xc;
            break;
        case 0:
        default:
            *max_lanes = 0;
            *max_rate_per_lane = 0;
    }
}
#endif

static void drm_parse_ycbcr420_deep_color_info(struct drm_connector *connector, const u8 *db)
{
    u8 dc_mask;
    struct drm_hdmi_info *hdmi = &connector->display_info.hdmi;

    dc_mask = db[0x7] & DRM_EDID_YCBCR420_DC_MASK;
    hdmi->y420_dc_modes = dc_mask;
}

static void drm_parse_hdmi_forum_vsdb(struct drm_connector *connector, const u8 *hf_vsdb)
{
    struct drm_display_info *display = &connector->display_info;
    struct drm_hdmi_info *hdmi = &display->hdmi;

    display->has_hdmi_infoframe = true;

    if (hf_vsdb[0x6] & 0x80) {
        hdmi->scdc.supported = true;
        if (hf_vsdb[0x6] & 0x40) {
            hdmi->scdc.read_request = true;
        }
    }

    /*
     * All HDMI 2.0 monitors must support scrambling at rates > 340 MHz.
     * And as per the spec, three factors confirm this:
     * * Availability of a HF-VSDB block in EDID (check)
     * * Non zero Max_TMDS_Char_Rate filed in HF-VSDB (let's check)
     * * SCDC support available (let's check)
     * Lets check it out.
     */

    if (hf_vsdb[0x5]) {
        /* max clock is 5000 KHz times block value */
        u32 max_tmds_clock = hf_vsdb[0x5] * 0x1388;
        struct drm_scdc *scdc = &hdmi->scdc;

        if (max_tmds_clock > 0x53020) {
            display->max_tmds_clock = max_tmds_clock;
            DRM_DEBUG_KMS("HF-VSDB: max TMDS clock %d kHz\n", display->max_tmds_clock);
        }

        if (scdc->supported) {
            scdc->scrambling.supported = true;

            /* Few sinks support scrambling for clocks < 340M */
            if ((hf_vsdb[0x6] & 0x8)) {
                scdc->scrambling.low_rates = true;
            }
        }
    }

#ifdef CONFIG_NO_GKI
    if (hf_vsdb[0x7]) {
        u8 max_frl_rate;
        u8 dsc_max_frl_rate;
        u8 dsc_max_slices;
        struct drm_hdmi_dsc_cap *hdmi_dsc = &hdmi->dsc_cap;

        DRM_DEBUG_KMS("hdmi_21 sink detected. parsing edid\n");
        max_frl_rate = (hf_vsdb[0x7] & DRM_EDID_MAX_FRL_RATE_MASK) >> 0x4;
        drm_get_max_frl_rate(max_frl_rate, &hdmi->max_lanes, &hdmi->max_frl_rate_per_lane);
        hdmi_dsc->v_1p2 = hf_vsdb[0xb] & DRM_EDID_DSC_1P2;

        if (hdmi_dsc->v_1p2) {
            hdmi_dsc->native_420 = hf_vsdb[0xb] & DRM_EDID_DSC_NATIVE_420;
            hdmi_dsc->all_bpp = hf_vsdb[0xb] & DRM_EDID_DSC_ALL_BPP;

            if (hf_vsdb[0xb] & DRM_EDID_DSC_16BPC) {
                hdmi_dsc->bpc_supported = 0x10;
            } else if (hf_vsdb[0xb] & DRM_EDID_DSC_12BPC) {
                hdmi_dsc->bpc_supported = 0xc;
            } else if (hf_vsdb[0xb] & DRM_EDID_DSC_10BPC) {
                hdmi_dsc->bpc_supported = 0xa;
            } else {
                hdmi_dsc->bpc_supported = 0x0;
            }

            dsc_max_frl_rate = (hf_vsdb[0xc] & DRM_EDID_DSC_MAX_FRL_RATE_MASK) >> 0x4;
            drm_get_max_frl_rate(dsc_max_frl_rate, &hdmi_dsc->max_lanes, &hdmi_dsc->max_frl_rate_per_lane);
            hdmi_dsc->total_chunk_kbytes = hf_vsdb[13] & DRM_EDID_DSC_TOTAL_CHUNK_KBYTES;

            dsc_max_slices = hf_vsdb[0xc] & DRM_EDID_DSC_MAX_SLICES;
            switch (dsc_max_slices) {
                case 0x1:
                    hdmi_dsc->max_slices = 0x1;
                    hdmi_dsc->clk_per_slice = 0x154;
                    break;
                case 0x2:
                    hdmi_dsc->max_slices = 0x2;
                    hdmi_dsc->clk_per_slice = 0x154;
                    break;
                case 0x3:
                    hdmi_dsc->max_slices = 0x4;
                    hdmi_dsc->clk_per_slice = 0x154;
                    break;
                case 0x4:
                    hdmi_dsc->max_slices = 0x8;
                    hdmi_dsc->clk_per_slice = 0x154;
                    break;
                case 0x5:
                    hdmi_dsc->max_slices = 0x8;
                    hdmi_dsc->clk_per_slice = 0x190;
                    break;
                case 0x6:
                    hdmi_dsc->max_slices = 0xc;
                    hdmi_dsc->clk_per_slice = 0x190;
                    break;
                case 0x7:
                    hdmi_dsc->max_slices = 0x10;
                    hdmi_dsc->clk_per_slice = 0x190;
                    break;
                case 0x0:
                default:
                    hdmi_dsc->max_slices = 0x0;
                    hdmi_dsc->clk_per_slice = 0x0;
            }
        }
    }
#endif

    drm_parse_ycbcr420_deep_color_info(connector, hf_vsdb);
}

static void drm_parse_hdmi_deep_color_info(struct drm_connector *connector, const u8 *hdmi)
{
    struct drm_display_info *info = &connector->display_info;
    unsigned int dc_bpc = 0;

    /* HDMI supports at least 8 bpc */
    info->bpc = 0x8;

    if (cea_db_payload_len(hdmi) < 0x6) {
        return;
    }

    if (hdmi[0x6] & DRM_EDID_HDMI_DC_30) {
        dc_bpc = 0xa;
        info->edid_hdmi_dc_modes |= DRM_EDID_HDMI_DC_30;
        DRM_DEBUG("%s: HDMI sink does deep color 30.\n", connector->name);
    }

    if (hdmi[0x6] & DRM_EDID_HDMI_DC_36) {
        dc_bpc = 0xc;
        info->edid_hdmi_dc_modes |= DRM_EDID_HDMI_DC_36;
        DRM_DEBUG("%s: HDMI sink does deep color 36.\n", connector->name);
    }

    if (hdmi[0x6] & DRM_EDID_HDMI_DC_48) {
        dc_bpc = 0x10;
        info->edid_hdmi_dc_modes |= DRM_EDID_HDMI_DC_48;
        DRM_DEBUG("%s: HDMI sink does deep color 48.\n", connector->name);
    }

    if (dc_bpc == 0) {
        DRM_DEBUG("%s: No deep color support on this HDMI sink.\n", connector->name);
        return;
    }

    DRM_DEBUG("%s: Assigning HDMI sink color depth as %d bpc.\n", connector->name, dc_bpc);
    info->bpc = dc_bpc;

    /* YCRCB444 is optional according to spec. */
    if (hdmi[0x6] & DRM_EDID_HDMI_DC_Y444) {
        DRM_DEBUG("%s: HDMI sink does YCRCB444 in deep color.\n", connector->name);
    }

    /*
     * Spec says that if any deep color mode is supported at all,
     * then deep color 36 bit must be supported.
     */
    if (!(hdmi[0x6] & DRM_EDID_HDMI_DC_36)) {
        DRM_DEBUG("%s: HDMI sink should do DC_36, but does not!\n", connector->name);
    }
}

static void drm_parse_hdmi_vsdb_video(struct drm_connector *connector, const u8 *db)
{
    struct drm_display_info *info = &connector->display_info;
    u8 len = cea_db_payload_len(db);

    info->is_hdmi = true;

    if (len >= 0x6) {
        info->dvi_dual = db[0x6] & 1;
    }
    if (len >= 0x7) {
        info->max_tmds_clock = db[0x7] * 0x1388;
    }

    DRM_DEBUG_KMS("HDMI: DVI dual %d, "
                  "max TMDS clock %d kHz\n",
                  info->dvi_dual, info->max_tmds_clock);

    drm_parse_hdmi_deep_color_info(connector, db);
}

static void drm_parse_cea_ext(struct drm_connector *connector, const struct edid *edid)
{
    struct drm_display_info *info = &connector->display_info;
    const u8 *edid_ext;
    int i, start, end;

    edid_ext = drm_find_cea_extension(edid);
    if (!edid_ext) {
        return;
    }

    info->cea_rev = edid_ext[1];

    /* The existence of a CEA block should imply RGB support */
    info->color_formats = DRM_COLOR_FORMAT_RGB444;
    if (edid_ext[0x3] & EDID_CEA_YCRCB444) {
        info->color_formats |= DRM_COLOR_FORMAT_YCRCB444;
    }
    if (edid_ext[0x3] & EDID_CEA_YCRCB422) {
        info->color_formats |= DRM_COLOR_FORMAT_YCRCB422;
    }

    if (cea_db_offsets(edid_ext, &start, &end)) {
        return;
    }

    for ((i) = (start); (i) < (end) && (i) + cea_db_payload_len(&(edid_ext)[(i)]) < (end);
        (i) += cea_db_payload_len(&(edid_ext)[(i)]) + 1) {
        const u8 *db = &edid_ext[i];

        if (cea_db_is_hdmi_vsdb(db)) {
            drm_parse_hdmi_vsdb_video(connector, db);
        }
        if (cea_db_is_hdmi_forum_vsdb(db)) {
            drm_parse_hdmi_forum_vsdb(connector, db);
        }
        if (cea_db_is_y420cmdb(db)) {
            drm_parse_y420cmdb_bitmap(connector, db);
        }
        if (cea_db_is_vcdb(db)) {
            drm_parse_vcdb(connector, db);
        }
        if (cea_db_is_hdmi_hdr_metadata_block(db)) {
            drm_parse_hdr_metadata_block(connector, db);
        }
    }
}

static void get_monitor_range(struct detailed_timing *timing, void *info_monitor_range)
{
    struct drm_monitor_range_info *monitor_range = info_monitor_range;
    const struct detailed_non_pixel *data = &timing->data.other_data;
    const struct detailed_data_monitor_range *range = &data->data.range;

    if (!is_display_descriptor((const u8 *)timing, EDID_DETAIL_MONITOR_RANGE)) {
        return;
    }

    /*
     * Check for flag range limits only. If flag == 1 then
     * no additional timing information provided.
     * Default GTF, GTF Secondary curve and CVT are not
     * supported
     */
    if (range->flags != DRM_EDID_RANGE_LIMITS_ONLY_FLAG) {
        return;
    }

    monitor_range->min_vfreq = range->min_vfreq;
    monitor_range->max_vfreq = range->max_vfreq;
}

static void drm_get_monitor_range(struct drm_connector *connector, const struct edid *edid)
{
    struct drm_display_info *info = &connector->display_info;

    if (!version_greater(edid, 1, 1)) {
        return;
    }

    drm_for_each_detailed_block((u8 *)edid, get_monitor_range, &info->monitor_range);

    DRM_DEBUG_KMS("Supported Monitor Refresh rate range is %d Hz - %d Hz\n", info->monitor_range.min_vfreq,
                  info->monitor_range.max_vfreq);
}

/* A connector has no EDID information, so we've got no EDID to compute quirks from. Reset
 * all of the values which would have been set from EDID
 */
void drm_reset_display_info(struct drm_connector *connector)
{
    struct drm_display_info *info = &connector->display_info;

    info->width_mm = 0;
    info->height_mm = 0;

    info->bpc = 0;
    info->color_formats = 0;
    info->cea_rev = 0;
    info->max_tmds_clock = 0;
    info->dvi_dual = false;
    info->is_hdmi = false;
    info->has_hdmi_infoframe = false;
    info->rgb_quant_range_selectable = false;
    memset(&info->hdmi, 0, sizeof(info->hdmi));

    info->non_desktop = 0;
    memset(&info->monitor_range, 0, sizeof(info->monitor_range));
}

u32 drm_add_display_info(struct drm_connector *connector, const struct edid *edid)
{
    struct drm_display_info *info = &connector->display_info;

    u32 quirks = edid_get_quirks(edid);

    drm_reset_display_info(connector);

    info->width_mm = edid->width_cm * 0xa;
    info->height_mm = edid->height_cm * 0xa;

    info->non_desktop = !!(quirks & EDID_QUIRK_NON_DESKTOP);

    drm_get_monitor_range(connector, edid);

    DRM_DEBUG_KMS("non_desktop set to %d\n", info->non_desktop);

    if (edid->revision < 0x3) {
        return quirks;
    }

    if (!(edid->input & DRM_EDID_INPUT_DIGITAL)) {
        return quirks;
    }
	info->color_formats |= DRM_COLOR_FORMAT_RGB444;
    drm_parse_cea_ext(connector, edid);

    /*
     * Digital sink with "DFP 1.x compliant TMDS" according to EDID 1.3?
     *
     * For such displays, the DFP spec 1.0, section 3.10 "EDID support"
     * tells us to assume 8 bpc color depth if the EDID doesn't have
     * extensions which tell otherwise.
     */
    if ((info->bpc == 0x0) && (edid->revision == 0x3) && (edid->input & DRM_EDID_DIGITAL_DFP_1_X)) {
        info->bpc = 0x8;
        DRM_DEBUG("%s: Assigning DFP sink color depth as %d bpc.\n", connector->name, info->bpc);
    }

    /* Only defined for 1.4 with digital displays */
    if (edid->revision < 0x4) {
        return quirks;
    }

    switch (edid->input & DRM_EDID_DIGITAL_DEPTH_MASK) {
        case DRM_EDID_DIGITAL_DEPTH_6:
            info->bpc = 0x6;
            break;
        case DRM_EDID_DIGITAL_DEPTH_8:
            info->bpc = 0x8;
            break;
        case DRM_EDID_DIGITAL_DEPTH_10:
            info->bpc = 0xa;
            break;
        case DRM_EDID_DIGITAL_DEPTH_12:
            info->bpc = 0xc;
            break;
        case DRM_EDID_DIGITAL_DEPTH_14:
            info->bpc = 0xe;
            break;
        case DRM_EDID_DIGITAL_DEPTH_16:
            info->bpc = 0x10;
            break;
        case DRM_EDID_DIGITAL_DEPTH_UNDEF:
        default:
            info->bpc = 0;
            break;
    }

    DRM_DEBUG("%s: Assigning EDID-1.4 digital sink color depth as %d bpc.\n", connector->name, info->bpc);

    if (edid->features & DRM_EDID_FEATURE_RGB_YCRCB444) {
        info->color_formats |= DRM_COLOR_FORMAT_YCRCB444;
    }
    if (edid->features & DRM_EDID_FEATURE_RGB_YCRCB422) {
        info->color_formats |= DRM_COLOR_FORMAT_YCRCB422;
    }
    return quirks;
}

static int validate_displayid(u8 *displayid, int length, int idx)
{
    int i, dispid_length;
    u8 csum = 0;
    struct displayid_hdr *base;

    base = (struct displayid_hdr *)&displayid[idx];

    DRM_DEBUG_KMS("base revision 0x%x, length %d, %d %d\n", base->rev, base->bytes, base->prod_id, base->ext_count);

    /* +1 for DispID checksum */
    dispid_length = sizeof(*base) + base->bytes + 1;
    if (dispid_length > length - idx) {
        return -EINVAL;
    }

    for (i = 0; i < dispid_length; i++) {
        csum += displayid[idx + i];
    }
    if (csum) {
        DRM_NOTE("DisplayID checksum invalid, remainder is %d\n", csum);
        return -EINVAL;
    }

    return 0;
}

static struct drm_display_mode *drm_mode_displayid_detailed(struct drm_device *dev,
                                                            struct displayid_detailed_timings_1 *timings)
{
    struct drm_display_mode *mode;
    unsigned pixel_clock =
        (timings->pixel_clock[0x0] | (timings->pixel_clock[0x1] << 0x8) | (timings->pixel_clock[0x2] << 0x10)) + 1;
    unsigned hactive = (timings->hactive[0] | timings->hactive[1] << 0x8) + 1;
    unsigned hblank = (timings->hblank[0] | timings->hblank[1] << 0x8) + 1;
    unsigned hsync = (timings->hsync[0] | (timings->hsync[1] & 0x7f) << 0x8) + 1;
    unsigned hsync_width = (timings->hsw[0] | timings->hsw[1] << 0x8) + 1;
    unsigned vactive = (timings->vactive[0] | timings->vactive[1] << 0x8) + 1;
    unsigned vblank = (timings->vblank[0] | timings->vblank[1] << 0x8) + 1;
    unsigned vsync = (timings->vsync[0] | (timings->vsync[1] & 0x7f) << 0x8) + 1;
    unsigned vsync_width = (timings->vsw[0] | timings->vsw[1] << 0x8) + 1;
    bool hsync_positive = (timings->hsync[1] >> 0x7) & 0x1;
    bool vsync_positive = (timings->vsync[1] >> 0x7) & 0x1;

    mode = drm_mode_create(dev);
    if (!mode) {
        return NULL;
    }

    mode->clock = pixel_clock * 0xa;
    mode->hdisplay = hactive;
    mode->hsync_start = mode->hdisplay + hsync;
    mode->hsync_end = mode->hsync_start + hsync_width;
    mode->htotal = mode->hdisplay + hblank;

    mode->vdisplay = vactive;
    mode->vsync_start = mode->vdisplay + vsync;
    mode->vsync_end = mode->vsync_start + vsync_width;
    mode->vtotal = mode->vdisplay + vblank;

    mode->flags = 0;
    mode->flags |= hsync_positive ? DRM_MODE_FLAG_PHSYNC : DRM_MODE_FLAG_NHSYNC;
    mode->flags |= vsync_positive ? DRM_MODE_FLAG_PVSYNC : DRM_MODE_FLAG_NVSYNC;
    mode->type = DRM_MODE_TYPE_DRIVER;

    if (timings->flags & 0x80) {
        mode->type |= DRM_MODE_TYPE_PREFERRED;
    }
    drm_mode_set_name(mode);

    return mode;
}

static int add_displayid_detailed_1_modes(struct drm_connector *connector, struct displayid_block *block)
{
    struct displayid_detailed_timing_block *det = (struct displayid_detailed_timing_block *)block;
    int i;
    int num_timings;
    struct drm_display_mode *newmode;
    int num_modes = 0;
    /* blocks must be multiple of 20 bytes length */
    if (block->num_bytes % 0x14) {
        return 0;
    }

    num_timings = block->num_bytes / 0x14;
    for (i = 0; i < num_timings; i++) {
        struct displayid_detailed_timings_1 *timings = &det->timings[i];

        newmode = drm_mode_displayid_detailed(connector->dev, timings);
        if (!newmode) {
            continue;
        }

        drm_mode_probed_add(connector, newmode);
        num_modes++;
    }
    return num_modes;
}

static int add_displayid_detailed_modes(struct drm_connector *connector, struct edid *edid)
{
    u8 *displayid;
    int length, idx;
    struct displayid_block *block;
    int num_modes = 0;
    int ext_index = 0;

    for (;;) {
        displayid = drm_find_displayid_extension(edid, &length, &idx, &ext_index);
        if (!displayid) {
            break;
        }

        idx += sizeof(struct displayid_hdr);
        for_each_displayid_db(displayid, block, idx, length)
        {
            switch (block->tag) {
                case DATA_BLOCK_TYPE_1_DETAILED_TIMING:
                    num_modes += add_displayid_detailed_1_modes(connector, block);
                    break;
                default:
                    break;
            }
        }
    }

    return num_modes;
}

/**
 * drm_add_edid_modes - add modes from EDID data, if available
 * @connector: connector we're probing
 * @edid: EDID data
 *
 * Add the specified modes to the connector's mode list. Also fills out the
 * &drm_display_info structure and ELD in @connector with any information which
 * can be derived from the edid.
 *
 * Return: The number of modes added or 0 if we couldn't find any.
 */
int drm_add_edid_modes(struct drm_connector *connector, struct edid *edid)
{
    int num_modes = 0;
    u32 quirks;

    if (edid == NULL) {
        clear_eld(connector);
        return 0;
    }
    if (!drm_edid_is_valid(edid)) {
        clear_eld(connector);
        drm_warn(connector->dev, "%s: EDID invalid.\n", connector->name);
        return 0;
    }

    drm_edid_to_eld(connector, edid);

    /*
     * CEA-861-F adds ycbcr capability map block, for HDMI 2.0 sinks.
     * To avoid multiple parsing of same block, lets parse that map
     * from sink info, before parsing CEA modes.
     */
    quirks = drm_add_display_info(connector, edid);

    /*
     * EDID spec says modes should be preferred in this order:
     * - preferred detailed mode
     * - other detailed modes from base block
     * - detailed modes from extension blocks
     * - CVT 3-byte code modes
     * - standard timing codes
     * - established timing codes
     * - modes inferred from GTF or CVT range information
     *
     * We get this pretty much right.
     *
     * XXX order for additional mode types in extension blocks?
     */
    num_modes += add_detailed_modes(connector, edid, quirks);
    num_modes += add_cvt_modes(connector, edid);
    num_modes += add_standard_modes(connector, edid);
    num_modes += add_established_modes(connector, edid);
    num_modes += add_cea_modes(connector, edid);
    num_modes += add_alternate_cea_modes(connector, edid);
    num_modes += add_displayid_detailed_modes(connector, edid);
    if (edid->features & DRM_EDID_FEATURE_DEFAULT_GTF) {
        num_modes += add_inferred_modes(connector, edid);
    }

    if (quirks & (EDID_QUIRK_PREFER_LARGE_60 | EDID_QUIRK_PREFER_LARGE_75)) {
        edid_fixup_preferred(connector, quirks);
    }

    if (quirks & EDID_QUIRK_FORCE_6BPC) {
        connector->display_info.bpc = 0x6;
    }

    if (quirks & EDID_QUIRK_FORCE_8BPC) {
        connector->display_info.bpc = 0x8;
    }

    if (quirks & EDID_QUIRK_FORCE_10BPC) {
        connector->display_info.bpc = 0xa;
    }

    if (quirks & EDID_QUIRK_FORCE_12BPC) {
        connector->display_info.bpc = 0xc;
    }

    return num_modes;
}
EXPORT_SYMBOL(drm_add_edid_modes);

/**
 * drm_add_override_edid_modes - add modes from override/firmware EDID
 * @connector: connector we're probing
 *
 * Add modes from the override/firmware EDID, if available. Only to be used from
 * drm_helper_probe_single_connector_modes() as a fallback for when DDC probe
 * failed during drm_get_edid() and caused the override/firmware EDID to be
 * skipped.
 *
 * Return: The number of modes added or 0 if we couldn't find any.
 */
int drm_add_override_edid_modes(struct drm_connector *connector)
{
    struct edid *override;
    int num_modes = 0;

    override = drm_get_override_edid(connector);
    if (override) {
        drm_connector_update_edid_property(connector, override);
        num_modes = drm_add_edid_modes(connector, override);
        kfree(override);

        DRM_DEBUG_KMS("[CONNECTOR:%d:%s] adding %d modes via fallback override/firmware EDID\n", connector->base.id,
                      connector->name, num_modes);
    }

    return num_modes;
}
EXPORT_SYMBOL(drm_add_override_edid_modes);

/**
 * drm_add_modes_noedid - add modes for the connectors without EDID
 * @connector: connector we're probing
 * @hdisplay: the horizontal display limit
 * @vdisplay: the vertical display limit
 *
 * Add the specified modes to the connector's mode list. Only when the
 * hdisplay/vdisplay is not beyond the given limit, it will be added.
 *
 * Return: The number of modes added or 0 if we couldn't find any.
 */
int drm_add_modes_noedid(struct drm_connector *connector, int hdisplay, int vdisplay)
{
    int i, count, num_modes = 0;
    struct drm_display_mode *mode;
    struct drm_device *dev = connector->dev;

    count = ARRAY_SIZE(drm_dmt_modes);
    if (hdisplay < 0) {
        hdisplay = 0;
    }
    if (vdisplay < 0) {
        vdisplay = 0;
    }

    for (i = 0; i < count; i++) {
        const struct drm_display_mode *ptr = &drm_dmt_modes[i];

        if (hdisplay && vdisplay) {
            /*
             * Only when two are valid, they will be used to check
             * whether the mode should be added to the mode list of
             * the connector.
             */
            if (ptr->hdisplay > hdisplay || ptr->vdisplay > vdisplay) {
                continue;
            }
        }
        if (drm_mode_vrefresh(ptr) > 0x3d) {
            continue;
        }
        mode = drm_mode_duplicate(dev, ptr);
        if (mode) {
            drm_mode_probed_add(connector, mode);
            num_modes++;
        }
    }
    return num_modes;
}
EXPORT_SYMBOL(drm_add_modes_noedid);

/**
 * drm_set_preferred_mode - Sets the preferred mode of a connector
 * @connector: connector whose mode list should be processed
 * @hpref: horizontal resolution of preferred mode
 * @vpref: vertical resolution of preferred mode
 *
 * Marks a mode as preferred if it matches the resolution specified by @hpref
 * and @vpref.
 */
void drm_set_preferred_mode(struct drm_connector *connector, int hpref, int vpref)
{
    struct drm_display_mode *mode;

    list_for_each_entry(mode, &connector->probed_modes, head)
    {
        if (mode->hdisplay == hpref && mode->vdisplay == vpref) {
            mode->type |= DRM_MODE_TYPE_PREFERRED;
        }
    }
}
EXPORT_SYMBOL(drm_set_preferred_mode);

static bool is_hdmi2_sink(const struct drm_connector *connector)
{
    /*
     * sil-sii8620 doesn't have a connector around when
     * we need one, so we have to be prepared for a NULL connector.
     */
    if (!connector) {
        return true;
    }

    return connector->display_info.hdmi.scdc.supported ||
           (connector->display_info.color_formats & DRM_COLOR_FORMAT_YCRCB420);
}

static inline bool is_eotf_supported(u8 output_eotf, u8 sink_eotf)
{
    return sink_eotf & BIT(output_eotf);
}

/**
 * drm_hdmi_infoframe_set_hdr_metadata() - fill an HDMI DRM infoframe with
 *                                         HDR metadata from userspace
 * @frame: HDMI DRM infoframe
 * @conn_state: Connector state containing HDR metadata
 *
 * Return: 0 on success or a negative error code on failure.
 */
int drm_hdmi_infoframe_set_hdr_metadata(struct hdmi_drm_infoframe *frame, const struct drm_connector_state *conn_state)
{
    struct drm_connector *connector;
    struct hdr_output_metadata *hdr_metadata;
    int err;

    if (!frame || !conn_state) {
        return -EINVAL;
    }

    connector = conn_state->connector;

    if (!conn_state->hdr_output_metadata) {
        return -EINVAL;
    }

    hdr_metadata = conn_state->hdr_output_metadata->data;

    if (!hdr_metadata || !connector) {
        return -EINVAL;
    }

    /* Sink EOTF is Bit map while infoframe is absolute values */
    if (!is_eotf_supported(hdr_metadata->hdmi_metadata_type1.eotf, connector->hdr_sink_metadata.hdmi_type1.eotf)) {
        DRM_DEBUG_KMS("EOTF Not Supported\n");
        return -EINVAL;
    }

    err = hdmi_drm_infoframe_init(frame);
    if (err < 0) {
        return err;
    }

    frame->eotf = hdr_metadata->hdmi_metadata_type1.eotf;
    frame->metadata_type = hdr_metadata->hdmi_metadata_type1.metadata_type;

    BUILD_BUG_ON(sizeof(frame->display_primaries) != sizeof(hdr_metadata->hdmi_metadata_type1.display_primaries));
    BUILD_BUG_ON(sizeof(frame->white_point) != sizeof(hdr_metadata->hdmi_metadata_type1.white_point));

    memcpy(&frame->display_primaries, &hdr_metadata->hdmi_metadata_type1.display_primaries,
           sizeof(frame->display_primaries));

    memcpy(&frame->white_point, &hdr_metadata->hdmi_metadata_type1.white_point, sizeof(frame->white_point));

    frame->max_display_mastering_luminance = hdr_metadata->hdmi_metadata_type1.max_display_mastering_luminance;
    frame->min_display_mastering_luminance = hdr_metadata->hdmi_metadata_type1.min_display_mastering_luminance;
    frame->max_fall = hdr_metadata->hdmi_metadata_type1.max_fall;
    frame->max_cll = hdr_metadata->hdmi_metadata_type1.max_cll;

    return 0;
}
EXPORT_SYMBOL(drm_hdmi_infoframe_set_hdr_metadata);

static u8 drm_mode_hdmi_vic(const struct drm_connector *connector, const struct drm_display_mode *mode)
{
    bool has_hdmi_infoframe = connector ? connector->display_info.has_hdmi_infoframe : false;

    if (!has_hdmi_infoframe) {
        return 0;
    }

    /* No HDMI VIC when signalling 3D video format */
    if (mode->flags & DRM_MODE_FLAG_3D_MASK) {
        return 0;
    }

    return drm_match_hdmi_mode(mode);
}

static u8 drm_mode_cea_vic(const struct drm_connector *connector, const struct drm_display_mode *mode)
{
    u8 vic;

    /*
     * HDMI spec says if a mode is found in HDMI 1.4b 4K modes
     * we should send its VIC in vendor infoframes, else send the
     * VIC in AVI infoframes. Lets check if this mode is present in
     * HDMI 1.4b 4K modes
     */
    if (drm_mode_hdmi_vic(connector, mode)) {
        return 0;
    }

    vic = drm_match_cea_mode(mode);
    /*
     * HDMI 1.4 VIC range: 1 <= VIC <= 64 (CEA-861-D) but
     * HDMI 2.0 VIC range: 1 <= VIC <= 107 (CEA-861-F). So we
     * have to make sure we dont break HDMI 1.4 sinks.
     */
    if (!is_hdmi2_sink(connector) && vic > 0x40) {
        return 0;
    }

    return vic;
}

/**
 * drm_hdmi_avi_infoframe_from_display_mode() - fill an HDMI AVI infoframe with
 *                                              data from a DRM display mode
 * @frame: HDMI AVI infoframe
 * @connector: the connector
 * @mode: DRM display mode
 *
 * Return: 0 on success or a negative error code on failure.
 */
int drm_hdmi_avi_infoframe_from_display_mode(struct hdmi_avi_infoframe *frame, const struct drm_connector *connector,
                                             const struct drm_display_mode *mode)
{
    enum hdmi_picture_aspect picture_aspect;
    u8 vic, hdmi_vic;

    if (!frame || !mode) {
        return -EINVAL;
    }

    hdmi_avi_infoframe_init(frame);

    if (mode->flags & DRM_MODE_FLAG_DBLCLK) {
        frame->pixel_repeat = 1;
    }

    vic = drm_mode_cea_vic(connector, mode);
    hdmi_vic = drm_mode_hdmi_vic(connector, mode);

    frame->picture_aspect = HDMI_PICTURE_ASPECT_NONE;

    /*
     * As some drivers don't support atomic, we can't use connector state.
     * So just initialize the frame with default values, just the same way
     * as it's done with other properties here.
     */
    frame->content_type = HDMI_CONTENT_TYPE_GRAPHICS;
    frame->itc = 0;

    /*
     * Populate picture aspect ratio from either
     * user input (if specified) or from the CEA/HDMI mode lists.
     */
    picture_aspect = mode->picture_aspect_ratio;
    if (picture_aspect == HDMI_PICTURE_ASPECT_NONE) {
        if (vic) {
            picture_aspect = drm_get_cea_aspect_ratio(vic);
        } else if (hdmi_vic) {
            picture_aspect = drm_get_hdmi_aspect_ratio(hdmi_vic);
        }
    }

    /*
     * The infoframe can't convey anything but none, 4:3
     * and 16:9, so if the user has asked for anything else
     * we can only satisfy it by specifying the right VIC.
     */
    if (picture_aspect > HDMI_PICTURE_ASPECT_16_9) {
        if (vic) {
            if (picture_aspect != drm_get_cea_aspect_ratio(vic)) {
                return -EINVAL;
            }
        } else if (hdmi_vic) {
            if (picture_aspect != drm_get_hdmi_aspect_ratio(hdmi_vic)) {
                return -EINVAL;
            }
        } else {
            return -EINVAL;
        }

        picture_aspect = HDMI_PICTURE_ASPECT_NONE;
    }

    frame->video_code = vic;
    frame->picture_aspect = picture_aspect;
    frame->active_aspect = HDMI_ACTIVE_ASPECT_PICTURE;
    frame->scan_mode = HDMI_SCAN_MODE_UNDERSCAN;

    return 0;
}
EXPORT_SYMBOL(drm_hdmi_avi_infoframe_from_display_mode);

/* HDMI Colorspace Spec Definitions */
#define FULL_COLORIMETRY_MASK 0x1FF
#define NORMAL_COLORIMETRY_MASK 0x3
#define EXTENDED_COLORIMETRY_MASK 0x7
#define EXTENDED_ACE_COLORIMETRY_MASK 0xF

#define C(x) ((x) << 0)
#define EC(x) ((x) << 2)
#define ACE(x) ((x) << 5)

#define HDMI_COLORIMETRY_NO_DATA 0x0
#define HDMI_COLORIMETRY_SMPTE_170M_YCC (C(1) | EC(0) | ACE(0))
#define HDMI_COLORIMETRY_BT709_YCC (C(2) | EC(0) | ACE(0))
#define HDMI_COLORIMETRY_XVYCC_601 (C(3) | EC(0) | ACE(0))
#define HDMI_COLORIMETRY_XVYCC_709 (C(3) | EC(1) | ACE(0))
#define HDMI_COLORIMETRY_SYCC_601 (C(3) | EC(2) | ACE(0))
#define HDMI_COLORIMETRY_OPYCC_601 (C(3) | EC(3) | ACE(0))
#define HDMI_COLORIMETRY_OPRGB (C(3) | EC(4) | ACE(0))
#define HDMI_COLORIMETRY_BT2020_CYCC (C(3) | EC(5) | ACE(0))
#define HDMI_COLORIMETRY_BT2020_RGB (C(3) | EC(6) | ACE(0))
#define HDMI_COLORIMETRY_BT2020_YCC (C(3) | EC(6) | ACE(0))
#define HDMI_COLORIMETRY_DCI_P3_RGB_D65 (C(3) | EC(7) | ACE(0))
#define HDMI_COLORIMETRY_DCI_P3_RGB_THEATER (C(3) | EC(7) | ACE(1))

static const u32 hdmi_colorimetry_val[] = {
    [DRM_MODE_COLORIMETRY_NO_DATA] = HDMI_COLORIMETRY_NO_DATA,
    [DRM_MODE_COLORIMETRY_SMPTE_170M_YCC] = HDMI_COLORIMETRY_SMPTE_170M_YCC,
    [DRM_MODE_COLORIMETRY_BT709_YCC] = HDMI_COLORIMETRY_BT709_YCC,
    [DRM_MODE_COLORIMETRY_XVYCC_601] = HDMI_COLORIMETRY_XVYCC_601,
    [DRM_MODE_COLORIMETRY_XVYCC_709] = HDMI_COLORIMETRY_XVYCC_709,
    [DRM_MODE_COLORIMETRY_SYCC_601] = HDMI_COLORIMETRY_SYCC_601,
    [DRM_MODE_COLORIMETRY_OPYCC_601] = HDMI_COLORIMETRY_OPYCC_601,
    [DRM_MODE_COLORIMETRY_OPRGB] = HDMI_COLORIMETRY_OPRGB,
    [DRM_MODE_COLORIMETRY_BT2020_CYCC] = HDMI_COLORIMETRY_BT2020_CYCC,
    [DRM_MODE_COLORIMETRY_BT2020_RGB] = HDMI_COLORIMETRY_BT2020_RGB,
    [DRM_MODE_COLORIMETRY_BT2020_YCC] = HDMI_COLORIMETRY_BT2020_YCC,
};

#undef C
#undef EC
#undef ACE

/**
 * drm_hdmi_avi_infoframe_colorspace() - fill the HDMI AVI infoframe
 *                                       colorspace information
 * @frame: HDMI AVI infoframe
 * @conn_state: connector state
 */
void drm_hdmi_avi_infoframe_colorspace(struct hdmi_avi_infoframe *frame, const struct drm_connector_state *conn_state)
{
    u32 colorimetry_val;
    u32 colorimetry_index = conn_state->colorspace & FULL_COLORIMETRY_MASK;

    if (colorimetry_index >= ARRAY_SIZE(hdmi_colorimetry_val)) {
        colorimetry_val = HDMI_COLORIMETRY_NO_DATA;
    } else {
        colorimetry_val = hdmi_colorimetry_val[colorimetry_index];
    }

    frame->colorimetry = colorimetry_val & NORMAL_COLORIMETRY_MASK;
    /*
     * Extend it for ACE formats as well. Modify the infoframe
     * structure and extend it in drivers/video/hdmi
     */
    frame->extended_colorimetry = (colorimetry_val >> 0x2) & EXTENDED_COLORIMETRY_MASK;
}
EXPORT_SYMBOL(drm_hdmi_avi_infoframe_colorspace);

/**
 * drm_hdmi_avi_infoframe_quant_range() - fill the HDMI AVI infoframe
 *                                        quantization range information
 * @frame: HDMI AVI infoframe
 * @connector: the connector
 * @mode: DRM display mode
 * @rgb_quant_range: RGB quantization range (Q)
 */
void drm_hdmi_avi_infoframe_quant_range(struct hdmi_avi_infoframe *frame, const struct drm_connector *connector,
                                        const struct drm_display_mode *mode,
                                        enum hdmi_quantization_range rgb_quant_range)
{
    const struct drm_display_info *info = &connector->display_info;

    /*
     * CEA-861:
     * "A Source shall not send a non-zero Q value that does not correspond
     *  to the default RGB Quantization Range for the transmitted Picture
     *  unless the Sink indicates support for the Q bit in a Video
     *  Capabilities Data Block."
     *
     * HDMI 2.0 recommends sending non-zero Q when it does match the
     * default RGB quantization range for the mode, even when QS=0.
     */
    if (info->rgb_quant_range_selectable || rgb_quant_range == drm_default_rgb_quant_range(mode)) {
        frame->quantization_range = rgb_quant_range;
    } else {
        frame->quantization_range = HDMI_QUANTIZATION_RANGE_DEFAULT;
    }

    /*
     * CEA-861-F:
     * "When transmitting any RGB colorimetry, the Source should set the
     *  YQ-field to match the RGB Quantization Range being transmitted
     *  (e.g., when Limited Range RGB, set YQ=0 or when Full Range RGB,
     *  set YQ=1) and the Sink shall ignore the YQ-field."
     *
     * Unfortunate certain sinks (eg. VIZ Model 67/E261VA) get confused
     * by non-zero YQ when receiving RGB. There doesn't seem to be any
     * good way to tell which version of CEA-861 the sink supports, so
     * we limit non-zero YQ to HDMI 2.0 sinks only as HDMI 2.0 is based
     * on on CEA-861-F.
     */
    if (!is_hdmi2_sink(connector) || rgb_quant_range == HDMI_QUANTIZATION_RANGE_LIMITED) {
        frame->ycc_quantization_range = HDMI_YCC_QUANTIZATION_RANGE_LIMITED;
    } else {
        frame->ycc_quantization_range = HDMI_YCC_QUANTIZATION_RANGE_FULL;
    }
}
EXPORT_SYMBOL(drm_hdmi_avi_infoframe_quant_range);

/**
 * drm_hdmi_avi_infoframe_bars() - fill the HDMI AVI infoframe
 *                                 bar information
 * @frame: HDMI AVI infoframe
 * @conn_state: connector state
 */
void drm_hdmi_avi_infoframe_bars(struct hdmi_avi_infoframe *frame, const struct drm_connector_state *conn_state)
{
    frame->right_bar = conn_state->tv.margins.right;
    frame->left_bar = conn_state->tv.margins.left;
    frame->top_bar = conn_state->tv.margins.top;
    frame->bottom_bar = conn_state->tv.margins.bottom;
}
EXPORT_SYMBOL(drm_hdmi_avi_infoframe_bars);

static enum hdmi_3d_structure s3d_structure_from_display_mode(const struct drm_display_mode *mode)
{
    u32 layout = mode->flags & DRM_MODE_FLAG_3D_MASK;

    switch (layout) {
        case DRM_MODE_FLAG_3D_FRAME_PACKING:
            return HDMI_3D_STRUCTURE_FRAME_PACKING;
        case DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE:
            return HDMI_3D_STRUCTURE_FIELD_ALTERNATIVE;
        case DRM_MODE_FLAG_3D_LINE_ALTERNATIVE:
            return HDMI_3D_STRUCTURE_LINE_ALTERNATIVE;
        case DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL:
            return HDMI_3D_STRUCTURE_SIDE_BY_SIDE_FULL;
        case DRM_MODE_FLAG_3D_L_DEPTH:
            return HDMI_3D_STRUCTURE_L_DEPTH;
        case DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH:
            return HDMI_3D_STRUCTURE_L_DEPTH_GFX_GFX_DEPTH;
        case DRM_MODE_FLAG_3D_TOP_AND_BOTTOM:
            return HDMI_3D_STRUCTURE_TOP_AND_BOTTOM;
        case DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF:
            return HDMI_3D_STRUCTURE_SIDE_BY_SIDE_HALF;
        default:
            return HDMI_3D_STRUCTURE_INVALID;
    }
}

/**
 * drm_hdmi_vendor_infoframe_from_display_mode() - fill an HDMI infoframe with
 * data from a DRM display mode
 * @frame: HDMI vendor infoframe
 * @connector: the connector
 * @mode: DRM display mode
 *
 * Note that there's is a need to send HDMI vendor infoframes only when using a
 * 4k or stereoscopic 3D mode. So when giving any other mode as input this
 * function will return -EINVAL, error that can be safely ignored.
 *
 * Return: 0 on success or a negative error code on failure.
 */
int drm_hdmi_vendor_infoframe_from_display_mode(struct hdmi_vendor_infoframe *frame,
                                                const struct drm_connector *connector,
                                                const struct drm_display_mode *mode)
{
    /*
     * sil-sii8620 doesn't have a connector around when
     * we need one, so we have to be prepared for a NULL connector.
     */
    bool has_hdmi_infoframe = connector ? connector->display_info.has_hdmi_infoframe : false;
    int err;

    if (!frame || !mode) {
        return -EINVAL;
    }

    if (!has_hdmi_infoframe) {
        return -EINVAL;
    }

    err = hdmi_vendor_infoframe_init(frame);
    if (err < 0) {
        return err;
    }

    /*
     * Even if it's not absolutely necessary to send the infoframe
     * (ie.vic==0 and s3d_struct==0) we will still send it if we
     * know that the sink can handle it. This is based on a
     * suggestion in HDMI 2.0 Appendix F. Apparently some sinks
     * have trouble realizing that they shuld switch from 3D to 2D
     * mode if the source simply stops sending the infoframe when
     * it wants to switch from 3D to 2D.
     */
    frame->vic = drm_mode_hdmi_vic(connector, mode);
    frame->s3d_struct = s3d_structure_from_display_mode(mode);

    return 0;
}
EXPORT_SYMBOL(drm_hdmi_vendor_infoframe_from_display_mode);

static void drm_parse_tiled_block(struct drm_connector *connector, const struct displayid_block *block)
{
    const struct displayid_tiled_block *tile = (struct displayid_tiled_block *)block;
    u16 w, h;
    u8 tile_v_loc, tile_h_loc;
    u8 num_v_tile, num_h_tile;
    struct drm_tile_group *tg;

    w = tile->tile_size[0x0] | tile->tile_size[0x1] << 0x8;
    h = tile->tile_size[0x2] | tile->tile_size[0x3] << 0x8;

    num_v_tile = (tile->topo[0] & 0xf) | (tile->topo[0x2] & 0x30);
    num_h_tile = (tile->topo[0] >> 0x4) | ((tile->topo[0x2] >> 0x2) & 0x30);
    tile_v_loc = (tile->topo[1] & 0xf) | ((tile->topo[0x2] & 0x3) << 0x4);
    tile_h_loc = (tile->topo[1] >> 0x4) | (((tile->topo[0x2] >> 0x2) & 0x3) << 0x4);

    connector->has_tile = true;
    if (tile->tile_cap & 0x80) {
        connector->tile_is_single_monitor = true;
    }

    connector->num_h_tile = num_h_tile + 1;
    connector->num_v_tile = num_v_tile + 1;
    connector->tile_h_loc = tile_h_loc;
    connector->tile_v_loc = tile_v_loc;
    connector->tile_h_size = w + 1;
    connector->tile_v_size = h + 1;

    DRM_DEBUG_KMS("tile cap 0x%x\n", tile->tile_cap);
    DRM_DEBUG_KMS("tile_size %d x %d\n", w + 1, h + 1);
    DRM_DEBUG_KMS("topo num tiles %dx%d, location %dx%d\n", num_h_tile + 1, num_v_tile + 1, tile_h_loc, tile_v_loc);
    DRM_DEBUG_KMS("vend %c%c%c\n", tile->topology_id[0], tile->topology_id[1], tile->topology_id[2]);

    tg = drm_mode_get_tile_group(connector->dev, tile->topology_id);
    if (!tg) {
        tg = drm_mode_create_tile_group(connector->dev, tile->topology_id);
    }
    if (!tg) {
        return;
    }

    if (connector->tile_group != tg) {
        /* if we haven't got a pointer,
           take the reference, drop ref to old tile group */
        if (connector->tile_group) {
            drm_mode_put_tile_group(connector->dev, connector->tile_group);
        }
        connector->tile_group = tg;
    } else {
        /* if same tile group, then release the ref we just took. */
        drm_mode_put_tile_group(connector->dev, tg);
    }
}

static void drm_displayid_parse_tiled(struct drm_connector *connector, const u8 *displayid, int length, int idx)
{
    const struct displayid_block *block;

    idx += sizeof(struct displayid_hdr);
    for_each_displayid_db(displayid, block, idx, length)
    {
        DRM_DEBUG_KMS("block id 0x%x, rev %d, len %d\n", block->tag, block->rev, block->num_bytes);

        switch (block->tag) {
            case DATA_BLOCK_TILED_DISPLAY:
                drm_parse_tiled_block(connector, block);
                break;
            default:
                DRM_DEBUG_KMS("found DisplayID tag 0x%x, unhandled\n", block->tag);
                break;
        }
    }
}

void drm_update_tile_info(struct drm_connector *connector, const struct edid *edid)
{
    const void *displayid = NULL;
    int ext_index = 0;
    int length, idx;

    connector->has_tile = false;
    for (;;) {
        displayid = drm_find_displayid_extension(edid, &length, &idx, &ext_index);
        if (!displayid) {
            break;
        }

        drm_displayid_parse_tiled(connector, displayid, length, idx);
    }

    if (!connector->has_tile && connector->tile_group) {
        drm_mode_put_tile_group(connector->dev, connector->tile_group);
        connector->tile_group = NULL;
    }
}
