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

#ifndef I_NET_POLICY_SERVICE_H
#define I_NET_POLICY_SERVICE_H

#include "iremote_broker.h"
#include "net_policy_constants.h"

namespace OHOS {
namespace NetManagerStandard {
class INetPolicyService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetPolicyService");
    enum {
        CMD_NSM_START = 0,
        CMD_NSM_SET_UID_POLICY = 1,
        CMD_NSM_GET_UID_POLICY = 2,
        CMD_NSM_GET_UIDS = 3,
        CMD_NSM_IS_NET_ACCESS_METERED = 4,
        CMD_NSM_IS_NET_ACCESS_IFACENAME = 5,
        CMD_NSM_END = 100,
    };

public:
    virtual NetPolicyResultCode SetUidPolicy(uint32_t uid, NetUidPolicy policy) = 0;
    virtual NetUidPolicy GetUidPolicy(uint32_t uid) = 0;
    virtual std::vector<uint32_t> GetUids(NetUidPolicy policy) = 0;
    virtual bool IsUidNetAccess(uint32_t uid, bool metered) = 0;
    virtual bool IsUidNetAccess(uint32_t uid, const std::string &ifaceName) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NET_POLICY_SERVICE_H
