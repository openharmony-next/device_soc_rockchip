/*
 * Copyright (c) 2023 Shenzhen Kaihong DID Co., Ltd.
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
#ifndef CODEC_LOG_WRAPPER_H
#define CODEC_LOG_WRAPPER_H

#include <hdf_log.h>
#define CODEC_LOGE(fmt, ...) HDF_LOGE("%{public}s: " fmt, __func__, ##__VA_ARGS__)

#define CODEC_LOGW(fmt, ...) HDF_LOGW("%{public}s: " fmt, __func__, ##__VA_ARGS__)

#define CODEC_LOGI(fmt, ...) HDF_LOGI("%{public}s: " fmt, __func__, ##__VA_ARGS__)

#define CODEC_LOGD(fmt, ...) HDF_LOGD("%{public}s: " fmt, __func__, ##__VA_ARGS__)
#endif
