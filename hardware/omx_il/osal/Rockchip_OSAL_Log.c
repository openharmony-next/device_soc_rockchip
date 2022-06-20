/*
 *
 * Copyright 2013 Rockchip Electronics Co., LTD.
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

/*
 * @file        Rockchip_OSAL_Log.h
 * @brief
 * @author      csy(csy@rock-chips.com)
 * @version     1.0.0
 * @history
 *   2013.11.26 : Create
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rockchip_OSAL_Env.h"
#include "Rockchip_OSAL_Log.h"

#ifndef OHOS
#include <utils/Log.h>
#include <android/log.h>

void _Rockchip_OSAL_Log(ROCKCHIP_LOG_LEVEL logLevel, OMX_U32 flag, const char *tag, const char *msg, ...)
{
    OMX_U32 value = 0;

    va_list argptr;

    va_start(argptr, msg);

    switch (logLevel) {
        case ROCKCHIP_LOG_TRACE: {
            Rockchip_OSAL_GetEnvU32("vendor.dump.omx.log", &value, 0);
            if (value) {
                __android_log_vprint(ANDROID_LOG_DEBUG, tag, msg, argptr);
            }
        }
            break;
        case ROCKCHIP_LOG_DEBUG: {
            Rockchip_OSAL_GetEnvU32("vendor.omx.log.debug", &value, 0);
            if (value & flag) {
                __android_log_vprint(ANDROID_LOG_DEBUG, tag, msg, argptr);
            }
        }
            break;
        case ROCKCHIP_LOG_INFO:
            __android_log_vprint(ANDROID_LOG_INFO, tag, msg, argptr);
            break;
        case ROCKCHIP_LOG_WARNING:
            __android_log_vprint(ANDROID_LOG_WARN, tag, msg, argptr);
            break;
        case ROCKCHIP_LOG_ERROR:
            __android_log_vprint(ANDROID_LOG_ERROR, tag, msg, argptr);
            break;
        default:
            __android_log_vprint(ANDROID_LOG_VERBOSE, tag, msg, argptr);
    }

    va_end(argptr);
}

#else
#include <hdf_log.h>
#define MPP_LOG_MAX_LEN 256
#define DESTMAX 1

void _Rockchip_OSAL_Log(ROCKCHIP_LOG_LEVEL logLevel, OMX_U32 flag, const char *tag, const char *msg, ...)
{
    va_list argptr;
    va_start(argptr, msg);

    char str[MPP_LOG_MAX_LEN] = {0};
    if (memset_s(str, sizeof(str), 0, MPP_LOG_MAX_LEN) != EOK) {
        return ;
    }
    char outputStr[MPP_LOG_MAX_LEN + 100] = {0};
    if (memset_s(outputStr, sizeof(outputStr), 0, MPP_LOG_MAX_LEN + 100) != EOK) { // 100:byte alignment
        return ;
    }

    if (sprintf_s(str, sizeof(str), "%s %s", tag, msg) != EOK) {
        return ;
    }

    switch (logLevel) {
        case ROCKCHIP_LOG_TRACE:
            HDF_LOGI(str, argptr);
            break;
        case ROCKCHIP_LOG_DEBUG:
            HDF_LOGD(str, argptr);
            break;
        case ROCKCHIP_LOG_INFO:
            HDF_LOGI(str, argptr);
            break;
        case ROCKCHIP_LOG_WARNING:
            HDF_LOGW(str, argptr);
            break;
        case ROCKCHIP_LOG_ERROR:
            HDF_LOGE(str, argptr);
            break;
        default:
            break;
    }
    va_end(argptr);
}
#endif