// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>

struct MicaWindow {
  struct MicaWindowData {
    winrt::Windows::UI::Composition::Visual rootVisual{ nullptr };
    winrt::Windows::UI::Composition::CompositionTarget compositionTarget{ nullptr };
    winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController controller{ nullptr };
    bool isSupported{ false };
  };
  /*struct MicaWindowData {
    Visual rootVisual{ nullptr };
    CompositionTarget compositionTarget{ nullptr };
    MicaController controller{ nullptr };
    bool isSupported{ false };
  };*/

  static MicaWindowData applyMica(const winrt::Windows::UI::Composition::Compositor& compositor, HWND window) noexcept;

};
