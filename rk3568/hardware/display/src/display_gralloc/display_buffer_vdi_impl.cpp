/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "display_buffer_vdi_impl.h"
#include "cinttypes"
#include "display_log.h"
#include "display_gralloc_gbm.h"
#include "hdf_base.h"
#include "v1_0/display_composer_type.h"

namespace OHOS {
namespace HDI {
namespace DISPLAY {
using namespace OHOS::HDI::Display::Composer::V1_0;

DisplayBufferVdiImpl::DisplayBufferVdiImpl()
{
#ifdef GRALLOC_GBM_SUPPORT
    int ret = GbmGrallocInitialize();
    if (ret != HDF_SUCCESS) {
        DISPLAY_LOGE("gbm construct failed");
    }
#endif
}

DisplayBufferVdiImpl::~DisplayBufferVdiImpl()
{
#ifdef GRALLOC_GBM_SUPPORT
    if (GbmGrallocUninitialize() != HDF_SUCCESS) {
        DISPLAY_LOGE("gbm distruct failed");
    }
#endif
}

int32_t DisplayBufferVdiImpl::AllocMem(const AllocInfo& info, BufferHandle*& handle) const
{
#ifdef GRALLOC_GBM_SUPPORT
    if (info.usage & HBM_USE_MEM_DMA) {
        return GbmAllocMem(&info, &handle);
    }
#endif
    DISPLAY_LOGE("the usage is not support 0x(%{public}" PRIx64 ")", info.usage);
    return HDF_ERR_NOT_SUPPORT;
}

void DisplayBufferVdiImpl::FreeMem(const BufferHandle& handle) const
{
#ifdef GRALLOC_GBM_SUPPORT
    if (handle.usage & HBM_USE_MEM_DMA) {
        GbmFreeMem(const_cast<BufferHandle *>(&handle));
        return;
    }
#endif
    DISPLAY_LOGE("the usage is not support 0x%{public}" PRIx64 "", handle.usage);
}

void* DisplayBufferVdiImpl::Mmap(const BufferHandle& handle) const
{
#ifdef GRALLOC_GBM_SUPPORT
    if (handle.usage & HBM_USE_MEM_DMA) {
        return GbmMmap(const_cast<BufferHandle *>(&handle));
    }
#endif
    DISPLAY_LOGE("the usage is not support 0x%{public}" PRIx64 "", handle.usage);
    return nullptr;
}

int32_t DisplayBufferVdiImpl::Unmap(const BufferHandle& handle) const
{
#ifdef GRALLOC_GBM_SUPPORT
    if (handle.usage & HBM_USE_MEM_DMA) {
        return GbmUnmap(const_cast<BufferHandle *>(&handle));
    }
#endif
    DISPLAY_LOGE("the usage is not support 0x%{public}" PRIx64 "", handle.usage);
    return HDF_ERR_NOT_SUPPORT;
}

int32_t DisplayBufferVdiImpl::FlushCache(const BufferHandle& handle) const
{
#ifdef GRALLOC_GBM_SUPPORT
    if (handle.usage & HBM_USE_MEM_DMA) {
        return GbmFlushCache(const_cast<BufferHandle *>(&handle));
    }
#endif
    DISPLAY_LOGE("the usage is not support 0x%{public}" PRIx64 "", handle.usage);
    return HDF_ERR_NOT_SUPPORT;
}

int32_t DisplayBufferVdiImpl::InvalidateCache(const BufferHandle& handle) const
{
#ifdef GRALLOC_GBM_SUPPORT
    if (handle.usage & HBM_USE_MEM_DMA) {
        return GbmInvalidateCache(const_cast<BufferHandle *>(&handle));
    }
#endif
    DISPLAY_LOGE("the usage is not support 0x%{public}" PRIx64 "", handle.usage);
    return HDF_ERR_NOT_SUPPORT;
}

int32_t DisplayBufferVdiImpl::IsSupportedAlloc(const std::vector<VerifyAllocInfo>& infos,
    std::vector<bool>& supporteds) const
{
    return HDF_ERR_NOT_SUPPORT;
}

extern "C" IDisplayBufferVdi* CreateDisplayBufferVdi()
{
    return new DisplayBufferVdiImpl();
}

extern "C" void DestroyDisplayBufferVdi(IDisplayBufferVdi* vdi)
{
    delete vdi;
}

} // namespace DISPLAY
} // namespace HDI
} // namespace OHOS
