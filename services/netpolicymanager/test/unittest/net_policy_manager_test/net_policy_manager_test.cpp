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

#include "net_mgr_log_wrapper.h"
#include "net_policy_define.h"
#include "net_policy_file.h"
#include "net_policy_traffic.h"
#include "net_policy_constants.h"
#include "net_policy_client.h"
#include "net_policy_service.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetPolicyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetPolicyManagerTest::SetUpTestCase() {}

void NetPolicyManagerTest::TearDownTestCase() {}

void NetPolicyManagerTest::SetUp() {}

void NetPolicyManagerTest::TearDown() {}

/**
 * @tc.name: NetPolicyManager001
 * @tc.desc: Test NetPolicyManager SetUidPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager001, TestSize.Level1)
{
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->SetUidPolicy(1,
        NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
    ASSERT_TRUE(result != NetPolicyResultCode::ERR_INVALID_UID);
}

/**
 * @tc.name: NetPolicyManager002
 * @tc.desc: Test NetPolicyManager GetUidPolicy.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager002, TestSize.Level1)
{
    NetUidPolicy result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidPolicy(1);
    ASSERT_TRUE(result == NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND);
}

/**
 * @tc.name: NetPolicyManager003
 * @tc.desc: Test NetPolicyManager GetUids.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager003, TestSize.Level1)
{
    std::vector<uint32_t> result;
    NetUidPolicy policy = NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND;
    result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUids(policy);
    ASSERT_TRUE(result.size() != 0);
}

/**
 * @tc.name: NetPolicyManager004
 * @tc.desc: Test NetPolicyManager IsUidNetAccess.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager004, TestSize.Level1)
{
    bool result = DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(1, false);
    ASSERT_TRUE(result == true);
}

/**
 * @tc.name: NetPolicyManager005
 * @tc.desc: Test NetPolicyManager IsUidNetAccess.
 * @tc.type: FUNC
 */
HWTEST_F(NetPolicyManagerTest, NetPolicyManager005, TestSize.Level1)
{
    bool result = DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(1, std::string("test"));
    ASSERT_TRUE(result == true);
}
}
}
