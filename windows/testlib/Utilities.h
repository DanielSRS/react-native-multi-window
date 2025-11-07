// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <DispatcherQueue.h>
#include <winrt/Windows.System.h>
// React Native and Composition helpers used by EnsureThreadLocalCompositor
#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Windows.UI.Composition.h>

namespace Utilities
{
    inline auto CreateDispatcherQueueControllerForCurrentThread()
    {
        namespace abi = ABI::Windows::System;

        DispatcherQueueOptions options
        {
            sizeof(DispatcherQueueOptions),
            DQTYPE_THREAD_CURRENT,
            DQTAT_COM_NONE
        };

        winrt::Windows::System::DispatcherQueueController controller{ nullptr };
        winrt::check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(winrt::put_abi(controller))));
        return controller;
    }

    // React property bag helpers used to retrieve a stored Compositor from the RN instance settings.
    inline auto PBNM = winrt::Microsoft::ReactNative::ReactPropertyBagHelper::GetNamespace(L"ReactNative.InstanceSettings");
    inline auto CompositorProperty = winrt::Microsoft::ReactNative::ReactPropertyBagHelper::GetName(PBNM, L"Windows::UI::Composition::Compositor");

    // Ensure a thread-local compositor is available. Attempts to reuse a compositor stored on the
    // ReactNative instance settings first; otherwise creates a new Compositor and ensures a
    // DispatcherQueueController exists for the current thread.
    inline winrt::Windows::UI::Composition::Compositor EnsureThreadLocalCompositor(
        winrt::Microsoft::ReactNative::ReactContext const &context
    ) {
        thread_local winrt::Windows::UI::Composition::Compositor compositor{nullptr};
        if (compositor) {
            return compositor;
        }
        auto reactHost = winrt::Microsoft::ReactNative::ReactNativeHost::FromContext(context.Handle());
        auto instanceSettings = reactHost.InstanceSettings();
        auto properties = instanceSettings.Properties();
        compositor = winrt::unbox_value<winrt::Windows::UI::Composition::Compositor>(properties.Get(CompositorProperty));
        if (compositor) {
            return compositor;
        }
        using winrt::Windows::System::DispatcherQueue;
        if (DispatcherQueue::GetForCurrentThread() == nullptr) {
            thread_local winrt::Windows::System::DispatcherQueueController controller{nullptr};
            if (!controller) {
                controller = CreateDispatcherQueueControllerForCurrentThread();
            }
        }
        compositor = winrt::Windows::UI::Composition::Compositor();
        return compositor;
    }
}