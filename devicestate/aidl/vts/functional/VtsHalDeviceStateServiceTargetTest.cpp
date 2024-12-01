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

#include "aidl/android/frameworks/devicestate/DeviceStateConfiguration.h"
#define LOG_TAG "VtsHalDeviceStateServiceTest"

#include <unordered_set>

#include "aidl/android/frameworks/devicestate/ErrorCode.h"

#include <aidl/Gtest.h>
#include <aidl/Vintf.h>
#include <aidl/android/frameworks/devicestate/BnDeviceStateListener.h>
#include <aidl/android/frameworks/devicestate/BnDeviceStateService.h>
#include <android-base/logging.h>
#include <android/binder_auto_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <utils/Condition.h>
#include <utils/Log.h>
#include <utils/Mutex.h>

#include <memory>

#include "aidl/android/frameworks/devicestate/IDeviceStateListener.h"
#include "aidl/android/frameworks/devicestate/IDeviceStateService.h"

#define NOTIFY_TIMEOUT_NS 2000000000

namespace android {

using ::aidl::android::frameworks::devicestate::BnDeviceStateListener;
using ::aidl::android::frameworks::devicestate::DeviceStateConfiguration;
using ::aidl::android::frameworks::devicestate::ErrorCode;
using ::aidl::android::frameworks::devicestate::IDeviceStateService;
using ::android::getAidlHalInstanceNames;
using ::android::PrintInstanceNameToString;
using ::ndk::enum_range;
using ::ndk::SpAIBinder;
using ::testing::InitGoogleTest;
using ::testing::TestWithParam;

class DeviceStateServiceTest : public ::testing::TestWithParam<std::string> {
   public:
    void SetUp() override {
        bool ret = ABinderProcess_setThreadPoolMaxThreadCount(/* numThreads= */ 5);
        ASSERT_TRUE(ret);
        ABinderProcess_startThreadPool();
        SpAIBinder binder(AServiceManager_waitForService(GetParam().c_str()));
        service = IDeviceStateService::fromBinder(binder);
        ASSERT_NE(service, nullptr);
    }

    std::shared_ptr<IDeviceStateService> service;
};

class DeviceStateListener : public BnDeviceStateListener {
   public:
    DeviceStateListener() : mInitialNotification(false) {
        mPublicPropertyMask = 0;
        for (const auto& it : enum_range<DeviceStateConfiguration::DeviceStatePropertyValue>()) {
            mPublicPropertyMask |= static_cast<unsigned long>(it);
        }
        mPublicPropertyMask = ~mPublicPropertyMask;
    }

    ::ndk::ScopedAStatus onDeviceStateChanged(
        const ::aidl::android::frameworks::devicestate::DeviceStateConfiguration& in_deviceState)
        override {
        Mutex::Autolock l(mLock);
        EXPECT_TRUE((in_deviceState.deviceProperties & mPublicPropertyMask) == 0);
        mInitialNotification = true;
        mNotifyCondition.broadcast();
        return ::ndk::ScopedAStatus::ok();
    }

    bool waitForDeviceStateChange() {
        Mutex::Autolock l(mLock);
        if (!mInitialNotification &&
            (mNotifyCondition.waitRelative(mLock, NOTIFY_TIMEOUT_NS) != android::OK)) {
            return false;
        }

        return mInitialNotification;
    }

   private:
    unsigned long mPublicPropertyMask;

    mutable Mutex mLock;
    mutable Condition mNotifyCondition;
    bool mInitialNotification;
};

TEST_P(DeviceStateServiceTest, RegisterAndUnregisterDeviceStateTest) {
    auto listener = ::ndk::SharedRefBase::make<DeviceStateListener>();
    EXPECT_TRUE(service->registerListener(listener).isOk());
    EXPECT_TRUE(listener->waitForDeviceStateChange());

    auto secondListener = ::ndk::SharedRefBase::make<DeviceStateListener>();
    auto ret = service->registerListener(secondListener);
    EXPECT_TRUE(!ret.isOk());
    EXPECT_TRUE(static_cast<ErrorCode>(ret.getServiceSpecificError()) == ErrorCode::ALREADY_EXISTS);

    ret = service->unregisterListener(listener);
    EXPECT_TRUE(ret.isOk());

    ret = service->unregisterListener(listener);
    EXPECT_TRUE(!ret.isOk());
    EXPECT_TRUE(static_cast<ErrorCode>(ret.getServiceSpecificError()) == ErrorCode::BAD_INPUT);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DeviceStateServiceTest);

INSTANTIATE_TEST_SUITE_P(
    PerInstance, DeviceStateServiceTest,
    testing::ValuesIn(getAidlHalInstanceNames(IDeviceStateService::descriptor)),
    PrintInstanceNameToString);

int main(int argc, char** argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
}  // namespace android
