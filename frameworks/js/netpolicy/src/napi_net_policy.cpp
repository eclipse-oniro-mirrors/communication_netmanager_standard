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

#include "napi_net_policy.h"
#include <memory>
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "net_mgr_log_wrapper.h"
#include "i_net_policy_service.h"
#include "net_conn_constants.h"
#include "net_policy_client.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
void NapiNetPolicy::ExecSetUidPolicy(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    NetUidPolicy policy = static_cast<NetUidPolicy>(context->policy);
    context->policyResult =
        static_cast<int32_t>(DelayedSingleton<NetPolicyClient>::GetInstance()->SetUidPolicy(context->uid, policy));
    NETMGR_LOGI("ExecSetUidPolicy, policy = [%{public}d], policyResult = [%{public}d]",
        context->policy, context->policyResult);
}

void NapiNetPolicy::ExecGetUids(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    NetUidPolicy policy = static_cast<NetUidPolicy>(context->policy);
    context->uidTogether = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUids(policy);
    NETMGR_LOGI("ExecGetUids, policy = [%{public}d], res.length = [%{public}d]",
        context->policy, static_cast<int>(context->uidTogether.size()));
}

void NapiNetPolicy::ExecGetUidPolicy(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    context->policyResult =
        static_cast<int32_t>(DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidPolicy(context->uid));
    NETMGR_LOGI("ExecGetUidPolicy, uid = [%{public}d], policyResult = [%{public}d]",
        context->uid, context->policyResult);
}

void NapiNetPolicy::ExecIsUidNetAccess(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    if (context->interfaceName.length() > 0) {
        context->result =
            DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(context->uid, context->interfaceName);
    } else {
        context->result =
            DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(context->uid, context->metered);
    }
}
void NapiNetPolicy::CompleteSetUidPolicy(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->policyResult, &info);
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->policyResult != static_cast<int32_t>(NetPolicyResultCode::ERR_NONE)) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back
        napi_value callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_CNT)] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->policyResult != static_cast<int32_t>(NetPolicyResultCode::ERR_NONE)) {
            callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_INDEX_0)] = info;
        } else {
            callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_INDEX_1)] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetPolicy::CompleteGetUids(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    std::vector<uint32_t>& res = context->uidTogether;
    napi_create_array(env, &info);
    for (unsigned int i = 0; i < res.size(); i++) {
        napi_value num;
        napi_create_uint32(env, res[i], &num);
        napi_set_element(env, info, i, num);
    }
    if (!context->callbackRef) {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
    } else {
        napi_value callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_CNT)] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_INDEX_1)] = info;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetPolicy::CompleteGetUidPolicy(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->policyResult, &info);
    if (!context->callbackRef) {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
    } else {
        napi_value callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_CNT)] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_INDEX_1)] = info;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetPolicy::CompleteIsUidNetAccess(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->result, &info);
    if (!context->callbackRef) {
        if (context->result == 0) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        napi_value callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_CNT)] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result == 0) {
            callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_INDEX_0)] = info;
        } else {
            callbackValues[static_cast<int32_t>(JS_CALLBACK_ARGV::CALLBACK_ARGV_INDEX_1)] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetPolicy::DeclareNapiNetPolicyData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_NONE",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_ALLOW_METERED_BACKGROUND",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_TEMPORARY_ALLOW_METERED",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_REJECT_METERED_BACKGROUND", NapiCommon::NapiValueByInt32(env,
            static_cast<int32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_ALLOW_ALL",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_ALLOW_ALL))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_REJECT_ALL",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_REJECT_ALL))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetPolicy::DeclareNapiNetPolicyResultData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("ERR_NONE",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetPolicyResultCode::ERR_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("ERR_INTERNAL_ERROR",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR))),
        DECLARE_NAPI_STATIC_PROPERTY("ERR_INVALID_UID",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetPolicyResultCode::ERR_INVALID_UID))),
        DECLARE_NAPI_STATIC_PROPERTY("ERR_INVALID_POLICY",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetPolicyResultCode::ERR_INVALID_POLICY))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetPolicy::DeclareNapiNetPolicyInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("setUidPolicy", SetUidPolicy),
        DECLARE_NAPI_FUNCTION("getUidPolicy", GetUidPolicy),
        DECLARE_NAPI_FUNCTION("getUids", GetUids),
        DECLARE_NAPI_FUNCTION("isUidNetAccess", IsUidNetAccess),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetPolicy::SetUidPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = static_cast<size_t>(JS_ARGV_NUM::ARGV_NUM_3);
    napi_value argv[] = {nullptr, nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_uint32(env, argv[static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_0)], &context->uid));
    NAPI_CALL(env, napi_get_value_uint32(env, argv[static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_1)],
        &context->policy));
    NETMGR_LOGI("JS agvc count = [%{public}d], argv[static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_0)] = [%{public}d], "
        "argv[ARGV_INDEX_1] = [%{public}d]",
        static_cast<int>(argc), context->uid, context->policy);
    napi_value result = nullptr;
    if (argc == static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_2)) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_3)) {
        NAPI_CALL(env, napi_create_reference(env, argv[static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_2)],
            CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOGE("SetUidPolicy exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "SetUidPolicy", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecSetUidPolicy,
        CompleteSetUidPolicy,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::GetUidPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = static_cast<size_t>(JS_ARGV_INDEX::ARGV_INDEX_2);
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_uint32(env, argv[static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_0)], &context->uid));
    NETMGR_LOGE("JS agvc count = [%{public}d], argv[0] = [%{public}d]",
        static_cast<int>(argc), context->uid);
    // Get and verify parameter[js]
    napi_value result = nullptr;
    if (argc == static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_1)) {
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_2)) {
        NAPI_CALL(env, napi_create_reference(env, argv[static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_1)],
            CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOGE("GetUidPolicy exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getUids", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetUidPolicy,
        CompleteGetUidPolicy,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::GetUids(napi_env env, napi_callback_info info)
{
    size_t argc = static_cast<size_t>(JS_ARGV_INDEX::ARGV_INDEX_2);
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_uint32(env, argv[static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_0)],
        &context->policy));
    NETMGR_LOGE("JS agvc count = [%{public}d], argv[0] = [%{public}d]",
        static_cast<int>(argc), context->policy);
    // Get and verify parameter[js]
    napi_value result = nullptr;
    if (argc == static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_1)) {
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_2)) {
        NAPI_CALL(env, napi_create_reference(env, argv[static_cast<int32_t>(JS_ARGV_INDEX::ARGV_INDEX_1)],
            CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOGE("GetUids exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getUids", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetUids,
        CompleteGetUids,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::IsUidNetAccess(napi_env env, napi_callback_info info)
{
    napi_valuetype valueType = napi_undefined;
    size_t argc = static_cast<size_t>(JS_ARGV_NUM::ARGV_NUM_3);
    napi_value argv[] = {nullptr, nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_uint32(env, argv[static_cast<int32_t>(JS_ARGV_NUM::ARGV_NUM_0)], &context->uid));
    napi_typeof(env, argv[static_cast<int32_t>(JS_ARGV_NUM::ARGV_NUM_1)], &valueType);
    if (napi_valuetype::napi_boolean == valueType) {
        NAPI_CALL(env, napi_get_value_bool(env, argv[static_cast<int32_t>(JS_ARGV_NUM::ARGV_NUM_1)],
            &context->metered));
    }
    if (napi_valuetype::napi_string == valueType) {
        char buf[BUFFER_BYTE] = {0};
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[static_cast<int32_t>(JS_ARGV_NUM::ARGV_NUM_1)], buf, sizeof(buf),
            &typeLen);
        context->interfaceName = buf;
        NETMGR_LOGE("interfaceName = [%{public}s].\n", context->interfaceName.c_str());
    }
    NETMGR_LOGI("JS agvc count = [%{public}d], argv[0] = [%{public}d], argv[1] = [%{public}d]",
        static_cast<int>(argc), context->uid, context->metered);
    napi_value result = nullptr;
    if (argc == static_cast<int32_t>(JS_ARGV_NUM::ARGV_NUM_2)) {
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == static_cast<int32_t>(JS_ARGV_NUM::ARGV_NUM_3)) {
        NAPI_CALL(env, napi_create_reference(env, argv[static_cast<int32_t>(JS_ARGV_NUM::ARGV_NUM_2)], CALLBACK_REF_CNT,
            &context->callbackRef));
    } else {
        NETMGR_LOGE("IsUidNetAccess exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "IsUidNetAccess", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecIsUidNetAccess,
        CompleteIsUidNetAccess,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::RegisterNetPolicyInterface(napi_env env, napi_value exports)
{
    NETMGR_LOGI("RegisterNetPolicyInterface");
    DeclareNapiNetPolicyInterface(env, exports);
    DeclareNapiNetPolicyData(env, exports);
    DeclareNapiNetPolicyResultData(env, exports);
    return nullptr;
}

static napi_module _netPolicyModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiNetPolicy::RegisterNetPolicyInterface,
    .nm_modname = "netmanager.netpolicy",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNetPolicyModule(void)
{
    napi_module_register(&_netPolicyModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
