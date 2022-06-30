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

#ifndef OMX_PLUGIN_H_
#define OMX_PLUGIN_H_

#include <mutex>
#include <vector>
#include "IOMXComponentMgr.h"

namespace OHOS {
namespace Media {
namespace OMX {
using namespace std;

struct OMXCore {
    typedef OMX_ERRORTYPE (*InitFunc)();
    typedef OMX_ERRORTYPE (*DeinitFunc)();
    typedef OMX_ERRORTYPE (*ComponentNameEnumFunc)(
            OMX_STRING, OMX_U32, OMX_U32);

    typedef OMX_ERRORTYPE (*GetHandleFunc)(
            OMX_HANDLETYPE *, OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE *);

    typedef OMX_ERRORTYPE (*FreeHandleFunc)(OMX_HANDLETYPE *);

    typedef OMX_ERRORTYPE (*GetRolesOfComponentFunc)(
            OMX_STRING, OMX_U32 *, OMX_U8 **);

    void *mLibHandle;

    InitFunc mInit;
    DeinitFunc mDeinit;
    ComponentNameEnumFunc mComponentNameEnum;
    GetHandleFunc mGetHandle;
    FreeHandleFunc mFreeHandle;
    GetRolesOfComponentFunc mGetRolesOfComponentHandle;

    OMX_U32 mNumComponents;
};

struct OMXComponent {
    OMX_COMPONENTTYPE *mComponent;
    OMXCore *mCore;
};

struct OMXPlugin : public IOMXComponentMgr {
    OMXPlugin();
    virtual ~OMXPlugin() override;

    virtual OMX_ERRORTYPE CreateComponentInstance(
            const OMX_STRING name,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component) override;

    virtual OMX_ERRORTYPE DeleteComponentInstance(
            OMX_COMPONENTTYPE *component) override;

    virtual OMX_ERRORTYPE EnumerateComponentsByIndex(
            OMX_U32 index,
            OMX_STRING name,
            size_t size) override;

    virtual OMX_ERRORTYPE GetRolesForComponent(
            const OMX_STRING name,
            vector<string> *roles) override;
private:

    mutex mMutex;
    vector<OMXCore*> mCores;
    vector<OMXComponent> mComponents;

    OMX_ERRORTYPE AddCore(const char* coreName);

    OMXPlugin(const OMXPlugin &);
    OMXPlugin &operator=(const OMXPlugin &);
};
} // OMX
} // Media
} // OHOS

#endif  // OMX_PLUGIN_H_
