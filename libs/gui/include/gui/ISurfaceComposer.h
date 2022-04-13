/*
 * Copyright (C) 2006 The Android Open Source Project
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

#pragma once

#include <android/gui/DisplayBrightness.h>
#include <android/gui/FrameTimelineInfo.h>
#include <android/gui/IDisplayEventConnection.h>
#include <android/gui/IFpsListener.h>
#include <android/gui/IHdrLayerInfoListener.h>
#include <android/gui/IRegionSamplingListener.h>
#include <android/gui/IScreenCaptureListener.h>
#include <android/gui/ITransactionTraceListener.h>
#include <android/gui/ITunnelModeEnabledListener.h>
#include <android/gui/IWindowInfosListener.h>
#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <ftl/flags.h>
#include <gui/ITransactionCompletedListener.h>
#include <gui/SpHash.h>
#include <math/vec4.h>
#include <stdint.h>
#include <sys/types.h>
#include <ui/ConfigStoreTypes.h>
#include <ui/DisplayId.h>
#include <ui/DisplayMode.h>
#include <ui/DisplayedFrameStats.h>
#include <ui/FrameStats.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicTypes.h>
#include <ui/PixelFormat.h>
#include <ui/Rotation.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Timers.h>
#include <utils/Vector.h>

#include <optional>
#include <unordered_set>
#include <vector>

#include <aidl/android/hardware/graphics/common/DisplayDecorationSupport.h>

namespace android {

struct client_cache_t;
struct ComposerState;
struct DisplayStatInfo;
struct DisplayState;
struct InputWindowCommands;
class HdrCapabilities;
class Rect;

using gui::FrameTimelineInfo;
using gui::IDisplayEventConnection;
using gui::IRegionSamplingListener;
using gui::IScreenCaptureListener;
using gui::SpHash;

namespace gui {

struct DisplayCaptureArgs;
struct LayerCaptureArgs;
class LayerDebugInfo;

} // namespace gui

namespace ui {

struct DisplayMode;
struct DisplayState;
struct DynamicDisplayInfo;

} // namespace ui

/*
 * This class defines the Binder IPC interface for accessing various
 * SurfaceFlinger features.
 */
class ISurfaceComposer: public IInterface {
public:
    DECLARE_META_INTERFACE(SurfaceComposer)

    static constexpr size_t MAX_LAYERS = 4096;

    // flags for setTransactionState()
    enum {
        eSynchronous = 0x01,
        eAnimation = 0x02,

        // Explicit indication that this transaction and others to follow will likely result in a
        // lot of layers being composed, and thus, SurfaceFlinger should wake-up earlier to avoid
        // missing frame deadlines. In this case SurfaceFlinger will wake up at
        // (sf vsync offset - debug.sf.early_phase_offset_ns). SurfaceFlinger will continue to be
        // in the early configuration until it receives eEarlyWakeupEnd. These flags are
        // expected to be used by WindowManager only and are guarded by
        // android.permission.ACCESS_SURFACE_FLINGER
        eEarlyWakeupStart = 0x08,
        eEarlyWakeupEnd = 0x10,
        eOneWay = 0x20
    };

    enum VsyncSource {
        eVsyncSourceApp = 0,
        eVsyncSourceSurfaceFlinger = 1
    };

    enum class EventRegistration {
        modeChanged = 1 << 0,
        frameRateOverride = 1 << 1,
    };

    using EventRegistrationFlags = ftl::Flags<EventRegistration>;

    /* return an IDisplayEventConnection */
    virtual sp<IDisplayEventConnection> createDisplayEventConnection(
            VsyncSource vsyncSource = eVsyncSourceApp,
            EventRegistrationFlags eventRegistration = {}) = 0;

    /* open/close transactions. requires ACCESS_SURFACE_FLINGER permission */
    virtual status_t setTransactionState(
            const FrameTimelineInfo& frameTimelineInfo, const Vector<ComposerState>& state,
            const Vector<DisplayState>& displays, uint32_t flags, const sp<IBinder>& applyToken,
            const InputWindowCommands& inputWindowCommands, int64_t desiredPresentTime,
            bool isAutoTimestamp, const client_cache_t& uncacheBuffer, bool hasListenerCallbacks,
            const std::vector<ListenerCallbacks>& listenerCallbacks, uint64_t transactionId) = 0;

    /* signal that we're done booting.
     * Requires ACCESS_SURFACE_FLINGER permission
     */
    virtual void bootFinished() = 0;
};

// ----------------------------------------------------------------------------

class BnSurfaceComposer: public BnInterface<ISurfaceComposer> {
public:
    enum ISurfaceComposerTag {
        // Note: BOOT_FINISHED must remain this value, it is called from
        // Java by ActivityManagerService.
        BOOT_FINISHED = IBinder::FIRST_CALL_TRANSACTION,
        CREATE_CONNECTION,       // Deprecated. Autogenerated by .aidl now.
        GET_STATIC_DISPLAY_INFO, // Deprecated. Autogenerated by .aidl now.
        CREATE_DISPLAY_EVENT_CONNECTION,
        CREATE_DISPLAY,             // Deprecated. Autogenerated by .aidl now.
        DESTROY_DISPLAY,            // Deprecated. Autogenerated by .aidl now.
        GET_PHYSICAL_DISPLAY_TOKEN, // Deprecated. Autogenerated by .aidl now.
        SET_TRANSACTION_STATE,
        AUTHENTICATE_SURFACE,           // Deprecated. Autogenerated by .aidl now.
        GET_SUPPORTED_FRAME_TIMESTAMPS, // Deprecated. Autogenerated by .aidl now.
        GET_DISPLAY_MODES,              // Deprecated. Use GET_DYNAMIC_DISPLAY_INFO instead.
        GET_ACTIVE_DISPLAY_MODE,        // Deprecated. Use GET_DYNAMIC_DISPLAY_INFO instead.
        GET_DISPLAY_STATE,
        CAPTURE_DISPLAY,             // Deprecated. Autogenerated by .aidl now.
        CAPTURE_LAYERS,              // Deprecated. Autogenerated by .aidl now.
        CLEAR_ANIMATION_FRAME_STATS, // Deprecated. Autogenerated by .aidl now.
        GET_ANIMATION_FRAME_STATS,   // Deprecated. Autogenerated by .aidl now.
        SET_POWER_MODE,              // Deprecated. Autogenerated by .aidl now.
        GET_DISPLAY_STATS,
        GET_HDR_CAPABILITIES,       // Deprecated. Use GET_DYNAMIC_DISPLAY_INFO instead.
        GET_DISPLAY_COLOR_MODES,    // Deprecated. Use GET_DYNAMIC_DISPLAY_INFO instead.
        GET_ACTIVE_COLOR_MODE,      // Deprecated. Use GET_DYNAMIC_DISPLAY_INFO instead.
        SET_ACTIVE_COLOR_MODE,      // Deprecated. Autogenerated by .aidl now.
        ENABLE_VSYNC_INJECTIONS,    // Deprecated. Autogenerated by .aidl now.
        INJECT_VSYNC,               // Deprecated. Autogenerated by .aidl now.
        GET_LAYER_DEBUG_INFO,       // Deprecated. Autogenerated by .aidl now.
        GET_COMPOSITION_PREFERENCE, // Deprecated. Autogenerated by .aidl now.
        GET_COLOR_MANAGEMENT,       // Deprecated. Autogenerated by .aidl now.
        GET_DISPLAYED_CONTENT_SAMPLING_ATTRIBUTES, // Deprecated. Autogenerated by .aidl now.
        SET_DISPLAY_CONTENT_SAMPLING_ENABLED,      // Deprecated. Autogenerated by .aidl now.
        GET_DISPLAYED_CONTENT_SAMPLE,
        GET_PROTECTED_CONTENT_SUPPORT,   // Deprecated. Autogenerated by .aidl now.
        IS_WIDE_COLOR_DISPLAY,           // Deprecated. Autogenerated by .aidl now.
        GET_DISPLAY_NATIVE_PRIMARIES,    // Deprecated. Autogenerated by .aidl now.
        GET_PHYSICAL_DISPLAY_IDS,        // Deprecated. Autogenerated by .aidl now.
        ADD_REGION_SAMPLING_LISTENER,    // Deprecated. Autogenerated by .aidl now.
        REMOVE_REGION_SAMPLING_LISTENER, // Deprecated. Autogenerated by .aidl now.
        SET_DESIRED_DISPLAY_MODE_SPECS,  // Deprecated. Autogenerated by .aidl now.
        GET_DESIRED_DISPLAY_MODE_SPECS,  // Deprecated. Autogenerated by .aidl now.
        GET_DISPLAY_BRIGHTNESS_SUPPORT,  // Deprecated. Autogenerated by .aidl now.
        SET_DISPLAY_BRIGHTNESS,          // Deprecated. Autogenerated by .aidl now.
        CAPTURE_DISPLAY_BY_ID,           // Deprecated. Autogenerated by .aidl now.
        NOTIFY_POWER_BOOST,              // Deprecated. Autogenerated by .aidl now.
        SET_GLOBAL_SHADOW_SETTINGS,
        GET_AUTO_LOW_LATENCY_MODE_SUPPORT, // Deprecated. Use GET_DYNAMIC_DISPLAY_INFO instead.
        SET_AUTO_LOW_LATENCY_MODE,         // Deprecated. Autogenerated by .aidl now.
        GET_GAME_CONTENT_TYPE_SUPPORT,     // Deprecated. Use GET_DYNAMIC_DISPLAY_INFO instead.
        SET_GAME_CONTENT_TYPE,             // Deprecated. Use GET_DYNAMIC_DISPLAY_INFO instead.
        SET_FRAME_RATE,                    // Deprecated. Autogenerated by .aidl now.
        // Deprecated. Use DisplayManager.setShouldAlwaysRespectAppRequestedMode(true);
        ACQUIRE_FRAME_RATE_FLEXIBILITY_TOKEN,
        SET_FRAME_TIMELINE_INFO,        // Deprecated. Autogenerated by .aidl now.
        ADD_TRANSACTION_TRACE_LISTENER, // Deprecated. Autogenerated by .aidl now.
        GET_GPU_CONTEXT_PRIORITY,
        GET_MAX_ACQUIRED_BUFFER_COUNT,
        GET_DYNAMIC_DISPLAY_INFO,            // Deprecated. Autogenerated by .aidl now.
        ADD_FPS_LISTENER,                    // Deprecated. Autogenerated by .aidl now.
        REMOVE_FPS_LISTENER,                 // Deprecated. Autogenerated by .aidl now.
        OVERRIDE_HDR_TYPES,                  // Deprecated. Autogenerated by .aidl now.
        ADD_HDR_LAYER_INFO_LISTENER,         // Deprecated. Autogenerated by .aidl now.
        REMOVE_HDR_LAYER_INFO_LISTENER,      // Deprecated. Autogenerated by .aidl now.
        ON_PULL_ATOM,                        // Deprecated. Autogenerated by .aidl now.
        ADD_TUNNEL_MODE_ENABLED_LISTENER,    // Deprecated. Autogenerated by .aidl now.
        REMOVE_TUNNEL_MODE_ENABLED_LISTENER, // Deprecated. Autogenerated by .aidl now.
        ADD_WINDOW_INFOS_LISTENER,           // Deprecated. Autogenerated by .aidl now.
        REMOVE_WINDOW_INFOS_LISTENER,        // Deprecated. Autogenerated by .aidl now.
        GET_PRIMARY_PHYSICAL_DISPLAY_ID,     // Deprecated. Autogenerated by .aidl now.
        GET_DISPLAY_DECORATION_SUPPORT,
        GET_BOOT_DISPLAY_MODE_SUPPORT, // Deprecated. Autogenerated by .aidl now.
        SET_BOOT_DISPLAY_MODE,         // Deprecated. Autogenerated by .aidl now.
        CLEAR_BOOT_DISPLAY_MODE,       // Deprecated. Autogenerated by .aidl now.
        SET_OVERRIDE_FRAME_RATE,       // Deprecated. Autogenerated by .aidl now.
        // Always append new enum to the end.
    };

    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

} // namespace android
