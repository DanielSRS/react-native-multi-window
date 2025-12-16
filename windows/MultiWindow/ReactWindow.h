#pragma once

#include <optional>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>

// ReactWindow
namespace winrt::MultiWindow {

  using AppWindow = winrt::Microsoft::UI::Windowing::AppWindow;
  using EventToken = winrt::event_token;
  using Visual = winrt::Windows::UI::Composition::Visual;
  using CompositionTarget = winrt::Windows::UI::Composition::CompositionTarget;
  using MicaController = winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController;
  using CompositionHwndHost = winrt::Microsoft::ReactNative::CompositionHwndHost;
  using IReactViewHost = winrt::Microsoft::ReactNative::IReactViewHost;

  enum class WindowType {
    DEFAULT = 0,
    //ACRYLIC = 1,
    MICA = 2,
  };

  /*struct DefaultWindowData {
    
  };*/

  struct MicaWindowData {
    Visual rootVisual{ nullptr };
    CompositionTarget compositionTarget{ nullptr };
    MicaController controller{ nullptr };
    bool isSupported{ false };
  };

  struct ReactWindow {
    WindowType type{ WindowType::DEFAULT };
    AppWindow window{ nullptr };
    EventToken changedToken{};
    EventToken destroyingToken{};
    CompositionHwndHost compositionHost{ nullptr };
    IReactViewHost viewHost{ nullptr };
    std::optional<MicaWindowData> micaWindowData{};
  };

  inline WindowType ParseWindowType(double value) noexcept {
    const auto type = static_cast<int>(value);
    switch (type) {
    case 1:
    case 2:
      return WindowType::MICA;
    default:
      return WindowType::DEFAULT;
    }
  }

} // namespace winrt::MultiWindow
