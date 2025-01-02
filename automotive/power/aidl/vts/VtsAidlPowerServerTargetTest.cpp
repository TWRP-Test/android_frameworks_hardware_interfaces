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
#include <aidl/android/frameworks/automotive/power/BnCarPowerStateChangeListener.h>
#include <aidl/android/frameworks/automotive/power/ICarPowerServer.h>
#include <android_car_feature.h>
#include <binder/ProcessState.h>

#include "PowerPolicyInterfaceTest.h"

namespace {

using ::aidl::android::frameworks::automotive::power::BnCarPowerStateChangeListener;
using ::aidl::android::frameworks::automotive::power::CarPowerState;
using ::aidl::android::frameworks::automotive::power::ICarPowerServer;
using ::android::ProcessState;
using ::android::car::feature::native_power_notifications;

class MockPowerStateChangeListener : public BnCarPowerStateChangeListener {
   public:
    MockPowerStateChangeListener() {}

    ndk::ScopedAStatus onStateChanged([[maybe_unused]] CarPowerState state) override {
        return ndk::ScopedAStatus::ok();
    }
};

}  // namespace

class CarPowerServerAidlTest : public ::testing::TestWithParam<std::string> {
   public:
    virtual void SetUp() override {
        if (!native_power_notifications()) {
            GTEST_SKIP() << "native_power_notification feature is not enabled";
        }
        powerPolicyTest.SetUp(GetParam());
    }

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

TEST_P(CarPowerServerAidlTest, TestRegisterPowerPolicyCallback) {
    powerPolicyTest.TestRegisterPowerPolicyCallback();
}

TEST_P(CarPowerServerAidlTest, TestRegisterPowerPolicyCallback_doubleRegistering) {
    powerPolicyTest.TestRegisterPowerPolicyCallback_doubleRegistering();
}

TEST_P(CarPowerServerAidlTest, TestUnegisterNotRegisteredPowerPolicyCallback) {
    powerPolicyTest.TestUnegisterNotRegisteredPowerPolicyCallback();
}

TEST_P(CarPowerServerAidlTest, TestRegisterPowerStateListener) {
    std::shared_ptr<MockPowerStateChangeListener> listener =
        ndk::SharedRefBase::make<MockPowerStateChangeListener>();
    std::shared_ptr<ICarPowerServer> powerServer = powerPolicyTest.powerPolicyServer;

    ndk::ScopedAStatus status = powerServer->registerPowerStateListener(listener);

    ASSERT_TRUE(status.isOk());

    status = powerServer->unregisterPowerStateListener(listener);

    ASSERT_TRUE(status.isOk());
}

TEST_P(CarPowerServerAidlTest, TestRegisterPowerStateListener_doubleRegistering) {
    std::shared_ptr<MockPowerStateChangeListener> listener =
        ndk::SharedRefBase::make<MockPowerStateChangeListener>();
    std::shared_ptr<ICarPowerServer> powerServer = powerPolicyTest.powerPolicyServer;

    ndk::ScopedAStatus status = powerServer->registerPowerStateListener(listener);

    ASSERT_TRUE(status.isOk());

    status = powerServer->registerPowerStateListener(listener);

    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(status.getServiceSpecificError(), EX_ILLEGAL_ARGUMENT);
}

TEST_P(CarPowerServerAidlTest, TestUnegisterNotRegisteredPowerStateListener) {
    std::shared_ptr<MockPowerStateChangeListener> listener =
        ndk::SharedRefBase::make<MockPowerStateChangeListener>();
    std::shared_ptr<ICarPowerServer> powerServer = powerPolicyTest.powerPolicyServer;

    ndk::ScopedAStatus status = powerServer->unregisterPowerStateListener(listener);

    ASSERT_FALSE(status.isOk());
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
