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

#include "napi_net_stats.h"
#include <memory>
#include <cinttypes>
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "net_mgr_log_wrapper.h"
#include "data_flow_statistics.h"
#include "napi_common.h"
#include "net_stats_client.h"
#include "i_net_stats_service.h"
#include "net_stats_event_listener_manager.h"
#include "event_context.h"
#include "net_stats_callback.h"

namespace OHOS {
namespace NetManagerStandard {
napi_value NapiNetStats::DeclareNapiNetStatsInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getCellularRxBytes", GetCellularRxBytes),
        DECLARE_NAPI_FUNCTION("getCellularTxBytes", GetCellularTxBytes),
        DECLARE_NAPI_FUNCTION("getAllRxBytes", GetAllRxBytes),
        DECLARE_NAPI_FUNCTION("getAllTxBytes", GetAllTxBytes),
        DECLARE_NAPI_FUNCTION("getUidRxBytes", GetUidRxBytes),
        DECLARE_NAPI_FUNCTION("getUidTxBytes", GetUidTxBytes),
        DECLARE_NAPI_FUNCTION("getIfaceRxBytes", GetIfaceRxBytes),
        DECLARE_NAPI_FUNCTION("getIfaceTxBytes", GetIfaceTxBytes),
        DECLARE_NAPI_FUNCTION("getIfaceStatsDetail", GetIfaceStatsDetail),
        DECLARE_NAPI_FUNCTION("getUidStatsDetail", GetUidStatsDetail),
        DECLARE_NAPI_FUNCTION("updateIfacesStats", UpdateIfacesStats),
        DECLARE_NAPI_FUNCTION("updateStatsData", UpdateStatsData),
        DECLARE_NAPI_FUNCTION("registerNetStatsCallback", RegisterNetStatsCallback),
        DECLARE_NAPI_FUNCTION("unregisterNetStatsCallback", UnregisterNetStatsCallback),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

void NapiNetStats::CompleteGetCellularRxBytes(napi_env env, napi_status status, void *data)
{
    CompleteGetBytes(env, status, data);
}

void NapiNetStats::CompleteGetCellularTxBytes(napi_env env, napi_status status, void *data)
{
    CompleteGetBytes(env, status, data);
}

void NapiNetStats::CompleteGetAllRxBytes(napi_env env, napi_status status, void *data)
{
    CompleteGetBytes(env, status, data);
}

void NapiNetStats::CompleteGetAllTxBytes(napi_env env, napi_status status, void *data)
{
    CompleteGetBytes(env, status, data);
}

void NapiNetStats::CompleteGetUidRxBytes(napi_env env, napi_status status, void *data)
{
    CompleteGetBytes(env, status, data);
}

void NapiNetStats::CompleteGetUidTxBytes(napi_env env, napi_status status, void *data)
{
    CompleteGetBytes(env, status, data);
}

void NapiNetStats::CompleteGetIfaceRxBytes(napi_env env, napi_status status, void *data)
{
    CompleteGetBytes(env, status, data);
}

void NapiNetStats::CompleteGetIfaceTxBytes(napi_env env, napi_status status, void *data)
{
    CompleteGetBytes(env, status, data);
}

void NapiNetStats::CompleteGetBytes(napi_env env, napi_status status, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = NapiCommon::CreateCodeMessage(env, "successful", context->bytes64);
    if (context->callbackRef == nullptr) { // promiss return
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
    } else { // call back
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetStats::CompleteGetIfaceStatsDetail(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteGetIfaceStatsDetail");
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    if (context->callbackRef == nullptr) {
        if (context->result != static_cast<int32_t>(NetStatsResultCode::ERR_NONE)) {
            napi_create_int32(env, context->result, &info);
            callbackValues[0] = info;
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            napi_create_object(env, &info);
            NapiCommon::SetPropertyInt64(env, info, "rxBytes", context->statsInfo.rxBytes_);
            NapiCommon::SetPropertyInt64(env, info, "txBytes", context->statsInfo.txBytes_);
            NapiCommon::SetPropertyInt64(env, info, "rxPackets", context->statsInfo.rxPackets_);
            NapiCommon::SetPropertyInt64(env, info, "txPackets", context->statsInfo.txPackets_);
            callbackValues[1] = info;
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetStats::CompleteGetUidStatsDetail(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteGetUidStatsDetail");
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    if (context->callbackRef == nullptr) {
        if (context->result != static_cast<int32_t>(NetStatsResultCode::ERR_NONE)) {
            napi_create_int32(env, context->result, &info);
            callbackValues[0] = info;
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            napi_create_object(env, &info);
            NapiCommon::SetPropertyInt64(env, info, "rxBytes", context->statsInfo.rxBytes_);
            NapiCommon::SetPropertyInt64(env, info, "txBytes", context->statsInfo.txBytes_);
            NapiCommon::SetPropertyInt64(env, info, "rxPackets", context->statsInfo.rxPackets_);
            NapiCommon::SetPropertyInt64(env, info, "txPackets", context->statsInfo.txPackets_);
            callbackValues[1] = info;
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetStats::CompleteUpdateIfacesStats(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteUpdateIfacesStats");
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    if (context->callbackRef == nullptr) {
        if (context->result != static_cast<int32_t>(NetStatsResultCode::ERR_NONE)) {
            napi_create_int32(env, context->result, &info);
            callbackValues[0] = info;
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            napi_create_int32(env, context->result, &info);
            callbackValues[1] = info;
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetStats::CompleteUpdateStatsData(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteUpdateStatsData");
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
    if (context->callbackRef == nullptr) {
        if (context->result != static_cast<int32_t>(NetStatsResultCode::ERR_NONE)) {
            napi_create_int32(env, context->result, &info);
            callbackValues[0] = info;
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            napi_create_int32(env, context->result, &info);
            callbackValues[1] = info;
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetStats::ExecGetCellularRxBytes(napi_env env, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    DataFlowStatistics flow;
    context->bytes64 = flow.GetCellularRxBytes();
    NETMGR_LOG_D("ExecGetCellularRxBytes, result = [%{public}" PRId64 "]", context->bytes64);
}

void NapiNetStats::ExecGetCellularTxBytes(napi_env env, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    DataFlowStatistics flow;
    context->bytes64 = flow.GetCellularTxBytes();
    NETMGR_LOG_D("ExecGetCellularTxBytes, result = [%{public}" PRId64 "]", context->bytes64);
}

void NapiNetStats::ExecGetAllRxBytes(napi_env env, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    DataFlowStatistics flow;
    context->bytes64 = flow.GetAllRxBytes();
    NETMGR_LOG_D("ExecGetAllRxBytes, result = [%{public}" PRId64 "]", context->bytes64);
}

void NapiNetStats::ExecGetAllTxBytes(napi_env env, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    DataFlowStatistics flow;
    context->bytes64 = flow.GetAllTxBytes();
    NETMGR_LOG_D("ExecGetAllTxBytes, result = [%{public}" PRId64 "]", context->bytes64);
}

void NapiNetStats::ExecGetUidRxBytes(napi_env env, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    DataFlowStatistics stats;
    context->bytes64 = stats.GetUidRxBytes(context->uid);
    NETMGR_LOG_D("get bytes = [%{public}" PRId64 "]", context->bytes64);
}

void NapiNetStats::ExecGetUidTxBytes(napi_env env, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    DataFlowStatistics stats;
    context->bytes64 = stats.GetUidTxBytes(context->uid);
    NETMGR_LOG_D("get bytes = [%{public}" PRId64 "]", context->bytes64);
}

void NapiNetStats::ExecGetIfaceRxBytes(napi_env env, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    DataFlowStatistics stats;
    context->bytes64 = stats.GetIfaceRxBytes(context->interfaceName);
    NETMGR_LOG_D("get bytes = [%{public}" PRId64 "]", context->bytes64);
}

void NapiNetStats::ExecGetIfaceTxBytes(napi_env env, void *data)
{
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    DataFlowStatistics stats;
    context->bytes64 = stats.GetIfaceTxBytes(context->interfaceName);
    NETMGR_LOG_D("get bytes = [%{public}" PRId64 "]", context->bytes64);
}

void NapiNetStats::ExecGetIfaceStatsDetail(napi_env env, void *data)
{
    NETMGR_LOG_I("ExecGetIfaceStatsDetail");
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = static_cast<int32_t>(DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceStatsDetail(
        context->interfaceName, context->start, context->end, context->statsInfo));
}

void NapiNetStats::ExecGetUidStatsDetail(napi_env env, void *data)
{
    NETMGR_LOG_I("ExecGetUidStatsDetail");
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = static_cast<int32_t>(DelayedSingleton<NetStatsClient>::GetInstance()->GetUidStatsDetail(
        context->interfaceName, context->uid, context->start, context->end, context->statsInfo));
}

void NapiNetStats::ExecUpdateIfacesStats(napi_env env, void *data)
{
    NETMGR_LOG_I("ExecGetUidStatsDetail");
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    NETMGR_LOG_I("rxBytes = [%{public}" PRId64 "]", context->statsInfo.rxBytes_);
    NETMGR_LOG_I("txBytes = [%{public}" PRId64 "]", context->statsInfo.txBytes_);
    NETMGR_LOG_I("rxPackets = [%{public}" PRId64 "]", context->statsInfo.rxPackets_);
    NETMGR_LOG_I("txPackets = [%{public}" PRId64 "]", context->statsInfo.txPackets_);
    context->result = static_cast<int32_t>(DelayedSingleton<NetStatsClient>::GetInstance()->UpdateIfacesStats(
        context->interfaceName, context->start, context->end, context->statsInfo));
}

void NapiNetStats::ExecUpdateStatsData(napi_env env, void *data)
{
    NETMGR_LOG_I("ExecUpdateStatsData");
    NetStatsAsyncContext *context = static_cast<NetStatsAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = static_cast<int32_t>(DelayedSingleton<NetStatsClient>::GetInstance()->UpdateStatsData());
}

napi_value NapiNetStats::GetCellularRxBytes(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_INDEX_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_INDEX_0) { // promise call
        if (context->callbackRef == nullptr) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_INDEX_1) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        // exception
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetCellularRxBytes", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetCellularRxBytes, CompleteGetCellularRxBytes,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::GetCellularTxBytes(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_INDEX_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_INDEX_0) { // promise call
        if (context->callbackRef == nullptr) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_INDEX_1) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        // exception
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetCellularTxBytes", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetCellularTxBytes, CompleteGetCellularTxBytes,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::GetAllRxBytes(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_INDEX_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_INDEX_0) { // promise call
        if (context->callbackRef == nullptr) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_INDEX_1) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        // exception
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAllRxBytes", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetAllRxBytes, CompleteGetAllRxBytes,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}
napi_value NapiNetStats::GetAllTxBytes(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_INDEX_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_INDEX_0) { // promise call
        if (context->callbackRef == nullptr) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_INDEX_1) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        // exception
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAllTxBytes", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetAllTxBytes, CompleteGetAllTxBytes,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::GetUidRxBytes(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_NUM_0], &context->uid));
    NETMGR_LOG_D("js argc = [%{public}zu], argv[0](uid) = [%{public}d]", argc, context->uid);

    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_NUM_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getUidRxBytes", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetUidRxBytes, CompleteGetUidRxBytes,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::GetUidTxBytes(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_NUM_0], &context->uid));
    NETMGR_LOG_D("js argc = [%{public}zu], argv[0](uid) = [%{public}d]", argc, context->uid);

    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_NUM_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getUidTxBytes", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetUidTxBytes, CompleteGetUidTxBytes,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::GetIfaceRxBytes(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    char buf[BUFFER_BYTE] = {0};
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_get_value_string_utf8(env, argv[ARGV_NUM_0], buf, sizeof(buf), &typeLen);
    context->interfaceName = buf;
    NETMGR_LOG_E("interfaceName = [%{public}s].\n", context->interfaceName.c_str());

    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_NUM_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getIfaceRxBytes", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetIfaceRxBytes, CompleteGetIfaceRxBytes,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::GetIfaceTxBytes(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    char buf[BUFFER_BYTE] = {0};
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_get_value_string_utf8(env, argv[ARGV_NUM_0], buf, sizeof(buf), &typeLen);
    context->interfaceName = buf;
    NETMGR_LOG_E("interfaceName = [%{public}s].\n", context->interfaceName.c_str());

    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_NUM_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getIfaceTxBytes", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetIfaceTxBytes, CompleteGetIfaceTxBytes,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::GetIfaceStatsDetail(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_D("GetIfaceStatsDetail");
    size_t argc = ARGV_NUM_4;
    napi_value argv[] = {nullptr, nullptr, nullptr, nullptr};
    char buf[BUFFER_BYTE] = {0};
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_get_value_string_utf8(env, argv[ARGV_INDEX_0], buf, sizeof(buf), &typeLen);
    context->interfaceName = buf;
    NETMGR_LOG_D("interfaceName = [%{public}s].\n", context->interfaceName.c_str());
    napi_get_value_uint32(env, argv[ARGV_INDEX_1], &context->start);
    napi_get_value_uint32(env, argv[ARGV_INDEX_2], &context->end);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_3) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_4) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_3], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetIfaceStatsDetail", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetIfaceStatsDetail, CompleteGetIfaceStatsDetail,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::GetUidStatsDetail(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_E("GetUidStatsDetail");
    size_t argc = ARGV_NUM_5;
    napi_value argv[] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    char buf[BUFFER_BYTE] = {0};
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_get_value_string_utf8(env, argv[ARGV_INDEX_0], buf, sizeof(buf), &typeLen);
    context->interfaceName = buf;
    NETMGR_LOG_D("interfaceName = [%{public}s].\n", context->interfaceName.c_str());
    napi_get_value_uint32(env, argv[ARGV_INDEX_1], &context->uid);
    napi_get_value_uint32(env, argv[ARGV_INDEX_2], &context->start);
    napi_get_value_uint32(env, argv[ARGV_INDEX_3], &context->end);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_4) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_5) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_4], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetUidStatsDetail", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetUidStatsDetail, CompleteGetUidStatsDetail,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::UpdateIfacesStats(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_5;
    napi_value argv[] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    char buf[BUFFER_BYTE] = {0};
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_get_value_string_utf8(env, argv[ARGV_INDEX_0], buf, sizeof(buf), &typeLen);
    context->interfaceName = buf;
    NETMGR_LOG_E("interfaceName = [%{public}s].\n", context->interfaceName.c_str());
    napi_get_value_uint32(env, argv[ARGV_INDEX_1], &context->start);
    napi_get_value_uint32(env, argv[ARGV_INDEX_2], &context->end);
    // parse object
    NapiCommon::GetPropertyInt64(env, argv[ARGV_INDEX_3], "rxBytes", context->statsInfo.rxBytes_);
    NapiCommon::GetPropertyInt64(env, argv[ARGV_INDEX_3], "txBytes", context->statsInfo.txBytes_);
    NapiCommon::GetPropertyInt64(env, argv[ARGV_INDEX_3], "rxPackets", context->statsInfo.rxPackets_);
    NapiCommon::GetPropertyInt64(env, argv[ARGV_INDEX_3], "txPackets", context->statsInfo.txPackets_);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_4) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_5) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_4], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "UpdateIfacesStats", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecUpdateIfacesStats, CompleteUpdateIfacesStats,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetStats::UpdateStatsData(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetStatsAsyncContext *context = std::make_unique<NetStatsAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(
                env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_1) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "UpdateStatsData", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecUpdateStatsData, CompleteUpdateStatsData,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

bool MatchRegisterNetStatsCallbackParameters(napi_env env, napi_value argv[], size_t argc)
{
    switch (argc) {
        case ARGV_INDEX_1: {
            return NapiCommon::MatchParameters(env, argv, {napi_function});
        }
        default: {
            return false;
        }
    }
}

void NapiNetStats::ExecRegisterNetStatsCallback(napi_env env, void *data)
{
    NETMGR_LOG_I("ExecRegisterNetStatsCallback start");
    BaseContext *context = static_cast<BaseContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    EventListener listen;
    listen.callbackRef = context->callbackRef;
    listen.env = env;
    listen.eventId = INetStatsService::CMD_NSM_REGISTER_NET_STATS_CALLBACK;
    NetStatsEventListenerManager::GetInstance().AddEventListener(listen);
    context->resolved = true;
}

void NapiNetStats::CompleteRegisterNetStatsCallback(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteRegisterNetStatsCallback start");
    auto context = static_cast<BaseContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            callbackValue = NapiCommon::CreateErrorMessage(env, "register net stats error");
        }
    } else {
        callbackValue = NapiCommon::CreateErrorMessage(
            env, "register net stats error cause napi_status = " + std::to_string(status));
    }
    NapiCommon::Handle1ValueCallback(env, context, callbackValue);
}

napi_value NapiNetStats::RegisterNetStatsCallback(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("RegisterNetStatsCallback start");
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, MatchRegisterNetStatsCallbackParameters(env, argv, argc), "type mismatch");
    auto context = std::make_unique<BaseContext>().release();
    napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef);
    napi_value result = NapiCommon::HandleAsyncWork(
        env, context, "RegisterNetStatsCallback", ExecRegisterNetStatsCallback, CompleteRegisterNetStatsCallback);
    return result;
}

void NapiNetStats::ExecUnregisterNetStatsCallback(napi_env env, void *data)
{
    NETMGR_LOG_I("ExecUnregisterNetStatsCallback start");
    BaseContext *context = static_cast<BaseContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    EventListener listen;
    listen.callbackRef = context->callbackRef;
    listen.env = env;
    listen.eventId = INetStatsService::CMD_NSM_UNREGISTER_NET_STATS_CALLBACK;
    NetStatsEventListenerManager::GetInstance().RemoveEventListener(listen);
    context->resolved = true;
}

void NapiNetStats::CompleteUnregisterNetStatsCallback(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteUnregisterNetStatsCallback start");
    auto context = static_cast<BaseContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            callbackValue = NapiCommon::CreateErrorMessage(env, "unregister net stats error");
        }
    } else {
        callbackValue = NapiCommon::CreateErrorMessage(
            env, "unregister net stats error cause napi_status = " + std::to_string(status));
    }
    NapiCommon::Handle1ValueCallback(env, context, callbackValue);
}

napi_value NapiNetStats::UnregisterNetStatsCallback(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("UnregisterNetStatsCallback start");
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, MatchRegisterNetStatsCallbackParameters(env, argv, argc), "type mismatch");
    auto context = std::make_unique<BaseContext>().release();
    napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef);
    napi_value result = NapiCommon::HandleAsyncWork(env, context, "UnregisterNetStatsCallback",
        ExecUnregisterNetStatsCallback, CompleteUnregisterNetStatsCallback);
    return result;
}

napi_value NapiNetStats::RegisterNetStatsInterface(napi_env env, napi_value exports)
{
    DeclareNapiNetStatsInterface(env, exports);
    return nullptr;
}

static napi_module _netStatsModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiNetStats::RegisterNetStatsInterface,
    .nm_modname = "netmanager.netstats",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNetStatsModule(void)
{
    napi_module_register(&_netStatsModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
