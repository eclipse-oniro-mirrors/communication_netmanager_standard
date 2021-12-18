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

#include <gtest/gtest.h>

#include "net_conn_client.h"
#include "net_conn_types.h"
#include "net_conn_callback_test.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int WAIT_TIME_SECOND_LONG = 60;
using namespace testing::ext;
class NetConnManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<NetLinkInfo> GetUpdateLinkInfoSample() const;

    sptr<NetConnCallbackTest> GetINetConnCallbackSample() const;
};

void NetConnManagerTest::SetUpTestCase() {}

void NetConnManagerTest::TearDownTestCase() {}

void NetConnManagerTest::SetUp() {}

void NetConnManagerTest::TearDown() {}

sptr<NetLinkInfo> NetConnManagerTest::GetUpdateLinkInfoSample() const
{
    sptr<NetLinkInfo> netLinkInfo = (std::make_unique<NetLinkInfo>()).release();
    netLinkInfo->ifaceName_ = "test";
    netLinkInfo->domain_ = "test";

    sptr<INetAddr> netAddr = (std::make_unique<INetAddr>()).release();
    netAddr->type_ = INetAddr::IPV4;
    netAddr->family_ = 0x10;
    netAddr->prefixlen_ = 0x17;
    netAddr->address_ = "192.168.2.0";
    netAddr->netMask_ = "192.255.255.255";
    netAddr->hostName_ = "netAddr";
    netLinkInfo->netAddrList_.push_back(*netAddr);

    sptr<INetAddr> dns = (std::make_unique<INetAddr>()).release();
    dns->type_ = INetAddr::IPV4;
    dns->family_ = 0x10;
    dns->prefixlen_ = 0x17;
    dns->address_ = "192.168.2.0";
    dns->netMask_ = "192.255.255.255";
    dns->hostName_ = "netAddr";
    netLinkInfo->dnsList_.push_back(*dns);

    sptr<Route> route = (std::make_unique<Route>()).release();
    route->iface_ = "iface0";
    route->destination_.type_ = INetAddr::IPV4;
    route->destination_.family_ = 0x10;
    route->destination_.prefixlen_ = 0x17;
    route->destination_.address_ = "192.168.2.0";
    route->destination_.netMask_ = "192.255.255.255";
    route->destination_.hostName_ = "netAddr";
    route->gateway_.type_ = INetAddr::IPV4;
    route->gateway_.family_ = 0x10;
    route->gateway_.prefixlen_ = 0x17;
    route->gateway_.address_ = "192.168.2.0";
    route->gateway_.netMask_ = "192.255.255.255";
    route->gateway_.hostName_ = "netAddr";
    netLinkInfo->routeList_.push_back(*route);

    netLinkInfo->mtu_ = 0x5DC;

    return netLinkInfo;
}

sptr<NetConnCallbackTest> NetConnManagerTest::GetINetConnCallbackSample() const
{
    sptr<NetConnCallbackTest> callback = (std::make_unique<NetConnCallbackTest>()).release();
    return callback;
}

/**
 * @tc.name: NetConnManager001
 * @tc.desc: Test NetConnManager SystemReady.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager001, TestSize.Level1)
{
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->SystemReady();
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: NetConnManager002
 * @tc.desc: Test NetConnManager RegisterNetSupplier.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager002, TestSize.Level1)
{
    uint64_t netCapabilities = 0x00;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident01";
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR, ident,
        netCapabilities);
    ASSERT_TRUE(result >= ERR_NONE);
}

/**
 * @tc.name: NetConnManager003
 * @tc.desc: Test NetConnManager UnregisterNetSupplier.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager003, TestSize.Level1)
{
    uint64_t netCapabilities = 0x01;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident02";
    int32_t resSupplierId = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities);
    ASSERT_TRUE(resSupplierId >= ERR_NONE);

    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetSupplier(resSupplierId);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager004
 * @tc.desc: Test NetConnManager UpdateNetSupplierInfo.
 * @tc.type: FUNC
 */

HWTEST_F(NetConnManagerTest, NetConnManager004, TestSize.Level1)
{
    uint64_t netCapabilities = 0x02;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident03";
    int32_t resSupplierId = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities);
    ASSERT_TRUE(resSupplierId >= ERR_NONE);

    sptr<NetSupplierInfo> netSupplierInfo = new NetSupplierInfo;
    netSupplierInfo->isAvailable_ = true;
    netSupplierInfo->isRoaming_ = true;
    netSupplierInfo->strength_ = 0x64;
    netSupplierInfo->frequency_ = 0x10;
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetSupplierInfo(resSupplierId,
        netSupplierInfo);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager005
 * @tc.desc: Test NetConnManager UpdateNetLinkInfo.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager005, TestSize.Level1)
{
    uint64_t netCapabilities = 0x03;
    netCapabilities |= NET_CAPABILITIES_INTERNET;
    netCapabilities |= NET_CAPABILITIES_MMS;

    std::string ident = "ident04";
    int32_t resSupplierId = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities);
    ASSERT_TRUE(resSupplierId >= ERR_NONE);

    sptr<NetLinkInfo> netLinkInfo = GetUpdateLinkInfoSample();
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(resSupplierId, netLinkInfo);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager006
 * @tc.desc: Test NetConnManager RegisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager006, TestSize.Level1)
{
    uint64_t netCapabilities = 0x02;
    netCapabilities |= NET_CAPABILITIES_INTERNET;

    std::string ident = "ident";
    int32_t resSupplierId = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities);
    ASSERT_TRUE(resSupplierId >= ERR_NONE);

    sptr<NetConnCallbackTest> callback = GetINetConnCallbackSample();
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    if (result == ERR_NONE) {
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        int32_t netState = callback->GetNetState();
        std::cout << "NetConnManager006 RegisterNetConnCallback netState:" << netState << std::endl;
        ASSERT_GT(netState, 0);     // >
    } else {
        std::cout << "NetConnManager006 RegisterNetConnCallback return fail" << std::endl;
    }

    result = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
    ASSERT_TRUE(result == ERR_NONE);
}

/**
 * @tc.name: NetConnManager007
 * @tc.desc: Test NetConnManager RegisterNetConnCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetConnManagerTest, NetConnManager007, TestSize.Level1)
{
    uint64_t netCapabilities = 0x02;
    netCapabilities |= NET_CAPABILITIES_INTERNET;

    std::string ident = "ident";
    int32_t resSupplierId = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(NET_TYPE_CELLULAR,
        ident, netCapabilities);
    ASSERT_TRUE(resSupplierId >= ERR_NONE);

    sptr<NetSpecifier> netSpecifier = (std::make_unique<NetSpecifier>()).release();
    netSpecifier->ident_ = "ident";
    netSpecifier->netType_ = NET_TYPE_CELLULAR;
    netSpecifier->netCapabilities_ = NET_CAPABILITIES_INTERNET;

    sptr<NetConnCallbackTest> callback = GetINetConnCallbackSample();
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(netSpecifier, callback);
    if (result == ERR_NONE) {
        callback->WaitFor(WAIT_TIME_SECOND_LONG);
        int32_t netState = callback->GetNetState();
        std::cout << "NetConnManager007 RegisterNetConnCallback netState:" << netState << std::endl;
        ASSERT_GT(netState, 0);     // >
    } else {
        std::cout << "NetConnManager007 RegisterNetConnCallback return fail" << std::endl;
    }

    result = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(netSpecifier, callback);
    ASSERT_TRUE(result == ERR_NONE);
}
} // namespace NetManagerStandard
} // namespace OHOS
