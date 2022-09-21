/*
 * Rockchip isp1 driver
 *
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _RKISP_REGS_H
#define _RKISP_REGS_H
#include "dev.h"
#include "regs_v2x.h"
#include "regs_v3x.h"

#define CIF_ISP_PACK_4BYTE(a, b, c, d)    \
    (((a) & 0xFF) << 0 | ((b) & 0xFF) << 8 | \
     ((c) & 0xFF) << 16 | ((d) & 0xFF) << 24)

#define CIF_ISP_PACK_2SHORT(a, b)    \
    (((a) & 0xFFFF) << 0 | ((b) & 0xFFFF) << 16)

/* GRF */
#define GRF_VI_CON0                 0x430
#define ISP_CIF_DATA_WIDTH_MASK         0x60006000
#define ISP_CIF_DATA_WIDTH_8B            (0 << 13 | 3 << 29)
#define ISP_CIF_DATA_WIDTH_10B            (BIT(13) | 3 << 29)
#define ISP_CIF_DATA_WIDTH_12B            (2 << 13 | 3 << 29)

/* ISP_CTRL */
#define CIF_ISP_CTRL_ISP_ENABLE            BIT(0)
#define CIF_ISP_CTRL_ISP_MODE_RAW_PICT        (0 << 1)
#define CIF_ISP_CTRL_ISP_MODE_ITU656        (1 << 1)
#define CIF_ISP_CTRL_ISP_MODE_ITU601        (2 << 1)
#define CIF_ISP_CTRL_ISP_MODE_BAYER_ITU601    (3 << 1)
#define CIF_ISP_CTRL_ISP_MODE_DATA_MODE        (4 << 1)
#define CIF_ISP_CTRL_ISP_MODE_BAYER_ITU656    (5 << 1)
#define CIF_ISP_CTRL_ISP_MODE_RAW_PICT_ITU656    (6 << 1)
#define CIF_ISP_CTRL_ISP_INFORM_ENABLE        BIT(4)
#define CIF_ISP_CTRL_ISP_GAMMA_IN_ENA        BIT(6)
#define CIF_ISP_CTRL_ISP_AWB_ENA        BIT(7)
#define CIF_ISP_CTRL_ISP_CFG_UPD_PERMANENT    BIT(8)
#define CIF_ISP_CTRL_ISP_CFG_UPD        BIT(9)
#define CIF_ISP_CTRL_ISP_GEN_CFG_UPD        BIT(10)
#define CIF_ISP_CTRL_ISP_GAMMA_OUT_ENA        BIT(11)
#define CIF_ISP_CTRL_ISP_FLASH_MODE_ENA        BIT(12)
#define CIF_ISP_CTRL_ISP_CSM_Y_FULL_ENA        BIT(13)
#define CIF_ISP_CTRL_ISP_CSM_C_FULL_ENA        BIT(14)

/* ISP_ACQ_PROP */
#define CIF_ISP_ACQ_PROP_POS_EDGE        BIT(0)
#define CIF_ISP_ACQ_PROP_HSYNC_LOW        BIT(1)
#define CIF_ISP_ACQ_PROP_VSYNC_LOW        BIT(2)
#define CIF_ISP_ACQ_PROP_BAYER_PAT_RGGB        (0 << 3)
#define CIF_ISP_ACQ_PROP_BAYER_PAT_GRBG        (1 << 3)
#define CIF_ISP_ACQ_PROP_BAYER_PAT_GBRG        (2 << 3)
#define CIF_ISP_ACQ_PROP_BAYER_PAT_BGGR        (3 << 3)
#define CIF_ISP_ACQ_PROP_BAYER_PAT(pat)        ((pat) << 3)
#define CIF_ISP_ACQ_PROP_YCBYCR            (0 << 7)
#define CIF_ISP_ACQ_PROP_YCRYCB            (1 << 7)
#define CIF_ISP_ACQ_PROP_CBYCRY            (2 << 7)
#define CIF_ISP_ACQ_PROP_CRYCBY            (3 << 7)
#define CIF_ISP_ACQ_PROP_FIELD_SEL_ALL        (0 << 9)
#define CIF_ISP_ACQ_PROP_FIELD_SEL_EVEN        (1 << 9)
#define CIF_ISP_ACQ_PROP_FIELD_SEL_ODD        (2 << 9)
#define CIF_ISP_ACQ_PROP_IN_SEL_12B        (0 << 12)
#define CIF_ISP_ACQ_PROP_IN_SEL_10B_ZERO    (1 << 12)
#define CIF_ISP_ACQ_PROP_IN_SEL_10B_MSB        (2 << 12)
#define CIF_ISP_ACQ_PROP_IN_SEL_8B_ZERO        (3 << 12)
#define CIF_ISP_ACQ_PROP_IN_SEL_8B_MSB        (4 << 12)
#define CIF_ISP_ACQ_PROP_DMA_RGB        BIT(15)
#define CIF_ISP_ACQ_PROP_DMA_YUV        BIT(16)

/* VI_DPCL */
#define CIF_VI_DPCL_DMA_JPEG            (0 << 0)
#define CIF_VI_DPCL_MP_MUX_MRSZ_MI        (1 << 0)
#define CIF_VI_DPCL_MP_MUX_MRSZ_JPEG        (2 << 0)
#define CIF_VI_DPCL_CHAN_MODE_MP        (1 << 2)
#define CIF_VI_DPCL_CHAN_MODE_SP        (2 << 2)
#define CIF_VI_DPCL_CHAN_MODE_MPSP        (3 << 2)
#define CIF_VI_DPCL_DMA_SW_SPMUX        (0 << 4)
#define CIF_VI_DPCL_DMA_SW_SI            (1 << 4)
#define CIF_VI_DPCL_DMA_SW_IE            (2 << 4)
#define CIF_VI_DPCL_DMA_SW_JPEG            (3 << 4)
#define CIF_VI_DPCL_DMA_SW_ISP            (4 << 4)
#define CIF_VI_DPCL_IF_SEL_PARALLEL        (0 << 8)
#define CIF_VI_DPCL_IF_SEL_SMIA            (1 << 8)
#define CIF_VI_DPCL_IF_SEL_MIPI            (2 << 8)
#define CIF_VI_DPCL_DMA_IE_MUX_DMA        BIT(10)
#define CIF_VI_DPCL_DMA_SP_MUX_DMA        BIT(11)

/* ISP_IMSC - ISP_MIS - ISP_RIS - ISP_ICR - ISP_ISR */
#define CIF_ISP_OFF                BIT(0)
#define CIF_ISP_FRAME                BIT(1)
#define CIF_ISP_DATA_LOSS            BIT(2)
#define CIF_ISP_PIC_SIZE_ERROR            BIT(3)
#define CIF_ISP_AWB_DONE            BIT(4)
#define CIF_ISP_FRAME_IN            BIT(5)
#define CIF_ISP_V_START                BIT(6)
#define CIF_ISP_H_START                BIT(7)
#define CIF_ISP_FLASH_ON            BIT(8)
#define CIF_ISP_FLASH_OFF            BIT(9)
#define CIF_ISP_SHUTTER_ON            BIT(10)
#define CIF_ISP_SHUTTER_OFF            BIT(11)
#define CIF_ISP_AFM_SUM_OF            BIT(12)
#define CIF_ISP_AFM_LUM_OF            BIT(13)
#define CIF_ISP_AFM_FIN                BIT(14)
#define CIF_ISP_HIST_MEASURE_RDY        BIT(15)
#define CIF_ISP_FLASH_CAP            BIT(17)
#define CIF_ISP_EXP_END                BIT(18)
#define CIF_ISP_VSM_END                BIT(19)

/* ISP_ERR */
#define CIF_ISP_ERR_INFORM_SIZE            BIT(0)
#define CIF_ISP_ERR_IS_SIZE            BIT(1)
#define CIF_ISP_ERR_OUTFORM_SIZE        BIT(2)

/* MI_CTRL */
#define CIF_MI_CTRL_MP_ENABLE            (1 << 0)
#define CIF_MI_CTRL_SP_ENABLE            (2 << 0)
#define CIF_MI_CTRL_JPEG_ENABLE            (4 << 0)
#define CIF_MI_CTRL_RAW_ENABLE            (8 << 0)
#define CIF_MI_CTRL_HFLIP            BIT(4)
#define CIF_MI_CTRL_VFLIP            BIT(5)
#define CIF_MI_CTRL_ROT                BIT(6)
#define CIF_MI_BYTE_SWAP            BIT(7)
#define CIF_MI_SP_Y_FULL_YUV2RGB        BIT(8)
#define CIF_MI_SP_CBCR_FULL_YUV2RGB        BIT(9)
#define CIF_MI_SP_422NONCOSITEED        BIT(10)
#define CIF_MI_MP_PINGPONG_ENABLE        BIT(11)
#define CIF_MI_SP_PINGPONG_ENABLE        BIT(12)
#define CIF_MI_MP_AUTOUPDATE_ENABLE        BIT(13)
#define CIF_MI_SP_AUTOUPDATE_ENABLE        BIT(14)
#define CIF_MI_LAST_PIXEL_SIG_ENABLE        BIT(15)
#define CIF_MI_CTRL_BURST_LEN_LUM_4        (0 << 16)
#define CIF_MI_CTRL_BURST_LEN_LUM_8        (1 << 16)
#define CIF_MI_CTRL_BURST_LEN_LUM_16        (2 << 16)
#define CIF_MI_CTRL_BURST_LEN_CHROM_4        (0 << 18)
#define CIF_MI_CTRL_BURST_LEN_CHROM_8        (1 << 18)
#define CIF_MI_CTRL_BURST_LEN_CHROM_16        (2 << 18)
#define CIF_MI_CTRL_INIT_BASE_EN        BIT(20)
#define CIF_MI_CTRL_INIT_OFFSET_EN        BIT(21)
#define MI_CTRL_MP_WRITE_YUV_PLA_OR_RAW8    (0 << 22)
#define MI_CTRL_MP_WRITE_YUV_SPLA        (1 << 22)
#define MI_CTRL_MP_WRITE_YUVINT            (2 << 22)
#define MI_CTRL_MP_WRITE_RAW12            (2 << 22)
#define MI_CTRL_SP_WRITE_PLA            (0 << 24)
#define MI_CTRL_SP_WRITE_SPLA            (1 << 24)
#define MI_CTRL_SP_WRITE_INT            (2 << 24)
#define MI_CTRL_SP_INPUT_YUV400            (0 << 26)
#define MI_CTRL_SP_INPUT_YUV420            (1 << 26)
#define MI_CTRL_SP_INPUT_YUV422            (2 << 26)
#define MI_CTRL_SP_INPUT_YUV444            (3 << 26)
#define MI_CTRL_SP_OUTPUT_YUV400        (0 << 28)
#define MI_CTRL_SP_OUTPUT_YUV420        (1 << 28)
#define MI_CTRL_SP_OUTPUT_YUV422        (2 << 28)
#define MI_CTRL_SP_OUTPUT_YUV444        (3 << 28)
#define MI_CTRL_SP_OUTPUT_RGB565        (4 << 28)
#define MI_CTRL_SP_OUTPUT_RGB666        (5 << 28)
#define MI_CTRL_SP_OUTPUT_RGB888        (6 << 28)

#define MI_CTRL_MP_FMT_MASK            GENMASK(23, 22)
#define MI_CTRL_SP_FMT_MASK            GENMASK(30, 24)

/* MI_INIT */
#define CIF_MI_INIT_SKIP            BIT(2)
#define CIF_MI_INIT_SOFT_UPD            BIT(4)

/* MI_CTRL_SHD */
#define CIF_MI_CTRL_SHD_MP_IN_ENABLED        BIT(0)
#define CIF_MI_CTRL_SHD_SP_IN_ENABLED        BIT(1)
#define CIF_MI_CTRL_SHD_JPEG_IN_ENABLED        BIT(2)
#define CIF_MI_CTRL_SHD_RAW_IN_ENABLED        BIT(3)
#define CIF_MI_CTRL_SHD_MP_OUT_ENABLED        BIT(16)
#define CIF_MI_CTRL_SHD_SP_OUT_ENABLED        BIT(17)
#define CIF_MI_CTRL_SHD_JPEG_OUT_ENABLED    BIT(18)
#define CIF_MI_CTRL_SHD_RAW_OUT_ENABLED        BIT(19)

/* MI_CTRL2 */
#define CIF_MI_CTRL2_MIPI_RAW0_PINGPONG_EN    BIT(2)
#define CIF_MI_CTRL2_MIPI_RAW0_AUTO_UPDATE    BIT(1)
#define CIF_MI_CTRL2_MIPI_RAW0_ENABLE        BIT(0)

/* RSZ_CTRL */
#define CIF_RSZ_CTRL_SCALE_HY_ENABLE        BIT(0)
#define CIF_RSZ_CTRL_SCALE_HC_ENABLE        BIT(1)
#define CIF_RSZ_CTRL_SCALE_VY_ENABLE        BIT(2)
#define CIF_RSZ_CTRL_SCALE_VC_ENABLE        BIT(3)
#define CIF_RSZ_CTRL_SCALE_HY_UP        BIT(4)
#define CIF_RSZ_CTRL_SCALE_HC_UP        BIT(5)
#define CIF_RSZ_CTRL_SCALE_VY_UP        BIT(6)
#define CIF_RSZ_CTRL_SCALE_VC_UP        BIT(7)
#define CIF_RSZ_CTRL_CFG_UPD            BIT(8)
#define CIF_RSZ_CTRL_CFG_UPD_AUTO        BIT(9)
#define CIF_RSZ_SCALER_FACTOR            BIT(16)

/* MI_IMSC - MI_MIS - MI_RIS - MI_ICR - MI_ISR */
#define CIF_MI_FRAME(stream) ({ \
    typeof(stream) __stream = (stream); \
    !__stream->config ? 0 : \
    __stream->config->frame_end_id; \
})
#define CIF_MI_MP_FRAME                BIT(0)
#define CIF_MI_SP_FRAME                BIT(1)
#define CIF_MI_MBLK_LINE            BIT(2)
#define CIF_MI_FILL_MP_Y            BIT(3)
#define CIF_MI_WRAP_MP_Y            BIT(4)
#define CIF_MI_WRAP_MP_CB            BIT(5)
#define CIF_MI_WRAP_MP_CR            BIT(6)
#define CIF_MI_WRAP_SP_Y            BIT(7)
#define CIF_MI_WRAP_SP_CB            BIT(8)
#define CIF_MI_WRAP_SP_CR            BIT(9)
#define CIF_MI_DMA_READY            BIT(11)

/* MI_STATUS */
#define CIF_MI_STATUS_MP_Y_FIFO_FULL        BIT(0)
#define CIF_MI_STATUS_SP_Y_FIFO_FULL        BIT(4)

/* MI_DMA_CTRL */
#define CIF_MI_DMA_CTRL_BURST_LEN_LUM_4        (0 << 0)
#define CIF_MI_DMA_CTRL_BURST_LEN_LUM_8        BIT(0)
#define CIF_MI_DMA_CTRL_BURST_LEN_LUM_16    BIT(1)
#define CIF_MI_DMA_CTRL_BURST_LEN_CHROM_4    (0 << 2)
#define CIF_MI_DMA_CTRL_BURST_LEN_CHROM_8    BIT(2)
#define CIF_MI_DMA_CTRL_BURST_LEN_CHROM_16    BIT(3)
#define CIF_MI_DMA_CTRL_READ_FMT_PLANAR        (0 << 4)
#define CIF_MI_DMA_CTRL_READ_FMT_SPLANAR    (1 << 4)
#define CIF_MI_DMA_CTRL_READ_FMT_PACKED         (2 << 4)
#define CIF_MI_DMA_CTRL_FMT_YUV400        (0 << 6)
#define CIF_MI_DMA_CTRL_FMT_YUV420        (1 << 6)
#define CIF_MI_DMA_CTRL_FMT_YUV422        (2 << 6)
#define CIF_MI_DMA_CTRL_FMT_YUV444        (3 << 6)
#define CIF_MI_DMA_CTRL_BYTE_SWAP        BIT(8)
#define CIF_MI_DMA_CTRL_CONTINUOUS_ENA        BIT(9)
#define CIF_MI_DMA_CTRL_RGB_BAYER_NO        (0 << 12)
#define CIF_MI_DMA_CTRL_RGB_BAYER_8BIT        (1 << 12)
#define CIF_MI_DMA_CTRL_RGB_BAYER_16BIT        (2 << 12)
/* MI_DMA_START */
#define CIF_MI_DMA_START_ENABLE            BIT(0)
/* MI_XTD_FORMAT_CTRL  */
#define CIF_MI_XTD_FMT_CTRL_MP_CB_CR_SWAP    BIT(0)
#define CIF_MI_XTD_FMT_CTRL_SP_CB_CR_SWAP    BIT(1)
#define CIF_MI_XTD_FMT_CTRL_DMA_CB_CR_SWAP    BIT(2)

/* CCL */
#define CIF_CCL_CIF_CLK_DIS            BIT(2)
/* VI_ISP_CLK_CTRL */
#define CIF_CLK_CTRL_ISP_RAW            BIT(0)
#define CIF_CLK_CTRL_ISP_RGB            BIT(1)
#define CIF_CLK_CTRL_ISP_YUV            BIT(2)
#define CIF_CLK_CTRL_ISP_3A            BIT(3)
#define CIF_CLK_CTRL_MIPI_RAW            BIT(4)
#define CIF_CLK_CTRL_ISP_IE            BIT(5)
#define CIF_CLK_CTRL_RSZ_RAM            BIT(6)
#define CIF_CLK_CTRL_JPEG_RAM            BIT(7)
#define CIF_CLK_CTRL_ACLK_ISP            BIT(8)
#define CIF_CLK_CTRL_MI_IDC            BIT(9)
#define CIF_CLK_CTRL_MI_MP            BIT(10)
#define CIF_CLK_CTRL_MI_JPEG            BIT(11)
#define CIF_CLK_CTRL_MI_DP            BIT(12)
#define CIF_CLK_CTRL_MI_Y12            BIT(13)
#define CIF_CLK_CTRL_MI_SP            BIT(14)
#define CIF_CLK_CTRL_MI_RAW0            BIT(15)
#define CIF_CLK_CTRL_MI_RAW1            BIT(16)
#define CIF_CLK_CTRL_MI_READ            BIT(17)
#define CIF_CLK_CTRL_MI_RAWRD            BIT(18)
#define CIF_CLK_CTRL_CP                BIT(19)
#define CIF_CLK_CTRL_IE                BIT(20)
#define CIF_CLK_CTRL_SI                BIT(21)
#define CIF_CLK_CTRL_RSZM            BIT(22)
#define CIF_CLK_CTRL_DPMUX            BIT(23)
#define CIF_CLK_CTRL_JPEG            BIT(24)
#define CIF_CLK_CTRL_RSZS            BIT(25)
#define CIF_CLK_CTRL_MIPI            BIT(26)
#define CIF_CLK_CTRL_MARVINMI            BIT(27)
/* ICCL */
#define CIF_ICCL_ISP_CLK            BIT(0)
#define CIF_ICCL_CP_CLK                BIT(1)
#define CIF_ICCL_RES_2                BIT(2)
#define CIF_ICCL_MRSZ_CLK            BIT(3)
#define CIF_ICCL_SRSZ_CLK            BIT(4)
#define CIF_ICCL_JPEG_CLK            BIT(5)
#define CIF_ICCL_MI_CLK                BIT(6)
#define CIF_ICCL_RES_7                BIT(7)
#define CIF_ICCL_IE_CLK                BIT(8)
#define CIF_ICCL_SIMP_CLK            BIT(9)
#define CIF_ICCL_SMIA_CLK            BIT(10)
#define CIF_ICCL_MIPI_CLK            BIT(11)
#define CIF_ICCL_DCROP_CLK            BIT(12)
/* IRCL */
#define CIF_IRCL_ISP_SW_RST            BIT(0)
#define CIF_IRCL_CP_SW_RST            BIT(1)
#define CIF_IRCL_YCS_SW_RST            BIT(2)
#define CIF_IRCL_MRSZ_SW_RST            BIT(3)
#define CIF_IRCL_SRSZ_SW_RST            BIT(4)
#define CIF_IRCL_JPEG_SW_RST            BIT(5)
#define CIF_IRCL_MI_SW_RST            BIT(6)
#define CIF_IRCL_CIF_SW_RST            BIT(7)
#define CIF_IRCL_IE_SW_RST            BIT(8)
#define CIF_IRCL_SI_SW_RST            BIT(9)
#define CIF_IRCL_MIPI_SW_RST            BIT(11)

/* C_PROC_CTR */
#define CIF_C_PROC_CTR_ENABLE            BIT(0)
#define CIF_C_PROC_YOUT_FULL            BIT(1)
#define CIF_C_PROC_YIN_FULL            BIT(2)
#define CIF_C_PROC_COUT_FULL            BIT(3)
#define CIF_C_PROC_CTRL_RESERVED        0xFFFFFFFE
#define CIF_C_PROC_CONTRAST_RESERVED        0xFFFFFF00
#define CIF_C_PROC_BRIGHTNESS_RESERVED        0xFFFFFF00
#define CIF_C_PROC_HUE_RESERVED            0xFFFFFF00
#define CIF_C_PROC_SATURATION_RESERVED        0xFFFFFF00
#define CIF_C_PROC_MACC_RESERVED        0xE000E000
#define CIF_C_PROC_TONE_RESERVED        0xF000
/* DUAL_CROP_CTRL */
#define CIF_DUAL_CROP_MP_MODE_BYPASS        (0 << 0)
#define CIF_DUAL_CROP_MP_MODE_YUV        (1 << 0)
#define CIF_DUAL_CROP_MP_MODE_RAW        (2 << 0)
#define CIF_DUAL_CROP_SP_MODE_BYPASS        (0 << 2)
#define CIF_DUAL_CROP_SP_MODE_YUV        (1 << 2)
#define CIF_DUAL_CROP_SP_MODE_RAW        (2 << 2)
#define CIF_DUAL_CROP_CFG_UPD_PERMANENT        BIT(4)
#define CIF_DUAL_CROP_CFG_UPD            BIT(5)
#define CIF_DUAL_CROP_GEN_CFG_UPD        BIT(6)

/* IMG_EFF_CTRL */
#define CIF_IMG_EFF_CTRL_ENABLE            BIT(0)
#define CIF_IMG_EFF_CTRL_MODE_BLACKWHITE    (0 << 1)
#define CIF_IMG_EFF_CTRL_MODE_NEGATIVE        (1 << 1)
#define CIF_IMG_EFF_CTRL_MODE_SEPIA        (2 << 1)
#define CIF_IMG_EFF_CTRL_MODE_COLOR_SEL        (3 << 1)
#define CIF_IMG_EFF_CTRL_MODE_EMBOSS        (4 << 1)
#define CIF_IMG_EFF_CTRL_MODE_SKETCH        (5 << 1)
#define CIF_IMG_EFF_CTRL_MODE_SHARPEN        (6 << 1)
#define CIF_IMG_EFF_CTRL_MODE_RKSHARPEN        (7 << 1)
#define CIF_IMG_EFF_CTRL_CFG_UPD        BIT(4)
#define CIF_IMG_EFF_CTRL_YCBCR_FULL        BIT(5)

#define CIF_IMG_EFF_CTRL_MODE_BLACKWHITE_SHIFT    0
#define CIF_IMG_EFF_CTRL_MODE_NEGATIVE_SHIFT    1
#define CIF_IMG_EFF_CTRL_MODE_SEPIA_SHIFT    2
#define CIF_IMG_EFF_CTRL_MODE_COLOR_SEL_SHIFT    3
#define CIF_IMG_EFF_CTRL_MODE_EMBOSS_SHIFT    4
#define CIF_IMG_EFF_CTRL_MODE_SKETCH_SHIFT    5
#define CIF_IMG_EFF_CTRL_MODE_SHARPEN_SHIFT    6
#define CIF_IMG_EFF_CTRL_MODE_MASK        0xE

#define MAYBE_UNUSED                    __maybe_unused
#define CIF_ISP_EXP_OFFSET_SEV          7
#define CIF_ISP_EXP_OFFSET_TEN            10
#define CIF_ISP_EXP_OFFSET_TWE            12
#define CIF_ISP_EXP_OFFSET_FOR            14
#define CIF_ISP_EXP_OFFSET_THI            16
#define CIF_ISP_EXP_OFFSET_EIT            18
#define CIF_ISP_EXP_OFFSET_TW            20
#define CIF_ISP_EXP_OFFSET_TT            22
#define CIF_ISP_EXP_OFFSET_TWF            24

/* IMG_EFF_COLOR_SEL */
#define CIF_IMG_EFF_COLOR_RGB            0
#define CIF_IMG_EFF_COLOR_B            (1 << 0)
#define CIF_IMG_EFF_COLOR_G            (2 << 0)
#define CIF_IMG_EFF_COLOR_GB            (3 << 0)
#define CIF_IMG_EFF_COLOR_R            (4 << 0)
#define CIF_IMG_EFF_COLOR_RB            (5 << 0)
#define CIF_IMG_EFF_COLOR_RG            (6 << 0)
#define CIF_IMG_EFF_COLOR_RGB2            (7 << 0)

/* MIPI_CTRL */
#define CIF_MIPI_CTRL_OUTPUT_ENA        BIT(0)
#define CIF_MIPI_CTRL_FLUSH_FIFO        BIT(1)
#define CIF_MIPI_CTRL_SHUTDOWNLANES(a)        (((a) & 0xF) << 8)
#define CIF_MIPI_CTRL_NUM_LANES(a)        (((a) & 0x3) << 12)
#define CIF_MIPI_CTRL_ERR_SOT_HS_SKIP        BIT(16)
#define CIF_MIPI_CTRL_ERR_SOT_SYNC_HS_SKIP    BIT(17)
#define CIF_MIPI_CTRL_CLOCKLANE_ENA        BIT(18)

/* MIPI_DATA_SEL */
#define CIF_MIPI_DATA_SEL_VC(a)            (((a) & 0x3) << 6)
#define CIF_MIPI_DATA_SEL_DT(a)            (((a) & 0x3F) << 0)
/* MIPI DATA_TYPE */
#define CIF_CSI2_DT_EBD                0x12
#define CIF_CSI2_DT_YUV420_8b            0x18
#define CIF_CSI2_DT_YUV420_10b            0x19
#define CIF_CSI2_DT_YUV422_8b            0x1E
#define CIF_CSI2_DT_YUV422_10b            0x1F
#define CIF_CSI2_DT_RGB565            0x22
#define CIF_CSI2_DT_RGB666            0x23
#define CIF_CSI2_DT_RGB888            0x24
#define CIF_CSI2_DT_RAW8            0x2A
#define CIF_CSI2_DT_RAW10            0x2B
#define CIF_CSI2_DT_RAW12            0x2C
#define CIF_CSI2_DT_SPD                0x2F

/* MIPI_IMSC, MIPI_RIS, MIPI_MIS, MIPI_ICR, MIPI_ISR */
#define CIF_MIPI_SYNC_FIFO_OVFLW(a)        (((a) & 0xF) << 0)
#define CIF_MIPI_ERR_SOT(a)            (((a) & 0xF) << 4)
#define CIF_MIPI_ERR_SOT_SYNC(a)        (((a) & 0xF) << 8)
#define CIF_MIPI_ERR_EOT_SYNC(a)        (((a) & 0xF) << 12)
#define CIF_MIPI_ERR_CTRL(a)            (((a) & 0xF) << 16)
#define CIF_MIPI_ERR_PROTOCOL            BIT(20)
#define CIF_MIPI_ERR_ECC1            BIT(21)
#define CIF_MIPI_ERR_ECC2            BIT(22)
#define CIF_MIPI_ERR_CS                BIT(23)
#define CIF_MIPI_FRAME_END            BIT(24)
#define CIF_MIPI_ADD_DATA_OVFLW            BIT(25)
#define CIF_MIPI_ADD_DATA_WATER_MARK        BIT(26)

#define CIF_MIPI_ERR_CSI  (CIF_MIPI_ERR_PROTOCOL | \
    CIF_MIPI_ERR_ECC1 | \
    CIF_MIPI_ERR_ECC2 | \
    CIF_MIPI_ERR_CS)

#define CIF_MIPI_ERR_DPHY  (CIF_MIPI_ERR_SOT(0xF) | \
    CIF_MIPI_ERR_SOT_SYNC(0xF) | \
    CIF_MIPI_ERR_EOT_SYNC(0xF) | \
    CIF_MIPI_ERR_CTRL(0xF))

/* SUPER_IMPOSE */
#define CIF_SUPER_IMP_CTRL_NORMAL_MODE        BIT(0)
#define CIF_SUPER_IMP_CTRL_REF_IMG_MEM        BIT(1)
#define CIF_SUPER_IMP_CTRL_TRANSP_DIS        BIT(2)

/* ISP HISTOGRAM CALCULATION : ISP_HIST_PROP */
#define CIF_ISP_HIST_PROP_MODE_DIS_V10        (0 << 0)
#define CIF_ISP_HIST_PROP_MODE_RGB_V10        (1 << 0)
#define CIF_ISP_HIST_PROP_MODE_RED_V10        (2 << 0)
#define CIF_ISP_HIST_PROP_MODE_GREEN_V10    (3 << 0)
#define CIF_ISP_HIST_PROP_MODE_BLUE_V10        (4 << 0)
#define CIF_ISP_HIST_PROP_MODE_LUM_V10        (5 << 0)
#define CIF_ISP_HIST_PROP_MODE_MASK_V10        0x7
#define CIF_ISP_HIST_PREDIV_SET_V10(x)        (((x) & 0x7F) << 3)
#define CIF_ISP_HIST_WEIGHT_SET_V10(v0, v1, v2, v3)    \
                     (((v0) & 0x1F) | (((v1) & 0x1F) << 8)  |\
                     (((v2) & 0x1F) << 16) | \
                     (((v3) & 0x1F) << 24))

#define CIF_ISP_HIST_WINDOW_OFFSET_RESERVED_V10    0xFFFFF000
#define CIF_ISP_HIST_WINDOW_SIZE_RESERVED_V10    0xFFFFF800
#define CIF_ISP_HIST_WEIGHT_RESERVED_V10    0xE0E0E0E0
#define CIF_ISP_MAX_HIST_PREDIVIDER_V10        0x0000007F
#define CIF_ISP_HIST_ROW_NUM_V10        5
#define CIF_ISP_HIST_COLUMN_NUM_V10        5

/* ISP HISTOGRAM CALCULATION : CIF_ISP_HIST */
#define CIF_ISP_HIST_CTRL_EN_SET_V12(x)        (((x) & 0x01) << 0)
#define CIF_ISP_HIST_CTRL_EN_MASK_V12        CIF_ISP_HIST_CTRL_EN_SET_V12(0x01)
#define CIF_ISP_HIST_CTRL_STEPSIZE_SET_V12(x)    (((x) & 0x7F) << 1)
#define CIF_ISP_HIST_CTRL_MODE_SET_V12(x)    (((x) & 0x07) << 8)
#define CIF_ISP_HIST_CTRL_MODE_MASK_V12        CIF_ISP_HIST_CTRL_MODE_SET_V12(0x07)
#define CIF_ISP_HIST_CTRL_AUTOSTOP_SET_V12(x)    (((x) & 0x01) << 11)
#define CIF_ISP_HIST_CTRL_WATERLINE_SET_V12(x)    (((x) & 0xFFF) << 12)
#define CIF_ISP_HIST_CTRL_DATASEL_SET_V12(x)    (((x) & 0x07) << 24)
#define CIF_ISP_HIST_CTRL_INTRSEL_SET_V12(x)    (((x) & 0x01) << 27)
#define CIF_ISP_HIST_CTRL_WNDNUM_SET_V12(x)    (((x) & 0x03) << 28)
#define CIF_ISP_HIST_CTRL_DBGEN_SET_V12(x)    (((x) & 0x01) << 30)
#define CIF_ISP_HIST_ROW_NUM_V12        15
#define CIF_ISP_HIST_COLUMN_NUM_V12        15
#define CIF_ISP_HIST_WEIGHT_REG_SIZE_V12    \
                (CIF_ISP_HIST_ROW_NUM_V12 * CIF_ISP_HIST_COLUMN_NUM_V12)

#define CIF_ISP_HIST_WEIGHT_SET_V12(v0, v1, v2, v3)    \
                (((v0) & 0x3F) | (((v1) & 0x3F) << 8) |\
                (((v2) & 0x3F) << 16) |\
                (((v3) & 0x3F) << 24))

#define CIF_ISP_HIST_OFFS_SET_V12(v0, v1)    \
                (((v0) & 0x1FFF) | (((v1) & 0x1FFF) << 16))
#define CIF_ISP_HIST_SIZE_SET_V12(v0, v1)    \
                (((v0) & 0x7FF) | (((v1) & 0x7FF) << 16))

#define CIF_ISP_HIST_GET_BIN0_V12(x)    \
                ((x) & 0xFFFF)
#define CIF_ISP_HIST_GET_BIN1_V12(x)    \
                (((x) >> 16) & 0xFFFF)

/* AUTO FOCUS MEASUREMENT:  ISP_AFM_CTRL */
#define ISP_AFM_CTRL_ENABLE            BIT(0)

/* SHUTTER CONTROL */
#define CIF_ISP_SH_CTRL_SH_ENA            BIT(0)
#define CIF_ISP_SH_CTRL_REP_EN            BIT(1)
#define CIF_ISP_SH_CTRL_SRC_SH_TRIG        BIT(2)
#define CIF_ISP_SH_CTRL_EDGE_POS        BIT(3)
#define CIF_ISP_SH_CTRL_POL_LOW            BIT(4)

/* FLASH MODULE */
/* ISP_FLASH_CMD */
#define CIF_FLASH_CMD_PRELIGHT_ON        BIT(0)
#define CIF_FLASH_CMD_FLASH_ON            BIT(1)
#define CIF_FLASH_CMD_PRE_FLASH_ON        BIT(2)
/* ISP_FLASH_CONFIG */
#define CIF_FLASH_CONFIG_PRELIGHT_END        BIT(0)
#define CIF_FLASH_CONFIG_VSYNC_POS        BIT(1)
#define CIF_FLASH_CONFIG_PRELIGHT_LOW        BIT(2)
#define CIF_FLASH_CONFIG_SRC_FL_TRIG        BIT(3)
#define CIF_FLASH_CONFIG_DELAY(a)        (((a) & 0xF) << 4)

/* Demosaic:  ISP_DEMOSAIC */
#define CIF_ISP_DEMOSAIC_BYPASS            BIT(10)
#define CIF_ISP_DEMOSAIC_TH(x)            ((x) & 0xFF)

/* AWB */
/* ISP_AWB_PROP */
#define CIF_ISP_AWB_YMAX_CMP_EN            BIT(2)
#define CIF_ISP_AWB_YMAX_READ(x)        (((x) >> 2) & 1)
#define CIF_ISP_AWB_MODE_RGB_EN            ((1 << 31) | (0x2 << 0))
#define CIF_ISP_AWB_MODE_YCBCR_EN        ((0 << 31) | (0x2 << 0))
#define CIF_ISP_AWB_MODE_RGB            BIT(31)
#define CIF_ISP_AWB_ENABLE            (0x2 << 0)
#define CIF_ISP_AWB_MODE_MASK_NONE        0xFFFFFFFC
#define CIF_ISP_AWB_MODE_READ(x)        ((x) & 3)
#define CIF_ISP_AWB_SET_FRAMES_V12(x)        (((x) & 0x07) << 28)
#define CIF_ISP_AWB_SET_FRAMES_MASK_V12        CIF_ISP_AWB_SET_FRAMES_V12(0x07)
/* ISP_AWB_GAIN_RB, ISP_AWB_GAIN_G  */
#define CIF_ISP_AWB_GAIN_R_SET(x)        (((x) & 0x3FF) << 16)
#define CIF_ISP_AWB_GAIN_R_READ(x)        (((x) >> 16) & 0x3FF)
#define CIF_ISP_AWB_GAIN_B_SET(x)        ((x) & 0x3FF)
#define CIF_ISP_AWB_GAIN_B_READ(x)        ((x) & 0x3FF)
/* ISP_AWB_REF */
#define CIF_ISP_AWB_REF_CR_SET(x)        (((x) & 0xFF) << 8)
#define CIF_ISP_AWB_REF_CR_READ(x)        (((x) >> 8) & 0xFF)
#define CIF_ISP_AWB_REF_CB_READ(x)        ((x) & 0xFF)
/* ISP_AWB_THRESH */
#define CIF_ISP_AWB_MAX_CS_SET(x)        (((x) & 0xFF) << 8)
#define CIF_ISP_AWB_MAX_CS_READ(x)        (((x) >> 8) & 0xFF)
#define CIF_ISP_AWB_MIN_C_READ(x)        ((x) & 0xFF)
#define CIF_ISP_AWB_MIN_Y_SET(x)        (((x) & 0xFF) << 16)
#define CIF_ISP_AWB_MIN_Y_READ(x)        (((x) >> 16) & 0xFF)
#define CIF_ISP_AWB_MAX_Y_SET(x)        (((x) & 0xFF) << 24)
#define CIF_ISP_AWB_MAX_Y_READ(x)        (((x) >> 24) & 0xFF)
/* ISP_AWB_MEAN */
#define CIF_ISP_AWB_GET_MEAN_CR_R(x)        ((x) & 0xFF)
#define CIF_ISP_AWB_GET_MEAN_CB_B(x)        (((x) >> 8) & 0xFF)
#define CIF_ISP_AWB_GET_MEAN_Y_G(x)        (((x) >> 16) & 0xFF)
/* ISP_AWB_WHITE_CNT */
#define CIF_ISP_AWB_GET_PIXEL_CNT(x)        ((x) & 0x3FFFFFF)

#define CIF_ISP_AWB_GAINS_MAX_VAL        0x000003FF
#define CIF_ISP_AWB_WINDOW_OFFSET_MAX        0x00000FFF
#define CIF_ISP_AWB_WINDOW_MAX_SIZE        0x00001FFF
#define CIF_ISP_AWB_CBCR_MAX_REF        0x000000FF
#define CIF_ISP_AWB_THRES_MAX_YC        0x000000FF

/* AE */
/* ISP_EXP_CTRL */
#define CIF_ISP_EXP_ENA                BIT(0)
#define CIF_ISP_EXP_CTRL_AUTOSTOP        BIT(1)
#define CIF_ISP_EXP_CTRL_WNDNUM_SET_V12(x)    (((x) & 0x03) << 2)
/*
 *'1' luminance calculation according to  Y=(R+G+B) x 0.332 (85/256)
 *'0' luminance calculation according to Y=16+0.25R+0.5G+0.1094B
 */
#define CIF_ISP_EXP_CTRL_MEASMODE_1        BIT(31)

/* ISP_EXP_H_SIZE */
#define CIF_ISP_EXP_H_SIZE_SET_V10(x)        ((x) & 0x7FF)
#define CIF_ISP_EXP_HEIGHT_MASK_V10        0x000007FF
/* ISP_EXP_V_SIZE : vertical size must be a multiple of 2). */
#define CIF_ISP_EXP_V_SIZE_SET_V10(x)        ((x) & 0x7FE)

/* ISP_EXP_H_OFFSET */
#define CIF_ISP_EXP_H_OFFSET_SET_V10(x)        ((x) & 0x1FFF)
#define CIF_ISP_EXP_MAX_HOFFS_V10        2424
/* ISP_EXP_V_OFFSET */
#define CIF_ISP_EXP_V_OFFSET_SET_V10(x)        ((x) & 0x1FFF)
#define CIF_ISP_EXP_MAX_VOFFS_V10        1806

#define CIF_ISP_EXP_ROW_NUM_V10            5
#define CIF_ISP_EXP_COLUMN_NUM_V10        5
#define CIF_ISP_EXP_NUM_LUMA_REGS_V10 \
    (CIF_ISP_EXP_ROW_NUM_V10 * CIF_ISP_EXP_COLUMN_NUM_V10)
#define CIF_ISP_EXP_BLOCK_MAX_HSIZE_V10        516
#define CIF_ISP_EXP_BLOCK_MIN_HSIZE_V10        35
#define CIF_ISP_EXP_BLOCK_MAX_VSIZE_V10        390
#define CIF_ISP_EXP_BLOCK_MIN_VSIZE_V10        28
#define CIF_ISP_EXP_MAX_HSIZE_V10    \
    (CIF_ISP_EXP_BLOCK_MAX_HSIZE_V10 * CIF_ISP_EXP_COLUMN_NUM_V10 + 1)
#define CIF_ISP_EXP_MIN_HSIZE_V10    \
    (CIF_ISP_EXP_BLOCK_MIN_HSIZE_V10 * CIF_ISP_EXP_COLUMN_NUM_V10 + 1)
#define CIF_ISP_EXP_MAX_VSIZE_V10    \
    (CIF_ISP_EXP_BLOCK_MAX_VSIZE_V10 * CIF_ISP_EXP_ROW_NUM_V10 + 1)
#define CIF_ISP_EXP_MIN_VSIZE_V10    \
    (CIF_ISP_EXP_BLOCK_MIN_VSIZE_V10 * CIF_ISP_EXP_ROW_NUM_V10 + 1)

/* ISP_EXP_H_SIZE */
#define CIF_ISP_EXP_H_SIZE_SET_V12(x)        ((x) & 0x7FF)
#define CIF_ISP_EXP_HEIGHT_MASK_V12        0x000007FF
/* ISP_EXP_V_SIZE : vertical size must be a multiple of 2). */
#define CIF_ISP_EXP_V_SIZE_SET_V12(x)        (((x) & 0x7FE) << 16)

/* ISP_EXP_H_OFFSET */
#define CIF_ISP_EXP_H_OFFSET_SET_V12(x)        ((x) & 0x1FFF)
#define CIF_ISP_EXP_MAX_HOFFS_V12        0x1FFF
/* ISP_EXP_V_OFFSET */
#define CIF_ISP_EXP_V_OFFSET_SET_V12(x)        (((x) & 0x1FFF) << 16)
#define CIF_ISP_EXP_MAX_VOFFS_V12        0x1FFF

#define CIF_ISP_EXP_ROW_NUM_V12            15
#define CIF_ISP_EXP_COLUMN_NUM_V12        15
#define CIF_ISP_EXP_NUM_LUMA_REGS_V12 \
    (CIF_ISP_EXP_ROW_NUM_V12 * CIF_ISP_EXP_COLUMN_NUM_V12)
#define CIF_ISP_EXP_BLOCK_MAX_HSIZE_V12        0x7FF
#define CIF_ISP_EXP_BLOCK_MIN_HSIZE_V12        0xE
#define CIF_ISP_EXP_BLOCK_MAX_VSIZE_V12        0x7FE
#define CIF_ISP_EXP_BLOCK_MIN_VSIZE_V12        0xE
#define CIF_ISP_EXP_MAX_HSIZE_V12    \
    (CIF_ISP_EXP_BLOCK_MAX_HSIZE_V12 * CIF_ISP_EXP_COLUMN_NUM_V12 + 1)
#define CIF_ISP_EXP_MIN_HSIZE_V12    \
    (CIF_ISP_EXP_BLOCK_MIN_HSIZE_V12 * CIF_ISP_EXP_COLUMN_NUM_V12 + 1)
#define CIF_ISP_EXP_MAX_VSIZE_V12    \
    (CIF_ISP_EXP_BLOCK_MAX_VSIZE_V12 * CIF_ISP_EXP_ROW_NUM_V12 + 1)
#define CIF_ISP_EXP_MIN_VSIZE_V12    \
    (CIF_ISP_EXP_BLOCK_MIN_VSIZE_V12 * CIF_ISP_EXP_ROW_NUM_V12 + 1)

#define CIF_ISP_EXP_GET_MEAN_xy0_V12(x)        ((x) & 0xFF)
#define CIF_ISP_EXP_GET_MEAN_xy1_V12(x)        (((x) >> 8) & 0xFF)
#define CIF_ISP_EXP_GET_MEAN_xy2_V12(x)        (((x) >> 16) & 0xFF)
#define CIF_ISP_EXP_GET_MEAN_xy3_V12(x)        (((x) >> 24) & 0xFF)

/* LSC: ISP_LSC_CTRL */
#define CIF_ISP_LSC_CTRL_ENA            BIT(0)
#define CIF_ISP_LSC_SECT_SIZE_RESERVED        0xFC00FC00
#define CIF_ISP_LSC_GRAD_RESERVED_V10        0xF000F000
#define CIF_ISP_LSC_SAMPLE_RESERVED_V10        0xF000F000
#define CIF_ISP_LSC_GRAD_RESERVED_V12        0xE000E000
#define CIF_ISP_LSC_SAMPLE_RESERVED_V12        0xE000E000
#define CIF_ISP_LSC_SECTORS_MAX            17
#define CIF_ISP_LSC_TABLE_DATA_V10(v0, v1)     \
    (((v0) & 0xFFF) | (((v1) & 0xFFF) << 12))
#define CIF_ISP_LSC_TABLE_DATA_V12(v0, v1)     \
    (((v0) & 0x1FFF) | (((v1) & 0x1FFF) << 13))
#define CIF_ISP_LSC_SECT_SIZE(v0, v1)      \
    (((v0) & 0xFFF) | (((v1) & 0xFFF) << 16))
#define CIF_ISP_LSC_GRAD_SIZE(v0, v1)      \
    (((v0) & 0xFFF) | (((v1) & 0xFFF) << 16))

/* LSC: ISP_LSC_TABLE_SEL */
#define CIF_ISP_LSC_TABLE_0            0
#define CIF_ISP_LSC_TABLE_1            1

/* LSC: ISP_LSC_STATUS */
#define CIF_ISP_LSC_ACTIVE_TABLE        BIT(1)
#define CIF_ISP_LSC_TABLE_ADDRESS_0        0
#define CIF_ISP_LSC_TABLE_ADDRESS_153        153

/* FLT */
/* ISP_FILT_MODE */
#define CIF_ISP_FLT_ENA                BIT(0)

/*
 * 0: green filter static mode (active filter factor = FILT_FAC_MID)
 * 1: dynamic noise reduction/sharpen Default
 */
#define CIF_ISP_FLT_MODE_DNR            BIT(1)
#define CIF_ISP_FLT_MODE_MAX            1
#define CIF_ISP_FLT_CHROMA_V_MODE(x)        (((x) & 0x3) << 4)
#define CIF_ISP_FLT_CHROMA_H_MODE(x)        (((x) & 0x3) << 6)
#define CIF_ISP_FLT_CHROMA_MODE_MAX        3
#define CIF_ISP_FLT_GREEN_STAGE1(x)        (((x) & 0xF) << 8)
#define CIF_ISP_FLT_GREEN_STAGE1_MAX        8
#define CIF_ISP_FLT_THREAD_RESERVED        0xFFFFFC00
#define CIF_ISP_FLT_FAC_RESERVED        0xFFFFFFC0
#define CIF_ISP_FLT_LUM_WEIGHT_RESERVED        0xFFF80000

#define CIF_ISP_CTK_COEFF_RESERVED        0xFFFFF800
#define CIF_ISP_XTALK_OFFSET_RESERVED        0xFFFFF000

#define CIF_ISP_FLT_LEVEL_OLD_LP        BIT(16)

/* GOC */
#define CIF_ISP_GAMMA_OUT_MODE_EQU        BIT(0)
#define CIF_ISP_GOC_MODE_MAX            1
#define CIF_ISP_GOC_RESERVED            0xFFFFF800
/* ISP_CTRL BIT 11*/
#define CIF_ISP_CTRL_ISP_GAMMA_OUT_ENA_READ(x)    (((x) >> 11) & 1)

/* DPCC */
/* ISP_DPCC_MODE */
#define CIF_ISP_DPCC_ENA            BIT(0)
#define CIF_ISP_DPCC_MODE_MAX            0x07
#define CIF_ISP_DPCC_OUTPUTMODE_MAX        0x0F
#define CIF_ISP_DPCC_SETUSE_MAX            0x0F
#define CIF_ISP_DPCC_METHODS_SET_RESERVED    0xFFFFE000
#define CIF_ISP_DPCC_LINE_THRESH_RESERVED    0xFFFF0000
#define CIF_ISP_DPCC_LINE_MAD_FAC_RESERVED    0xFFFFC0C0
#define CIF_ISP_DPCC_PG_FAC_RESERVED        0xFFFFC0C0
#define CIF_ISP_DPCC_RND_THRESH_RESERVED    0xFFFF0000
#define CIF_ISP_DPCC_RG_FAC_RESERVED        0xFFFFC0C0
#define CIF_ISP_DPCC_RO_LIMIT_RESERVED        0xFFFFF000
#define CIF_ISP_DPCC_RND_OFFS_RESERVED        0xFFFFF000

/* BLS */
/* ISP_BLS_CTRL */
#define CIF_ISP_BLS_ENA                BIT(0)
#define CIF_ISP_BLS_MODE_MEASURED        BIT(1)
#define CIF_ISP_BLS_MODE_FIXED            0
#define CIF_ISP_BLS_WINDOW_1            (1 << 2)
#define CIF_ISP_BLS_WINDOW_2            (2 << 2)

/* GAMMA-IN */
#define CIFISP_DEGAMMA_X_RESERVED    \
    ((1 << 31) | (1 << 27) | (1 << 23) | (1 << 19) |\
    (1 << 15) | (1 << 11) | (1 << 7) | (1 << 3))
#define CIFISP_DEGAMMA_Y_RESERVED               0xFFFFF000

/* GAMMA-OUT */
#define CIF_ISP_GAMMA_REG_VALUE_V12(x, y)    \
    (((x) & 0xFFF) << 16 | ((y) & 0xFFF) << 0)

/* AFM */
#define CIF_ISP_AFM_ENA                BIT(0)
#define CIF_ISP_AFM_THRES_RESERVED        0xFFFF0000
#define CIF_ISP_AFM_VAR_SHIFT_RESERVED        0xFFF8FFF8
#define CIF_ISP_AFM_WINDOW_X_RESERVED        0xE000
#define CIF_ISP_AFM_WINDOW_Y_RESERVED        0xF000
#define CIF_ISP_AFM_WINDOW_X_MIN        0x5
#define CIF_ISP_AFM_WINDOW_Y_MIN        0x2
#define CIF_ISP_AFM_WINDOW_X(x)            (((x) & 0x1FFF) << 16)
#define CIF_ISP_AFM_WINDOW_Y(x)            ((x) & 0x1FFF)
#define CIF_ISP_AFM_SET_SHIFT_a_V12(x, y)    (((x) & 0x7) << 16 | ((y) & 0x7) << 0)
#define CIF_ISP_AFM_SET_SHIFT_b_V12(x, y)    (((x) & 0x7) << 20 | ((y) & 0x7) << 4)
#define CIF_ISP_AFM_SET_SHIFT_c_V12(x, y)    (((x) & 0x7) << 24 | ((y) & 0x7) << 8)
#define CIF_ISP_AFM_GET_LUM_SHIFT_a_V12(x)    (((x) & 0x70000) >> 16)
#define CIF_ISP_AFM_GET_AFM_SHIFT_a_V12(x)    ((x) & 0x7)

/* DPF */
#define CIF_ISP_DPF_MODE_EN            BIT(0)
#define CIF_ISP_DPF_MODE_B_FLT_DIS        BIT(1)
#define CIF_ISP_DPF_MODE_GB_FLT_DIS        BIT(2)
#define CIF_ISP_DPF_MODE_GR_FLT_DIS        BIT(3)
#define CIF_ISP_DPF_MODE_R_FLT_DIS        BIT(4)
#define CIF_ISP_DPF_MODE_RB_FLTSIZE_9x9        BIT(5)
#define CIF_ISP_DPF_MODE_NLL_SEGMENTATION    BIT(6)
#define CIF_ISP_DPF_MODE_AWB_GAIN_COMP        BIT(7)
#define CIF_ISP_DPF_MODE_LSC_GAIN_COMP        BIT(8)
#define CIF_ISP_DPF_MODE_USE_NF_GAIN        BIT(9)
#define CIF_ISP_DPF_NF_GAIN_RESERVED        0xFFFFF000
#define CIF_ISP_DPF_SPATIAL_COEFF_MAX        0x1F
#define CIF_ISP_DPF_NLL_COEFF_N_MAX        0x3FF

/* CSI0 */
#define CIF_ISP_CSI0_IMASK_LINECNT        BIT(12)
#define CIF_ISP_CSI0_IMASK_RAW1_OUT_V_END    BIT(11)
#define CIF_ISP_CSI0_IMASK_RAW0_OUT_V_END    BIT(10)
#define CIF_ISP_CSI0_IMASK_FRAME_END(a)        (((a) & 0x3F) << 0)

#define CIF_ISP_CSI0_IMASK2_PHY_ERRSOTHS(a)    (((a) & 0x0F) << 4)
#define CIF_ISP_CSI0_IMASK2_PHY_ERRCONTROL(a)    (((a) & 0x0F) << 16)
#define CIF_ISP_CSI0_IMASK1_PHY_ERRSOTSYNC(a)    (((a) & 0x0F) << 8)
#define CIF_ISP_CSI0_IMASK1_PHY_ERREOTSYNC(a)    (((a) & 0x0F) << 4)

#define CIF_ISP_CSI0_DMATX0_VC(a)        (((a) & 0xFF) << 8)
#define CIF_ISP_CSI0_DMATX0_SIMG_SWP        BIT(2)
#define CIF_ISP_CSI0_DMATX0_SIMG_MODE        BIT(1)
#define CIF_ISP_CSI0_DMATX0_EN            BIT(0)

/* =================================================================== */
/*                            CIF Registers                            */
/* =================================================================== */
#define CIF_CTRL_BASE            0x00000000
#define CIF_CCL                (CIF_CTRL_BASE + 0x00000000)
#define CIF_VI_ID            (CIF_CTRL_BASE + 0x00000008)
#define CIF_VI_ISP_CLK_CTRL_V12        (CIF_CTRL_BASE + 0x0000000C)
#define CIF_ICCL            (CIF_CTRL_BASE + 0x00000010)
#define CIF_IRCL            (CIF_CTRL_BASE + 0x00000014)
#define CIF_VI_DPCL            (CIF_CTRL_BASE + 0x00000018)

#define CIF_IMG_EFF_BASE        0x00000200
#define CIF_IMG_EFF_CTRL        (CIF_IMG_EFF_BASE + 0x00000000)
#define CIF_IMG_EFF_COLOR_SEL        (CIF_IMG_EFF_BASE + 0x00000004)
#define CIF_IMG_EFF_MAT_1        (CIF_IMG_EFF_BASE + 0x00000008)
#define CIF_IMG_EFF_MAT_2        (CIF_IMG_EFF_BASE + 0x0000000C)
#define CIF_IMG_EFF_MAT_3        (CIF_IMG_EFF_BASE + 0x00000010)
#define CIF_IMG_EFF_MAT_4        (CIF_IMG_EFF_BASE + 0x00000014)
#define CIF_IMG_EFF_MAT_5        (CIF_IMG_EFF_BASE + 0x00000018)
#define CIF_IMG_EFF_TINT        (CIF_IMG_EFF_BASE + 0x0000001C)
#define CIF_IMG_EFF_CTRL_SHD        (CIF_IMG_EFF_BASE + 0x00000020)
#define CIF_IMG_EFF_SHARPEN        (CIF_IMG_EFF_BASE + 0x00000024)

#define CIF_RKSHARP_CTRL        (CIF_IMG_EFF_BASE + 0x00000030)
#define CIF_RKSHARP_YAVG_THR        (CIF_IMG_EFF_BASE + 0x00000034)
#define CIF_RKSHARP_DELTA_P0_P1        (CIF_IMG_EFF_BASE + 0x00000038)
#define CIF_RKSHARP_DELTA_P2_P3        (CIF_IMG_EFF_BASE + 0x0000003c)
#define CIF_RKSHARP_DELTA_P4        (CIF_IMG_EFF_BASE + 0x00000040)
#define CIF_RKSHARP_NPIXEL_P0_P1_P2_P3    (CIF_IMG_EFF_BASE + 0x00000044)
#define CIF_RKSHARP_NPIXEL_P4        (CIF_IMG_EFF_BASE + 0x00000048)
#define CIF_RKSHARP_GAUSS_FLAT_COE1    (CIF_IMG_EFF_BASE + 0x0000004c)
#define CIF_RKSHARP_GAUSS_FLAT_COE2    (CIF_IMG_EFF_BASE + 0x00000050)
#define CIF_RKSHARP_GAUSS_FLAT_COE3    (CIF_IMG_EFF_BASE + 0x00000054)
#define CIF_RKSHARP_GAUSS_NOISE_COE1    (CIF_IMG_EFF_BASE + 0x00000058)
#define CIF_RKSHARP_GAUSS_NOISE_COE2    (CIF_IMG_EFF_BASE + 0x0000005c)
#define CIF_RKSHARP_GAUSS_NOISE_COE3    (CIF_IMG_EFF_BASE + 0x00000060)
#define CIF_RKSHARP_GAUSS_OTHER_COE1    (CIF_IMG_EFF_BASE + 0x00000064)
#define CIF_RKSHARP_GAUSS_OTHER_COE2    (CIF_IMG_EFF_BASE + 0x00000068)
#define CIF_RKSHARP_GAUSS_OTHER_COE3    (CIF_IMG_EFF_BASE + 0x0000006c)
#define CIF_RKSHARP_LINE1_FILTER_COE1    (CIF_IMG_EFF_BASE + 0x00000070)
#define CIF_RKSHARP_LINE1_FILTER_COE2    (CIF_IMG_EFF_BASE + 0x00000074)
#define CIF_RKSHARP_LINE2_FILTER_COE1    (CIF_IMG_EFF_BASE + 0x00000078)
#define CIF_RKSHARP_LINE2_FILTER_COE2    (CIF_IMG_EFF_BASE + 0x0000007c)
#define CIF_RKSHARP_LINE2_FILTER_COE3    (CIF_IMG_EFF_BASE + 0x00000080)
#define CIF_RKSHARP_LINE3_FILTER_COE1    (CIF_IMG_EFF_BASE + 0x00000084)
#define CIF_RKSHARP_LINE3_FILTER_COE2    (CIF_IMG_EFF_BASE + 0x00000088)
#define CIF_RKSHARP_GRAD_SEQ_P0_P1    (CIF_IMG_EFF_BASE + 0x0000008c)
#define CIF_RKSHARP_GRAD_SEQ_P2_P3    (CIF_IMG_EFF_BASE + 0x00000090)
#define CIF_RKSHARP_SHARP_FACTOR_P0_P1_P2    (CIF_IMG_EFF_BASE + 0x00000094)
#define CIF_RKSHARP_SHARP_FACTOR_P3_P4        (CIF_IMG_EFF_BASE + 0x00000098)
#define CIF_RKSHARP_UV_GAUSS_FLAT_COE11_COE14    (CIF_IMG_EFF_BASE + 0x0000009c)
#define CIF_RKSHARP_UV_GAUSS_FLAT_COE15_COE23    (CIF_IMG_EFF_BASE + 0x000000a0)
#define CIF_RKSHARP_UV_GAUSS_FLAT_COE24_COE32    (CIF_IMG_EFF_BASE + 0x000000a4)
#define CIF_RKSHARP_UV_GAUSS_FLAT_COE33_COE35    (CIF_IMG_EFF_BASE + 0x000000a8)
#define CIF_RKSHARP_UV_GAUSS_NOISE_COE11_COE14    (CIF_IMG_EFF_BASE + 0x000000ac)
#define CIF_RKSHARP_UV_GAUSS_NOISE_COE15_COE23    (CIF_IMG_EFF_BASE + 0x000000b0)
#define CIF_RKSHARP_UV_GAUSS_NOISE_COE24_COE32    (CIF_IMG_EFF_BASE + 0x000000b4)
#define CIF_RKSHARP_UV_GAUSS_NOISE_COE33_COE35    (CIF_IMG_EFF_BASE + 0x000000b8)
#define CIF_RKSHARP_UV_GAUSS_OTHER_COE11_COE14    (CIF_IMG_EFF_BASE + 0x000000bc)
#define CIF_RKSHARP_UV_GAUSS_OTHER_COE15_COE23    (CIF_IMG_EFF_BASE + 0x000000c0)
#define CIF_RKSHARP_UV_GAUSS_OTHER_COE24_COE32    (CIF_IMG_EFF_BASE + 0x000000c4)
#define CIF_RKSHARP_UV_GAUSS_OTHER_COE33_COE35    (CIF_IMG_EFF_BASE + 0x000000c8)

#define CIF_SUPER_IMP_BASE        0x00000300
#define CIF_SUPER_IMP_CTRL        (CIF_SUPER_IMP_BASE + 0x00000000)
#define CIF_SUPER_IMP_OFFSET_X        (CIF_SUPER_IMP_BASE + 0x00000004)
#define CIF_SUPER_IMP_OFFSET_Y        (CIF_SUPER_IMP_BASE + 0x00000008)
#define CIF_SUPER_IMP_COLOR_Y        (CIF_SUPER_IMP_BASE + 0x0000000C)
#define CIF_SUPER_IMP_COLOR_CB        (CIF_SUPER_IMP_BASE + 0x00000010)
#define CIF_SUPER_IMP_COLOR_CR        (CIF_SUPER_IMP_BASE + 0x00000014)

#define CIF_ISP_BASE            0x00000400
#define CIF_ISP_CTRL            (CIF_ISP_BASE + 0x00000000)
#define CIF_ISP_ACQ_PROP        (CIF_ISP_BASE + 0x00000004)
#define CIF_ISP_ACQ_H_OFFS        (CIF_ISP_BASE + 0x00000008)
#define CIF_ISP_ACQ_V_OFFS        (CIF_ISP_BASE + 0x0000000C)
#define CIF_ISP_ACQ_H_SIZE        (CIF_ISP_BASE + 0x00000010)
#define CIF_ISP_ACQ_V_SIZE        (CIF_ISP_BASE + 0x00000014)
#define CIF_ISP_ACQ_NR_FRAMES        (CIF_ISP_BASE + 0x00000018)
#define CIF_ISP_GAMMA_DX_LO        (CIF_ISP_BASE + 0x0000001C)
#define CIF_ISP_GAMMA_DX_HI        (CIF_ISP_BASE + 0x00000020)
#define CIF_ISP_GAMMA_R_Y0        (CIF_ISP_BASE + 0x00000024)
#define CIF_ISP_GAMMA_R_Y1        (CIF_ISP_BASE + 0x00000028)
#define CIF_ISP_GAMMA_R_Y2        (CIF_ISP_BASE + 0x0000002C)
#define CIF_ISP_GAMMA_R_Y3        (CIF_ISP_BASE + 0x00000030)
#define CIF_ISP_GAMMA_R_Y4        (CIF_ISP_BASE + 0x00000034)
#define CIF_ISP_GAMMA_R_Y5        (CIF_ISP_BASE + 0x00000038)
#define CIF_ISP_GAMMA_R_Y6        (CIF_ISP_BASE + 0x0000003C)
#define CIF_ISP_GAMMA_R_Y7        (CIF_ISP_BASE + 0x00000040)
#define CIF_ISP_GAMMA_R_Y8        (CIF_ISP_BASE + 0x00000044)
#define CIF_ISP_GAMMA_R_Y9        (CIF_ISP_BASE + 0x00000048)
#define CIF_ISP_GAMMA_R_Y10        (CIF_ISP_BASE + 0x0000004C)
#define CIF_ISP_GAMMA_R_Y11        (CIF_ISP_BASE + 0x00000050)
#define CIF_ISP_GAMMA_R_Y12        (CIF_ISP_BASE + 0x00000054)
#define CIF_ISP_GAMMA_R_Y13        (CIF_ISP_BASE + 0x00000058)
#define CIF_ISP_GAMMA_R_Y14        (CIF_ISP_BASE + 0x0000005C)
#define CIF_ISP_GAMMA_R_Y15        (CIF_ISP_BASE + 0x00000060)
#define CIF_ISP_GAMMA_R_Y16        (CIF_ISP_BASE + 0x00000064)
#define CIF_ISP_GAMMA_G_Y0        (CIF_ISP_BASE + 0x00000068)
#define CIF_ISP_GAMMA_G_Y1        (CIF_ISP_BASE + 0x0000006C)
#define CIF_ISP_GAMMA_G_Y2        (CIF_ISP_BASE + 0x00000070)
#define CIF_ISP_GAMMA_G_Y3        (CIF_ISP_BASE + 0x00000074)
#define CIF_ISP_GAMMA_G_Y4        (CIF_ISP_BASE + 0x00000078)
#define CIF_ISP_GAMMA_G_Y5        (CIF_ISP_BASE + 0x0000007C)
#define CIF_ISP_GAMMA_G_Y6        (CIF_ISP_BASE + 0x00000080)
#define CIF_ISP_GAMMA_G_Y7        (CIF_ISP_BASE + 0x00000084)
#define CIF_ISP_GAMMA_G_Y8        (CIF_ISP_BASE + 0x00000088)
#define CIF_ISP_GAMMA_G_Y9        (CIF_ISP_BASE + 0x0000008C)
#define CIF_ISP_GAMMA_G_Y10        (CIF_ISP_BASE + 0x00000090)
#define CIF_ISP_GAMMA_G_Y11        (CIF_ISP_BASE + 0x00000094)
#define CIF_ISP_GAMMA_G_Y12        (CIF_ISP_BASE + 0x00000098)
#define CIF_ISP_GAMMA_G_Y13        (CIF_ISP_BASE + 0x0000009C)
#define CIF_ISP_GAMMA_G_Y14        (CIF_ISP_BASE + 0x000000A0)
#define CIF_ISP_GAMMA_G_Y15        (CIF_ISP_BASE + 0x000000A4)
#define CIF_ISP_GAMMA_G_Y16        (CIF_ISP_BASE + 0x000000A8)
#define CIF_ISP_GAMMA_B_Y0        (CIF_ISP_BASE + 0x000000AC)
#define CIF_ISP_GAMMA_B_Y1        (CIF_ISP_BASE + 0x000000B0)
#define CIF_ISP_GAMMA_B_Y2        (CIF_ISP_BASE + 0x000000B4)
#define CIF_ISP_GAMMA_B_Y3        (CIF_ISP_BASE + 0x000000B8)
#define CIF_ISP_GAMMA_B_Y4        (CIF_ISP_BASE + 0x000000BC)
#define CIF_ISP_GAMMA_B_Y5        (CIF_ISP_BASE + 0x000000C0)
#define CIF_ISP_GAMMA_B_Y6        (CIF_ISP_BASE + 0x000000C4)
#define CIF_ISP_GAMMA_B_Y7        (CIF_ISP_BASE + 0x000000C8)
#define CIF_ISP_GAMMA_B_Y8        (CIF_ISP_BASE + 0x000000CC)
#define CIF_ISP_GAMMA_B_Y9        (CIF_ISP_BASE + 0x000000D0)
#define CIF_ISP_GAMMA_B_Y10        (CIF_ISP_BASE + 0x000000D4)
#define CIF_ISP_GAMMA_B_Y11        (CIF_ISP_BASE + 0x000000D8)
#define CIF_ISP_GAMMA_B_Y12        (CIF_ISP_BASE + 0x000000DC)
#define CIF_ISP_GAMMA_B_Y13        (CIF_ISP_BASE + 0x000000E0)
#define CIF_ISP_GAMMA_B_Y14        (CIF_ISP_BASE + 0x000000E4)
#define CIF_ISP_GAMMA_B_Y15        (CIF_ISP_BASE + 0x000000E8)
#define CIF_ISP_GAMMA_B_Y16        (CIF_ISP_BASE + 0x000000EC)

#define CIF_ISP_AWB_PROP_V10        (CIF_ISP_BASE + 0x00000110)
#define CIF_ISP_AWB_WND_H_OFFS_V10    (CIF_ISP_BASE + 0x00000114)
#define CIF_ISP_AWB_WND_V_OFFS_V10    (CIF_ISP_BASE + 0x00000118)
#define CIF_ISP_AWB_WND_H_SIZE_V10    (CIF_ISP_BASE + 0x0000011C)
#define CIF_ISP_AWB_WND_V_SIZE_V10    (CIF_ISP_BASE + 0x00000120)
#define CIF_ISP_AWB_FRAMES_V10        (CIF_ISP_BASE + 0x00000124)
#define CIF_ISP_AWB_REF_V10        (CIF_ISP_BASE + 0x00000128)
#define CIF_ISP_AWB_THRESH_V10        (CIF_ISP_BASE + 0x0000012C)
#define CIF_ISP_AWB_GAIN_G_V10        (CIF_ISP_BASE + 0x00000138)
#define CIF_ISP_AWB_GAIN_RB_V10        (CIF_ISP_BASE + 0x0000013C)
#define CIF_ISP_AWB_WHITE_CNT_V10    (CIF_ISP_BASE + 0x00000140)
#define CIF_ISP_AWB_MEAN_V10        (CIF_ISP_BASE + 0x00000144)

#define CIF_ISP_AWB_PROP_V12        (CIF_ISP_BASE + 0x00000110)
#define CIF_ISP_AWB_SIZE_V12        (CIF_ISP_BASE + 0x00000114)
#define CIF_ISP_AWB_OFFS_V12        (CIF_ISP_BASE + 0x00000118)
#define CIF_ISP_AWB_REF_V12        (CIF_ISP_BASE + 0x0000011C)
#define CIF_ISP_AWB_THRESH_V12        (CIF_ISP_BASE + 0x00000120)
#define CIF_ISP_X_COOR12_V12        (CIF_ISP_BASE + 0x00000124)
#define CIF_ISP_X_COOR34_V12        (CIF_ISP_BASE + 0x00000128)
#define CIF_ISP_AWB_WHITE_CNT_V12    (CIF_ISP_BASE + 0x0000012C)
#define CIF_ISP_AWB_MEAN_V12        (CIF_ISP_BASE + 0x00000130)
#define CIF_ISP_DEGAIN_V12        (CIF_ISP_BASE + 0x00000134)
#define CIF_ISP_AWB_GAIN_G_V12        (CIF_ISP_BASE + 0x00000138)
#define CIF_ISP_AWB_GAIN_RB_V12        (CIF_ISP_BASE + 0x0000013C)
#define CIF_ISP_REGION_LINE_V12        (CIF_ISP_BASE + 0x00000140)
#define CIF_ISP_WP_CNT_REGION0_V12    (CIF_ISP_BASE + 0x00000160)
#define CIF_ISP_WP_CNT_REGION1_V12    (CIF_ISP_BASE + 0x00000164)
#define CIF_ISP_WP_CNT_REGION2_V12    (CIF_ISP_BASE + 0x00000168)
#define CIF_ISP_WP_CNT_REGION3_V12    (CIF_ISP_BASE + 0x0000016C)

#define CIF_ISP_CC_COEFF_0        (CIF_ISP_BASE + 0x00000170)
#define CIF_ISP_CC_COEFF_1        (CIF_ISP_BASE + 0x00000174)
#define CIF_ISP_CC_COEFF_2        (CIF_ISP_BASE + 0x00000178)
#define CIF_ISP_CC_COEFF_3        (CIF_ISP_BASE + 0x0000017C)
#define CIF_ISP_CC_COEFF_4        (CIF_ISP_BASE + 0x00000180)
#define CIF_ISP_CC_COEFF_5        (CIF_ISP_BASE + 0x00000184)
#define CIF_ISP_CC_COEFF_6        (CIF_ISP_BASE + 0x00000188)
#define CIF_ISP_CC_COEFF_7        (CIF_ISP_BASE + 0x0000018C)
#define CIF_ISP_CC_COEFF_8        (CIF_ISP_BASE + 0x00000190)
#define CIF_ISP_OUT_H_OFFS        (CIF_ISP_BASE + 0x00000194)
#define CIF_ISP_OUT_V_OFFS        (CIF_ISP_BASE + 0x00000198)
#define CIF_ISP_OUT_H_SIZE        (CIF_ISP_BASE + 0x0000019C)
#define CIF_ISP_OUT_V_SIZE        (CIF_ISP_BASE + 0x000001A0)
#define CIF_ISP_DEMOSAIC        (CIF_ISP_BASE + 0x000001A4)
#define CIF_ISP_FLAGS_SHD        (CIF_ISP_BASE + 0x000001A8)
#define CIF_ISP_OUT_H_OFFS_SHD        (CIF_ISP_BASE + 0x000001AC)
#define CIF_ISP_OUT_V_OFFS_SHD        (CIF_ISP_BASE + 0x000001B0)
#define CIF_ISP_OUT_H_SIZE_SHD        (CIF_ISP_BASE + 0x000001B4)
#define CIF_ISP_OUT_V_SIZE_SHD        (CIF_ISP_BASE + 0x000001B8)
#define CIF_ISP_IMSC            (CIF_ISP_BASE + 0x000001BC)
#define CIF_ISP_RIS            (CIF_ISP_BASE + 0x000001C0)
#define CIF_ISP_MIS            (CIF_ISP_BASE + 0x000001C4)
#define CIF_ISP_ICR            (CIF_ISP_BASE + 0x000001C8)
#define CIF_ISP_ISR            (CIF_ISP_BASE + 0x000001CC)
#define CIF_ISP_CT_COEFF_0        (CIF_ISP_BASE + 0x000001D0)
#define CIF_ISP_CT_COEFF_1        (CIF_ISP_BASE + 0x000001D4)
#define CIF_ISP_CT_COEFF_2        (CIF_ISP_BASE + 0x000001D8)
#define CIF_ISP_CT_COEFF_3        (CIF_ISP_BASE + 0x000001DC)
#define CIF_ISP_CT_COEFF_4        (CIF_ISP_BASE + 0x000001E0)
#define CIF_ISP_CT_COEFF_5        (CIF_ISP_BASE + 0x000001E4)
#define CIF_ISP_CT_COEFF_6        (CIF_ISP_BASE + 0x000001E8)
#define CIF_ISP_CT_COEFF_7        (CIF_ISP_BASE + 0x000001EC)
#define CIF_ISP_CT_COEFF_8        (CIF_ISP_BASE + 0x000001F0)
#define CIF_ISP_GAMMA_OUT_MODE_V10    (CIF_ISP_BASE + 0x000001F4)
#define CIF_ISP_GAMMA_OUT_Y_0_V10    (CIF_ISP_BASE + 0x000001F8)
#define CIF_ISP_GAMMA_OUT_Y_1_V10    (CIF_ISP_BASE + 0x000001FC)
#define CIF_ISP_GAMMA_OUT_Y_2_V10    (CIF_ISP_BASE + 0x00000200)
#define CIF_ISP_GAMMA_OUT_Y_3_V10    (CIF_ISP_BASE + 0x00000204)
#define CIF_ISP_GAMMA_OUT_Y_4_V10    (CIF_ISP_BASE + 0x00000208)
#define CIF_ISP_GAMMA_OUT_Y_5_V10    (CIF_ISP_BASE + 0x0000020C)
#define CIF_ISP_GAMMA_OUT_Y_6_V10    (CIF_ISP_BASE + 0x00000210)
#define CIF_ISP_GAMMA_OUT_Y_7_V10    (CIF_ISP_BASE + 0x00000214)
#define CIF_ISP_GAMMA_OUT_Y_8_V10    (CIF_ISP_BASE + 0x00000218)
#define CIF_ISP_GAMMA_OUT_Y_9_V10    (CIF_ISP_BASE + 0x0000021C)
#define CIF_ISP_GAMMA_OUT_Y_10_V10    (CIF_ISP_BASE + 0x00000220)
#define CIF_ISP_GAMMA_OUT_Y_11_V10    (CIF_ISP_BASE + 0x00000224)
#define CIF_ISP_GAMMA_OUT_Y_12_V10    (CIF_ISP_BASE + 0x00000228)
#define CIF_ISP_GAMMA_OUT_Y_13_V10    (CIF_ISP_BASE + 0x0000022C)
#define CIF_ISP_GAMMA_OUT_Y_14_V10    (CIF_ISP_BASE + 0x00000230)
#define CIF_ISP_GAMMA_OUT_Y_15_V10    (CIF_ISP_BASE + 0x00000234)
#define CIF_ISP_GAMMA_OUT_Y_16_V10    (CIF_ISP_BASE + 0x00000238)
#define CIF_ISP_ERR            (CIF_ISP_BASE + 0x0000023C)
#define CIF_ISP_ERR_CLR            (CIF_ISP_BASE + 0x00000240)
#define CIF_ISP_FRAME_COUNT        (CIF_ISP_BASE + 0x00000244)
#define CIF_ISP_CT_OFFSET_R        (CIF_ISP_BASE + 0x00000248)
#define CIF_ISP_CT_OFFSET_G        (CIF_ISP_BASE + 0x0000024C)
#define CIF_ISP_CT_OFFSET_B        (CIF_ISP_BASE + 0x00000250)
#define CIF_ISP_GAMMA_OUT_MODE_V12    (CIF_ISP_BASE + 0x00000300)
#define CIF_ISP_GAMMA_OUT_Y_0_V12    (CIF_ISP_BASE + 0x00000304)

#define CIF_ISP_FLASH_BASE        0x00000660
#define CIF_ISP_FLASH_CMD        (CIF_ISP_FLASH_BASE + 0x00000000)
#define CIF_ISP_FLASH_CONFIG        (CIF_ISP_FLASH_BASE + 0x00000004)
#define CIF_ISP_FLASH_PREDIV        (CIF_ISP_FLASH_BASE + 0x00000008)
#define CIF_ISP_FLASH_DELAY        (CIF_ISP_FLASH_BASE + 0x0000000C)
#define CIF_ISP_FLASH_TIME        (CIF_ISP_FLASH_BASE + 0x00000010)
#define CIF_ISP_FLASH_MAXP        (CIF_ISP_FLASH_BASE + 0x00000014)

#define CIF_ISP_SH_BASE            0x00000680
#define CIF_ISP_SH_CTRL            (CIF_ISP_SH_BASE + 0x00000000)
#define CIF_ISP_SH_PREDIV        (CIF_ISP_SH_BASE + 0x00000004)
#define CIF_ISP_SH_DELAY        (CIF_ISP_SH_BASE + 0x00000008)
#define CIF_ISP_SH_TIME            (CIF_ISP_SH_BASE + 0x0000000C)

#define CIF_C_PROC_BASE            0x00000800
#define CIF_C_PROC_CTRL            (CIF_C_PROC_BASE + 0x00000000)
#define CIF_C_PROC_CONTRAST        (CIF_C_PROC_BASE + 0x00000004)
#define CIF_C_PROC_BRIGHTNESS        (CIF_C_PROC_BASE + 0x00000008)
#define CIF_C_PROC_SATURATION        (CIF_C_PROC_BASE + 0x0000000C)
#define CIF_C_PROC_HUE            (CIF_C_PROC_BASE + 0x00000010)

#define CIF_DUAL_CROP_BASE        0x00000880
#define CIF_DUAL_CROP_CTRL        (CIF_DUAL_CROP_BASE + 0x00000000)
#define CIF_DUAL_CROP_M_H_OFFS        (CIF_DUAL_CROP_BASE + 0x00000004)
#define CIF_DUAL_CROP_M_V_OFFS        (CIF_DUAL_CROP_BASE + 0x00000008)
#define CIF_DUAL_CROP_M_H_SIZE        (CIF_DUAL_CROP_BASE + 0x0000000C)
#define CIF_DUAL_CROP_M_V_SIZE        (CIF_DUAL_CROP_BASE + 0x00000010)
#define CIF_DUAL_CROP_S_H_OFFS        (CIF_DUAL_CROP_BASE + 0x00000014)
#define CIF_DUAL_CROP_S_V_OFFS        (CIF_DUAL_CROP_BASE + 0x00000018)
#define CIF_DUAL_CROP_S_H_SIZE        (CIF_DUAL_CROP_BASE + 0x0000001C)
#define CIF_DUAL_CROP_S_V_SIZE        (CIF_DUAL_CROP_BASE + 0x00000020)
#define CIF_DUAL_CROP_M_H_OFFS_SHD    (CIF_DUAL_CROP_BASE + 0x00000024)
#define CIF_DUAL_CROP_M_V_OFFS_SHD    (CIF_DUAL_CROP_BASE + 0x00000028)
#define CIF_DUAL_CROP_M_H_SIZE_SHD    (CIF_DUAL_CROP_BASE + 0x0000002C)
#define CIF_DUAL_CROP_M_V_SIZE_SHD    (CIF_DUAL_CROP_BASE + 0x00000030)
#define CIF_DUAL_CROP_S_H_OFFS_SHD    (CIF_DUAL_CROP_BASE + 0x00000034)
#define CIF_DUAL_CROP_S_V_OFFS_SHD    (CIF_DUAL_CROP_BASE + 0x00000038)
#define CIF_DUAL_CROP_S_H_SIZE_SHD    (CIF_DUAL_CROP_BASE + 0x0000003C)
#define CIF_DUAL_CROP_S_V_SIZE_SHD    (CIF_DUAL_CROP_BASE + 0x00000040)

#define CIF_MRSZ_BASE            0x00000C00
#define CIF_MRSZ_CTRL            (CIF_MRSZ_BASE + 0x00000000)
#define CIF_MRSZ_SCALE_HY        (CIF_MRSZ_BASE + 0x00000004)
#define CIF_MRSZ_SCALE_HCB        (CIF_MRSZ_BASE + 0x00000008)
#define CIF_MRSZ_SCALE_HCR        (CIF_MRSZ_BASE + 0x0000000C)
#define CIF_MRSZ_SCALE_VY        (CIF_MRSZ_BASE + 0x00000010)
#define CIF_MRSZ_SCALE_VC        (CIF_MRSZ_BASE + 0x00000014)
#define CIF_MRSZ_PHASE_HY        (CIF_MRSZ_BASE + 0x00000018)
#define CIF_MRSZ_PHASE_HC        (CIF_MRSZ_BASE + 0x0000001C)
#define CIF_MRSZ_PHASE_VY        (CIF_MRSZ_BASE + 0x00000020)
#define CIF_MRSZ_PHASE_VC        (CIF_MRSZ_BASE + 0x00000024)
#define CIF_MRSZ_SCALE_LUT_ADDR        (CIF_MRSZ_BASE + 0x00000028)
#define CIF_MRSZ_SCALE_LUT        (CIF_MRSZ_BASE + 0x0000002C)
#define CIF_MRSZ_CTRL_SHD        (CIF_MRSZ_BASE + 0x00000030)
#define CIF_MRSZ_SCALE_HY_SHD        (CIF_MRSZ_BASE + 0x00000034)
#define CIF_MRSZ_SCALE_HCB_SHD        (CIF_MRSZ_BASE + 0x00000038)
#define CIF_MRSZ_SCALE_HCR_SHD        (CIF_MRSZ_BASE + 0x0000003C)
#define CIF_MRSZ_SCALE_VY_SHD        (CIF_MRSZ_BASE + 0x00000040)
#define CIF_MRSZ_SCALE_VC_SHD        (CIF_MRSZ_BASE + 0x00000044)
#define CIF_MRSZ_PHASE_HY_SHD        (CIF_MRSZ_BASE + 0x00000048)
#define CIF_MRSZ_PHASE_HC_SHD        (CIF_MRSZ_BASE + 0x0000004C)
#define CIF_MRSZ_PHASE_VY_SHD        (CIF_MRSZ_BASE + 0x00000050)
#define CIF_MRSZ_PHASE_VC_SHD        (CIF_MRSZ_BASE + 0x00000054)

#define CIF_SRSZ_BASE            0x00001000
#define CIF_SRSZ_CTRL            (CIF_SRSZ_BASE + 0x00000000)
#define CIF_SRSZ_SCALE_HY        (CIF_SRSZ_BASE + 0x00000004)
#define CIF_SRSZ_SCALE_HCB        (CIF_SRSZ_BASE + 0x00000008)
#define CIF_SRSZ_SCALE_HCR        (CIF_SRSZ_BASE + 0x0000000C)
#define CIF_SRSZ_SCALE_VY        (CIF_SRSZ_BASE + 0x00000010)
#define CIF_SRSZ_SCALE_VC        (CIF_SRSZ_BASE + 0x00000014)
#define CIF_SRSZ_PHASE_HY        (CIF_SRSZ_BASE + 0x00000018)
#define CIF_SRSZ_PHASE_HC        (CIF_SRSZ_BASE + 0x0000001C)
#define CIF_SRSZ_PHASE_VY        (CIF_SRSZ_BASE + 0x00000020)
#define CIF_SRSZ_PHASE_VC        (CIF_SRSZ_BASE + 0x00000024)
#define CIF_SRSZ_SCALE_LUT_ADDR        (CIF_SRSZ_BASE + 0x00000028)
#define CIF_SRSZ_SCALE_LUT        (CIF_SRSZ_BASE + 0x0000002C)
#define CIF_SRSZ_CTRL_SHD        (CIF_SRSZ_BASE + 0x00000030)
#define CIF_SRSZ_SCALE_HY_SHD        (CIF_SRSZ_BASE + 0x00000034)
#define CIF_SRSZ_SCALE_HCB_SHD        (CIF_SRSZ_BASE + 0x00000038)
#define CIF_SRSZ_SCALE_HCR_SHD        (CIF_SRSZ_BASE + 0x0000003C)
#define CIF_SRSZ_SCALE_VY_SHD        (CIF_SRSZ_BASE + 0x00000040)
#define CIF_SRSZ_SCALE_VC_SHD        (CIF_SRSZ_BASE + 0x00000044)
#define CIF_SRSZ_PHASE_HY_SHD        (CIF_SRSZ_BASE + 0x00000048)
#define CIF_SRSZ_PHASE_HC_SHD        (CIF_SRSZ_BASE + 0x0000004C)
#define CIF_SRSZ_PHASE_VY_SHD        (CIF_SRSZ_BASE + 0x00000050)
#define CIF_SRSZ_PHASE_VC_SHD        (CIF_SRSZ_BASE + 0x00000054)

#define CIF_MI_BASE            0x00001400
#define CIF_MI_CTRL            (CIF_MI_BASE + 0x00000000)
#define CIF_MI_INIT            (CIF_MI_BASE + 0x00000004)
#define CIF_MI_MP_Y_BASE_AD_INIT    (CIF_MI_BASE + 0x00000008)
#define CIF_MI_MP_Y_SIZE_INIT        (CIF_MI_BASE + 0x0000000C)
#define CIF_MI_MP_Y_OFFS_CNT_INIT    (CIF_MI_BASE + 0x00000010)
#define CIF_MI_MP_Y_OFFS_CNT_START    (CIF_MI_BASE + 0x00000014)
#define CIF_MI_MP_Y_IRQ_OFFS_INIT    (CIF_MI_BASE + 0x00000018)
#define CIF_MI_MP_CB_BASE_AD_INIT    (CIF_MI_BASE + 0x0000001C)
#define CIF_MI_MP_CB_SIZE_INIT        (CIF_MI_BASE + 0x00000020)
#define CIF_MI_MP_CB_OFFS_CNT_INIT    (CIF_MI_BASE + 0x00000024)
#define CIF_MI_MP_CB_OFFS_CNT_START    (CIF_MI_BASE + 0x00000028)
#define CIF_MI_MP_CR_BASE_AD_INIT    (CIF_MI_BASE + 0x0000002C)
#define CIF_MI_MP_CR_SIZE_INIT        (CIF_MI_BASE + 0x00000030)
#define CIF_MI_MP_CR_OFFS_CNT_INIT    (CIF_MI_BASE + 0x00000034)
#define CIF_MI_MP_CR_OFFS_CNT_START    (CIF_MI_BASE + 0x00000038)
#define CIF_MI_SP_Y_BASE_AD_INIT    (CIF_MI_BASE + 0x0000003C)
#define CIF_MI_SP_Y_SIZE_INIT        (CIF_MI_BASE + 0x00000040)
#define CIF_MI_SP_Y_OFFS_CNT_INIT    (CIF_MI_BASE + 0x00000044)
#define CIF_MI_SP_Y_OFFS_CNT_START    (CIF_MI_BASE + 0x00000048)
#define CIF_MI_SP_Y_LLENGTH        (CIF_MI_BASE + 0x0000004C)
#define CIF_MI_SP_CB_BASE_AD_INIT    (CIF_MI_BASE + 0x00000050)
#define CIF_MI_SP_CB_SIZE_INIT        (CIF_MI_BASE + 0x00000054)
#define CIF_MI_SP_CB_OFFS_CNT_INIT    (CIF_MI_BASE + 0x00000058)
#define CIF_MI_SP_CB_OFFS_CNT_START    (CIF_MI_BASE + 0x0000005C)
#define CIF_MI_SP_CR_BASE_AD_INIT    (CIF_MI_BASE + 0x00000060)
#define CIF_MI_SP_CR_SIZE_INIT        (CIF_MI_BASE + 0x00000064)
#define CIF_MI_SP_CR_OFFS_CNT_INIT    (CIF_MI_BASE + 0x00000068)
#define CIF_MI_SP_CR_OFFS_CNT_START    (CIF_MI_BASE + 0x0000006C)
#define CIF_MI_BYTE_CNT            (CIF_MI_BASE + 0x00000070)
#define CIF_MI_CTRL_SHD            (CIF_MI_BASE + 0x00000074)
#define CIF_MI_MP_Y_BASE_AD_SHD        (CIF_MI_BASE + 0x00000078)
#define CIF_MI_MP_Y_SIZE_SHD        (CIF_MI_BASE + 0x0000007C)
#define CIF_MI_MP_Y_OFFS_CNT_SHD    (CIF_MI_BASE + 0x00000080)
#define CIF_MI_MP_Y_IRQ_OFFS_SHD    (CIF_MI_BASE + 0x00000084)
#define CIF_MI_MP_CB_BASE_AD_SHD    (CIF_MI_BASE + 0x00000088)
#define CIF_MI_MP_CB_SIZE_SHD        (CIF_MI_BASE + 0x0000008C)
#define CIF_MI_MP_CB_OFFS_CNT_SHD    (CIF_MI_BASE + 0x00000090)
#define CIF_MI_MP_CR_BASE_AD_SHD    (CIF_MI_BASE + 0x00000094)
#define CIF_MI_MP_CR_SIZE_SHD        (CIF_MI_BASE + 0x00000098)
#define CIF_MI_MP_CR_OFFS_CNT_SHD    (CIF_MI_BASE + 0x0000009C)
#define CIF_MI_SP_Y_BASE_AD_SHD        (CIF_MI_BASE + 0x000000A0)
#define CIF_MI_SP_Y_SIZE_SHD        (CIF_MI_BASE + 0x000000A4)
#define CIF_MI_SP_Y_OFFS_CNT_SHD    (CIF_MI_BASE + 0x000000A8)
#define CIF_MI_SP_CB_BASE_AD_SHD    (CIF_MI_BASE + 0x000000B0)
#define CIF_MI_SP_CB_SIZE_SHD        (CIF_MI_BASE + 0x000000B4)
#define CIF_MI_SP_CB_OFFS_CNT_SHD    (CIF_MI_BASE + 0x000000B8)
#define CIF_MI_SP_CR_BASE_AD_SHD    (CIF_MI_BASE + 0x000000BC)
#define CIF_MI_SP_CR_SIZE_SHD        (CIF_MI_BASE + 0x000000C0)
#define CIF_MI_SP_CR_OFFS_CNT_SHD    (CIF_MI_BASE + 0x000000C4)
#define CIF_MI_DMA_Y_PIC_START_AD    (CIF_MI_BASE + 0x000000C8)
#define CIF_MI_DMA_Y_PIC_WIDTH        (CIF_MI_BASE + 0x000000CC)
#define CIF_MI_DMA_Y_LLENGTH        (CIF_MI_BASE + 0x000000D0)
#define CIF_MI_DMA_Y_PIC_SIZE        (CIF_MI_BASE + 0x000000D4)
#define CIF_MI_DMA_CB_PIC_START_AD    (CIF_MI_BASE + 0x000000D8)
#define CIF_MI_DMA_CR_PIC_START_AD    (CIF_MI_BASE + 0x000000E8)
#define CIF_MI_IMSC            (CIF_MI_BASE + 0x000000F8)
#define CIF_MI_RIS            (CIF_MI_BASE + 0x000000FC)
#define CIF_MI_MIS            (CIF_MI_BASE + 0x00000100)
#define CIF_MI_ICR            (CIF_MI_BASE + 0x00000104)
#define CIF_MI_ISR            (CIF_MI_BASE + 0x00000108)
#define CIF_MI_STATUS            (CIF_MI_BASE + 0x0000010C)
#define CIF_MI_STATUS_CLR        (CIF_MI_BASE + 0x00000110)
#define CIF_MI_SP_Y_PIC_WIDTH        (CIF_MI_BASE + 0x00000114)
#define CIF_MI_SP_Y_PIC_HEIGHT        (CIF_MI_BASE + 0x00000118)
#define CIF_MI_SP_Y_PIC_SIZE        (CIF_MI_BASE + 0x0000011C)
#define CIF_MI_DMA_CTRL            (CIF_MI_BASE + 0x00000120)
#define CIF_MI_DMA_START        (CIF_MI_BASE + 0x00000124)
#define CIF_MI_DMA_STATUS        (CIF_MI_BASE + 0x00000128)
#define CIF_MI_PIXEL_COUNT        (CIF_MI_BASE + 0x0000012C)
#define CIF_MI_MP_Y_BASE_AD_INIT2    (CIF_MI_BASE + 0x00000130)
#define CIF_MI_MP_CB_BASE_AD_INIT2    (CIF_MI_BASE + 0x00000134)
#define CIF_MI_MP_CR_BASE_AD_INIT2    (CIF_MI_BASE + 0x00000138)
#define CIF_MI_SP_Y_BASE_AD_INIT2    (CIF_MI_BASE + 0x0000013C)
#define CIF_MI_SP_CB_BASE_AD_INIT2    (CIF_MI_BASE + 0x00000140)
#define CIF_MI_SP_CR_BASE_AD_INIT2    (CIF_MI_BASE + 0x00000144)
#define CIF_MI_XTD_FORMAT_CTRL        (CIF_MI_BASE + 0x00000148)
#define CIF_MI_CTRL2            (CIF_MI_BASE + 0x00000150)
#define CIF_MI_RAW0_BASE_AD_INIT    (CIF_MI_BASE + 0x00000160)
#define CIF_MI_RAW0_BASE_AD_INIT2    (CIF_MI_BASE + 0x00000164)
#define CIF_MI_RAW0_IRQ_OFFS_INIT    (CIF_MI_BASE + 0x00000168)
#define CIF_MI_RAW0_SIZE_INIT        (CIF_MI_BASE + 0x0000016c)
#define CIF_MI_RAW0_OFFS_CNT_INIT    (CIF_MI_BASE + 0x00000170)
#define CIF_MI_RAW0_LENGTH        (CIF_MI_BASE + 0x00000174)
#define CIF_MI_RAW0_OFFS_CNT_START_SHD    (CIF_MI_BASE + 0x00000178)
#define CIF_MI_RAW0_BASE_AS_SHD        (CIF_MI_BASE + 0x00000180)
#define CIF_MI_RAW0_IRQ_OFFS_INI_SHD    (CIF_MI_BASE + 0x00000184)
#define CIF_MI_RAW0_SIZE_INIT_SHD    (CIF_MI_BASE + 0x00000188)
#define CIF_MI_RAW0_OFFS_CNT_INIT_SHD    (CIF_MI_BASE + 0x0000018c)

#define CIF_SMIA_BASE            0x00001A00
#define CIF_SMIA_CTRL            (CIF_SMIA_BASE + 0x00000000)
#define CIF_SMIA_STATUS            (CIF_SMIA_BASE + 0x00000004)
#define CIF_SMIA_IMSC            (CIF_SMIA_BASE + 0x00000008)
#define CIF_SMIA_RIS            (CIF_SMIA_BASE + 0x0000000C)
#define CIF_SMIA_MIS            (CIF_SMIA_BASE + 0x00000010)
#define CIF_SMIA_ICR            (CIF_SMIA_BASE + 0x00000014)
#define CIF_SMIA_ISR            (CIF_SMIA_BASE + 0x00000018)
#define CIF_SMIA_DATA_FORMAT_SEL    (CIF_SMIA_BASE + 0x0000001C)
#define CIF_SMIA_SOF_EMB_DATA_LINES    (CIF_SMIA_BASE + 0x00000020)
#define CIF_SMIA_EMB_HSTART        (CIF_SMIA_BASE + 0x00000024)
#define CIF_SMIA_EMB_HSIZE        (CIF_SMIA_BASE + 0x00000028)
#define CIF_SMIA_EMB_VSTART        (CIF_SMIA_BASE + 0x0000002c)
#define CIF_SMIA_NUM_LINES        (CIF_SMIA_BASE + 0x00000030)
#define CIF_SMIA_EMB_DATA_FIFO        (CIF_SMIA_BASE + 0x00000034)
#define CIF_SMIA_EMB_DATA_WATERMARK    (CIF_SMIA_BASE + 0x00000038)

#define CIF_MIPI_BASE            0x00001C00
#define CIF_MIPI_CTRL            (CIF_MIPI_BASE + 0x00000000)
#define CIF_MIPI_STATUS            (CIF_MIPI_BASE + 0x00000004)
#define CIF_MIPI_IMSC            (CIF_MIPI_BASE + 0x00000008)
#define CIF_MIPI_RIS            (CIF_MIPI_BASE + 0x0000000C)
#define CIF_MIPI_MIS            (CIF_MIPI_BASE + 0x00000010)
#define CIF_MIPI_ICR            (CIF_MIPI_BASE + 0x00000014)
#define CIF_MIPI_ISR            (CIF_MIPI_BASE + 0x00000018)
#define CIF_MIPI_CUR_DATA_ID        (CIF_MIPI_BASE + 0x0000001C)
#define CIF_MIPI_IMG_DATA_SEL        (CIF_MIPI_BASE + 0x00000020)
#define CIF_MIPI_ADD_DATA_SEL_1        (CIF_MIPI_BASE + 0x00000024)
#define CIF_MIPI_ADD_DATA_SEL_2        (CIF_MIPI_BASE + 0x00000028)
#define CIF_MIPI_ADD_DATA_SEL_3        (CIF_MIPI_BASE + 0x0000002C)
#define CIF_MIPI_ADD_DATA_SEL_4        (CIF_MIPI_BASE + 0x00000030)
#define CIF_MIPI_ADD_DATA_FIFO        (CIF_MIPI_BASE + 0x00000034)
#define CIF_MIPI_FIFO_FILL_LEVEL    (CIF_MIPI_BASE + 0x00000038)
#define CIF_MIPI_COMPRESSED_MODE    (CIF_MIPI_BASE + 0x0000003C)
#define CIF_MIPI_FRAME            (CIF_MIPI_BASE + 0x00000040)
#define CIF_MIPI_GEN_SHORT_DT        (CIF_MIPI_BASE + 0x00000044)
#define CIF_MIPI_GEN_SHORT_8_9        (CIF_MIPI_BASE + 0x00000048)
#define CIF_MIPI_GEN_SHORT_A_B        (CIF_MIPI_BASE + 0x0000004C)
#define CIF_MIPI_GEN_SHORT_C_D        (CIF_MIPI_BASE + 0x00000050)
#define CIF_MIPI_GEN_SHORT_E_F        (CIF_MIPI_BASE + 0x00000054)

#define CIF_ISP_AFM_BASE        0x00002000
#define CIF_ISP_AFM_CTRL        (CIF_ISP_AFM_BASE + 0x00000000)
#define CIF_ISP_AFM_LT_A        (CIF_ISP_AFM_BASE + 0x00000004)
#define CIF_ISP_AFM_RB_A        (CIF_ISP_AFM_BASE + 0x00000008)
#define CIF_ISP_AFM_LT_B        (CIF_ISP_AFM_BASE + 0x0000000C)
#define CIF_ISP_AFM_RB_B        (CIF_ISP_AFM_BASE + 0x00000010)
#define CIF_ISP_AFM_LT_C        (CIF_ISP_AFM_BASE + 0x00000014)
#define CIF_ISP_AFM_RB_C        (CIF_ISP_AFM_BASE + 0x00000018)
#define CIF_ISP_AFM_THRES        (CIF_ISP_AFM_BASE + 0x0000001C)
#define CIF_ISP_AFM_VAR_SHIFT        (CIF_ISP_AFM_BASE + 0x00000020)
#define CIF_ISP_AFM_SUM_A        (CIF_ISP_AFM_BASE + 0x00000024)
#define CIF_ISP_AFM_SUM_B        (CIF_ISP_AFM_BASE + 0x00000028)
#define CIF_ISP_AFM_SUM_C        (CIF_ISP_AFM_BASE + 0x0000002C)
#define CIF_ISP_AFM_LUM_A        (CIF_ISP_AFM_BASE + 0x00000030)
#define CIF_ISP_AFM_LUM_B        (CIF_ISP_AFM_BASE + 0x00000034)
#define CIF_ISP_AFM_LUM_C        (CIF_ISP_AFM_BASE + 0x00000038)

#define CIF_ISP_LSC_BASE        0x00002200
#define CIF_ISP_LSC_CTRL        (CIF_ISP_LSC_BASE + 0x00000000)
#define CIF_ISP_LSC_R_TABLE_ADDR    (CIF_ISP_LSC_BASE + 0x00000004)
#define CIF_ISP_LSC_GR_TABLE_ADDR    (CIF_ISP_LSC_BASE + 0x00000008)
#define CIF_ISP_LSC_B_TABLE_ADDR    (CIF_ISP_LSC_BASE + 0x0000000C)
#define CIF_ISP_LSC_GB_TABLE_ADDR    (CIF_ISP_LSC_BASE + 0x00000010)
#define CIF_ISP_LSC_R_TABLE_DATA    (CIF_ISP_LSC_BASE + 0x00000014)
#define CIF_ISP_LSC_GR_TABLE_DATA    (CIF_ISP_LSC_BASE + 0x00000018)
#define CIF_ISP_LSC_B_TABLE_DATA    (CIF_ISP_LSC_BASE + 0x0000001C)
#define CIF_ISP_LSC_GB_TABLE_DATA    (CIF_ISP_LSC_BASE + 0x00000020)
#define CIF_ISP_LSC_XGRAD_01        (CIF_ISP_LSC_BASE + 0x00000024)
#define CIF_ISP_LSC_XGRAD_23        (CIF_ISP_LSC_BASE + 0x00000028)
#define CIF_ISP_LSC_XGRAD_45        (CIF_ISP_LSC_BASE + 0x0000002C)
#define CIF_ISP_LSC_XGRAD_67        (CIF_ISP_LSC_BASE + 0x00000030)
#define CIF_ISP_LSC_YGRAD_01        (CIF_ISP_LSC_BASE + 0x00000034)
#define CIF_ISP_LSC_YGRAD_23        (CIF_ISP_LSC_BASE + 0x00000038)
#define CIF_ISP_LSC_YGRAD_45        (CIF_ISP_LSC_BASE + 0x0000003C)
#define CIF_ISP_LSC_YGRAD_67        (CIF_ISP_LSC_BASE + 0x00000040)
#define CIF_ISP_LSC_XSIZE_01        (CIF_ISP_LSC_BASE + 0x00000044)
#define CIF_ISP_LSC_XSIZE_23        (CIF_ISP_LSC_BASE + 0x00000048)
#define CIF_ISP_LSC_XSIZE_45        (CIF_ISP_LSC_BASE + 0x0000004C)
#define CIF_ISP_LSC_XSIZE_67        (CIF_ISP_LSC_BASE + 0x00000050)
#define CIF_ISP_LSC_YSIZE_01        (CIF_ISP_LSC_BASE + 0x00000054)
#define CIF_ISP_LSC_YSIZE_23        (CIF_ISP_LSC_BASE + 0x00000058)
#define CIF_ISP_LSC_YSIZE_45        (CIF_ISP_LSC_BASE + 0x0000005C)
#define CIF_ISP_LSC_YSIZE_67        (CIF_ISP_LSC_BASE + 0x00000060)
#define CIF_ISP_LSC_TABLE_SEL        (CIF_ISP_LSC_BASE + 0x00000064)
#define CIF_ISP_LSC_STATUS        (CIF_ISP_LSC_BASE + 0x00000068)

#define CIF_ISP_IS_BASE            0x00002300
#define CIF_ISP_IS_CTRL            (CIF_ISP_IS_BASE + 0x00000000)
#define CIF_ISP_IS_RECENTER        (CIF_ISP_IS_BASE + 0x00000004)
#define CIF_ISP_IS_H_OFFS        (CIF_ISP_IS_BASE + 0x00000008)
#define CIF_ISP_IS_V_OFFS        (CIF_ISP_IS_BASE + 0x0000000C)
#define CIF_ISP_IS_H_SIZE        (CIF_ISP_IS_BASE + 0x00000010)
#define CIF_ISP_IS_V_SIZE        (CIF_ISP_IS_BASE + 0x00000014)
#define CIF_ISP_IS_MAX_DX        (CIF_ISP_IS_BASE + 0x00000018)
#define CIF_ISP_IS_MAX_DY        (CIF_ISP_IS_BASE + 0x0000001C)
#define CIF_ISP_IS_DISPLACE        (CIF_ISP_IS_BASE + 0x00000020)
#define CIF_ISP_IS_H_OFFS_SHD        (CIF_ISP_IS_BASE + 0x00000024)
#define CIF_ISP_IS_V_OFFS_SHD        (CIF_ISP_IS_BASE + 0x00000028)
#define CIF_ISP_IS_H_SIZE_SHD        (CIF_ISP_IS_BASE + 0x0000002C)
#define CIF_ISP_IS_V_SIZE_SHD        (CIF_ISP_IS_BASE + 0x00000030)

#define CIF_ISP_HIST_BASE_V10        0x00002400
#define CIF_ISP_HIST_PROP_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000000)
#define CIF_ISP_HIST_H_OFFS_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000004)
#define CIF_ISP_HIST_V_OFFS_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000008)
#define CIF_ISP_HIST_H_SIZE_V10        (CIF_ISP_HIST_BASE_V10 + 0x0000000C)
#define CIF_ISP_HIST_V_SIZE_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000010)
#define CIF_ISP_HIST_BIN_0_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000014)
#define CIF_ISP_HIST_BIN_1_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000018)
#define CIF_ISP_HIST_BIN_2_V10        (CIF_ISP_HIST_BASE_V10 + 0x0000001C)
#define CIF_ISP_HIST_BIN_3_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000020)
#define CIF_ISP_HIST_BIN_4_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000024)
#define CIF_ISP_HIST_BIN_5_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000028)
#define CIF_ISP_HIST_BIN_6_V10        (CIF_ISP_HIST_BASE_V10 + 0x0000002C)
#define CIF_ISP_HIST_BIN_7_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000030)
#define CIF_ISP_HIST_BIN_8_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000034)
#define CIF_ISP_HIST_BIN_9_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000038)
#define CIF_ISP_HIST_BIN_10_V10        (CIF_ISP_HIST_BASE_V10 + 0x0000003C)
#define CIF_ISP_HIST_BIN_11_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000040)
#define CIF_ISP_HIST_BIN_12_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000044)
#define CIF_ISP_HIST_BIN_13_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000048)
#define CIF_ISP_HIST_BIN_14_V10        (CIF_ISP_HIST_BASE_V10 + 0x0000004C)
#define CIF_ISP_HIST_BIN_15_V10        (CIF_ISP_HIST_BASE_V10 + 0x00000050)
#define CIF_ISP_HIST_WEIGHT_00TO30_V10    (CIF_ISP_HIST_BASE_V10 + 0x00000054)
#define CIF_ISP_HIST_WEIGHT_40TO21_V10    (CIF_ISP_HIST_BASE_V10 + 0x00000058)
#define CIF_ISP_HIST_WEIGHT_31TO12_V10    (CIF_ISP_HIST_BASE_V10 + 0x0000005C)
#define CIF_ISP_HIST_WEIGHT_22TO03_V10    (CIF_ISP_HIST_BASE_V10 + 0x00000060)
#define CIF_ISP_HIST_WEIGHT_13TO43_V10    (CIF_ISP_HIST_BASE_V10 + 0x00000064)
#define CIF_ISP_HIST_WEIGHT_04TO34_V10    (CIF_ISP_HIST_BASE_V10 + 0x00000068)
#define CIF_ISP_HIST_WEIGHT_44_V10    (CIF_ISP_HIST_BASE_V10 + 0x0000006C)

#define CIF_ISP_FILT_BASE        0x00002500
#define CIF_ISP_FILT_MODE        (CIF_ISP_FILT_BASE + 0x00000000)
#define CIF_ISP_FILT_THRESH_BL0        (CIF_ISP_FILT_BASE + 0x00000028)
#define CIF_ISP_FILT_THRESH_BL1        (CIF_ISP_FILT_BASE + 0x0000002c)
#define CIF_ISP_FILT_THRESH_SH0        (CIF_ISP_FILT_BASE + 0x00000030)
#define CIF_ISP_FILT_THRESH_SH1        (CIF_ISP_FILT_BASE + 0x00000034)
#define CIF_ISP_FILT_LUM_WEIGHT        (CIF_ISP_FILT_BASE + 0x00000038)
#define CIF_ISP_FILT_FAC_SH1        (CIF_ISP_FILT_BASE + 0x0000003c)
#define CIF_ISP_FILT_FAC_SH0        (CIF_ISP_FILT_BASE + 0x00000040)
#define CIF_ISP_FILT_FAC_MID        (CIF_ISP_FILT_BASE + 0x00000044)
#define CIF_ISP_FILT_FAC_BL0        (CIF_ISP_FILT_BASE + 0x00000048)
#define CIF_ISP_FILT_FAC_BL1        (CIF_ISP_FILT_BASE + 0x0000004C)
#define CIF_ISP_FILT_ISP_CAC_CTRL    (CIF_ISP_FILT_BASE + 0x00000080)
#define CIF_ISP_FILT_CAC_COUNT_START    (CIF_ISP_FILT_BASE + 0x00000084)
#define CIF_ISP_FILT_CAC_A        (CIF_ISP_FILT_BASE + 0x00000088)
#define CIF_ISP_FILT_CAC_B        (CIF_ISP_FILT_BASE + 0x0000008c)
#define CIF_ISP_FILT_CAC_C        (CIF_ISP_FILT_BASE + 0x00000090)
#define CIF_ISP_FILT_CAC_X_NORM        (CIF_ISP_FILT_BASE + 0x00000094)
#define CIF_ISP_FILT_CAC_Y_NORM        (CIF_ISP_FILT_BASE + 0x00000098)
#define CIF_ISP_FILT_LU_DIVID        (CIF_ISP_FILT_BASE + 0x000000a0)
#define CIF_ISP_FILT_THGRAD_DIVID0123    (CIF_ISP_FILT_BASE + 0x000000a4)
#define CIF_ISP_FILT_THGRAD_DIVID4    (CIF_ISP_FILT_BASE + 0x000000a8)
#define CIF_ISP_FILT_THDIFF_DIVID0123    (CIF_ISP_FILT_BASE + 0x000000ac)
#define CIF_ISP_FILT_THDIFF_DIVID4    (CIF_ISP_FILT_BASE + 0x000000b0)
#define CIF_ISP_FILT_THCSC_DIVID0123    (CIF_ISP_FILT_BASE + 0x000000b4)
#define CIF_ISP_FILT_THCSC_DIVID4    (CIF_ISP_FILT_BASE + 0x000000b8)
#define CIF_ISP_FILT_THVAR_DIVID01    (CIF_ISP_FILT_BASE + 0x000000bc)
#define CIF_ISP_FILT_THVAR_DIVID23    (CIF_ISP_FILT_BASE + 0x000000c0)
#define CIF_ISP_FILT_THVAR_DIVID4    (CIF_ISP_FILT_BASE + 0x000000c4)
#define CIF_ISP_FILT_TH_GRAD        (CIF_ISP_FILT_BASE + 0x000000c8)
#define CIF_ISP_FILT_TH_DIFF        (CIF_ISP_FILT_BASE + 0x000000cc)
#define CIF_ISP_FILT_TH_CSC        (CIF_ISP_FILT_BASE + 0x000000d0)
#define CIF_ISP_FILT_TH_VAR        (CIF_ISP_FILT_BASE + 0x000000d4)
#define CIF_ISP_FILT_LELEL_SEL        (CIF_ISP_FILT_BASE + 0x000000d8)
#define CIF_ISP_FILT_R_FCT        (CIF_ISP_FILT_BASE + 0x000000dc)
#define CIF_ISP_FILT_B_FCT        (CIF_ISP_FILT_BASE + 0x000000e0)

#define CIF_ISP_CAC_BASE        0x00002580
#define CIF_ISP_CAC_CTRL        (CIF_ISP_CAC_BASE + 0x00000000)
#define CIF_ISP_CAC_COUNT_START        (CIF_ISP_CAC_BASE + 0x00000004)
#define CIF_ISP_CAC_A            (CIF_ISP_CAC_BASE + 0x00000008)
#define CIF_ISP_CAC_B            (CIF_ISP_CAC_BASE + 0x0000000C)
#define CIF_ISP_CAC_C            (CIF_ISP_CAC_BASE + 0x00000010)
#define CIF_ISP_X_NORM            (CIF_ISP_CAC_BASE + 0x00000014)
#define CIF_ISP_Y_NORM            (CIF_ISP_CAC_BASE + 0x00000018)

#define CIF_ISP_EXP_BASE        0x00002600
#define CIF_ISP_EXP_CTRL        (CIF_ISP_EXP_BASE + 0x00000000)
#define CIF_ISP_EXP_H_OFFSET_V10    (CIF_ISP_EXP_BASE + 0x00000004)
#define CIF_ISP_EXP_V_OFFSET_V10    (CIF_ISP_EXP_BASE + 0x00000008)
#define CIF_ISP_EXP_H_SIZE_V10        (CIF_ISP_EXP_BASE + 0x0000000C)
#define CIF_ISP_EXP_V_SIZE_V10        (CIF_ISP_EXP_BASE + 0x00000010)
#define CIF_ISP_EXP_SIZE_V12        (CIF_ISP_EXP_BASE + 0x00000004)
#define CIF_ISP_EXP_OFFS_V12        (CIF_ISP_EXP_BASE + 0x00000008)
#define CIF_ISP_EXP_MEAN_V12        (CIF_ISP_EXP_BASE + 0x0000000c)
#define CIF_ISP_EXP_MEAN_00_V10        (CIF_ISP_EXP_BASE + 0x00000014)
#define CIF_ISP_EXP_MEAN_10_V10        (CIF_ISP_EXP_BASE + 0x00000018)
#define CIF_ISP_EXP_MEAN_20_V10        (CIF_ISP_EXP_BASE + 0x0000001c)
#define CIF_ISP_EXP_MEAN_30_V10        (CIF_ISP_EXP_BASE + 0x00000020)
#define CIF_ISP_EXP_MEAN_40_V10        (CIF_ISP_EXP_BASE + 0x00000024)
#define CIF_ISP_EXP_MEAN_01_V10        (CIF_ISP_EXP_BASE + 0x00000028)
#define CIF_ISP_EXP_MEAN_11_V10        (CIF_ISP_EXP_BASE + 0x0000002c)
#define CIF_ISP_EXP_MEAN_21_V10        (CIF_ISP_EXP_BASE + 0x00000030)
#define CIF_ISP_EXP_MEAN_31_V10        (CIF_ISP_EXP_BASE + 0x00000034)
#define CIF_ISP_EXP_MEAN_41_V10        (CIF_ISP_EXP_BASE + 0x00000038)
#define CIF_ISP_EXP_MEAN_02_V10        (CIF_ISP_EXP_BASE + 0x0000003c)
#define CIF_ISP_EXP_MEAN_12_V10        (CIF_ISP_EXP_BASE + 0x00000040)
#define CIF_ISP_EXP_MEAN_22_V10        (CIF_ISP_EXP_BASE + 0x00000044)
#define CIF_ISP_EXP_MEAN_32_V10        (CIF_ISP_EXP_BASE + 0x00000048)
#define CIF_ISP_EXP_MEAN_42_V10        (CIF_ISP_EXP_BASE + 0x0000004c)
#define CIF_ISP_EXP_MEAN_03_V10        (CIF_ISP_EXP_BASE + 0x00000050)
#define CIF_ISP_EXP_MEAN_13_V10        (CIF_ISP_EXP_BASE + 0x00000054)
#define CIF_ISP_EXP_MEAN_23_V10        (CIF_ISP_EXP_BASE + 0x00000058)
#define CIF_ISP_EXP_MEAN_33_V10        (CIF_ISP_EXP_BASE + 0x0000005c)
#define CIF_ISP_EXP_MEAN_43_V10        (CIF_ISP_EXP_BASE + 0x00000060)
#define CIF_ISP_EXP_MEAN_04_V10        (CIF_ISP_EXP_BASE + 0x00000064)
#define CIF_ISP_EXP_MEAN_14_V10        (CIF_ISP_EXP_BASE + 0x00000068)
#define CIF_ISP_EXP_MEAN_24_V10        (CIF_ISP_EXP_BASE + 0x0000006c)
#define CIF_ISP_EXP_MEAN_34_V10        (CIF_ISP_EXP_BASE + 0x00000070)
#define CIF_ISP_EXP_MEAN_44_V10        (CIF_ISP_EXP_BASE + 0x00000074)

#define CIF_ISP_BLS_BASE        0x00002700
#define CIF_ISP_BLS_CTRL        (CIF_ISP_BLS_BASE + 0x00000000)
#define CIF_ISP_BLS_SAMPLES        (CIF_ISP_BLS_BASE + 0x00000004)
#define CIF_ISP_BLS_H1_START        (CIF_ISP_BLS_BASE + 0x00000008)
#define CIF_ISP_BLS_H1_STOP        (CIF_ISP_BLS_BASE + 0x0000000c)
#define CIF_ISP_BLS_V1_START        (CIF_ISP_BLS_BASE + 0x00000010)
#define CIF_ISP_BLS_V1_STOP        (CIF_ISP_BLS_BASE + 0x00000014)
#define CIF_ISP_BLS_H2_START        (CIF_ISP_BLS_BASE + 0x00000018)
#define CIF_ISP_BLS_H2_STOP        (CIF_ISP_BLS_BASE + 0x0000001c)
#define CIF_ISP_BLS_V2_START        (CIF_ISP_BLS_BASE + 0x00000020)
#define CIF_ISP_BLS_V2_STOP        (CIF_ISP_BLS_BASE + 0x00000024)
#define CIF_ISP_BLS_A_FIXED        (CIF_ISP_BLS_BASE + 0x00000028)
#define CIF_ISP_BLS_B_FIXED        (CIF_ISP_BLS_BASE + 0x0000002c)
#define CIF_ISP_BLS_C_FIXED        (CIF_ISP_BLS_BASE + 0x00000030)
#define CIF_ISP_BLS_D_FIXED        (CIF_ISP_BLS_BASE + 0x00000034)
#define CIF_ISP_BLS_A_MEASURED        (CIF_ISP_BLS_BASE + 0x00000038)
#define CIF_ISP_BLS_B_MEASURED        (CIF_ISP_BLS_BASE + 0x0000003c)
#define CIF_ISP_BLS_C_MEASURED        (CIF_ISP_BLS_BASE + 0x00000040)
#define CIF_ISP_BLS_D_MEASURED        (CIF_ISP_BLS_BASE + 0x00000044)

#define CIF_ISP_DPF_BASE        0x00002800
#define CIF_ISP_DPF_MODE        (CIF_ISP_DPF_BASE + 0x00000000)
#define CIF_ISP_DPF_STRENGTH_R        (CIF_ISP_DPF_BASE + 0x00000004)
#define CIF_ISP_DPF_STRENGTH_G        (CIF_ISP_DPF_BASE + 0x00000008)
#define CIF_ISP_DPF_STRENGTH_B        (CIF_ISP_DPF_BASE + 0x0000000C)
#define CIF_ISP_DPF_S_WEIGHT_G_1_4    (CIF_ISP_DPF_BASE + 0x00000010)
#define CIF_ISP_DPF_S_WEIGHT_G_5_6    (CIF_ISP_DPF_BASE + 0x00000014)
#define CIF_ISP_DPF_S_WEIGHT_RB_1_4    (CIF_ISP_DPF_BASE + 0x00000018)
#define CIF_ISP_DPF_S_WEIGHT_RB_5_6    (CIF_ISP_DPF_BASE + 0x0000001C)
#define CIF_ISP_DPF_NULL_COEFF_0    (CIF_ISP_DPF_BASE + 0x00000020)
#define CIF_ISP_DPF_NULL_COEFF_1    (CIF_ISP_DPF_BASE + 0x00000024)
#define CIF_ISP_DPF_NULL_COEFF_2    (CIF_ISP_DPF_BASE + 0x00000028)
#define CIF_ISP_DPF_NULL_COEFF_3    (CIF_ISP_DPF_BASE + 0x0000002C)
#define CIF_ISP_DPF_NULL_COEFF_4    (CIF_ISP_DPF_BASE + 0x00000030)
#define CIF_ISP_DPF_NULL_COEFF_5    (CIF_ISP_DPF_BASE + 0x00000034)
#define CIF_ISP_DPF_NULL_COEFF_6    (CIF_ISP_DPF_BASE + 0x00000038)
#define CIF_ISP_DPF_NULL_COEFF_7    (CIF_ISP_DPF_BASE + 0x0000003C)
#define CIF_ISP_DPF_NULL_COEFF_8    (CIF_ISP_DPF_BASE + 0x00000040)
#define CIF_ISP_DPF_NULL_COEFF_9    (CIF_ISP_DPF_BASE + 0x00000044)
#define CIF_ISP_DPF_NULL_COEFF_10    (CIF_ISP_DPF_BASE + 0x00000048)
#define CIF_ISP_DPF_NULL_COEFF_11    (CIF_ISP_DPF_BASE + 0x0000004C)
#define CIF_ISP_DPF_NULL_COEFF_12    (CIF_ISP_DPF_BASE + 0x00000050)
#define CIF_ISP_DPF_NULL_COEFF_13    (CIF_ISP_DPF_BASE + 0x00000054)
#define CIF_ISP_DPF_NULL_COEFF_14    (CIF_ISP_DPF_BASE + 0x00000058)
#define CIF_ISP_DPF_NULL_COEFF_15    (CIF_ISP_DPF_BASE + 0x0000005C)
#define CIF_ISP_DPF_NULL_COEFF_16    (CIF_ISP_DPF_BASE + 0x00000060)
#define CIF_ISP_DPF_NF_GAIN_R        (CIF_ISP_DPF_BASE + 0x00000064)
#define CIF_ISP_DPF_NF_GAIN_GR        (CIF_ISP_DPF_BASE + 0x00000068)
#define CIF_ISP_DPF_NF_GAIN_GB        (CIF_ISP_DPF_BASE + 0x0000006C)
#define CIF_ISP_DPF_NF_GAIN_B        (CIF_ISP_DPF_BASE + 0x00000070)

#define CIF_ISP_DPCC_BASE        0x00002900
#define CIF_ISP_DPCC_MODE        (CIF_ISP_DPCC_BASE + 0x00000000)
#define CIF_ISP_DPCC_OUTPUT_MODE    (CIF_ISP_DPCC_BASE + 0x00000004)
#define CIF_ISP_DPCC_SET_USE        (CIF_ISP_DPCC_BASE + 0x00000008)
#define CIF_ISP_DPCC_METHODS_SET_1    (CIF_ISP_DPCC_BASE + 0x0000000C)
#define CIF_ISP_DPCC_METHODS_SET_2    (CIF_ISP_DPCC_BASE + 0x00000010)
#define CIF_ISP_DPCC_METHODS_SET_3    (CIF_ISP_DPCC_BASE + 0x00000014)
#define CIF_ISP_DPCC_LINE_THRESH_1    (CIF_ISP_DPCC_BASE + 0x00000018)
#define CIF_ISP_DPCC_LINE_MAD_FAC_1    (CIF_ISP_DPCC_BASE + 0x0000001C)
#define CIF_ISP_DPCC_PG_FAC_1        (CIF_ISP_DPCC_BASE + 0x00000020)
#define CIF_ISP_DPCC_RND_THRESH_1    (CIF_ISP_DPCC_BASE + 0x00000024)
#define CIF_ISP_DPCC_RG_FAC_1        (CIF_ISP_DPCC_BASE + 0x00000028)
#define CIF_ISP_DPCC_LINE_THRESH_2    (CIF_ISP_DPCC_BASE + 0x0000002C)
#define CIF_ISP_DPCC_LINE_MAD_FAC_2    (CIF_ISP_DPCC_BASE + 0x00000030)
#define CIF_ISP_DPCC_PG_FAC_2        (CIF_ISP_DPCC_BASE + 0x00000034)
#define CIF_ISP_DPCC_RND_THRESH_2    (CIF_ISP_DPCC_BASE + 0x00000038)
#define CIF_ISP_DPCC_RG_FAC_2        (CIF_ISP_DPCC_BASE + 0x0000003C)
#define CIF_ISP_DPCC_LINE_THRESH_3    (CIF_ISP_DPCC_BASE + 0x00000040)
#define CIF_ISP_DPCC_LINE_MAD_FAC_3    (CIF_ISP_DPCC_BASE + 0x00000044)
#define CIF_ISP_DPCC_PG_FAC_3        (CIF_ISP_DPCC_BASE + 0x00000048)
#define CIF_ISP_DPCC_RND_THRESH_3    (CIF_ISP_DPCC_BASE + 0x0000004C)
#define CIF_ISP_DPCC_RG_FAC_3        (CIF_ISP_DPCC_BASE + 0x00000050)
#define CIF_ISP_DPCC_RO_LIMITS        (CIF_ISP_DPCC_BASE + 0x00000054)
#define CIF_ISP_DPCC_RND_OFFS        (CIF_ISP_DPCC_BASE + 0x00000058)
#define CIF_ISP_DPCC_BPT_CTRL        (CIF_ISP_DPCC_BASE + 0x0000005C)
#define CIF_ISP_DPCC_BPT_NUMBER        (CIF_ISP_DPCC_BASE + 0x00000060)
#define CIF_ISP_DPCC_BPT_ADDR        (CIF_ISP_DPCC_BASE + 0x00000064)
#define CIF_ISP_DPCC_BPT_DATA        (CIF_ISP_DPCC_BASE + 0x00000068)

#define CIF_ISP_WDR_BASE        0x00002A00
#define CIF_ISP_WDR_CTRL        (CIF_ISP_WDR_BASE + 0x00000000)
#define CIF_ISP_WDR_TONECURVE_1        (CIF_ISP_WDR_BASE + 0x00000004)
#define CIF_ISP_WDR_TONECURVE_2        (CIF_ISP_WDR_BASE + 0x00000008)
#define CIF_ISP_WDR_TONECURVE_3        (CIF_ISP_WDR_BASE + 0x0000000C)
#define CIF_ISP_WDR_TONECURVE_4        (CIF_ISP_WDR_BASE + 0x00000010)
#define CIF_ISP_WDR_TONECURVE_YM_0    (CIF_ISP_WDR_BASE + 0x00000014)
#define CIF_ISP_WDR_TONECURVE_YM_1    (CIF_ISP_WDR_BASE + 0x00000018)
#define CIF_ISP_WDR_TONECURVE_YM_2    (CIF_ISP_WDR_BASE + 0x0000001C)
#define CIF_ISP_WDR_TONECURVE_YM_3    (CIF_ISP_WDR_BASE + 0x00000020)
#define CIF_ISP_WDR_TONECURVE_YM_4    (CIF_ISP_WDR_BASE + 0x00000024)
#define CIF_ISP_WDR_TONECURVE_YM_5    (CIF_ISP_WDR_BASE + 0x00000028)
#define CIF_ISP_WDR_TONECURVE_YM_6    (CIF_ISP_WDR_BASE + 0x0000002C)
#define CIF_ISP_WDR_TONECURVE_YM_7    (CIF_ISP_WDR_BASE + 0x00000030)
#define CIF_ISP_WDR_TONECURVE_YM_8    (CIF_ISP_WDR_BASE + 0x00000034)
#define CIF_ISP_WDR_TONECURVE_YM_9    (CIF_ISP_WDR_BASE + 0x00000038)
#define CIF_ISP_WDR_TONECURVE_YM_10    (CIF_ISP_WDR_BASE + 0x0000003C)
#define CIF_ISP_WDR_TONECURVE_YM_11    (CIF_ISP_WDR_BASE + 0x00000040)
#define CIF_ISP_WDR_TONECURVE_YM_12    (CIF_ISP_WDR_BASE + 0x00000044)
#define CIF_ISP_WDR_TONECURVE_YM_13    (CIF_ISP_WDR_BASE + 0x00000048)
#define CIF_ISP_WDR_TONECURVE_YM_14    (CIF_ISP_WDR_BASE + 0x0000004C)
#define CIF_ISP_WDR_TONECURVE_YM_15    (CIF_ISP_WDR_BASE + 0x00000050)
#define CIF_ISP_WDR_TONECURVE_YM_16    (CIF_ISP_WDR_BASE + 0x00000054)
#define CIF_ISP_WDR_TONECURVE_YM_17    (CIF_ISP_WDR_BASE + 0x00000058)
#define CIF_ISP_WDR_TONECURVE_YM_18    (CIF_ISP_WDR_BASE + 0x0000005C)
#define CIF_ISP_WDR_TONECURVE_YM_19    (CIF_ISP_WDR_BASE + 0x00000060)
#define CIF_ISP_WDR_TONECURVE_YM_20    (CIF_ISP_WDR_BASE + 0x00000064)
#define CIF_ISP_WDR_TONECURVE_YM_21    (CIF_ISP_WDR_BASE + 0x00000068)
#define CIF_ISP_WDR_TONECURVE_YM_22    (CIF_ISP_WDR_BASE + 0x0000006C)
#define CIF_ISP_WDR_TONECURVE_YM_23    (CIF_ISP_WDR_BASE + 0x00000070)
#define CIF_ISP_WDR_TONECURVE_YM_24    (CIF_ISP_WDR_BASE + 0x00000074)
#define CIF_ISP_WDR_TONECURVE_YM_25    (CIF_ISP_WDR_BASE + 0x00000078)
#define CIF_ISP_WDR_TONECURVE_YM_26    (CIF_ISP_WDR_BASE + 0x0000007C)
#define CIF_ISP_WDR_TONECURVE_YM_27    (CIF_ISP_WDR_BASE + 0x00000080)
#define CIF_ISP_WDR_TONECURVE_YM_28    (CIF_ISP_WDR_BASE + 0x00000084)
#define CIF_ISP_WDR_TONECURVE_YM_29    (CIF_ISP_WDR_BASE + 0x00000088)
#define CIF_ISP_WDR_TONECURVE_YM_30    (CIF_ISP_WDR_BASE + 0x0000008C)
#define CIF_ISP_WDR_TONECURVE_YM_31    (CIF_ISP_WDR_BASE + 0x00000090)
#define CIF_ISP_WDR_TONECURVE_YM_32    (CIF_ISP_WDR_BASE + 0x00000094)
#define CIF_ISP_WDR_OFFSET        (CIF_ISP_WDR_BASE + 0x00000098)
#define CIF_ISP_WDR_DELTAMIN        (CIF_ISP_WDR_BASE + 0x0000009C)
#define CIF_ISP_WDR_TONECURVE_1_SHD    (CIF_ISP_WDR_BASE + 0x000000A0)
#define CIF_ISP_WDR_TONECURVE_2_SHD    (CIF_ISP_WDR_BASE + 0x000000A4)
#define CIF_ISP_WDR_TONECURVE_3_SHD    (CIF_ISP_WDR_BASE + 0x000000A8)
#define CIF_ISP_WDR_TONECURVE_4_SHD    (CIF_ISP_WDR_BASE + 0x000000AC)
#define CIF_ISP_WDR_TONECURVE_YM_0_SHD    (CIF_ISP_WDR_BASE + 0x000000B0)
#define CIF_ISP_WDR_TONECURVE_YM_1_SHD    (CIF_ISP_WDR_BASE + 0x000000B4)
#define CIF_ISP_WDR_TONECURVE_YM_2_SHD    (CIF_ISP_WDR_BASE + 0x000000B8)
#define CIF_ISP_WDR_TONECURVE_YM_3_SHD    (CIF_ISP_WDR_BASE + 0x000000BC)
#define CIF_ISP_WDR_TONECURVE_YM_4_SHD    (CIF_ISP_WDR_BASE + 0x000000C0)
#define CIF_ISP_WDR_TONECURVE_YM_5_SHD    (CIF_ISP_WDR_BASE + 0x000000C4)
#define CIF_ISP_WDR_TONECURVE_YM_6_SHD    (CIF_ISP_WDR_BASE + 0x000000C8)
#define CIF_ISP_WDR_TONECURVE_YM_7_SHD    (CIF_ISP_WDR_BASE + 0x000000CC)
#define CIF_ISP_WDR_TONECURVE_YM_8_SHD    (CIF_ISP_WDR_BASE + 0x000000D0)
#define CIF_ISP_WDR_TONECURVE_YM_9_SHD    (CIF_ISP_WDR_BASE + 0x000000D4)
#define CIF_ISP_WDR_TONECURVE_YM_10_SHD    (CIF_ISP_WDR_BASE + 0x000000D8)
#define CIF_ISP_WDR_TONECURVE_YM_11_SHD    (CIF_ISP_WDR_BASE + 0x000000DC)
#define CIF_ISP_WDR_TONECURVE_YM_12_SHD    (CIF_ISP_WDR_BASE + 0x000000E0)
#define CIF_ISP_WDR_TONECURVE_YM_13_SHD    (CIF_ISP_WDR_BASE + 0x000000E4)
#define CIF_ISP_WDR_TONECURVE_YM_14_SHD    (CIF_ISP_WDR_BASE + 0x000000E8)
#define CIF_ISP_WDR_TONECURVE_YM_15_SHD    (CIF_ISP_WDR_BASE + 0x000000EC)
#define CIF_ISP_WDR_TONECURVE_YM_16_SHD    (CIF_ISP_WDR_BASE + 0x000000F0)
#define CIF_ISP_WDR_TONECURVE_YM_17_SHD    (CIF_ISP_WDR_BASE + 0x000000F4)
#define CIF_ISP_WDR_TONECURVE_YM_18_SHD    (CIF_ISP_WDR_BASE + 0x000000F8)
#define CIF_ISP_WDR_TONECURVE_YM_19_SHD    (CIF_ISP_WDR_BASE + 0x000000FC)
#define CIF_ISP_WDR_TONECURVE_YM_20_SHD    (CIF_ISP_WDR_BASE + 0x00000100)
#define CIF_ISP_WDR_TONECURVE_YM_21_SHD    (CIF_ISP_WDR_BASE + 0x00000104)
#define CIF_ISP_WDR_TONECURVE_YM_22_SHD    (CIF_ISP_WDR_BASE + 0x00000108)
#define CIF_ISP_WDR_TONECURVE_YM_23_SHD    (CIF_ISP_WDR_BASE + 0x0000010C)
#define CIF_ISP_WDR_TONECURVE_YM_24_SHD    (CIF_ISP_WDR_BASE + 0x00000110)
#define CIF_ISP_WDR_TONECURVE_YM_25_SHD    (CIF_ISP_WDR_BASE + 0x00000114)
#define CIF_ISP_WDR_TONECURVE_YM_26_SHD    (CIF_ISP_WDR_BASE + 0x00000118)
#define CIF_ISP_WDR_TONECURVE_YM_27_SHD    (CIF_ISP_WDR_BASE + 0x0000011C)
#define CIF_ISP_WDR_TONECURVE_YM_28_SHD    (CIF_ISP_WDR_BASE + 0x00000120)
#define CIF_ISP_WDR_TONECURVE_YM_29_SHD    (CIF_ISP_WDR_BASE + 0x00000124)
#define CIF_ISP_WDR_TONECURVE_YM_30_SHD    (CIF_ISP_WDR_BASE + 0x00000128)
#define CIF_ISP_WDR_TONECURVE_YM_31_SHD    (CIF_ISP_WDR_BASE + 0x0000012C)
#define CIF_ISP_WDR_TONECURVE_YM_32_SHD    (CIF_ISP_WDR_BASE + 0x00000130)

#define CIF_ISP_RKWDR_CTRL0        (CIF_ISP_WDR_BASE + 0x00000150)
#define CIF_ISP_RKWDR_CTRL1        (CIF_ISP_WDR_BASE + 0x00000154)
#define CIF_ISP_RKWDR_BLKOFF0        (CIF_ISP_WDR_BASE + 0x00000158)
#define CIF_ISP_RKWDR_AVGCLIP        (CIF_ISP_WDR_BASE + 0x0000015c)
#define CIF_ISP_RKWDR_COE_0        (CIF_ISP_WDR_BASE + 0x00000160)
#define CIF_ISP_RKWDR_COE_1        (CIF_ISP_WDR_BASE + 0x00000164)
#define CIF_ISP_RKWDR_COE_2        (CIF_ISP_WDR_BASE + 0x00000168)
#define CIF_ISP_RKWDR_COE_OFF        (CIF_ISP_WDR_BASE + 0x0000016c)
#define CIF_ISP_RKWDR_OVERL        (CIF_ISP_WDR_BASE + 0x00000170)
#define CIF_ISP_RKWDR_BLKOFF1        (CIF_ISP_WDR_BASE + 0x00000174)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW0_0TO3 (CIF_ISP_WDR_BASE + 0x00000180)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW0_4TO7 (CIF_ISP_WDR_BASE + 0x00000184)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW1_0TO3 (CIF_ISP_WDR_BASE + 0x00000188)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW1_4TO7 (CIF_ISP_WDR_BASE + 0x0000018c)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW2_0TO3 (CIF_ISP_WDR_BASE + 0x00000190)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW2_4TO7 (CIF_ISP_WDR_BASE + 0x00000194)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW3_0TO3 (CIF_ISP_WDR_BASE + 0x00000198)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW3_4TO7 (CIF_ISP_WDR_BASE + 0x0000019c)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW4_0TO3 (CIF_ISP_WDR_BASE + 0x000001a0)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW4_4TO7 (CIF_ISP_WDR_BASE + 0x000001a4)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW5_0TO3 (CIF_ISP_WDR_BASE + 0x000001a8)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW5_4TO7 (CIF_ISP_WDR_BASE + 0x000001ac)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW6_0TO3 (CIF_ISP_WDR_BASE + 0x000001b0)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW6_4TO7 (CIF_ISP_WDR_BASE + 0x000001b4)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW7_0TO3 (CIF_ISP_WDR_BASE + 0x000001b8)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW7_4TO7 (CIF_ISP_WDR_BASE + 0x000001bc)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW8_0TO3 (CIF_ISP_WDR_BASE + 0x000001c0)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW8_4TO7 (CIF_ISP_WDR_BASE + 0x000001c4)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW9_0TO3 (CIF_ISP_WDR_BASE + 0x000001c8)
#define CIF_ISP_RKWDR_BLKMEAN8_ROW9_4TO7 (CIF_ISP_WDR_BASE + 0x000001cc)

#define CIF_ISP_HIST_BASE_V12        0x00002C00
#define CIF_ISP_HIST_CTRL_V12        (CIF_ISP_HIST_BASE_V12 + 0x00000000)
#define CIF_ISP_HIST_SIZE_V12        (CIF_ISP_HIST_BASE_V12 + 0x00000004)
#define CIF_ISP_HIST_OFFS_V12        (CIF_ISP_HIST_BASE_V12 + 0x00000008)
#define CIF_ISP_HIST_DBG1_V12        (CIF_ISP_HIST_BASE_V12 + 0x0000000C)
#define CIF_ISP_HIST_DBG2_V12        (CIF_ISP_HIST_BASE_V12 + 0x0000001C)
#define CIF_ISP_HIST_DBG3_V12        (CIF_ISP_HIST_BASE_V12 + 0x0000002C)
#define CIF_ISP_HIST_WEIGHT_V12        (CIF_ISP_HIST_BASE_V12 + 0x0000003C)
#define CIF_ISP_HIST_BIN_V12        (CIF_ISP_HIST_BASE_V12 + 0x00000120)

#define CIF_ISP_VSM_BASE        0x00002F00
#define CIF_ISP_VSM_MODE        (CIF_ISP_VSM_BASE + 0x00000000)
#define CIF_ISP_VSM_H_OFFS        (CIF_ISP_VSM_BASE + 0x00000004)
#define CIF_ISP_VSM_V_OFFS        (CIF_ISP_VSM_BASE + 0x00000008)
#define CIF_ISP_VSM_H_SIZE        (CIF_ISP_VSM_BASE + 0x0000000C)
#define CIF_ISP_VSM_V_SIZE        (CIF_ISP_VSM_BASE + 0x00000010)
#define CIF_ISP_VSM_H_SEGMENTS        (CIF_ISP_VSM_BASE + 0x00000014)
#define CIF_ISP_VSM_V_SEGMENTS        (CIF_ISP_VSM_BASE + 0x00000018)
#define CIF_ISP_VSM_DELTA_H        (CIF_ISP_VSM_BASE + 0x0000001C)
#define CIF_ISP_VSM_DELTA_V        (CIF_ISP_VSM_BASE + 0x00000020)

#define CIF_ISP_CSI0_BASE        0x00007000
#define CIF_ISP_CSI0_CTRL0        (CIF_ISP_CSI0_BASE + 0x00000000)
#define CIF_ISP_CSI0_CTRL1        (CIF_ISP_CSI0_BASE + 0x00000004)
#define CIF_ISP_CSI0_CTRL2        (CIF_ISP_CSI0_BASE + 0x00000008)
#define CIF_ISP_CSI0_CSI2_RESETN    (CIF_ISP_CSI0_BASE + 0x00000010)
#define CIF_ISP_CSI0_PHY_STATE_RO    (CIF_ISP_CSI0_BASE + 0x00000014)
#define CIF_ISP_CSI0_DATA_IDS_1        (CIF_ISP_CSI0_BASE + 0x00000018)
#define CIF_ISP_CSI0_DATA_IDS_2        (CIF_ISP_CSI0_BASE + 0x0000001c)
#define CIF_ISP_CSI0_ERR1        (CIF_ISP_CSI0_BASE + 0x00000020)
#define CIF_ISP_CSI0_ERR2        (CIF_ISP_CSI0_BASE + 0x00000024)
#define CIF_ISP_CSI0_ERR3        (CIF_ISP_CSI0_BASE + 0x00000028)
#define CIF_ISP_CSI0_MASK1        (CIF_ISP_CSI0_BASE + 0x0000002c)
#define CIF_ISP_CSI0_MASK2        (CIF_ISP_CSI0_BASE + 0x00000030)
#define CIF_ISP_CSI0_MASK3        (CIF_ISP_CSI0_BASE + 0x00000034)
#define CIF_ISP_CSI0_SET_HEARDER    (CIF_ISP_CSI0_BASE + 0x00000038)
#define CIF_ISP_CSI0_CUR_HEADER_RO    (CIF_ISP_CSI0_BASE + 0x0000003c)
#define CIF_ISP_CSI0_DMATX0_CTRL    (CIF_ISP_CSI0_BASE + 0x00000040)
#define CIF_ISP_CSI0_DMATX0_LINECNT_RO    (CIF_ISP_CSI0_BASE + 0x00000044)
#define CIF_ISP_CSI0_DMATX0_PIC_SIZE    (CIF_ISP_CSI0_BASE + 0x00000048)
#define CIF_ISP_CSI0_DMATX0_PIC_OFF    (CIF_ISP_CSI0_BASE + 0x0000004c)
#define CIF_ISP_CSI0_FRAME_NUM_RO    (CIF_ISP_CSI0_BASE + 0x00000070)
#define CIF_ISP_CSI0_ISP_LINECNT_RO    (CIF_ISP_CSI0_BASE + 0x00000074)
#define CIF_ISP_CSI0_TX_IBUF_STATUS_RO    (CIF_ISP_CSI0_BASE + 0x00000078)
#define CIF_ISP_CSI0_VERSION        (CIF_ISP_CSI0_BASE + 0x0000007c)

void rkisp_disable_dcrop(struct rkisp_stream *stream, bool async);
void rkisp_config_dcrop(struct rkisp_stream *stream, struct v4l2_rect *rect, bool async);

void rkisp_dump_rsz_regs(struct rkisp_stream *stream);
void rkisp_disable_rsz(struct rkisp_stream *stream, bool async);
void rkisp_config_rsz(struct rkisp_stream *stream, struct v4l2_rect *in_y,
              struct v4l2_rect *in_c, struct v4l2_rect *out_y,
              struct v4l2_rect *out_c, bool async);

static inline void config_mi_ctrl(struct rkisp_stream *stream, u32 burst)
{
    void __iomem *base = stream->ispdev->base_addr;
    void __iomem *addr = base + CIF_MI_CTRL;
    u32 reg;

    reg = readl(addr) & ~GENMASK(19, 16);
    writel(reg | burst, addr);
    reg = readl(addr);
    writel(reg | CIF_MI_CTRL_INIT_BASE_EN, addr);
    reg = readl(addr);
    writel(reg | CIF_MI_CTRL_INIT_OFFSET_EN, addr);
}

static inline bool mp_is_stream_stopped(void __iomem *base)
{
    int en;

    en = CIF_MI_CTRL_SHD_MP_IN_ENABLED | CIF_MI_CTRL_SHD_RAW_OUT_ENABLED;
    return !(readl(base + CIF_MI_CTRL_SHD) & en);
}

static inline bool sp_is_stream_stopped(void __iomem *base)
{
    return !(readl(base + CIF_MI_CTRL_SHD) & CIF_MI_CTRL_SHD_SP_IN_ENABLED);
}

static inline void isp_set_bits(void __iomem *addr, u32 bit_mask, u32 val)
{
    u32 tmp = readl(addr) & ~bit_mask;

    writel(tmp | val, addr);
}

static inline void isp_clear_bits(void __iomem *addr, u32 bit_mask)
{
    u32 val = readl(addr);

    writel(val & ~bit_mask, addr);
}

static inline void mi_set_y_size(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.y_size_init);
}

static inline void mi_set_cb_size(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.cb_size_init);
}

static inline void mi_set_cr_size(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.cr_size_init);
}

static inline void mi_set_y_addr(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.y_base_ad_init);
}

static inline void mi_set_cb_addr(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.cb_base_ad_init);
}

static inline void mi_set_cr_addr(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.cr_base_ad_init);
}

static inline void mi_set_y_offset(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.y_offs_cnt_init);
}

static inline void mi_set_cb_offset(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.cb_offs_cnt_init);
}

static inline void mi_set_cr_offset(struct rkisp_stream *stream, int val)
{
    void __iomem *base = stream->ispdev->base_addr;

    writel(val, base + stream->config->mi.cr_offs_cnt_init);
}

static inline void mi_frame_end_int_enable(struct rkisp_stream *stream)
{
    struct rkisp_hw_dev *hw = stream->ispdev->hw_dev;
    void __iomem *base = !hw->is_unite ?
        hw->base_addr : hw->base_next_addr;
    void __iomem *addr = base + CIF_MI_IMSC;

    writel(CIF_MI_FRAME(stream) | readl(addr), addr);
}

static inline void mi_frame_end_int_disable(struct rkisp_stream *stream)
{
    struct rkisp_hw_dev *hw = stream->ispdev->hw_dev;
    void __iomem *base = !hw->is_unite ?
        hw->base_addr : hw->base_next_addr;
    void __iomem *addr = base + CIF_MI_IMSC;

    writel(~CIF_MI_FRAME(stream) & readl(addr), addr);
}

static inline void mi_frame_end_int_clear(struct rkisp_stream *stream)
{
    struct rkisp_hw_dev *hw = stream->ispdev->hw_dev;
    void __iomem *base = !hw->is_unite ?
        hw->base_addr : hw->base_next_addr;
    void __iomem *addr = base + CIF_MI_ICR;

    writel(CIF_MI_FRAME(stream), addr);
}

static inline void stream_data_path(struct rkisp_stream *stream)
{
    struct rkisp_device *dev = stream->ispdev;
    bool is_unite = dev->hw_dev->is_unite;
    u32 dpcl = 0;

    if (stream->id == RKISP_STREAM_MP)
        dpcl |= CIF_VI_DPCL_CHAN_MODE_MP | CIF_VI_DPCL_MP_MUX_MRSZ_MI;
    else if (stream->id == RKISP_STREAM_SP)
        dpcl |= CIF_VI_DPCL_CHAN_MODE_SP;

    if (dpcl)
        rkisp_unite_set_bits(dev, CIF_VI_DPCL, 0, dpcl, true, is_unite);
}

static inline void mp_set_uv_swap(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_XTD_FORMAT_CTRL;
    u32 reg = readl(addr) & ~BIT(0);

    writel(reg | CIF_MI_XTD_FMT_CTRL_MP_CB_CR_SWAP, addr);
}

static inline void sp_set_uv_swap(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_XTD_FORMAT_CTRL;
    u32 reg = readl(addr) & ~BIT(1);

    writel(reg | CIF_MI_XTD_FMT_CTRL_SP_CB_CR_SWAP, addr);
}

static inline void sp_set_y_width(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_SP_Y_PIC_WIDTH);
}

static inline void sp_set_y_height(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_SP_Y_PIC_HEIGHT);
}

static inline void sp_set_y_line_length(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_SP_Y_LLENGTH);
}

static inline void mp_mi_ctrl_set_format(void __iomem *base, u32 val)
{
    void __iomem *addr = base + CIF_MI_CTRL;
    u32 reg = readl(addr) & ~MI_CTRL_MP_FMT_MASK;

    writel(reg | val, addr);
}

static inline void sp_mi_ctrl_set_format(void __iomem *base, u32 val)
{
    void __iomem *addr = base + CIF_MI_CTRL;
    u32 reg = readl(addr) & ~MI_CTRL_SP_FMT_MASK;

    writel(reg | val, addr);
}

static inline void mi_ctrl_mpyuv_enable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(CIF_MI_CTRL_MP_ENABLE | readl(addr), addr);
}

static inline void mi_ctrl_mpyuv_disable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(~CIF_MI_CTRL_MP_ENABLE & readl(addr), addr);
}

static inline void mi_ctrl_mp_disable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(~(CIF_MI_CTRL_MP_ENABLE | CIF_MI_CTRL_RAW_ENABLE) & readl(addr),
           addr);
}

static inline void mi_ctrl_spyuv_enable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(CIF_MI_CTRL_SP_ENABLE | readl(addr), addr);
}

static inline void mi_ctrl_spyuv_disable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(~CIF_MI_CTRL_SP_ENABLE & readl(addr), addr);
}

static inline void mi_ctrl_sp_disable(void __iomem *base)
{
    mi_ctrl_spyuv_disable(base);
}

static inline void mi_ctrl_mpraw_enable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(CIF_MI_CTRL_RAW_ENABLE | readl(addr), addr);
}

static inline void mi_ctrl_mpraw_disable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(~CIF_MI_CTRL_RAW_ENABLE & readl(addr), addr);
}

static inline void mp_mi_ctrl_autoupdate_en(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(readl(addr) | CIF_MI_MP_AUTOUPDATE_ENABLE, addr);
}

static inline void sp_mi_ctrl_autoupdate_en(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL;

    writel(readl(addr) | CIF_MI_SP_AUTOUPDATE_ENABLE, addr);
}

static inline void force_cfg_update(struct rkisp_device *dev)
{
    u32 val = CIF_MI_CTRL_INIT_OFFSET_EN | CIF_MI_CTRL_INIT_BASE_EN;
    bool is_unite = dev->hw_dev->is_unite;

    dev->hw_dev->is_mi_update = true;
    rkisp_unite_set_bits(dev, CIF_MI_CTRL, 0, val, false, is_unite);
    val = CIF_MI_INIT_SOFT_UPD;
    rkisp_unite_write(dev, CIF_MI_INIT, val, true, is_unite);
}

static inline void dmatx0_ctrl(void __iomem *base, u32 val)
{
    writel(val, base + CIF_ISP_CSI0_DMATX0_CTRL);
}

static inline void dmatx0_enable(void __iomem *base)
{
    void __iomem *addr = base + CIF_ISP_CSI0_DMATX0_CTRL;

    writel(CIF_ISP_CSI0_DMATX0_EN | readl(addr), addr);
}

static inline void dmatx0_disable(void __iomem *base)
{
    void __iomem *addr = base + CIF_ISP_CSI0_DMATX0_CTRL;

    writel(~CIF_ISP_CSI0_DMATX0_EN & readl(addr), addr);
}

static inline void dmatx0_set_pic_size(void __iomem *base,
                    u32 width, u32 height)
{
    writel(height << 16 | width,
        base + CIF_ISP_CSI0_DMATX0_PIC_SIZE);
}

static inline void dmatx0_set_pic_off(void __iomem *base, u32 val)
{
    writel(val, base + CIF_ISP_CSI0_DMATX0_PIC_OFF);
}

static inline void mi_raw0_set_size(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_RAW0_SIZE_INIT);
}

static inline void mi_raw0_set_offs(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_RAW0_OFFS_CNT_INIT);
}

static inline void mi_raw0_set_length(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_RAW0_LENGTH);
}

static inline void mi_raw0_set_irq_offs(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_RAW0_IRQ_OFFS_INIT);
}

static inline void mi_raw0_set_addr(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_RAW0_BASE_AD_INIT);
}

static inline void mi_mipi_raw0_enable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL2;

    writel(CIF_MI_CTRL2_MIPI_RAW0_ENABLE | readl(addr), addr);
}

static inline void mi_mipi_raw0_disable(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_CTRL2;

    writel(~CIF_MI_CTRL2_MIPI_RAW0_ENABLE & readl(addr), addr);
}

static inline void mi_ctrl2(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_CTRL2);
}

static inline void mi_dmarx_ready_enable(struct rkisp_stream *stream)
{
    void __iomem *base = stream->ispdev->base_addr;
    void __iomem *addr = base + CIF_MI_IMSC;

    writel(CIF_MI_DMA_READY | readl(addr), addr);
}

static inline void mi_dmarx_ready_disable(struct rkisp_stream *stream)
{
    void __iomem *base = stream->ispdev->base_addr;
    void __iomem *addr = base + CIF_MI_IMSC;

    writel(~CIF_MI_DMA_READY & readl(addr), addr);
}

static inline void dmarx_set_uv_swap(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_XTD_FORMAT_CTRL;
    u32 reg = readl(addr) & ~BIT(2);

    writel(reg | CIF_MI_XTD_FMT_CTRL_DMA_CB_CR_SWAP, addr);
}

static inline void dmarx_set_y_width(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_DMA_Y_PIC_WIDTH);
}

static inline void dmarx_set_y_line_length(void __iomem *base, u32 val)
{
    writel(val, base + CIF_MI_DMA_Y_LLENGTH);
}

static inline void dmarx_ctrl(void __iomem *base, u32 val)
{
    void __iomem *addr = base + CIF_MI_DMA_CTRL;

    writel(val | readl(addr), addr);
}

static inline void mi_dmarx_start(void __iomem *base)
{
    void __iomem *addr = base + CIF_MI_DMA_START;

    writel(CIF_MI_DMA_START_ENABLE, addr);
}

#endif /* _RKISP_REGS_H */
