/*
 * Copyright (C) 2021 HiHope Open Source Organization .
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

#ifndef VideoExt_h
#define VideoExt_h

#include <OMX_IVCommon.h>
#include <codec_omx_ext.h>
typedef enum OMX_RANGE {
    RangeUnspecified,
    RangeFull,
    RangeLimited,
    RangeOther = 0xff,
} OMX_RANGE;

typedef enum OMX_PRIMARIES {
    PrimariesUnspecified,
    PrimariesBT709_5,       // Rec.ITU-R BT.709-5 or equivalent
    PrimariesBT470_6M,      // Rec.ITU-R BT.470-6 System M or equivalent
    PrimariesBT601_6_625,   // Rec.ITU-R BT.601-6 625 or equivalent
    PrimariesBT601_6_525,   // Rec.ITU-R BT.601-6 525 or equivalent
    PrimariesGenericFilm,   // Generic Film
    PrimariesBT2020,        // Rec.ITU-R BT.2020 or equivalent
    PrimariesOther = 0xff,
} OMX_PRIMARIES;

typedef enum OMX_TRANSFER {
    TransferUnspecified,
    TransferLinear,         // Linear transfer characteristics
    TransferSRGB,           // sRGB or equivalent
    TransferSMPTE170M,      // SMPTE 170M or equivalent (e.g. BT.601/709/2020)
    TransferGamma22,        // Assumed display gamma 2.2
    TransferGamma28,        // Assumed display gamma 2.8
    TransferST2084,         // SMPTE ST 2084 for 10/12/14/16 bit systems
    TransferHLG,            // ARIB STD-B67 hybrid-log-gamma
    // transfers unlikely to be required by Android
    TransferSMPTE240M = 0x40, // SMPTE 240M
    TransferXvYCC,          // IEC 61966-2-4
    TransferBT1361,         // Rec.ITU-R BT.1361 extended gamut
    TransferST428,          // SMPTE ST 428-1
    TransferOther = 0xff,
} OMX_TRANSFER;

typedef enum OMX_MATRIXCOEFFS {
    MatrixUnspecified,
    MatrixBT709_5,          // Rec.ITU-R BT.709-5 or equivalent
    MatrixBT470_6M,         // KR=0.30, KB=0.11 or equivalent
    MatrixBT601_6,          // Rec.ITU-R BT.601-6 625 or equivalent
    MatrixSMPTE240M,        // SMPTE 240M or equivalent
    MatrixBT2020,           // Rec.ITU-R BT.2020 non-constant luminance
    MatrixBT2020Constant,   // Rec.ITU-R BT.2020 constant luminance
    MatrixOther = 0xff,
} OMX_MATRIXCOEFFS;

typedef struct OMX_COLORASPECTS {
    OMX_RANGE mRange;                // IN/OUT
    OMX_PRIMARIES mPrimaries;        // IN/OUT
    OMX_TRANSFER mTransfer;          // IN/OUT
    OMX_MATRIXCOEFFS mMatrixCoeffs;  // IN/OUT
} OMX_COLORASPECTS;

typedef struct ISO_COLORASPECTS {
    OMX_U32 mRange;                 // IN/OUT
    OMX_U32 mPrimaries;             // IN/OUT
    OMX_U32 mTransfer;              // IN/OUT
    OMX_U32 mMatrixCoeffs;          // IN/OUT
} ISO_COLORASPECTS;

typedef struct OMX_CONFIG_DESCRIBECOLORASPECTSPARAMS {
    OMX_U32  nSize;                // IN
    OMX_VERSIONTYPE nVersion;      // IN
    OMX_U32  nPortIndex;           // IN
    OMX_BOOL bRequestingDataSpace; // IN
    OMX_BOOL bDataSpaceChanged;    // IN
    OMX_U32  nPixelFormat;         // IN
    OMX_U32  nDataSpace;           // OUT
    OMX_COLORASPECTS sAspects;  // IN/OUT
} OMX_CONFIG_DESCRIBECOLORASPECTSPARAMS;

typedef enum OMX_VIDEO_CODINGTYPEEXT {
    OMX_VIDEO_CodingVP8EXT = 9,        /**< Google VP8, formerly known as On2 VP8 */
    OMX_VIDEO_CodingVP9,        /**< Google VP9 */
    OMX_VIDEO_AVCLevel52  = 0x10000,
}OMX_VIDEO_CODINGTYPEEXT;

typedef enum OMX_VIDEO_VP9PROFILETYPE {
    OMX_VIDEO_VP9Profile0       = 0x1,
    OMX_VIDEO_VP9Profile1       = 0x2,
    OMX_VIDEO_VP9Profile2       = 0x4,
    OMX_VIDEO_VP9Profile3       = 0x8,
    // HDR profiles also support passing HDR metadata
    OMX_VIDEO_VP9Profile2HDR    = 0x1000,
    OMX_VIDEO_VP9Profile3HDR    = 0x2000,
    OMX_VIDEO_VP9ProfileUnknown = 0x6EFFFFFF,
    OMX_VIDEO_VP9ProfileMax     = 0x7FFFFFFF
} OMX_VIDEO_VP9PROFILETYPE;

/** VP9 levels */
typedef enum OMX_VIDEO_VP9LEVELTYPE {
    OMX_VIDEO_VP9Level1       = 0x1,
    OMX_VIDEO_VP9Level11      = 0x2,
    OMX_VIDEO_VP9Level2       = 0x4,
    OMX_VIDEO_VP9Level21      = 0x8,
    OMX_VIDEO_VP9Level3       = 0x10,
    OMX_VIDEO_VP9Level31      = 0x20,
    OMX_VIDEO_VP9Level4       = 0x40,
    OMX_VIDEO_VP9Level41      = 0x80,
    OMX_VIDEO_VP9Level5       = 0x100,
    OMX_VIDEO_VP9Level51      = 0x200,
    OMX_VIDEO_VP9Level52      = 0x400,
    OMX_VIDEO_VP9Level6       = 0x800,
    OMX_VIDEO_VP9Level61      = 0x1000,
    OMX_VIDEO_VP9Level62      = 0x2000,
    OMX_VIDEO_VP9LevelUnknown = 0x6EFFFFFF,
    OMX_VIDEO_VP9LevelMax     = 0x7FFFFFFF
} OMX_VIDEO_VP9LEVELTYPE;

#define OMX_VIDEO_ANDROID_MAXVP8TEMPORALLAYERS 3

/** VP8 temporal layer patterns */
typedef enum OMX_VIDEO_ANDROID_VPXTEMPORALLAYERPATTERNTYPE {
    OMX_VIDEO_VPXTemporalLayerPatternNone = 0,
    OMX_VIDEO_VPXTemporalLayerPatternWebRTC = 1,
    OMX_VIDEO_VPXTemporalLayerPatternMax = 0x7FFFFFFF
} OMX_VIDEO_ANDROID_VPXTEMPORALLAYERPATTERNTYPE;

typedef struct OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nKeyFrameInterval;        // distance between consecutive key_frames (including one
    // of the key_frames). 0 means interval is unspecified and
    // can be freely chosen by the codec. 1 means a stream of
    // only key_frames.

    OMX_VIDEO_ANDROID_VPXTEMPORALLAYERPATTERNTYPE eTemporalPattern;
    OMX_U32 nTemporalLayerCount;
    OMX_U32 nTemporalLayerBitrateRatio[OMX_VIDEO_ANDROID_MAXVP8TEMPORALLAYERS];
    OMX_U32 nMinQuantizer;
    OMX_U32 nMaxQuantizer;
} OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE;

/** Structure to define if dependent slice segments should be used */
typedef struct OMX_VIDEO_SLICESEGMENTSTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bDepedentSegments;
    OMX_BOOL bEnableLoopFilterAcrossSlices;
} OMX_VIDEO_SLICESEGMENTSTYPE;

/** Structure to return timestamps of rendered output frames as well as EOS
 *  for tunneled components.
 */
typedef struct OMX_VIDEO_RENDEREVENTTYPE {
    OMX_S64 nMediaTimeUs;  // timestamp of rendered video frame
    OMX_S64 nSystemTimeNs; // system monotonic time at the time frame was rendered
    // Use INT64_MAX for nMediaTimeUs to signal that the EOS
    // has been reached. In this case, nSystemTimeNs MUST be
    // the system time when the last frame was rendered.
    // This MUST be done in addition to returning (and
    // following) the render information for the last frame.
} OMX_VIDEO_RENDEREVENTTYPE;

/** Dolby Vision Profile enum type */
typedef enum OMX_VIDEO_DOLBYVISIONPROFILETYPE {
    OMX_VIDEO_DolbyVisionProfileUnknown = 0x0,
    OMX_VIDEO_DolbyVisionProfileDvavPer = 0x1,
    OMX_VIDEO_DolbyVisionProfileDvavPen = 0x2,
    OMX_VIDEO_DolbyVisionProfileDvheDer = 0x4,
    OMX_VIDEO_DolbyVisionProfileDvheDen = 0x8,
    OMX_VIDEO_DolbyVisionProfileDvheDtr = 0x10,
    OMX_VIDEO_DolbyVisionProfileDvheStn = 0x20,
    OMX_VIDEO_DolbyVisionProfileDvheDth = 0x40,
    OMX_VIDEO_DolbyVisionProfileDvheDtb = 0x80,
    OMX_VIDEO_DolbyVisionProfileMax     = 0x7FFFFFFF
} OMX_VIDEO_DOLBYVISIONPROFILETYPE;

/** Dolby Vision Level enum type */
typedef enum OMX_VIDEO_DOLBYVISIONLEVELTYPE {
    OMX_VIDEO_DolbyVisionLevelUnknown = 0x0,
    OMX_VIDEO_DolbyVisionLevelHd24    = 0x1,
    OMX_VIDEO_DolbyVisionLevelHd30    = 0x2,
    OMX_VIDEO_DolbyVisionLevelFhd24   = 0x4,
    OMX_VIDEO_DolbyVisionLevelFhd30   = 0x8,
    OMX_VIDEO_DolbyVisionLevelFhd60   = 0x10,
    OMX_VIDEO_DolbyVisionLevelUhd24   = 0x20,
    OMX_VIDEO_DolbyVisionLevelUhd30   = 0x40,
    OMX_VIDEO_DolbyVisionLevelUhd48   = 0x80,
    OMX_VIDEO_DolbyVisionLevelUhd60   = 0x100,
    OMX_VIDEO_DolbyVisionLevelmax     = 0x7FFFFFFF
} OMX_VIDEO_DOLBYVISIONLEVELTYPE;

/**
 * Structure for configuring video compression intra refresh period
 *
 * STRUCT MEMBERS:
 *  nSize               : Size of the structure in bytes
 *  nVersion            : OMX specification version information
 *  nPortIndex          : Port that this structure applies to
 *  nRefreshPeriod      : Intra refreh period in frames. Value 0 means disable intra refresh
 */
typedef struct OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nRefreshPeriod;
} OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE;

/** Maximum number of temporal layers supported by AVC/HEVC */
#define OMX_VIDEO_ANDROID_MAXTEMPORALLAYERS 8

/** temporal layer patterns */
typedef enum OMX_VIDEO_ANDROID_TEMPORALLAYERINGPATTERNTYPE {
    OMX_VIDEO_AndroidTemporalLayeringPatternNone = 0,
    // pattern as defined by WebRTC
    OMX_VIDEO_AndroidTemporalLayeringPatternWebRTC = 1 << 0,
    // pattern where frames in any layer other than the base layer only depend on at most the very
    // last frame from each preceding layer (other than the base layer.)
    OMX_VIDEO_AndroidTemporalLayeringPatternAndroid = 1 << 1,
} OMX_VIDEO_ANDROID_TEMPORALLAYERINGPATTERNTYPE;

/**
 * Android specific param for configuration of temporal layering.
 * Android only supports temporal layering where successive layers each double the
 * previous layer's framerate.
 * NOTE: Reading this parameter at run-time SHALL return actual run-time values.
 *
 *  nSize                      : Size of the structure in bytes
 *  nVersion                   : OMX specification version information
 *  nPortIndex                 : Port that this structure applies to (output port for encoders)
 *  eSupportedPatterns         : A bitmask of supported layering patterns
 *  nLayerCountMax             : Max number of temporal coding layers supported
 *                               by the encoder (must be at least 1, 1 meaning temporal layering
 *                               is NOT supported)
 *  nBLayerCountMax            : Max number of layers that can contain B frames
 *                               (0) to (nLayerCountMax - 1)
 *  ePattern                   : Layering pattern.
 *  nPLayerCountActual         : Number of temporal layers to be coded with non-B frames,
 *                               starting from and including the base-layer.
 *                               (1 to nLayerCountMax - nBLayerCountActual)
 *                               If nPLayerCountActual is 1 and nBLayerCountActual is 0, temporal
 *                               layering is disabled. Otherwise, it is enabled.
 *  nBLayerCountActual         : Number of temporal layers to be coded with B frames,
 *                               starting after non-B layers.
 *                               (0 to nBLayerCountMax)
 *  bBitrateRatiosSpecified    : Flag to indicate if layer-wise bitrate
 *                               distribution is specified.
 *  nBitrateRatios             : Bitrate ratio (100 based) per layer (index 0 is base layer).
 *                               Honored if bBitrateRatiosSpecified is set.
 *                               i.e for 4 layers with desired distribution (25% 25% 25% 25%),
 *                               nBitrateRatio = {25, 50, 75, 100, ... }
 *                               Values in indices not less than 'the actual number of layers
 *                               minus 1' MAY be ignored and assumed to be 100.
 */
typedef struct OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_VIDEO_ANDROID_TEMPORALLAYERINGPATTERNTYPE eSupportedPatterns;
    OMX_U32 nLayerCountMax;
    OMX_U32 nBLayerCountMax;
    OMX_VIDEO_ANDROID_TEMPORALLAYERINGPATTERNTYPE ePattern;
    OMX_U32 nPLayerCountActual;
    OMX_U32 nBLayerCountActual;
    OMX_BOOL bBitrateRatiosSpecified;
    OMX_U32 nBitrateRatios[OMX_VIDEO_ANDROID_MAXTEMPORALLAYERS];
} OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE;

/**
 * Android specific config for changing the temporal-layer count or
 * bitrate-distribution at run-time.
 *
 *  nSize                      : Size of the structure in bytes
 *  nVersion                   : OMX specification version information
 *  nPortIndex                 : Port that this structure applies to (output port for encoders)
 *  ePattern                   : Layering pattern.
 *  nPLayerCountActual         : Number of temporal layers to be coded with non-B frames.
 *                               (same OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE limits apply.)
 *  nBLayerCountActual         : Number of temporal layers to be coded with B frames.
 *                               (same OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE limits apply.)
 *  bBitrateRatiosSpecified    : Flag to indicate if layer-wise bitrate
 *                               distribution is specified.
 *  nBitrateRatios             : Bitrate ratio (100 based, Q16 values) per layer (0 is base layer).
 *                               Honored if bBitrateRatiosSpecified is set.
 *                               (same OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE limits apply.)
 */
typedef struct OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_VIDEO_ANDROID_TEMPORALLAYERINGPATTERNTYPE ePattern;
    OMX_U32 nPLayerCountActual;
    OMX_U32 nBLayerCountActual;
    OMX_BOOL bBitrateRatiosSpecified;
    OMX_U32 nBitrateRatios[OMX_VIDEO_ANDROID_MAXTEMPORALLAYERS];
} OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE;

#endif