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

#ifndef NET_SUPPLIER_H
#define NET_SUPPLIER_H

#include <string>
#include <set>
#include <vector>
#include <map>
#include "network.h"
#include "net_supplier_info.h"
#include "net_specifier.h"
#include "i_net_supplier_callback.h"
#include "i_net_conn_callback.h"

namespace OHOS {
namespace NetManagerStandard {
enum ServiceState {
    SERVICE_STATE_UNKNOWN = 0,
    SERVICE_STATE_IDLE = 1,
    SERVICE_STATE_CONNECTING = 2,
    SERVICE_STATE_READY = 3,
    SERVICE_STATE_CONNECTED = 4,
    SERVICE_STATE_DISCONNECTING = 5,
    SERVICE_STATE_DISCONNECTED = 6,
    SERVICE_STATE_FAILURE = 7,
};
enum CallbackType {
    CALL_TYPE_UNKNOWN = 0,
    CALL_TYPE_AVAILABLE = 1,
    CALL_TYPE_LOSTING = 2,
    CALL_TYPE_LOST = 3,
    CALL_TYPE_UPDATE_CAP = 4,
    CALL_TYPE_UPDATE_LINK = 5,
    CALL_TYPE_UNAVAILABLE = 6,
    CALL_TYPE_BLOCK_STATUS = 7,
};

class NetSupplier : public virtual RefBase {
public:
    NetSupplier(NetworkType netSupplierType, const std::string &netSupplierIdent, uint64_t netCapabilities);
    ~NetSupplier();
    bool operator==(const NetSupplier &netSupplier) const;
    void SetNetwork(const sptr<Network> &network);
    void UpdateNetSupplierInfo(const NetSupplierInfo &netSupplierInfo);
    void UpdateNetCapabilities(uint64_t netCapabilities);
    int32_t UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo);
    uint32_t GetSupplierId() const;
    NetworkType GetNetSupplierType() const;
    std::string GetNetSupplierIdent() const;
    uint64_t GetNetCapabilities() const;
    NetLinkInfo GetNetLinkInfo() const;
    bool GetRoaming() const;
    int8_t GetStrength() const;
    uint16_t GetFrequency() const;
    int32_t GetSupplierUid() const;
    sptr<Network> GetNetwork() const;
    int32_t GetNetId() const;
    sptr<NetHandler> GetNetHandler() const;
    void UpdateServiceState(ServiceState serviceState);
    ServiceState GetServiceState() const;
    bool IsConnecting() const;
    bool IsConnected() const;
    void SetNetValid(bool ifValid);
    bool IfNetValid();
    void SetNetScore(int32_t score);
    int32_t GetNetScore() const;
    void SetRealScore(int32_t score);
    int32_t GetRealScore();
    bool SupplierConnection(uint64_t netCapabilities);
    bool SupplierDisconnection(uint64_t netCapabilities);
    bool RequestToConnect(uint32_t reqId);
    void AddRequsetIdToList(uint32_t requestId);
    int32_t SelectAsBestNetwork(uint32_t reqId);
    void ReceiveBestScore(uint32_t reqId, int32_t bestScore);
    int32_t CancleRequest(uint32_t reqId);
    void RemoveBestRequest(uint32_t reqId);
    std::set<uint32_t>& GetBestRequestList();
    void SetDefault();
    void ClearDefault();
    void UpdateNetStateForTest(int32_t netState);
    void RegisterSupplierCallback(const sptr<INetSupplierCallback> &callback);

private:
    const int32_t REG_OK = 0;

    NetworkType netSupplierType_ = NET_TYPE_UNKNOWN;
    std::string netSupplierIdent_;
    uint64_t netCapabilities_ = NET_CAPABILITIES_NONE;
    NetLinkInfo netLinkInfo_;
    NetSupplierInfo netSupplierInfo_;
    uint32_t supplierId_ = 0;
    ServiceState state_ = SERVICE_STATE_IDLE;
    int32_t netScore_ = 0;
    int32_t netRealScore_ = 0;
    bool ifNetValid_ = false;
    std::set<uint32_t> requestList_;
    std::set<uint32_t> bestReqList_;
    sptr<INetSupplierCallback> netController_ = nullptr;
    sptr<Network> network_ = nullptr;
    sptr<NetHandler> netHandler_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_SUPPLIER_H
