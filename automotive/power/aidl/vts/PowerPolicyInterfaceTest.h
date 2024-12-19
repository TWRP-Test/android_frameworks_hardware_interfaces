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
#include <aidl/android/frameworks/automotive/powerpolicy/BnCarPowerPolicyChangeCallback.h>
#include <aidl/android/frameworks/automotive/powerpolicy/CarPowerPolicy.h>
#include <aidl/android/frameworks/automotive/powerpolicy/CarPowerPolicyFilter.h>
#include <aidl/android/frameworks/automotive/powerpolicy/PowerComponent.h>
#include <android-base/stringprintf.h>
#include <android/binder_auto_utils.h>
#include <android/binder_manager.h>
#include <android/binder_status.h>

namespace aafap = aidl::android::frameworks::automotive::powerpolicy;

class MockPowerPolicyChangeCallback : public aafap::BnCarPowerPolicyChangeCallback {
   public:
    MockPowerPolicyChangeCallback() {}

    ndk::ScopedAStatus onPolicyChanged(
        [[maybe_unused]] const aafap::CarPowerPolicy& policy) override {
        return ndk::ScopedAStatus::ok();
    }
};

template <typename T>
class PowerPolicyInterfaceTest {
   public:
    void SetUp(const std::string& serviceName) {
        ndk::SpAIBinder binder(AServiceManager_getService(serviceName.c_str()));
        ASSERT_NE(binder.get(), nullptr);
        powerPolicyServer = T::fromBinder(binder);
    }

    void TestGetCurrentPowerPolicy() {
        aafap::CarPowerPolicy policy;

        ndk::ScopedAStatus status = powerPolicyServer->getCurrentPowerPolicy(&policy);

        ASSERT_TRUE(status.isOk() || status.getServiceSpecificError() == EX_ILLEGAL_STATE);
    }

    void TestGetPowerComponentState() {
        bool state;
        for (const auto componentId : ndk::enum_range<aafap::PowerComponent>()) {
            if (componentId >= aafap::PowerComponent::MINIMUM_CUSTOM_COMPONENT_VALUE) {
                continue;
            }
            ndk::ScopedAStatus status =
                powerPolicyServer->getPowerComponentState(componentId, &state);
            std::string errMsg =
                android::base::StringPrintf("Getting state of component(%d) fails", componentId);
            ASSERT_TRUE(status.isOk()) << errMsg;
        }
    }

    void TestGetPowerComponentState_invalidComponent() {
        bool state;
        aafap::PowerComponent invalidComponent = static_cast<aafap::PowerComponent>(-1);

        ndk::ScopedAStatus status =
            powerPolicyServer->getPowerComponentState(invalidComponent, &state);

        ASSERT_FALSE(status.isOk());
    }

    void TestRegisterCallback() {
        std::shared_ptr<MockPowerPolicyChangeCallback> callback =
            ndk::SharedRefBase::make<MockPowerPolicyChangeCallback>();
        aafap::CarPowerPolicyFilter filter;
        filter.components.push_back(aafap::PowerComponent::AUDIO);

        ndk::ScopedAStatus status =
            powerPolicyServer->registerPowerPolicyChangeCallback(callback, filter);

        ASSERT_TRUE(status.isOk());

        status = powerPolicyServer->unregisterPowerPolicyChangeCallback(callback);

        ASSERT_TRUE(status.isOk());
    }

    void TestRegisterCallback_doubleRegistering() {
        std::shared_ptr<MockPowerPolicyChangeCallback> callback =
            ndk::SharedRefBase::make<MockPowerPolicyChangeCallback>();
        aafap::CarPowerPolicyFilter filter;
        filter.components.push_back(aafap::PowerComponent::AUDIO);

        ndk::ScopedAStatus status =
            powerPolicyServer->registerPowerPolicyChangeCallback(callback, filter);

        ASSERT_TRUE(status.isOk());

        status = powerPolicyServer->registerPowerPolicyChangeCallback(callback, filter);

        ASSERT_FALSE(status.isOk());
        ASSERT_EQ(status.getServiceSpecificError(), EX_ILLEGAL_ARGUMENT);
    }

    void TestUnegisterNotRegisteredCallback() {
        std::shared_ptr<MockPowerPolicyChangeCallback> callback =
            ndk::SharedRefBase::make<MockPowerPolicyChangeCallback>();

        ndk::ScopedAStatus status =
            powerPolicyServer->unregisterPowerPolicyChangeCallback(callback);

        ASSERT_FALSE(status.isOk());
    }

    std::shared_ptr<T> powerPolicyServer;
};
