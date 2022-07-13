/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd.
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

#ifndef __RK_TYPE_H__
#define __RK_TYPE_H__

#include <stddef.h>

#if defined(_WIN32) && !defined(__MINGW32CE__)

#define RK_U8 unsigned char
#define RK_U16 unsigned short
#define RK_U32 unsigned int
#define RK_ULONG unsigned long
#define RK_U64 unsigned __int64

#define RK_S8 signed char
#define RK_S16 signed short
#define RK_S32 signed int
#define RK_LONG signed long
#define RK_S64 signed __int64

#else

#define RK_U8 unsigned char
#define RK_U16 unsigned short
#define RK_U32 unsigned int
#define RK_ULONG unsigned long
#define RK_U64 unsigned long long int


#define RK_S8 signed char
#define RK_S16 signed short
#define RK_S32 signed int
#define RK_LONG signed long
#define RK_S64 signed long long int

#endif

#ifndef MODULE_TAG
#define MODULE_TAG              NULL
#endif

/**
 * @ingroup rk_mpi
 * @brief The type of mpp context
 * @details This type is used when calling mpp_init(), which including decoder,
 *          encoder and Image Signal Process(ISP). So far decoder and encoder
 *          are supported perfectly, and ISP will be supported in the future.
 */
typedef enum {
    MPP_CTX_DEC,  /**< decoder */
    MPP_CTX_ENC,  /**< encoder */
    MPP_CTX_ISP,  /**< isp */
    MPP_CTX_BUTT, /**< undefined */
} MppCtxType;

/**
 * @ingroup rk_mpi
 * @brief Enumeration used to define the possible video compression codings.
 *        sync with the omx_video.h
 *
 * @note  This essentially refers to file extensions. If the coding is
 *        being used to specify the ENCODE type, then additional work
 *        must be done to configure the exact flavor of the compression
 *        to be used.  For decode cases where the user application can
 *        not differentiate between MPEG-4 and H.264 bit streams, it is
 *        up to the codec to handle this.
 */
typedef enum {
    MPP_VIDEO_CodingUnused,             /**< Value when coding is N/A */
    MPP_VIDEO_CodingAutoDetect,         /**< Autodetection of coding type */
    MPP_VIDEO_CodingMPEG2,              /**< AKA: H.262 */
    MPP_VIDEO_CodingH263,               /**< H.263 */
    MPP_VIDEO_CodingMPEG4,              /**< MPEG-4 */
    MPP_VIDEO_CodingWMV,                /**< Windows Media Video (WMV1,WMV2,WMV3)*/
    MPP_VIDEO_CodingRV,                 /**< all versions of Real Video */
    MPP_VIDEO_CodingAVC,                /**< H.264/AVC */
    MPP_VIDEO_CodingMJPEG,              /**< Motion JPEG */
    MPP_VIDEO_CodingVP8,                /**< VP8 */
    MPP_VIDEO_CodingVP9,                /**< VP9 */
    MPP_VIDEO_CodingVC1 = 0x01000000,   /**< Windows Media Video (WMV1,WMV2,WMV3)*/
    MPP_VIDEO_CodingFLV1,               /**< Sorenson H.263 */
    MPP_VIDEO_CodingDIVX3,              /**< DIVX3 */
    MPP_VIDEO_CodingVP6,
    MPP_VIDEO_CodingHEVC,               /**< H.265/HEVC */
    MPP_VIDEO_CodingAVSPLUS,            /**< AVS+ */
    MPP_VIDEO_CodingAVS,                /**< AVS profile=0x20 */
    MPP_VIDEO_CodingKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
    MPP_VIDEO_CodingVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MPP_VIDEO_CodingMax = 0x7FFFFFFF
} MppCodingType;

/*
 * All external interface object list here.
 * The interface object is defined as void * for expandability
 * The cross include between these objects will introduce extra
 * compiling difficulty. So we move them together in this header.
 *
 * Object interface header list:
 *
 * MppCtx           - rk_mpi.h
 * MppParam         - rk_mpi.h
 *
 * MppFrame         - mpp_frame.h
 * MppPacket        - mpp_packet.h
 *
 * MppBuffer        - mpp_buffer.h
 * MppBufferGroup   - mpp_buffer.h
 *
 * MppTask          - mpp_task.h
 * MppMeta          - mpp_meta.h
 */

typedef void* MppCtx;
typedef void* MppParam;

typedef void* MppFrame;
typedef void* MppPacket;

typedef void* MppBuffer;
typedef void* MppBufferGroup;

typedef void* MppTask;
typedef void* MppMeta;

#endif /* __RK_TYPE_H__ */