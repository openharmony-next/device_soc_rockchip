/*
 * Copyright 2015 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MODULE_TAG "mpi_enc_test"

#include <string.h>
#include "rk_mpi.h"

#include "mpp_env.h"
#include "mpp_mem.h"
#include "mpp_time.h"
#include "mpp_debug.h"
#include "mpp_common.h"
#include "securec.h"
#include "utils_mpp.h"
#include "mpi_enc_utils.h"
//#include "camera.h"

MPP_RET test_ctx_init(MpiEncMultiCtxInfo *info)
{
    MpiEncTestArgs *cmd = info->cmd;
    MpiEncTestData *p = &info->ctx;
    MPP_RET ret = MPP_OK;

    // get paramter from cmd
    p->width        = cmd->width;
    p->height       = cmd->height;
    p->hor_stride   = (cmd->hor_stride) ? (cmd->hor_stride) :
                      (MPP_ALIGN(cmd->width, 16));
    p->ver_stride   = (cmd->ver_stride) ? (cmd->ver_stride) :
                      (MPP_ALIGN(cmd->height, 16));
    p->fmt          = cmd->format;
    p->type         = cmd->type;
    p->bps          = cmd->bps_target;
    p->bps_min      = cmd->bps_min;
    p->bps_max      = cmd->bps_max;
    p->rc_mode      = cmd->rc_mode;
    p->frame_num    = cmd->frame_num;
    if (cmd->type == MPP_VIDEO_CodingMJPEG && p->frame_num == 0) {
        mpp_log("jpege default encode only one frame. Use -n [num] for rc case\n");
        p->frame_num = 1;
    }
    p->gop_mode     = cmd->gop_mode;
    p->gop_len      = cmd->gop_len;
    p->vi_len       = cmd->vi_len;

    p->fps_in_flex  = cmd->fps_in_flex;
    p->fps_in_den   = cmd->fps_in_den;
    p->fps_in_num   = cmd->fps_in_num;
    p->fps_out_flex = cmd->fps_out_flex;
    p->fps_out_den  = cmd->fps_out_den;
    p->fps_out_num  = cmd->fps_out_num;
    p->mdinfo_size  = (MPP_VIDEO_CodingHEVC == cmd->type) ?
                      (MPP_ALIGN(p->hor_stride, 64) >> 6) *
                      (MPP_ALIGN(p->ver_stride, 64) >> 6) * 32 :
                      (MPP_ALIGN(p->hor_stride, 64) >> 6) *
                      (MPP_ALIGN(p->ver_stride, 16) >> 4) * 8;

    if (cmd->file_input) {
        if (!strncmp(cmd->file_input, "/dev/video", 10)) {
            mpp_log("open camera device");
            p->cam_ctx = camera_source_init(cmd->file_input, 4, p->width, p->height, p->fmt);
            mpp_log("new framecap ok");
            if (p->cam_ctx == NULL)
                mpp_err("open %s fail", cmd->file_input);
        } else {
            p->fp_input = fopen(cmd->file_input, "rb");
            if (NULL == p->fp_input) {
                mpp_err("failed to open input file %s\n", cmd->file_input);
                mpp_err("create default yuv image for test\n");
            }
        }
    }

    if (cmd->file_output) {
        p->fp_output = fopen(cmd->file_output, "w+b");
        if (NULL == p->fp_output) {
            mpp_err("failed to open output file %s\n", cmd->file_output);
            ret = MPP_ERR_OPEN_FILE;
        }
    }

    p->fp_output = fopen("/data/genan.h264", "w+b");

    if (cmd->file_slt) {
        p->fp_verify = fopen(cmd->file_slt, "wt");
        if (!p->fp_verify)
            mpp_err("failed to open verify file %s\n", cmd->file_slt);
    }

    // update resource parameter
    switch (p->fmt & MPP_FRAME_FMT_MASK) {
    case MPP_FMT_YUV420SP:
    case MPP_FMT_YUV420P: {
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64) * 3 / 2;
    } break;

    case MPP_FMT_YUV422_YUYV :
    case MPP_FMT_YUV422_YVYU :
    case MPP_FMT_YUV422_UYVY :
    case MPP_FMT_YUV422_VYUY :
    case MPP_FMT_YUV422P :
    case MPP_FMT_YUV422SP : {
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64) * 2;
    } break;
    case MPP_FMT_RGB444 :
    case MPP_FMT_BGR444 :
    case MPP_FMT_RGB555 :
    case MPP_FMT_BGR555 :
    case MPP_FMT_RGB565 :
    case MPP_FMT_BGR565 :
    case MPP_FMT_RGB888 :
    case MPP_FMT_BGR888 :
    case MPP_FMT_RGB101010 :
    case MPP_FMT_BGR101010 :
    case MPP_FMT_ARGB8888 :
    case MPP_FMT_ABGR8888 :
    case MPP_FMT_BGRA8888 :
    case MPP_FMT_RGBA8888 : {
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64);
    } break;

    default: {
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64) * 4;
    } break;
    }

    if (MPP_FRAME_FMT_IS_FBC(p->fmt)) {
        if ((p->fmt & MPP_FRAME_FBC_MASK) == MPP_FRAME_FBC_AFBC_V1)
            p->header_size = MPP_ALIGN(MPP_ALIGN(p->width, 16) * MPP_ALIGN(p->height, 16) / 16, SZ_4K);
        else
            p->header_size = MPP_ALIGN(p->width, 16) * MPP_ALIGN(p->height, 16) / 16;
    } else {
        p->header_size = 0;
    }

    return ret;
}

MPP_RET test_ctx_deinit(MpiEncTestData *p)
{
    if (p) {
        if (p->cam_ctx) {
            camera_source_deinit(p->cam_ctx);
            p->cam_ctx = NULL;
        }
        if (p->fp_input) {
            fclose(p->fp_input);
            p->fp_input = NULL;
        }
        if (p->fp_output) {
            fclose(p->fp_output);
            p->fp_output = NULL;
        }
        if (p->fp_verify) {
            fclose(p->fp_verify);
            p->fp_verify = NULL;
        }
    }
    return MPP_OK;
}

MPP_RET test_mpp_enc_cfg_setup(MpiEncMultiCtxInfo *info)
{
    MpiEncTestArgs *cmd = info->cmd;
    MpiEncTestData *p = &info->ctx;
    MppApi *mpi = p->mpi;
    MppCtx ctx = p->ctx;
    MppEncCfg cfg = p->cfg;
    RK_U32 quiet = cmd->quiet;
    MPP_RET ret;

    /* setup default parameter */
    if (p->fps_in_den == 0)
        p->fps_in_den = 1;
    if (p->fps_in_num == 0)
        p->fps_in_num = 30;
    if (p->fps_out_den == 0)
        p->fps_out_den = 1;
    if (p->fps_out_num == 0)
        p->fps_out_num = 30;

    if (!p->bps)
        p->bps = p->width * p->height / 8 * (p->fps_out_num / p->fps_out_den);

    mpp_enc_cfg_set_s32(cfg, "prep:width", p->width);
    mpp_enc_cfg_set_s32(cfg, "prep:height", p->height);
    mpp_enc_cfg_set_s32(cfg, "prep:hor_stride", p->hor_stride);
    mpp_enc_cfg_set_s32(cfg, "prep:ver_stride", p->ver_stride);
    mpp_enc_cfg_set_s32(cfg, "prep:format", p->fmt);

    mpp_enc_cfg_set_s32(cfg, "rc:mode", p->rc_mode);

    /* fix input / output frame rate */
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_flex", p->fps_in_flex);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_num", p->fps_in_num);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_denorm", p->fps_in_den);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_flex", p->fps_out_flex);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_num", p->fps_out_num);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_denorm", p->fps_out_den);
    mpp_enc_cfg_set_s32(cfg, "rc:gop", p->gop_len ? p->gop_len : p->fps_out_num * 2);

    /* drop frame or not when bitrate overflow */
    mpp_enc_cfg_set_u32(cfg, "rc:drop_mode", MPP_ENC_RC_DROP_FRM_DISABLED);
    mpp_enc_cfg_set_u32(cfg, "rc:drop_thd", 20);        /* 20% of max bps */
    mpp_enc_cfg_set_u32(cfg, "rc:drop_gap", 1);         /* Do not continuous drop frame */

    /* setup bitrate for different rc_mode */
    mpp_enc_cfg_set_s32(cfg, "rc:bps_target", p->bps);
    switch (p->rc_mode) {
    case MPP_ENC_RC_MODE_FIXQP : {
        /* do not setup bitrate on FIXQP mode */
    } break;
    case MPP_ENC_RC_MODE_CBR : {
        /* CBR mode has narrow bound */
        mpp_enc_cfg_set_s32(cfg, "rc:bps_max", p->bps_max ? p->bps_max : p->bps * 17 / 16);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_min", p->bps_min ? p->bps_min : p->bps * 15 / 16);
    } break;
    case MPP_ENC_RC_MODE_VBR :
    case MPP_ENC_RC_MODE_AVBR : {
        /* VBR mode has wide bound */
        mpp_enc_cfg_set_s32(cfg, "rc:bps_max", p->bps_max ? p->bps_max : p->bps * 17 / 16);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_min", p->bps_min ? p->bps_min : p->bps * 1 / 16);
    } break;
    default : {
        /* default use CBR mode */
        mpp_enc_cfg_set_s32(cfg, "rc:bps_max", p->bps_max ? p->bps_max : p->bps * 17 / 16);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_min", p->bps_min ? p->bps_min : p->bps * 15 / 16);
    } break;
    }

    /* setup qp for different codec and rc_mode */
    switch (p->type) {
    case MPP_VIDEO_CodingAVC :
    case MPP_VIDEO_CodingHEVC : {
        switch (p->rc_mode) {
        case MPP_ENC_RC_MODE_FIXQP : {
            RK_S32 fix_qp = cmd->qp_init;

            mpp_enc_cfg_set_s32(cfg, "rc:qp_init", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_max", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_min", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", fix_qp);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_ip", 0);
        } break;
        case MPP_ENC_RC_MODE_CBR :
        case MPP_ENC_RC_MODE_VBR :
        case MPP_ENC_RC_MODE_AVBR : {
            mpp_enc_cfg_set_s32(cfg, "rc:qp_init", -1);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_max", 51);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_min", 10);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", 51);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", 10);
            mpp_enc_cfg_set_s32(cfg, "rc:qp_ip", 2);
        } break;
        default : {
            mpp_err_f("unsupport encoder rc mode %d\n", p->rc_mode);
        } break;
        }
    } break;
    case MPP_VIDEO_CodingVP8 : {
        /* vp8 only setup base qp range */
        mpp_enc_cfg_set_s32(cfg, "rc:qp_init", 40);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_max",  127);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_min",  0);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_max_i", 127);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_min_i", 0);
        mpp_enc_cfg_set_s32(cfg, "rc:qp_ip", 6);
    } break;
    case MPP_VIDEO_CodingMJPEG : {
        /* jpeg use special codec config to control qtable */
        mpp_enc_cfg_set_s32(cfg, "jpeg:q_factor", 80);
        mpp_enc_cfg_set_s32(cfg, "jpeg:qf_max", 99);
        mpp_enc_cfg_set_s32(cfg, "jpeg:qf_min", 1);
    } break;
    default : {
    } break;
    }

    /* setup codec  */
    mpp_enc_cfg_set_s32(cfg, "codec:type", p->type);
    switch (p->type) {
    case MPP_VIDEO_CodingAVC : {
        /*
         * H.264 profile_idc parameter
         * 66  - Baseline profile
         * 77  - Main profile
         * 100 - High profile
         */
        mpp_enc_cfg_set_s32(cfg, "h264:profile", 100);
        /*
         * H.264 level_idc parameter
         * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
         * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
         * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
         * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
         * 50 / 51 / 52         - 4K@30fps
         */
        mpp_enc_cfg_set_s32(cfg, "h264:level", 40);
        mpp_enc_cfg_set_s32(cfg, "h264:cabac_en", 1);
        mpp_enc_cfg_set_s32(cfg, "h264:cabac_idc", 0);
        mpp_enc_cfg_set_s32(cfg, "h264:trans8x8", 1);
    } break;
    case MPP_VIDEO_CodingHEVC :
    case MPP_VIDEO_CodingMJPEG :
    case MPP_VIDEO_CodingVP8 : {
    } break;
    default : {
        mpp_err_f("unsupport encoder coding type %d\n", p->type);
    } break;
    }

    p->split_mode = 0;
    p->split_arg = 0;
    p->split_out = 0;

    mpp_env_get_u32("split_mode", &p->split_mode, MPP_ENC_SPLIT_NONE);
    mpp_env_get_u32("split_arg", &p->split_arg, 0);
    mpp_env_get_u32("split_out", &p->split_out, 0);

    if (p->split_mode) {
        mpp_log_q(quiet, "%p split mode %d arg %d out %d\n", ctx,
                  p->split_mode, p->split_arg, p->split_out);
        mpp_enc_cfg_set_s32(cfg, "split:mode", p->split_mode);
        mpp_enc_cfg_set_s32(cfg, "split:arg", p->split_arg);
        mpp_enc_cfg_set_s32(cfg, "split:out", p->split_out);
    }

    ret = mpi->control(ctx, MPP_ENC_SET_CFG, cfg);
    if (ret) {
        mpp_err("mpi control enc set cfg failed ret %d\n", ret);
        goto RET;
    }

    /* optional */
    p->sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
    ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->sei_mode);
    if (ret) {
        mpp_err("mpi control enc set sei cfg failed ret %d\n", ret);
        goto RET;
    }

    if (p->type == MPP_VIDEO_CodingAVC || p->type == MPP_VIDEO_CodingHEVC) {
        p->header_mode = MPP_ENC_HEADER_MODE_EACH_IDR;
        ret = mpi->control(ctx, MPP_ENC_SET_HEADER_MODE, &p->header_mode);
        if (ret) {
            mpp_err("mpi control enc set header mode failed ret %d\n", ret);
            goto RET;
        }
    }

    RK_U32 gop_mode = p->gop_mode;

    mpp_env_get_u32("gop_mode", &gop_mode, gop_mode);
    if (gop_mode) {
        MppEncRefCfg ref;

        mpp_enc_ref_cfg_init(&ref);

        if (p->gop_mode < 4)
            mpi_enc_gen_ref_cfg(ref, gop_mode);
        else
            mpi_enc_gen_smart_gop_ref_cfg(ref, p->gop_len, p->vi_len);

        ret = mpi->control(ctx, MPP_ENC_SET_REF_CFG, ref);
        if (ret) {
            mpp_err("mpi control enc set ref cfg failed ret %d\n", ret);
            goto RET;
        }
        mpp_enc_ref_cfg_deinit(&ref);
    }

    /* setup test mode by env */
    mpp_env_get_u32("osd_enable", &p->osd_enable, 0);
    mpp_env_get_u32("osd_mode", &p->osd_mode, MPP_ENC_OSD_PLT_TYPE_DEFAULT);
    mpp_env_get_u32("roi_enable", &p->roi_enable, 0);
    mpp_env_get_u32("user_data_enable", &p->user_data_enable, 0);

    if (p->roi_enable) {
        mpp_enc_roi_init(&p->roi_ctx, p->width, p->height, p->type, 4);
        mpp_assert(p->roi_ctx);
    }

RET:
    return ret;
}

int hal_mpp_get_sps(void *ctxs, unsigned char *buf, size_t *buf_size)
{
    MpiEncTestData *p = &((MpiEncMultiCtxInfo *)ctxs)->ctx;
    MppApi *mpi = p->mpi;
    MppCtx ctx = p->ctx;
    int ret;

    if (!p) {
        mpp_err("mpi control enc get extra info failed\n");
        return MPP_NOK;
    }

    if (p->type == MPP_VIDEO_CodingAVC || p->type == MPP_VIDEO_CodingHEVC) {
        MppPacket packet = NULL;

        /*
         * Can use packet with normal malloc buffer as input not pkt_buf.
         * Please refer to vpu_api_legacy.cpp for normal buffer case.
         * Using pkt_buf buffer here is just for simplifing demo.
         */
        mpp_packet_init_with_buffer(&packet, p->pkt_buf);
        /* NOTE: It is important to clear output packet length!! */
        mpp_packet_set_length(packet, 0);

        ret = mpi->control(ctx, MPP_ENC_GET_HDR_SYNC, packet);
        if (ret) {
            mpp_err("mpi control enc get extra info failed\n");
            return MPP_NOK;
        } else {
            /* get and write sps/pps for H.264 */
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);

            if (*buf_size < len) {
                mpp_err("mpi buffer size too small\n");
                ret = MPP_NOK;
                mpp_packet_deinit(&packet);
                return ret;
            }

            errno_t eok = memcpy_s(buf, len, ptr, len);
            if (eok != EOK) {
                mpp_err("memcpy_s failed\n");
                return MPP_NOK;
            }

            *buf_size = len;

        }
        mpp_packet_deinit(&packet);
    }

    ret = MPP_OK;
    return ret;
}

int hal_mpp_encode(void *ctxs, int dma_fd, unsigned char *buf, size_t *buf_size)
{
    MpiEncMultiCtxInfo *info = (MpiEncMultiCtxInfo *)ctxs;
    MpiEncTestArgs *cmd = info->cmd;
    MpiEncMultiCtxRet *enc_ret = &info->ret;
    MpiEncTestData *p = &info->ctx;
    MppApi *mpi = p->mpi;
    MppCtx ctx = p->ctx;
    RK_U32 quiet = cmd->quiet;
    RK_S32 chn = info->chn;
    DataCrc checkcrc;
    RK_S64 t_s = 0;
    RK_S64 t_e = 0;
    size_t bufPos = 0;

    t_s = mpp_time();

    MppBuffer hbuf = NULL;
    MppBufferInfo inf;
    memset(&inf, 0, sizeof(MppBufferInfo));
    inf.type = MPP_BUFFER_TYPE_EXT_DMA;
    inf.fd =  dma_fd;
    inf.size = p->frame_size & 0x07ffffff;
    inf.index = (p->frame_size & 0xf8000000) >> 27;
    mpp_buffer_import(&hbuf, &inf);

    memset(&checkcrc, 0, sizeof(checkcrc));
    checkcrc.sum = mpp_malloc(RK_ULONG, 512);

    //while (!p->pkt_eos) {
    MppMeta meta = NULL;
    MppFrame frame = NULL;
    MppPacket packet = NULL;
    RK_U32 eoi = 1;

    MPP_RET ret = mpp_frame_init(&frame);
    if (ret) {
        mpp_err_f("mpp_frame_init failed\n");
        goto RET;
    }

    mpp_frame_set_width(frame, p->width);
    mpp_frame_set_height(frame, p->height);
    mpp_frame_set_hor_stride(frame, p->hor_stride);
    mpp_frame_set_ver_stride(frame, p->ver_stride);
    mpp_frame_set_fmt(frame, p->fmt);
    mpp_frame_set_eos(frame, p->frm_eos);

    //mpp_frame_set_buffer(frame, p->frm_buf);
    mpp_frame_set_buffer(frame, hbuf);

    meta = mpp_frame_get_meta(frame);
    mpp_packet_init_with_buffer(&packet, p->pkt_buf);
    /* NOTE: It is important to clear output packet length!! */
    mpp_packet_set_length(packet, 0);
    mpp_meta_set_packet(meta, KEY_OUTPUT_PACKET, packet);
    mpp_meta_set_buffer(meta, KEY_MOTION_INFO, p->md_info);

    if (p->osd_enable || p->user_data_enable || p->roi_enable) {
        if (p->user_data_enable) {
            MppEncUserData user_data;
            char *str = "this is user data\n";

            if ((p->frame_count & 10) == 0) {
                user_data.pdata = str;
                user_data.len = strlen(str) + 1;
                mpp_meta_set_ptr(meta, KEY_USER_DATA, &user_data);
            }
            static RK_U8 uuid_debug_info[16] = {
                0x57, 0x68, 0x97, 0x80, 0xe7, 0x0c, 0x4b, 0x65,
                0xa9, 0x06, 0xae, 0x29, 0x94, 0x11, 0xcd, 0x9a
            };

            MppEncUserDataSet data_group;
            MppEncUserDataFull datas[2];
            char *str1 = "this is user data 1\n";
            char *str2 = "this is user data 2\n";
            data_group.count = 2;
            datas[0].len = strlen(str1) + 1;
            datas[0].pdata = str1;
            datas[0].uuid = uuid_debug_info;

            datas[1].len = strlen(str2) + 1;
            datas[1].pdata = str2;
            datas[1].uuid = uuid_debug_info;

            data_group.datas = datas;

            mpp_meta_set_ptr(meta, KEY_USER_DATAS, &data_group);
        }

        if (p->osd_enable) {
            /* gen and cfg osd plt */
            mpi_enc_gen_osd_plt(&p->osd_plt, p->frame_count);

            p->osd_plt_cfg.change = MPP_ENC_OSD_PLT_CFG_CHANGE_ALL;
            p->osd_plt_cfg.type = MPP_ENC_OSD_PLT_TYPE_USERDEF;
            p->osd_plt_cfg.plt = &p->osd_plt;

            ret = mpi->control(ctx, MPP_ENC_SET_OSD_PLT_CFG, &p->osd_plt_cfg);
            if (ret) {
                mpp_err("mpi control enc set osd plt failed ret %d\n", ret);
                goto RET;
            }

            /* gen and cfg osd plt */
            mpi_enc_gen_osd_data(&p->osd_data, p->buf_grp, p->width,
                                 p->height, p->frame_count);
            mpp_meta_set_ptr(meta, KEY_OSD_DATA, (void*)&p->osd_data);
        }

        if (p->roi_enable) {
            RoiRegionCfg *region = &p->roi_region;

            /* calculated in pixels */
            region->x = MPP_ALIGN(p->width / 8, 16);
            region->y = MPP_ALIGN(p->height / 8, 16);
            region->w = 128;
            region->h = 256;
            region->force_intra = 0;
            region->qp_mode = 1;
            region->qp_val = 24;

            mpp_enc_roi_add_region(p->roi_ctx, region);

            region->x = MPP_ALIGN(p->width / 2, 16);
            region->y = MPP_ALIGN(p->height / 4, 16);
            region->w = 256;
            region->h = 128;
            region->force_intra = 1;
            region->qp_mode = 1;
            region->qp_val = 10;

            mpp_enc_roi_add_region(p->roi_ctx, region);

            /* send roi info by metadata */
            mpp_enc_roi_setup_meta(p->roi_ctx, meta);
        }
    }

    if (!p->first_frm)
        p->first_frm = mpp_time();
    /*
     * NOTE: in non-block mode the frame can be resent.
     * The default input timeout mode is block.
     *
     * User should release the input frame to meet the requirements of
     * resource creator must be the resource destroyer.
     */
    ret = mpi->encode_put_frame(ctx, frame);
    if (ret) {
        mpp_err("chn %d encode put frame failed\n", chn);
        mpp_frame_deinit(&frame);
        if (hbuf) {
            mpp_buffer_put(hbuf);
        }
        goto RET;
    }

    mpp_frame_deinit(&frame);

    do {
        ret = mpi->encode_get_packet(ctx, &packet);
        if (ret) {
            if (hbuf) {
                mpp_buffer_put(hbuf);
            }
            mpp_err("chn %d encode get packet failed\n", chn);
            goto RET;
        }
        mpp_assert(packet);

        if (packet) {
            // write packet to file here
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);
            memcpy_s(&buf[bufPos], len, ptr, len);
            bufPos += len;
            char log_buf[256];
            RK_S32 log_size = sizeof(log_buf) - 1;
            RK_S32 log_len = 0;

            if (!p->first_pkt)
                p->first_pkt = mpp_time();

            p->pkt_eos = mpp_packet_get_eos(packet);
            if (p->fp_output)
                fwrite(ptr, 1, len, p->fp_output);

            log_len += snprintf(log_buf + log_len, log_size - log_len,
                                "encoded frame %-4d", p->frame_count);

            /* for low delay partition encoding */
            if (mpp_packet_is_partition(packet)) {
                eoi = mpp_packet_is_eoi(packet);

                log_len += snprintf(log_buf + log_len, log_size - log_len,
                                    " pkt %d", p->frm_pkt_cnt);
                p->frm_pkt_cnt = (eoi) ? (0) : (p->frm_pkt_cnt + 1);
            }

            log_len += snprintf(log_buf + log_len, log_size - log_len,
                                " size %-7zu", len);

            if (mpp_packet_has_meta(packet)) {
                meta = mpp_packet_get_meta(packet);
                RK_S32 temporal_id = 0;
                RK_S32 lt_idx = -1;
                RK_S32 avg_qp = -1;

                if (MPP_OK == mpp_meta_get_s32(meta, KEY_TEMPORAL_ID, &temporal_id))
                    log_len += snprintf(log_buf + log_len, log_size - log_len,
                                        " tid %d", temporal_id);

                if (MPP_OK == mpp_meta_get_s32(meta, KEY_LONG_REF_IDX, &lt_idx))
                    log_len += snprintf(log_buf + log_len, log_size - log_len,
                                        " lt %d", lt_idx);

                if (MPP_OK == mpp_meta_get_s32(meta, KEY_ENC_AVERAGE_QP, &avg_qp))
                    log_len += snprintf(log_buf + log_len, log_size - log_len,
                                        " qp %d", avg_qp);
            }

            mpp_log_q(quiet, "chn %d %s\n", chn, log_buf);

            //mpp_packet_deinit(&packet);
            fps_calc_inc(cmd->fps);

            p->stream_size += len;
            p->frame_count += eoi;

            if (p->pkt_eos) {
                mpp_log_q(quiet, "chn %d found last packet\n", chn);
                mpp_assert(p->frm_eos);
            }
        }
    } while (!eoi);

RET:
    MPP_FREE(checkcrc.sum);
    t_e = mpp_time();

    if (hbuf) {
        mpp_buffer_put(hbuf);
    }
    mpp_packet_deinit(&packet);

    enc_ret->elapsed_time = t_e - t_s;
    enc_ret->frame_count = p->frame_count;
    enc_ret->stream_size = p->stream_size;
    enc_ret->frame_rate = (float)p->frame_count * 1000000 / enc_ret->elapsed_time;
    enc_ret->bit_rate = (p->stream_size * 8 * (p->fps_out_num / p->fps_out_den)) / p->frame_count;
    enc_ret->delay = p->first_pkt - p->first_frm;

    *buf_size = bufPos;

    return ret;
}

void hal_mpp_ctx_delete(void *ctx)
{
    MpiEncMultiCtxInfo *info = (MpiEncMultiCtxInfo *)ctx;
    MpiEncTestData *p = &info->ctx;
    MpiEncTestArgs* cmd = info->cmd;
    int ret = 0;

    ret = p->mpi->reset(p->ctx);
    if (ret) {
        mpp_err("mpi->reset failed\n");
    }

    if (p->ctx) {
        mpp_destroy(p->ctx);
        p->ctx = NULL;
    }

    if (p->cfg) {
        mpp_enc_cfg_deinit(p->cfg);
        p->cfg = NULL;
    }

    if (p->frm_buf) {
        mpp_buffer_put(p->frm_buf);
        p->frm_buf = NULL;
    }

    if (p->pkt_buf) {
        mpp_buffer_put(p->pkt_buf);
        p->pkt_buf = NULL;
    }

    if (p->md_info) {
        mpp_buffer_put(p->md_info);
        p->md_info = NULL;
    }

    if (p->osd_data.buf) {
        mpp_buffer_put(p->osd_data.buf);
        p->osd_data.buf = NULL;
    }

    if (p->buf_grp) {
        mpp_buffer_group_put(p->buf_grp);
        p->buf_grp = NULL;
    }

    if (p->roi_ctx) {
        mpp_enc_roi_deinit(p->roi_ctx);
        p->roi_ctx = NULL;
    }

    test_ctx_deinit(p);
    MPP_FREE(info);

    if (NULL == cmd)
        return;

    if (cmd->cfg_ini) {
        iniparser_freedict(cmd->cfg_ini);
        cmd->cfg_ini = NULL;
    }

    if (cmd->fps) {
        fps_calc_deinit(cmd->fps);
        cmd->fps = NULL;
    }
    MPP_FREE(cmd);

    return;
}

void *hal_mpp_ctx_create(MpiEncTestArgs *args)
{
    MpiEncMultiCtxInfo *ctxs = mpp_calloc(MpiEncMultiCtxInfo, args->nthreads);
    if (NULL == ctxs) {
        mpp_err("failed to alloc context for instances\n");
        return NULL;
    }
    ctxs[0].cmd = args;
    ctxs[0].chn = 0;
    ctxs[0].name = "rk_camera";

    MpiEncMultiCtxInfo *info = ctxs;
    MpiEncTestArgs *cmd = info->cmd;
    MpiEncTestData *p = &info->ctx;
    MppPollType timeout = MPP_POLL_BLOCK;
    RK_U32 quiet = cmd->quiet;

    MPP_RET ret = test_ctx_init(info);
    if (ret) {
        mpp_err_f("test data init failed ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    ret = mpp_buffer_group_get_internal(&p->buf_grp, MPP_BUFFER_TYPE_DRM);
    if (ret) {
        mpp_err_f("failed to get mpp buffer group ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    ret = mpp_buffer_get(p->buf_grp, &p->frm_buf, p->frame_size + p->header_size);
    if (ret) {
        mpp_err_f("failed to get buffer for input frame ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    ret = mpp_buffer_get(p->buf_grp, &p->pkt_buf, p->frame_size);
    if (ret) {
        mpp_err_f("failed to get buffer for output packet ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    ret = mpp_buffer_get(p->buf_grp, &p->md_info, p->mdinfo_size);
    if (ret) {
        mpp_err_f("failed to get buffer for motion info output packet ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    // encoder demo
    ret = mpp_create(&p->ctx, &p->mpi);
    if (ret) {
        mpp_err("mpp_create failed ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    mpp_log_q(quiet, "%p encoder test start w %d h %d type %d\n",
              p->ctx, p->width, p->height, p->type);

    ret = p->mpi->control(p->ctx, MPP_SET_OUTPUT_TIMEOUT, &timeout);
    if (MPP_OK != ret) {
        mpp_err("mpi control set output timeout %d ret %d\n", timeout, ret);
        goto MPP_TEST_OUT;
    }

    ret = mpp_init(p->ctx, MPP_CTX_ENC, p->type);
    if (ret) {
        mpp_err("mpp_init failed ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    ret = mpp_enc_cfg_init(&p->cfg);
    if (ret) {
        mpp_err_f("mpp_enc_cfg_init failed ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    ret = test_mpp_enc_cfg_setup(info);
    if (ret) {
        mpp_err_f("test mpp setup failed ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    return info;

MPP_TEST_OUT:
    if (p->ctx) {
        mpp_destroy(p->ctx);
        p->ctx = NULL;
    }

    if (p->cfg) {
        mpp_enc_cfg_deinit(p->cfg);
        p->cfg = NULL;
    }

    if (p->frm_buf) {
        mpp_buffer_put(p->frm_buf);
        p->frm_buf = NULL;
    }

    if (p->pkt_buf) {
        mpp_buffer_put(p->pkt_buf);
        p->pkt_buf = NULL;
    }

    if (p->md_info) {
        mpp_buffer_put(p->md_info);
        p->md_info = NULL;
    }

    if (p->osd_data.buf) {
        mpp_buffer_put(p->osd_data.buf);
        p->osd_data.buf = NULL;
    }

    if (p->buf_grp) {
        mpp_buffer_group_put(p->buf_grp);
        p->buf_grp = NULL;
    }

    if (p->roi_ctx) {
        mpp_enc_roi_deinit(p->roi_ctx);
        p->roi_ctx = NULL;
    }

    return NULL;
}
