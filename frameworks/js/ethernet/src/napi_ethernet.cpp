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

#include "napi_ethernet.h"
#include <memory>
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "net_mgr_log_wrapper.h"
#include "i_ethernet_service.h"
#include "ethernet_client.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
NapiEthernet::NapiEthernet() {}

void NapiEthernet::ExecSetIfaceConfig(napi_env env, void *data)
{
    NETMGR_LOGI("ExecSetIfaceConfig");
    EthernetAsyncContext *context = (EthernetAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    INetAddr addr0, addr1;
    sptr<InterfaceConfiguration> config = std::make_unique<InterfaceConfiguration>().release();
    if (config == nullptr) {
        NETMGR_LOGE("config == nullptr");
        return;
    }
    addr0.address_ = context->dns0AddrInfo;
    addr1.address_ = context->dns1AddrInfo;
    config->mode_ = static_cast<IPSetMode>(context->ipMode);
    config->ipStatic_.ipAddr_.address_ = context->ipAddrInfo;
    config->ipStatic_.route_.address_ = context->routeAddrInfo;
    config->ipStatic_.gate_.address_ = context->gateAddrInfo;
    config->ipStatic_.netMask_.address_ = context->maskAddrInfo;
    config->ipStatic_.dnsServers_.push_back(addr0);
    config->ipStatic_.dnsServers_.push_back(addr1);
    context->result = DelayedSingleton<EthernetClient>::GetInstance()->SetIfaceConfig(
        context->ethernetName, config);
    NETMGR_LOGI("ExecSetIfaceConfig result =[%{public}d]", context->result);
}

void NapiEthernet::CompleteSetIfaceConfig(napi_env env, napi_status status, void *data)
{
    NETMGR_LOGI("CompleteSetIfaceConfig");
    EthernetAsyncContext* context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->result, &info);
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = info;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiEthernet::ExecGetIfaceConfig(napi_env env, void *data)
{
    NETMGR_LOGI("ExecGetIfaceConfig");
    EthernetAsyncContext *context = (EthernetAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    sptr<InterfaceConfiguration> config =
        DelayedSingleton<EthernetClient>::GetInstance()->GetIfaceConfig(context->ethernetName);
    if (config != nullptr) {
        context->ifActivate = 1;
        std::string tap;
        context->ipMode = config->mode_;
        NETMGR_LOGI("config->mode_ = [%{public}d]", config->mode_);
        context->ipAddrInfo = config->ipStatic_.ipAddr_.ToString(tap);
        NETMGR_LOGI("config->ipAddr_ = [%{public}s]", config->ipStatic_.ipAddr_.ToString(tap).c_str());
        context->routeAddrInfo = config->ipStatic_.route_.ToString(tap);
        NETMGR_LOGI("config->route_ = [%{public}s]", config->ipStatic_.route_.ToString(tap).c_str());
        context->gateAddrInfo = config->ipStatic_.gate_.ToString(tap);
        NETMGR_LOGI("config->gate_ = [%{public}s]", config->ipStatic_.gate_.ToString(tap).c_str());
        context->maskAddrInfo = config->ipStatic_.netMask_.ToString(tap);
        NETMGR_LOGI("config->netMask_ = [%{public}s]", config->ipStatic_.netMask_.ToString(tap).c_str());
        for (auto it = config->ipStatic_.dnsServers_.begin(); it != config->ipStatic_.dnsServers_.end(); ++it) {
            context->dns0AddrInfo = it->ToString(tap);
            context->dns1AddrInfo = it->ToString(tap);
            NETMGR_LOGI("config->dnsServers_ = [%{public}s]", it->ToString(tap).c_str());
        }
    } else {
        context->ifActivate = -1;
    }
}

void NapiEthernet::CompleteGetIfaceConfig(napi_env env, napi_status status, void *data)
{
    NETMGR_LOGI("CompleteGetIfaceConfig");
    EthernetAsyncContext* context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoFail = nullptr;
    napi_create_object(env, &info);
    napi_create_string_utf8(env, "fail", NAPI_AUTO_LENGTH, &infoFail);
    NpiCommon::SetPropertyInt32(env, info, "mode", context->ipMode);
    NpiCommon::SetPropertyString(env, info, "ipAddr", context->ipAddrInfo);
    NpiCommon::SetPropertyString(env, info, "routeAddr", context->routeAddrInfo);
    NpiCommon::SetPropertyString(env, info, "gateAddr", context->gateAddrInfo);
    NpiCommon::SetPropertyString(env, info, "maskAddr", context->maskAddrInfo);
    NpiCommon::SetPropertyString(env, info, "dns0Addr", context->dns0AddrInfo);
    NpiCommon::SetPropertyString(env, info, "dns1Addr", context->dns1AddrInfo);
    if (context->callbackRef == nullptr) {
        if (context->ifActivate == -1) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->ifActivate == 0) {
            callbackValues[ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiEthernet::ExecIsActivate(napi_env env, void *data)
{
    NETMGR_LOGI("ExecIsActivate");
    EthernetAsyncContext *context = (EthernetAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    context->ifActivate = DelayedSingleton<EthernetClient>::GetInstance()->IsActivate(context->ethernetName);
    NETMGR_LOGI("ifActivate == [%{public}d]", context->ifActivate);
}

void NapiEthernet::CompleteIsActivate(napi_env env, napi_status status, void *data)
{
    NETMGR_LOGI("CompleteIsActivate");
    EthernetAsyncContext* context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->ifActivate, &info);
    if (context->callbackRef == nullptr) {
        if (context->ifActivate == -1) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->ifActivate == -1) {
            callbackValues[ARGV_INDEX_0] = info;
        } else {
            callbackValues[ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiEthernet::ExecGetActivateInterfaces(napi_env env, void *data)
{
    NETMGR_LOGI("ExecGetActivateInterfaces");
    EthernetAsyncContext *context = (EthernetAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    context->ethernetNameList = DelayedSingleton<EthernetClient>::GetInstance()->GetActivateInterfaces();
}

void NapiEthernet::CompleteGetActivateInterfaces(napi_env env, napi_status status, void *data)
{
    NETMGR_LOGI("CompleteGetActivateInterfaces");
    EthernetAsyncContext* context = static_cast<EthernetAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOGE("context == nullptr");
        return;
    }
    // creat function return
    napi_value infoAttay = nullptr;
    napi_value info = nullptr;
    napi_create_array_with_length(env, context->ethernetNameList.size(), &infoAttay);
    for (size_t index = 0; index < context->ethernetNameList.size(); index++) {
        napi_create_string_utf8(env, context->ethernetNameList[index].c_str(), NAPI_AUTO_LENGTH, &info);
        napi_set_element(env, infoAttay, index, info);
    }
    if (context->callbackRef == nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, infoAttay));
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[CALLBACK_ARGV_INDEX_1] = infoAttay;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiEthernet::SetIfaceConfig(napi_env env, napi_callback_info info)
{
    NETMGR_LOGI("SetIfaceConfig");
    size_t argc = ARGV_NUM_4;
    napi_value argv[] = {nullptr, nullptr, nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOGI("SetIfaceConfig agvc = [%{public}d]", static_cast<int>(argc));
    EthernetAsyncContext* context = std::make_unique<EthernetAsyncContext>().release();
    // Parse Js argv
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0],
        context->ethernetName, ETHERNET_NAME_MAX_BYTE, &(context->ethernetNameRealBytes)));
    NETMGR_LOGI("SetIfaceConfig ethernetName = [%{public}s]", context->ethernetName);
    // Parse Js object [ip]
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_1], &context->ipMode));
    NETMGR_LOGI("SetIfaceConfig mode = [%{public}d]", context->ipMode);
    if (context->ipMode == IPSetMode::STATIC) {
        NpiCommon::GetPropertyString(env, argv[ARGV_INDEX_2], "ipAddr", context->ipAddrInfo);
        NpiCommon::GetPropertyString(env, argv[ARGV_INDEX_2], "routeAddr", context->routeAddrInfo);
        NpiCommon::GetPropertyString(env, argv[ARGV_INDEX_2], "gateAddr", context->gateAddrInfo);
        NpiCommon::GetPropertyString(env, argv[ARGV_INDEX_2], "maskAddr", context->maskAddrInfo);
        NpiCommon::GetPropertyString(env, argv[ARGV_INDEX_2], "dnsAddr0", context->dns0AddrInfo);
        NpiCommon::GetPropertyString(env, argv[ARGV_INDEX_2], "dnsAddr1", context->dns1AddrInfo);
    }
    napi_value result = nullptr;
    if ((argc == ARGV_NUM_2) || (argc == ARGV_NUM_3 && context->ipMode == IPSetMode::STATIC)) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if ((argc == ARGV_NUM_4) || (argc == ARGV_NUM_3 && context->ipMode == IPSetMode::DHCP)) {
        NAPI_CALL(env, napi_create_reference(env, argv[argc - 1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOGE("SetIfaceConfig  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "SetIfaceConfig", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecSetIfaceConfig,
        CompleteSetIfaceConfig,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiEthernet::GetIfaceConfig(napi_env env, napi_callback_info info)
{
    NETMGR_LOGI("GetIfaceConfig");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOGI("GetIfaceConfig agvc = [%{public}d]", static_cast<int>(argc));
    EthernetAsyncContext* context = std::make_unique<EthernetAsyncContext>().release();
    // Parse Js argv
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0],
        context->ethernetName, ETHERNET_NAME_MAX_BYTE, &(context->ethernetNameRealBytes)));
    NETMGR_LOGI("GetIfaceConfig [%{public}s]", context->ethernetName);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOGE("GetIfaceConfig  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetIfaceConfig", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetIfaceConfig,
        CompleteGetIfaceConfig,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiEthernet::IsActivate(napi_env env, napi_callback_info info)
{
    NETMGR_LOGI("IsActivate");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOGI("IsActivate agvc = [%{public}d]", static_cast<int>(argc));
    EthernetAsyncContext* context = std::make_unique<EthernetAsyncContext>().release();
    // Parse Js argv
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0],
        context->ethernetName, ETHERNET_NAME_MAX_BYTE, &(context->ethernetNameRealBytes)));
    NETMGR_LOGI("IsActivate [%{public}s]", context->ethernetName);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOGE("IsActivate  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "IsActivate", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecIsActivate,
        CompleteIsActivate,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiEthernet::GetActivateInterfaces(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOGI("GetActivateInterfaces agvc = [%{public}d]", static_cast<int>(argc));
    EthernetAsyncContext* context = std::make_unique<EthernetAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_1) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOGE("GetActivateInterfaces  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetActivateInterfaces", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetActivateInterfaces,
        CompleteGetActivateInterfaces,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiEthernet::DeclareEthernetData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("STATIC",
            NpiCommon::NapiValueByInt32(env, static_cast<int32_t>(IPSetMode::STATIC))),
        DECLARE_NAPI_STATIC_PROPERTY("DHCP",
            NpiCommon::NapiValueByInt32(env, static_cast<int32_t>(IPSetMode::DHCP))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiEthernet::DeclareEthernetInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("setIfaceConfig", SetIfaceConfig),
        DECLARE_NAPI_FUNCTION("getIfaceConfig", GetIfaceConfig),
        DECLARE_NAPI_FUNCTION("isActivate", IsActivate),
        DECLARE_NAPI_FUNCTION("getActivateInterfaces", GetActivateInterfaces),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiEthernet::RegisterEthernetInterface(napi_env env, napi_value exports)
{
    DeclareEthernetInterface(env, exports);
    DeclareEthernetData(env, exports);
    return nullptr;
}

static napi_module _ethernetModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiEthernet::RegisterEthernetInterface,
    .nm_modname = "netmanager.ethernet",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEthernetModule(void)
{
    napi_module_register(&_ethernetModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
