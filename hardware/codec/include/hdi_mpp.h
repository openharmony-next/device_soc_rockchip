/*
 * Copyright (c) 2022 Shenzhen Kaihong DID Co., Ltd.
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

#ifndef HDI_MPP_H
#define HDI_MPP_H

#include "codec_type.h"
#include "rk_mpi.h"
#include "hdi_mpp_mpi.h"

typedef enum {
    KEY_START = 0xF000,
    KEY_DEFAULT_CFG_RK,             /**< Default config. Used for RK codec. */
    KEY_SPLIT_PARSE_RK,             /**< Split parse. Used for RK codec. */
    KEY_DEC_FRAME_NUM_RK,           /**< Decode frame number. Used for RK codec. */
    KEY_EXT_SETUP_DROP_MODE_RK,     /**< Drop mode setup. Used for RK codec. */
    KEY_EXT_ENC_VALIDATE_SETUP_RK,  /**< Validate config setup. Used for RK codec. */
    KEY_ENC_SETUP_AVC_RK,           /**< AVC config setup. Used for RK codec. */
    KEY_ENC_FRAME_NUM_RK,           /**< Frame num setup. Used for RK codec. */
    KEY_END = 0xFFFF,
} ParamExtKey;

typedef struct {
    int32_t profile;
    int32_t level;
    int32_t cabacEn;
    int32_t cabacIdc;
    int32_t trans8x8;
} RKHdiEncSetupAVC;

typedef struct {
    int32_t horStride;
    int32_t verStride;
} RKHdiStrideSetup;

typedef struct {
    int32_t fpsInFlex;
    int32_t fpsInNum;
    int32_t fpsInDen;
    int32_t fpsOutFlex;
    int32_t fpsOutNum;
    int32_t fpsOutDen;
} RKHdiFpsSetup;

typedef struct {
    uint32_t dropMode;
    uint32_t dropThd;
    uint32_t dropGap;
} RKHdiDropSetup;

typedef struct {
    int32_t rcMode;
    int32_t bpsTarget;
    int32_t bpsMax;
    int32_t bpsMin;
    int32_t qpInit;
    int32_t qpMax;
    int32_t qpMin;
    int32_t qpMaxI;
    int32_t qpMinI;
    int32_t qpIp;
} RKHdiRcSetup;

typedef struct {
    VenCodeGopMode gopMode;
    uint32_t gopLen;
    int32_t gop;
    int32_t viLen;
} RKHdiGopSetup;

typedef struct {
    int32_t mimeCodecType;
    RKHdiEncSetupAVC avcSetup;
} RKHdiCodecTypeSetup;

typedef struct {
    int32_t splitMode;
    int32_t splitArg;
} RKHdiSplitSetup;

typedef struct {
    int32_t width;
    int32_t height;
    RKHdiStrideSetup stride;
    CodecPixelFormat fmt;
    RKHdiFpsSetup fps;
    RKHdiDropSetup drop;
    RKHdiRcSetup rc;
    RKHdiGopSetup gop;
    RKHdiCodecTypeSetup codecType;
} RKHdiEncodeSetup;

typedef struct {
    MppCtx ctx;
    RKMppApi *mppApi;
    char *componentName;
    MppCtxType ctxType;
    MppCodingType codingType;
    CodecCallback *pCallbacks;
    MppDecCfg cfg;
    MppApi *mpi;

    MppBufferGroup frmGrp;
    MppPacket packet;
    size_t packetSize;
    MppFrame frame;
    RK_S32 frameCount;
    RK_S32 frameErr;
    RK_S32 frameNum;
    size_t maxUsage;

    MppBuffer frmBuf;
    size_t headerSize;
    size_t frameSize;
    MppBuffer pktBuf;

    RK_U32 width;
    RK_U32 height;
    RK_U32 horStride;
    RK_U32 verStride;
    MppFrameFormat fmt;
} RKHdiBaseComponent;

#endif // HDI_MPP_H