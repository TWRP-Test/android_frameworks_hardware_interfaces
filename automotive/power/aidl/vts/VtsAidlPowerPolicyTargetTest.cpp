/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <aidl/Vintf.h>
#include <aidl/android/frameworks/automotive/powerpolicy/ICarPowerPolicyServer.h>
#include <binder/ProcessState.h>

#include "PowerPolicyInterfaceTest.h"

namespace {

using ::aidl::android::frameworks::automotive::powerpolicy::ICarPowerPolicyServer;
using ::android::ProcessState;

}  // namespace

class PowerPolicyAidlTest : public ::testing::TestWithParam<std::string> {
   public:
    virtual void SetUp() override { powerPolicyTest.SetUp(GetParam()); }

    PowerPolicyInterfaceTest<ICarPowerPolicyServer> powerPolicyTest;
};

TEST_P(PowerPolicyAidlTest, TestGetCurrentPowerPolicy) {
    powerPolicyTest.TestGetCurrentPowerPolicy();
}

TEST_P(PowerPolicyAidlTest, TestGetPowerComponentState) {
    powerPolicyTest.TestGetPowerComponentState();
}

TEST_P(PowerPolicyAidlTest, TestGetPowerComponentState_invalidComponent) {
    powerPolicyTest.TestGetPowerComponentState_invalidComponent();
}

TEST_P(PowerPolicyAidlTest, TestRegisterCallback) {
    powerPolicyTest.TestRegisterCallback();
}

TEST_P(PowerPolicyAidlTest, TestRegisterCallback_doubleRegistering) {
    powerPolicyTest.TestRegisterCallback_doubleRegistering();
}

TEST_P(PowerPolicyAidlTest, TestUnegisterNotRegisteredCallback) {
    powerPolicyTest.TestUnegisterNotRegisteredCallback();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(PowerPolicyAidlTest);
INSTANTIATE_TEST_SUITE_P(
    CarPowerPolicyServer, PowerPolicyAidlTest,
    ::testing::ValuesIn(android::getAidlHalInstanceNames(ICarPowerPolicyServer::descriptor)),
    android::PrintInstanceNameToString);

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ProcessState::self()->setThreadPoolMaxThreadCount(1);
    ProcessState::self()->startThreadPool();
    return RUN_ALL_TESTS();
}
