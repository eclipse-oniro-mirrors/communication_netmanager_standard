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

#ifndef NAPI_NET_POLICY_H
#define NAPI_NET_POLICY_H

#include <string>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "net_policy_constants.h"
#include "net_policy_quota_policy.h"
#include "net_policy_cellular_policy.h"
#include "base_context.h"

namespace OHOS {
constexpr int32_t EVENT_CONTENT_MAX_BYTE = 64;
namespace NetManagerStandard {
// net policy async context
struct NetPolicyAsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    char eventContent[EVENT_CONTENT_MAX_BYTE] = {0};
    size_t eventContentRealBytes = 0;
    int32_t eventPolicyId = 0;
    // uid
    uint32_t uid = 0;
    // policy
    uint32_t policy = 0;
    // result of policy napi
    int32_t policyResult = 0;
    // vector of uid
    std::vector<uint32_t> uidTogether;
    // result of bool type
    bool result = false;
    // metered
    bool metered = false;
    // interface name
    std::string interfaceName;
    // quotaPolicys
    std::vector<NetPolicyQuotaPolicy> quotaPolicys;
    // quotaPolicy
    NetPolicyQuotaPolicy quotaPolicy;
    // result code of NetPolicyResultCode
    NetPolicyResultCode resultCode = NetPolicyResultCode::ERR_NONE;
    // argument of SetIdleTrustlist function
    bool isTrustList = false;
    std::vector<uint32_t> uids;
};

struct GetNetPolicyContext : BaseContext {
    std::vector<NetPolicyQuotaPolicy> result;
};

struct PolicyResetFactoryContext : BaseContext {
    std::string subscriberId;
};

struct GetBackgroundPolicyContext : BaseContext {
    NetBackgroundPolicy backgroundPolicy;
};

struct CellularPolicysContext : BaseContext {
    std::vector<NetPolicyCellularPolicy> input;
};

class NapiNetPolicy {
public:
    NapiNetPolicy();
    ~NapiNetPolicy() = default;
    static napi_value RegisterNetPolicyInterface(napi_env env, napi_value exports);
    static napi_value DeclareNapiNetPolicyInterface(napi_env env, napi_value exports);
    static napi_value DeclareNapiNetPolicyData(napi_env env, napi_value exports);
    static napi_value DeclareNapiNetPolicyResultData(napi_env env, napi_value exports);

    static void ExecSetPolicyByUid(napi_env env, void *data);
    static void ExecGetUidsByPolicy(napi_env env, void *data);
    static void ExecGetPolicyByUid(napi_env env, void *data);
    static void ExecSetNetPolicies(napi_env env, void *data);
    static void ExecSetSnoozePolicy(napi_env env, void *data);
    static void ExecPolicyResetFactory(napi_env env, void *data);
    static void ExecGetBackgroundPolicy(napi_env env, void *data);
    static void ExecOn(napi_env env, void *data);
    static void ExecOff(napi_env env, void *data);
    static void CompleteSetPolicyByUid(napi_env env, napi_status status, void *data);
    static void CompleteGetUidsByPolicy(napi_env env, napi_status status, void *data);
    static void CompleteGetPolicyByUid(napi_env env, napi_status status, void *data);
    static void CompleteSetNetPolicies(napi_env env, napi_status status, void *data);
    static void CompleteSetSnoozePolicy(napi_env env, napi_status status, void *data);
    static void CompleteNetPolicyResultCode(napi_env env, napi_status status, void *data);
    static void CompletePolicyResetFactory(napi_env env, napi_status status, void *data);
    static void CompleteGetBackgroundPolicy(napi_env env, napi_status status, void *data);
    static void CompleteOn(napi_env env, napi_status status, void *data);
    static void CompleteOff(napi_env env, napi_status status, void *data);

    // Declare napi interfaces for JS
    static NetPolicyQuotaPolicy ReadQuotaPolicy(napi_env env, napi_value value);
    static napi_value SetPolicyByUid(napi_env env, napi_callback_info info);
    static napi_value GetPolicyByUid(napi_env env, napi_callback_info info);
    static napi_value GetUidsByPolicy(napi_env env, napi_callback_info info);
    static napi_value SetNetPolicies(napi_env env, napi_callback_info info);
    static napi_value GetNetPolicies(napi_env env, napi_callback_info info);
    static napi_value SetSnoozePolicy(napi_env env, napi_callback_info info);
    static napi_value PolicyResetFactory(napi_env env, napi_callback_info info);
    static napi_value GetBackgroundPolicy(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_NET_POLICY_H
