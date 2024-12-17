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

#include <aidl/Gtest.h>
#include <aidl/Vintf.h>
#include <aidl/android/frameworks/automotive/power/ICarPowerServer.h>
#include <aidl/android/frameworks/automotive/powerpolicy/CarPowerPolicy.h>
#include <android/binder_manager.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

namespace {

using ::aidl::android::frameworks::automotive::power::ICarPowerServer;
using ::aidl::android::frameworks::automotive::powerpolicy::CarPowerPolicy;
using ::android::ProcessState;
using ::ndk::ScopedAStatus;
using ::ndk::SpAIBinder;

}  // namespace

class CarPowerServerAidlTest : public ::testing::TestWithParam<std::string> {
   public:
    virtual void SetUp() override {
        SpAIBinder binder(AServiceManager_getService(GetParam().c_str()));
        ASSERT_NE(binder.get(), nullptr);
        carPowerServer = ICarPowerServer::fromBinder(binder);
    }

    std::shared_ptr<ICarPowerServer> carPowerServer;
};

TEST_P(CarPowerServerAidlTest, TestGetCurrentPowerPolicy) {
    CarPowerPolicy policy;

    ScopedAStatus status = carPowerServer->getCurrentPowerPolicy(&policy);

    ASSERT_TRUE(status.isOk() || status.getServiceSpecificError() == EX_ILLEGAL_STATE);
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
