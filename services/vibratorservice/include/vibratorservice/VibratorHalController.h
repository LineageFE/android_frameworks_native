/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef ANDROID_OS_VIBRATORHALCONTROLLER_H
#define ANDROID_OS_VIBRATORHALCONTROLLER_H

#include <android-base/thread_annotations.h>
#include <android/hardware/vibrator/IVibrator.h>

#include <vibratorservice/VibratorCallbackScheduler.h>
#include <vibratorservice/VibratorHalWrapper.h>

namespace android {

namespace vibrator {

// Handles the connection to he underlying HAL implementation available.
class HalConnector {
public:
    HalConnector() = default;
    virtual ~HalConnector() = default;

    virtual std::shared_ptr<HalWrapper> connect(std::shared_ptr<CallbackScheduler> scheduler);
};

// Controller for Vibrator HAL handle.
// This relies on HalConnector to connect to the underlying Vibrator HAL service and reconnects to
// it after each failed api call. This also ensures connecting to the service is thread-safe.
class HalController : public HalWrapper {
public:
    HalController()
          : HalController(std::make_unique<HalConnector>(), std::make_shared<CallbackScheduler>()) {
    }
    HalController(std::unique_ptr<HalConnector> halConnector,
                  std::shared_ptr<CallbackScheduler> callbackScheduler)
          : HalWrapper(std::move(callbackScheduler)),
            mHalConnector(std::move(halConnector)),
            mConnectedHal(nullptr) {}
    virtual ~HalController() = default;

    HalResult<void> ping() final override;

    HalResult<void> on(std::chrono::milliseconds timeout,
                       const std::function<void()>& completionCallback) final override;
    HalResult<void> off() final override;

    HalResult<void> setAmplitude(int32_t amplitude) final override;
    HalResult<void> setExternalControl(bool enabled) final override;

    HalResult<void> alwaysOnEnable(int32_t id, hardware::vibrator::Effect effect,
                                   hardware::vibrator::EffectStrength strength) final override;
    HalResult<void> alwaysOnDisable(int32_t id) final override;

    HalResult<Capabilities> getCapabilities() final override;
    HalResult<std::vector<hardware::vibrator::Effect>> getSupportedEffects() final override;

    HalResult<std::chrono::milliseconds> performEffect(
            hardware::vibrator::Effect effect, hardware::vibrator::EffectStrength strength,
            const std::function<void()>& completionCallback) final override;

    HalResult<void> performComposedEffect(
            const std::vector<hardware::vibrator::CompositeEffect>& primitiveEffects,
            const std::function<void()>& completionCallback) final override;

private:
    std::unique_ptr<HalConnector> mHalConnector;
    std::mutex mConnectedHalMutex;
    // Shared pointer to allow local copies to be used by different threads.
    std::shared_ptr<HalWrapper> mConnectedHal GUARDED_BY(mConnectedHalMutex);

    std::shared_ptr<HalWrapper> initHal();

    template <typename T>
    HalResult<T> processHalResult(HalResult<T> result, const char* functionName);

    template <typename T>
    using hal_fn = std::function<HalResult<T>(std::shared_ptr<HalWrapper>)>;

    template <typename T>
    HalResult<T> apply(hal_fn<T>& halFn, const char* functionName);
};

}; // namespace vibrator

}; // namespace android

#endif // ANDROID_OS_VIBRATORHALCONTROLLER_H