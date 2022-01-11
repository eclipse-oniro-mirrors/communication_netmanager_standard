/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NAPI_NETCONN_OBSERVER_H
#define NAPI_NETCONN_OBSERVER_H

#include "net_conn_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
struct CapabilitiesEvent {
    CapabilitiesEvent(int32_t netId = 0, uint64_t netCap = 0)
        : netId(netId), netCap(netCap) {}
    int32_t netId;
    uint64_t netCap;
};

struct ConnectionEvent {
    ConnectionEvent(int32_t netId = 0, sptr<NetLinkInfo> linkInfo = nullptr)
        : netId(netId), linkInfo(linkInfo) {}
    int32_t netId;
    sptr<NetLinkInfo> linkInfo;
};

class NapiNetConnObserver : public NetConnCallbackStub {
public:
    int32_t NetAvailable(sptr<NetHandler> &netHandler);
    int32_t NetCapabilitiesChange(sptr<NetHandler> &netHandler, const uint64_t &netCap);
    int32_t NetConnectionPropertiesChange(sptr<NetHandler> &netHandler, const sptr<NetLinkInfo> &info);
    int32_t NetLost(sptr<NetHandler> &netHandler);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_NETCONN_OBSERVER_H
