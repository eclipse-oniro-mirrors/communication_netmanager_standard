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

#ifndef NET_POLICY_DEFINE_H
#define NET_POLICY_DEFINE_H

namespace OHOS {
namespace NetManagerStandard {
const mode_t CHOWN_RWX_USR_GRP = 0770;
const char POLICY_FILE_NAME[] = "/data/system/net_policy.json";
const char CONFIG_HOS_VERSION[] = "hosVersion";
const char CONFIG_UID_POLICY[] = "uidPolicy";
const char CONFIG_UID[] = "uid";
const char CONFIG_POLICY[] = "policy";
const char HOS_VERSION[] = "1.0";
const int32_t CONVERT_LENGTH_TEN  = 10;

/* network allow policy mask */
const uint32_t NET_POLICY_ALLOW_MASK = 0b00100011;

struct UidPolicy {
    std::string uid;
    std::string policy;
};

struct NetPolicy {
    std::vector<UidPolicy> uidPolicys;
    std::string hosVersion;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_DEFINE_H