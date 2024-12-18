/*
 * Copyright (C) 2024 The Android Open Source Project
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
#include <aidl/android/frameworks/automotive/power/ICarPowerServer.h>
#include <binder/ProcessState.h>

#include "PowerPolicyInterfaceTest.h"

namespace {

using ::aidl::android::frameworks::automotive::power::ICarPowerServer;
using ::android::ProcessState;

}  // namespace

class CarPowerServerAidlTest : public ::testing::TestWithParam<std::string> {
   public:
    virtual void SetUp() override { powerPolicyTest.SetUp(GetParam()); }

    PowerPolicyInterfaceTest<ICarPowerServer> powerPolicyTest;
};

TEST_P(CarPowerServerAidlTest, TestGetCurrentPowerPolicy) {
    powerPolicyTest.TestGetCurrentPowerPolicy();
}

TEST_P(CarPowerServerAidlTest, TestGetPowerComponentState) {
    powerPolicyTest.TestGetPowerComponentState();
}

TEST_P(CarPowerServerAidlTest, TestGetPowerComponentState_invalidComponent) {
    powerPolicyTest.TestGetPowerComponentState_invalidComponent();
}

TEST_P(CarPowerServerAidlTest, TestRegisterCallback) {
    powerPolicyTest.TestRegisterCallback();
}

TEST_P(CarPowerServerAidlTest, TestRegisterCallback_doubleRegistering) {
    powerPolicyTest.TestRegisterCallback_doubleRegistering();
}

TEST_P(CarPowerServerAidlTest, TestUnegisterNotRegisteredCallback) {
    powerPolicyTest.TestUnegisterNotRegisteredCallback();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(CarPowerServerAidlTest);
INSTANTIATE_TEST_SUITE_P(
    CarPowerServer, CarPowerServerAidlTest,
    ::testing::ValuesIn(android::getAidlHalInstanceNames(ICarPowerServer::descriptor)),
    android::PrintInstanceNameToString);

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ProcessState::self()->setThreadPoolMaxThreadCount(1);
    ProcessState::self()->startThreadPool();
    return RUN_ALL_TESTS();
}
