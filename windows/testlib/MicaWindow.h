// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include "DesktopWindow.h"

#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>

struct MicaWindow : DesktopWindow<MicaWindow>
{
    static const std::wstring ClassName;
    static void RegisterWindowClass();

    MicaWindow(const winrt::Windows::UI::Composition::Compositor& compositor, const std::wstring& windowTitle);

    LRESULT MessageHandler(const UINT message, const WPARAM wparam, const LPARAM lparam) noexcept;

    winrt::Windows::UI::Composition::Visual Root() { return m_target.Root(); }
    void Root(const winrt::Windows::UI::Composition::Visual& visual) { m_target.Root(visual); }

    struct ApplyMicaResult
    {
        winrt::Windows::UI::Composition::CompositionTarget Target{ nullptr };
        winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController Controller{ nullptr };
        bool IsSupported{ false };
    };

    static ApplyMicaResult applyMica(const winrt::Windows::UI::Composition::Compositor& compositor, HWND window) noexcept;

private:
    winrt::Windows::UI::Composition::CompositionTarget m_target{ nullptr };
    winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController m_micaController{ nullptr };
    bool m_isMicaSupported{ false };
};

namespace winrt::testlib::detail::mica
{
    void Untrack(MicaWindow *window) noexcept;
}