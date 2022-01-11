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

#ifndef EVENT_CONTEXT_H
#define EVENT_CONTEXT_H

#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace NetManagerStandard {
// net policy
const std::string NET_UID_POLICY_CHANGE = "netUidPolicyChange";
const std::string NET_CELLULAR_POLICY_CHANGE = "netCellularPolicyChange";
const std::string NET_STRATEGY_SWITCH_CHANGE = "netStrategySwitchChange";
// net status
const std::string NET_AVAILABLE_CHANGE = "netAvailableChange";
const std::string NET_CAPABILITIES_CHANGE = "netCapabilitiesChange";
const std::string NET_CONNECTION_CHANGE = "connectionPropertiesChange";
const std::string NET_LOST_CHANGE = "netLostChange";
constexpr int32_t EVENT_CONTENT_MAX_BYTE = 64;

enum {
    // net policy
    EVENT_NET_UID_POLICY_CHANGE = 21,
    EVENT_NET_CELLULAR_POLICY_CHANGE = 22,
    EVENT_NET_STRATEGY_SWITCH_CHANGE = 23,
    // net status
    EVENT_NET_AVAILABLE_CHANGE = 31,
    EVENT_NET_CAPABILITIES_CHANGE = 32,
    EVENT_NET_CONNECTION_CHANGE = 33,
    EVENT_NET_LOST_CHANGE = 34,
};

// event listener context
struct EventListener {
    EventListener() {}
    int32_t eventId = 0;
    napi_env env = nullptr;
    napi_ref callbackRef = nullptr;
};

struct NetStatusListenser
    : public EventListener {
    NetStatusListenser() {}
    std::string identifier;
    int32_t netType = 0;
    int32_t netCapabilities = 0;
    int32_t netId = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // EVENT_CONTEXT_H
