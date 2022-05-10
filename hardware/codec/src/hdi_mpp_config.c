/*
 * Copyright (c) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hdi_mpp_config.h"
#include <securec.h>
#include <hdf_base.h>
#include <hdf_log.h>
#include "mpp_common.h"

#define HDF_LOG_TAG codec_hdi_mpp_config
#define MPP_ALIGN_STRIDE_WITH_EIGHT         8
#define MPP_ALIGN_STRIDE_WITH_SIXTEEN       16
#define MPP_ALIGN_STRIDE_WITH_SIXTY_FOUR    64
#define MPP_ALIGN_MULTIPLE_WITH_TWO         2
#define MPP_ALIGN_MULTIPLE_WITH_THREE       3
#define MPP_ALIGN_MULTIPLE_WITH_FOUR        4
#define MPP_ALIGN_DIVISOR_WITH_TWO          2
#define MPP_ALIGN_DIVISOR_WITH_THREE        3
#define MPP_ALIGN_DIVISOR_WITH_FOUR         4
#define MPP_ALIGN_DIVISOR_WITH_SIXTEEN      16
#define GOP_MODE_THRESHOLD_VALUE            4

int32_t SetSplitParse(RKHdiBaseComponent *pBaseComponent, MppDecCfg cfg)
{
    MPP_RET ret = MPP_OK;
    MppCtx ctx = pBaseComponent->ctx;
    RK_U32 need_split = 1;

    ret = pBaseComponent->mppApi->HdiMppDecCfgSetU32(cfg, "base:split_parse", need_split);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: %{public}p failed to set split_parse ret %{public}d", __func__, ctx, ret);
        return HDF_FAILURE;
    }

    ret = pBaseComponent->mpi->control(ctx, MPP_DEC_SET_CFG, cfg);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: failed to set cfg %{public}p ret %{public}d", __func__, cfg, ret);
        UINTPTR comp = ctx;
        UINTPTR appData = NULL;
        EventType event = EVENT_ERROR;
        uint32_t data1 = 0;
        uint32_t data2 = 0;
        UINTPTR eventData = NULL;
        pBaseComponent->pCallbacks->OnEvent(comp, appData, event, data1, data2, eventData);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t SetCodecFrameNum(RKHdiBaseComponent *pBaseComponent, Param *params)
{
    RK_S32 num = 0;
    memcpy_s(&num, sizeof(RK_S32), params->val, params->size);
    pBaseComponent->frameNum = num;
    HDF_LOGI("%{public}s: set frame number: %{public}d", __func__, pBaseComponent->frameNum);
    return HDF_SUCCESS;
}

int32_t GetDefaultConfig(RKHdiBaseComponent *pBaseComponent)
{
    MPP_RET ret = MPP_OK;
    MppCtx ctx = pBaseComponent->ctx;
    RKMppApi *mppApi = pBaseComponent->mppApi;

    mppApi->HdiMppDecCfgInit(&pBaseComponent->cfg);
    ret = pBaseComponent->mpi->control(ctx, MPP_DEC_GET_CFG, pBaseComponent->cfg);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: config MPP_DEC_GET_CFG failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t GetBufferSize(RKHdiBaseComponent *pBaseComponent, Param *params)
{
    if (params->val == NULL) {
        return HDF_FAILURE;
    }

    RKMppApi *mppApi = pBaseComponent->mppApi;
    RK_U32 buf_size = 0;
    if (pBaseComponent->frame) {
        buf_size = mppApi->HdiMppFrameGetBufferSize(pBaseComponent->frame);
    }

    memcpy_s(params->val, params->size, &buf_size, sizeof(RK_U32));
    return HDF_SUCCESS;
}

CodecPixelFormat ConvertRKFormat2HdiFormat(MppFrameFormat fmtRK)
{
    if (fmtRK == MPP_FMT_YUV420SP) {
        return YVU_SEMIPLANAR_420;
    } else {
        return PIX_FORMAT_INVALID;
    }
}

MppFrameFormat ConvertHdiFormat2RKFormat(CodecPixelFormat fmtHDI)
{
    if (fmtHDI == YVU_SEMIPLANAR_420) {
        return MPP_FMT_YUV420SP;
    } else {
        return MPP_FMT_BUTT;
    }
}

MppEncRcMode ConvertHdiRcMode2RKRcMode(VenCodeRcMode hdiRcMode)
{
    MppEncRcMode rkRcMode = MPP_ENC_RC_MODE_VBR;
    switch (hdiRcMode) {
        case VENCOD_RC_CBR:
            rkRcMode = MPP_ENC_RC_MODE_CBR;
            break;
        case VENCOD_RC_VBR:
            rkRcMode = MPP_ENC_RC_MODE_VBR;
            break;
        case VENCOD_RC_AVBR:
            rkRcMode = MPP_ENC_RC_MODE_AVBR;
            break;
        case VENCOD_RC_FIXQP:
            rkRcMode = MPP_ENC_RC_MODE_FIXQP;
            break;
        default:
            break;
    }
    return rkRcMode;
}

MppEncRcGopMode ConvertHdiGopMode2RKGopMode(VenCodeGopMode hdiGopMode)
{
    MppEncRcGopMode rkGopMode = MPP_ENC_RC_NORMAL_P;
    switch (hdiGopMode) {
        case VENCOD_GOPMODE_NORMALP:
            rkGopMode = MPP_ENC_RC_NORMAL_P;
            break;
        case VENCOD_GOPMODE_SMARTP:
            rkGopMode = MPP_ENC_RC_SMART_P;
            break;
        default:
            break;
    }
    return rkGopMode;
}

MppCodingType ConvertHdiMimeCodecType2RKCodecType(AvCodecMime mime)
{
    MppCodingType type = MPP_VIDEO_CodingMax;
    switch (mime) {
        case MEDIA_MIMETYPE_IMAGE_JPEG:
            type = MPP_VIDEO_CodingMJPEG;
            break;
        case MEDIA_MIMETYPE_VIDEO_AVC:
            type = MPP_VIDEO_CodingAVC;
            break;
        case MEDIA_MIMETYPE_VIDEO_HEVC:
            type = MPP_VIDEO_CodingHEVC;
            break;
        default:
            break;
    }
    return type;
}

int32_t GetDefaultHorStrideExt(int32_t width, MppFrameFormat fmt)
{
    int32_t stride = 0;

    switch (fmt & MPP_FRAME_FMT_MASK) {
        case MPP_FMT_RGB565:
        case MPP_FMT_BGR565:
        case MPP_FMT_RGB555:
        case MPP_FMT_BGR555:
        case MPP_FMT_RGB444:
        case MPP_FMT_BGR444:
        case MPP_FMT_YUV422_YUYV:
        case MPP_FMT_YUV422_YVYU:
        case MPP_FMT_YUV422_UYVY:
        case MPP_FMT_YUV422_VYUY: {
            /* NOTE: for vepu limitation */
            stride = MPP_ALIGN(width, MPP_ALIGN_STRIDE_WITH_EIGHT) * MPP_ALIGN_MULTIPLE_WITH_TWO;
        }
            break;
        case MPP_FMT_RGB888:
        case MPP_FMT_BGR888: {
            /* NOTE: for vepu limitation */
            stride = MPP_ALIGN(width, MPP_ALIGN_STRIDE_WITH_EIGHT) * MPP_ALIGN_MULTIPLE_WITH_THREE;
        }
            break;
        case MPP_FMT_RGB101010:
        case MPP_FMT_BGR101010:
        case MPP_FMT_ARGB8888:
        case MPP_FMT_ABGR8888:
        case MPP_FMT_BGRA8888:
        case MPP_FMT_RGBA8888: {
            /* NOTE: for vepu limitation */
            stride = MPP_ALIGN(width, MPP_ALIGN_STRIDE_WITH_EIGHT) * MPP_ALIGN_MULTIPLE_WITH_FOUR;
        }
            break;
        default: {
            HDF_LOGE("%{public}s: do not support type %{public}d", __func__, fmt);
        }
            break;
    }

    return stride;
}

int32_t GetDefaultHorStride(int32_t width, CodecPixelFormat fmtHDI)
{
    int32_t stride = 0;
    MppFrameFormat fmt = ConvertHdiFormat2RKFormat(fmtHDI);

    switch (fmt & MPP_FRAME_FMT_MASK) {
        case MPP_FMT_YUV420SP:
        case MPP_FMT_YUV420SP_VU: {
            stride = MPP_ALIGN(width, MPP_ALIGN_STRIDE_WITH_EIGHT);
        }
            break;
        case MPP_FMT_YUV420P: {
            /* NOTE: 420P need to align to 16 so chroma can align to 8 */
            stride = MPP_ALIGN(width, MPP_ALIGN_STRIDE_WITH_SIXTEEN);
        }
            break;
        case MPP_FMT_YUV422P:
        case MPP_FMT_YUV422SP:
        case MPP_FMT_YUV422SP_VU: {
            /* NOTE: 422 need to align to 8 so chroma can align to 16 */
            stride = MPP_ALIGN(width, MPP_ALIGN_STRIDE_WITH_EIGHT);
        }
            break;
        default:
            break;
    }
    if (stride == 0) {
        stride = GetDefaultHorStrideExt(width, fmt);
    }

    return stride;
}

int32_t GetDecOutputPixelFormat(RKHdiBaseComponent *pBaseComponent, Param *params)
{
    if (params->val == NULL) {
        return HDF_FAILURE;
    }

    RKMppApi *mppApi = pBaseComponent->mppApi;
    MppFrameFormat fmtRK  = MPP_FMT_YUV420SP;
    CodecPixelFormat format = YVU_SEMIPLANAR_420;
    if (pBaseComponent->frame) {
        fmtRK = mppApi->HdiMppFrameGetFormat(pBaseComponent->frame);
        format = ConvertRKFormat2HdiFormat(fmtRK);
    }
    // Rk mpp Only NV12 is supported
    memcpy_s(params->val, params->size, &format, sizeof(CodecPixelFormat));
    return HDF_SUCCESS;
}

uint32_t GetFrameBufferSize(RKHdiBaseComponent *pBaseComponent)
{
    uint32_t frameSize = 0;
    switch (pBaseComponent->fmt) {
        case MPP_FMT_YUV420SP:
        case MPP_FMT_YUV420P:
            frameSize = MPP_ALIGN(pBaseComponent->horStride, MPP_ALIGN_STRIDE_WITH_SIXTY_FOUR)
                * MPP_ALIGN(pBaseComponent->verStride, MPP_ALIGN_STRIDE_WITH_SIXTY_FOUR)
                * MPP_ALIGN_MULTIPLE_WITH_THREE / MPP_ALIGN_DIVISOR_WITH_TWO;
            break;

        case MPP_FMT_YUV422_YUYV:
        case MPP_FMT_YUV422_YVYU:
        case MPP_FMT_YUV422_UYVY:
        case MPP_FMT_YUV422_VYUY:
        case MPP_FMT_YUV422P:
        case MPP_FMT_YUV422SP:
        case MPP_FMT_RGB444:
        case MPP_FMT_BGR444:
        case MPP_FMT_RGB555:
        case MPP_FMT_BGR555:
        case MPP_FMT_RGB565:
        case MPP_FMT_BGR565:
            frameSize = MPP_ALIGN(pBaseComponent->horStride, MPP_ALIGN_STRIDE_WITH_SIXTY_FOUR)
                * MPP_ALIGN(pBaseComponent->verStride, MPP_ALIGN_STRIDE_WITH_SIXTY_FOUR)
                * MPP_ALIGN_MULTIPLE_WITH_TWO;
            break;

        default:
            frameSize = MPP_ALIGN(pBaseComponent->horStride, MPP_ALIGN_STRIDE_WITH_SIXTY_FOUR)
                * MPP_ALIGN(pBaseComponent->verStride, MPP_ALIGN_STRIDE_WITH_SIXTY_FOUR)
                * MPP_ALIGN_MULTIPLE_WITH_FOUR;
            break;
    }
    return frameSize;
}

uint32_t GetFrameHeaderSize(RKHdiBaseComponent *pBaseComponent)
{
    uint32_t headerSize = 0;
    if (MPP_FRAME_FMT_IS_FBC(pBaseComponent->fmt)) {
        headerSize = MPP_ALIGN(MPP_ALIGN(pBaseComponent->width, MPP_ALIGN_STRIDE_WITH_SIXTEEN)
            * MPP_ALIGN(pBaseComponent->height, MPP_ALIGN_STRIDE_WITH_SIXTEEN)
            / MPP_ALIGN_DIVISOR_WITH_SIXTEEN, SZ_4K);
    } else {
        headerSize = 0;
    }
    return headerSize;
}

int32_t GetMppBuffers(RKHdiBaseComponent *pBaseComponent)
{
    MPP_RET ret = MPP_OK;
    RKMppApi *mppApi = pBaseComponent->mppApi;
    ret = mppApi->HdiMppBufferGroupGet(&pBaseComponent->frmGrp,
        MPP_BUFFER_TYPE_DRM, MPP_BUFFER_INTERNAL, NULL, __func__);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: failed to get mpp buffer group ret %{public}d", __func__, ret);
        return HDF_FAILURE;
    }

    ret = mppApi->HdiMppBufferGetWithTag(pBaseComponent->frmGrp, &pBaseComponent->frmBuf,
        pBaseComponent->frameSize + pBaseComponent->headerSize, MODULE_TAG, __func__);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: failed to get buffer for input frame ret %{public}d", __func__, ret);
        return HDF_FAILURE;
    }

    ret = mppApi->HdiMppBufferGetWithTag(pBaseComponent->frmGrp, &pBaseComponent->pktBuf,
        pBaseComponent->frameSize, MODULE_TAG, __func__);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: failed to get buffer for output packet ret %{public}d", __func__, ret);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t SetParamWidth(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    MPP_RET ret = MPP_OK;
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RK_S32 *width = (RK_S32 *)param->val;
    ret = mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "prep:width", *width);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: config mpp set width failed!", __func__);
        return HDF_FAILURE;
    }
    pBaseComponent->width = *width;
    return HDF_SUCCESS;
}

int32_t SetParamHeight(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    MPP_RET ret = MPP_OK;
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RK_S32 *height = (RK_S32 *)param->val;
    ret = mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "prep:height", *height);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: config mpp set height failed!", __func__);
        return HDF_FAILURE;
    }
    pBaseComponent->height = *height;
    return HDF_SUCCESS;
}

int32_t SetParamPixleFmt(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    MPP_RET ret = MPP_OK;
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RK_S32 *pixFmt = (RK_S32 *)param->val;
    MppFrameFormat rkPixFmt = ConvertHdiFormat2RKFormat(*pixFmt);
    ret = mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "prep:format", rkPixFmt);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: config mpp set pixFmt failed!", __func__);
        return HDF_FAILURE;
    }
    pBaseComponent->fmt = rkPixFmt;
    return HDF_SUCCESS;
}

int32_t SetParamStride(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RKHdiStrideSetup *strideSet = (RKHdiStrideSetup *)param->val;
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "prep:hor_stride", strideSet->horStride);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "prep:ver_stride", strideSet->verStride);
    pBaseComponent->horStride = strideSet->horStride;
    pBaseComponent->verStride = strideSet->verStride;
    return HDF_SUCCESS;
}

int32_t SetParamFps(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RKHdiFpsSetup *rkFpsSet = (RKHdiFpsSetup *)param->val;
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:fps_in_flex", rkFpsSet->fpsInFlex);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:fps_in_num", rkFpsSet->fpsInNum);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:fps_in_denorm", rkFpsSet->fpsInDen);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:fps_out_flex", rkFpsSet->fpsOutFlex);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:fps_out_num", rkFpsSet->fpsOutNum);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:fps_out_denorm", rkFpsSet->fpsOutDen);
    return HDF_SUCCESS;
}

int32_t SetParamDrop(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RKHdiDropSetup *rkDropSet = (RKHdiDropSetup *)param->val;
    mppApi->HdiMppEncCfgSetU32(pBaseComponent->cfg, "rc:drop_mode", rkDropSet->dropMode);
    mppApi->HdiMppEncCfgSetU32(pBaseComponent->cfg, "rc:drop_thd", rkDropSet->dropThd);
    mppApi->HdiMppEncCfgSetU32(pBaseComponent->cfg, "rc:drop_gap", rkDropSet->dropGap);
    return HDF_SUCCESS;
}

int32_t SetParamRateControl(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RKHdiRcSetup *rateControlSet = (RKHdiRcSetup *)param->val;
    MppEncRcMode rkRcMode = ConvertHdiRcMode2RKRcMode(rateControlSet->rcMode);

    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:mode", rkRcMode);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:bps_target", rateControlSet->bpsTarget);

    if (rateControlSet->rcMode != MPP_ENC_RC_MODE_FIXQP) {
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:bps_max", rateControlSet->bpsMax);
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:bps_min", rateControlSet->bpsMin);
    }

    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:qp_init", rateControlSet->qpInit);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:qp_max", rateControlSet->qpMax);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:qp_min", rateControlSet->qpMin);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:qp_max_i", rateControlSet->qpMaxI);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:qp_min_i", rateControlSet->qpMinI);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:qp_ip", rateControlSet->qpIp);
    return HDF_SUCCESS;
}

int32_t SetParamGop(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    MPP_RET ret = MPP_OK;
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RKHdiGopSetup *gopSet = (RKHdiGopSetup *)param->val;

    ret = mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "rc:gop", gopSet->gop);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: config mpp set gop failed!", __func__);
        return HDF_FAILURE;
    }

    RK_U32 gopMode = 0;
    RK_U32 defaultGopMode = ConvertHdiGopMode2RKGopMode(gopSet->gopMode);
    mppApi->HdiMppEnvGetU32("gop_mode", &gopMode, defaultGopMode);
    HDF_LOGI("%{public}s: mpp enc gop_mode: %{public}d", __func__, gopMode);
    if (gopMode) {
        MppEncRefCfg ref = NULL;
        mppApi->HdiMppEncRefCfgInit(&ref);
        if (gopMode < GOP_MODE_THRESHOLD_VALUE) {
            mppApi->HdiMppEncGenRefCfg(ref, gopMode);
        } else {
            mppApi->HdiMppEncGenSmartGopRefCfg(ref, gopSet->gopLen, gopSet->viLen);
        }
        ret = pBaseComponent->mpi->control(pBaseComponent->ctx, MPP_ENC_SET_REF_CFG, ref);
        if (ret != MPP_OK) {
            HDF_LOGE("%{public}s: mpi control enc set ref cfg failed ret %{public}d", __func__, ret);
            return HDF_FAILURE;
        }
        mppApi->HdiMppEncRefCfgDeinit(&ref);
    }
    return HDF_SUCCESS;
}

int32_t SetParamMimeCodecType(RKHdiBaseComponent *pBaseComponent, Param *param)
{
    RKMppApi *mppApi = pBaseComponent->mppApi;
    RKHdiCodecTypeSetup *codecTypeSet = (RKHdiCodecTypeSetup *)param->val;
    codecTypeSet->mimeCodecType = ConvertHdiMimeCodecType2RKCodecType(codecTypeSet->mimeCodecType);

    pBaseComponent->codingType = codecTypeSet->mimeCodecType;
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "codec:type", codecTypeSet->mimeCodecType);
    if (codecTypeSet->mimeCodecType == MPP_VIDEO_CodingAVC) {
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:profile", codecTypeSet->avcSetup.profile);
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:level", codecTypeSet->avcSetup.level);
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:cabac_en", codecTypeSet->avcSetup.cabacEn);
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:cabac_idc", codecTypeSet->avcSetup.cabacIdc);
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:trans8x8", codecTypeSet->avcSetup.trans8x8);
    }
    return HDF_SUCCESS;
}

int32_t ValidateEncSetup(RKHdiBaseComponent *pBaseComponent, Param *params)
{
    MPP_RET ret = MPP_OK;
    RKMppApi *mppApi = pBaseComponent->mppApi;

    RK_U32 split_mode = 0;
    RK_U32 split_arg = 0;
    mppApi->HdiMppEnvGetU32("split_mode", &split_mode, MPP_ENC_SPLIT_NONE);
    mppApi->HdiMppEnvGetU32("split_arg", &split_arg, 0);

    if (split_mode) {
        HDF_LOGI("%{public}s: %{public}p split_mode %{public}d split_arg %{public}d",
            __func__, pBaseComponent->ctx, split_mode, split_arg);
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "split:mode", split_mode);
        mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "split:arg", split_arg);
    }

    ret = pBaseComponent->mpi->control(pBaseComponent->ctx, MPP_ENC_SET_CFG, pBaseComponent->cfg);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: mpi control enc set cfg failed ret %{public}d", __func__, ret);
        return HDF_FAILURE;
    }

    /* optional */
    MppEncSeiMode sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
    ret = pBaseComponent->mpi->control(pBaseComponent->ctx, MPP_ENC_SET_SEI_CFG, &sei_mode);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: mpi control enc set sei cfg failed ret %{public}d", __func__, ret);
        return HDF_FAILURE;
    }

    if ((pBaseComponent->codingType == MPP_VIDEO_CodingAVC) || (pBaseComponent->codingType == MPP_VIDEO_CodingHEVC)) {
        MppEncHeaderMode header_mode = MPP_ENC_HEADER_MODE_EACH_IDR;
        ret = pBaseComponent->mpi->control(pBaseComponent->ctx, MPP_ENC_SET_HEADER_MODE, &header_mode);
        if (ret != MPP_OK) {
            HDF_LOGE("%{public}s: mpi control enc set header mode failed ret %{public}d", __func__, ret);
            return HDF_FAILURE;
        }
    }

    pBaseComponent->frameSize = GetFrameBufferSize(pBaseComponent);
    pBaseComponent->headerSize = GetFrameHeaderSize(pBaseComponent);
    ret = GetMppBuffers(pBaseComponent);
    if (ret != MPP_OK) {
        HDF_LOGE("%{public}s: GetMppBuffers failed ret %{public}d", __func__, ret);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t SetEncSetupAVC(RKHdiBaseComponent *pBaseComponent, Param *params)
{
    RKMppApi *mppApi = pBaseComponent->mppApi;

    RKHdiEncSetupAVC *encSetupAVC = (RKHdiEncSetupAVC *)params->val;
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:profile", encSetupAVC->profile);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:level", encSetupAVC->level);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:cabac_en", encSetupAVC->cabacEn);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:cabac_idc", encSetupAVC->cabacIdc);
    mppApi->HdiMppEncCfgSetS32(pBaseComponent->cfg, "h264:trans8x8", encSetupAVC->trans8x8);

    HDF_LOGI("%{public}s: set AVC encode profile:%{public}d, level:%{public}d",
        __func__, encSetupAVC->profile, encSetupAVC->level);
    return HDF_SUCCESS;
}
