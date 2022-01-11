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

#ifndef NET_POLICY_CALLBACK_H
#define NET_POLICY_CALLBACK_H

#include <string>
#include <vector>

#include "i_net_policy_callback.h"
#include "net_policy_cellular_policy.h"

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyCallback : public virtual RefBase {
public:
    void RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback);
    void UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback);
    int32_t NotifyNetUidPolicyChanged(uint32_t uid, NetUidPolicy policy);
    int32_t NotifyNetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicies);
    int32_t NotifyNetStrategySwitch(const std::string &subscriberId, bool enable);
    int32_t NotifyNetBackgroundPolicyChanged(bool isBackgroundPolicyAllow);

private:
    std::vector<sptr<INetPolicyCallback>> netPolicyCallback_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_CALLBACK_H