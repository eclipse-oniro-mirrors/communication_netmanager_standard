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

#ifndef I_NET_CONN_CALLBACK_H
#define I_NET_CONN_CALLBACK_H

#include <string>

#include "iremote_broker.h"

#include "net_conn_callback_info.h"

namespace OHOS {
namespace NetManagerStandard {
class INetConnCallback : public IRemoteBroker {
public:
    virtual ~INetConnCallback() = default;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetConnCallback");
    enum {
        NET_CONN_STATE_CHANGED = 0,
    };

public:
    virtual int32_t NetConnStateChanged(const sptr<NetConnCallbackInfo> &info) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NET_CONN_CALLBACK_H
