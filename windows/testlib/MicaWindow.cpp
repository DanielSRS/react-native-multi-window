// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"

#include "MicaWindow.h"

namespace winrt
{
    using namespace Microsoft::UI::Composition::SystemBackdrops;
    using namespace Windows::UI::Composition;
}

// static
const std::wstring MicaWindow::ClassName = L"MicaWindow";

// static
void MicaWindow::RegisterWindowClass()
{
    static bool registered = false;
    if (registered)
    {
        return;
    }

    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIconW(instance, IDI_APPLICATION);
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = ClassName.c_str();
    wcex.hIconSm = LoadIconW(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            winrt::throw_last_error();
        }
    }

    registered = true;
}

// Create the main window and enable MICA
MicaWindow::MicaWindow(const winrt::Compositor& compositor, const std::wstring& windowTitle)
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    if (m_window)
    {
        winrt::throw_hresult(E_UNEXPECTED);
    }

    HWND window = CreateWindowExW(
        WS_EX_COMPOSITED,
        ClassName.c_str(),
        windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        nullptr,
        nullptr,
        instance,
        this);
    if (!window)
    {
        winrt::throw_last_error();
    }

    // WM_NCCREATE associates the HWND with the instance, but fall back to the returned handle if needed.
    if (!m_window)
    {
        m_window = window;
    }

    ShowWindow(m_window, SW_SHOWDEFAULT);
    UpdateWindow(m_window);

    m_target = CreateWindowTarget(compositor);
    if (!m_target)
    {
        winrt::throw_hresult(E_FAIL);
    }

    auto root = compositor.CreateContainerVisual();
    if (!root)
    {
        winrt::throw_hresult(E_FAIL);
    }
    m_target.Root(root);

    m_micaController = winrt::MicaController();
    m_isMicaSupported = m_micaController.SetTarget(winrt::Microsoft::UI::WindowId{ reinterpret_cast<uint64_t>(m_window) }, m_target);
}

// Don't forget to free memory on WM_DESTROY!
LRESULT MicaWindow::MessageHandler(const UINT message, const WPARAM wparam, const LPARAM lparam) noexcept
{
    if (WM_DESTROY == message)
    {
        m_micaController = nullptr;
        winrt::testlib::detail::mica::Untrack(this);
    }

    return base_type::MessageHandler(message, wparam, lparam);
}