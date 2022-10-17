/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DRM_DEVICE_H
#define DRM_DEVICE_H
#include <unordered_map>
#include <memory>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "drm_connector.h"
#include "drm_crtc.h"
#include "drm_encoder.h"
#include "drm_plane.h"
#include "hdi_device_common.h"
#include "hdi_device_interface.h"
#include "hdi_display.h"
#include "hdi_shared_fd.h"

namespace OHOS {
    namespace HDI {
        namespace DISPLAY {
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

            class DrmPropertyEnum {
            public:
                explicit DrmPropertyEnum(drm_mode_property_enum *e) : value(e->value), name(e->name)
                {
                }
                ~DrmPropertyEnum() {};

                uint64_t value;
                std::string name;
            };

            struct DrmProperty {
                uint32_t propId;
                uint64_t value;
                uint32_t type;
                uint32_t flags;
                std::string name;
                std::vector<uint64_t> values;
                std::vector<DrmPropertyEnum> enums;
                std::vector<uint32_t> blob_ids;
            };

            class DrmDevice : public HdiDeviceInterface, std::enable_shared_from_this<DrmDevice> {
            public:
                static std::shared_ptr<HdiDeviceInterface> Create();
                static uint32_t ConvertToDrmFormat(PixelFormat fmtIn);
                static int GetDrmFd();
                DrmDevice();
                ~DrmDevice() override
                {
                }

                std::vector<std::shared_ptr<DrmPlane>> GetDrmPlane(uint32_t pipe, uint32_t type);

                int32_t GetCrtcProperty(const DrmCrtc &crtc, const std::string &name, DrmProperty &prop);
                int32_t GetConnectorProperty(const DrmConnector &connector, const std::string &name, DrmProperty &prop);
                int32_t GetPlaneProperty(const DrmPlane &plane, const std::string &name, DrmProperty &prop);

                int32_t GetProperty(uint32_t objId, uint32_t objType, const std::string &name, DrmProperty &prop);
                std::shared_ptr<DrmEncoder> GetDrmEncoderFromId(uint32_t id);
                std::shared_ptr<DrmConnector> GetDrmConnectorFromId(uint32_t id);
                std::shared_ptr<DrmCrtc> GetDrmCrtcFromId(uint32_t id);
                void CreateCrtc(drmModeCrtcPtr c);
                std::unordered_map<uint32_t, std::shared_ptr<HdiDisplay>> DiscoveryDisplay() override;
                int32_t Init() override;
                void DeInit() override;
                bool HandleHotplug(uint32_t dispId, bool plugIn) override;

            private:
                static FdPtr mDrmFd;
                static std::shared_ptr<DrmDevice> mInstance;
                void FindAllCrtc(const drmModeResPtr &drmRes);
                void FindAllEncoder(const drmModeResPtr &drmRes);
                void FindAllConnector(const drmModeResPtr &drmRes);
                void FindAllPlane();
                int InitNetLink();
                IdMapPtr<HdiDisplay> mDisplays;
                IdMapPtr<DrmCrtc> mCrtcs;
                IdMapPtr<DrmEncoder> mEncoders;
                IdMapPtr<DrmConnector> mConnectors;
                std::vector<std::shared_ptr<DrmPlane>> mPlanes;
                std::unordered_map<uint32_t, uint32_t> dispConnectorIdMaps_;
            };
        } // namespace OHOS
    }     // namespace HDI
} // namespace DISPLAY

#endif // DRM_DEVICE_H
