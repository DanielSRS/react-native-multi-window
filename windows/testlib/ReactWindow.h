#pragma once

#include <utility>
#include <variant>

#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.UI.Composition.h>

namespace winrt::testlib::detail
{

struct ReactWindow
{
  enum class WindowType
  {
    DEFAULT = 0,
    MICA = 2,
  };

  struct AppWindowData
  {
    winrt::Microsoft::UI::Windowing::AppWindow Window{nullptr};
    winrt::event_token ChangedToken{};
    winrt::event_token DestroyingToken{};
  };

  struct MicaAppWindowData
  {
    winrt::Microsoft::UI::Windowing::AppWindow Window{nullptr};
    winrt::event_token ChangedToken{};
    winrt::event_token DestroyingToken{};
    winrt::Windows::UI::Composition::Visual RootVisual{nullptr};
    winrt::Windows::UI::Composition::CompositionTarget CompositionTarget{nullptr};
    winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController Controller{nullptr};
    bool IsSupported{false};
  };

  using WindowVariant = std::variant<std::monostate, AppWindowData, MicaAppWindowData>;

  WindowType Kind{WindowType::DEFAULT};
  WindowVariant Window{std::monostate{}};
  winrt::Microsoft::ReactNative::CompositionHwndHost CompositionHost{nullptr};
  winrt::Microsoft::ReactNative::IReactViewHost ViewHost{nullptr};

  static ReactWindow CreateAppWindow(
      winrt::Microsoft::UI::Windowing::AppWindow const &appWindow,
      winrt::Microsoft::ReactNative::CompositionHwndHost const &compositionHost,
      winrt::Microsoft::ReactNative::IReactViewHost const &viewHost)
  {
    ReactWindow result;
  result.Kind = WindowType::DEFAULT;
    result.Window = AppWindowData{appWindow};
    result.CompositionHost = compositionHost;
    result.ViewHost = viewHost;
    return result;
  }

  static ReactWindow CreateMicaAppWindow(
      winrt::Microsoft::UI::Windowing::AppWindow const &appWindow,
      winrt::Windows::UI::Composition::Visual const &rootVisual,
      winrt::Windows::UI::Composition::CompositionTarget const &compositionTarget,
      winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController const &controller,
      bool isSupported,
      winrt::Microsoft::ReactNative::CompositionHwndHost const &compositionHost,
      winrt::Microsoft::ReactNative::IReactViewHost const &viewHost)
  {
    ReactWindow result;
  result.Kind = WindowType::MICA;
    result.Window = MicaAppWindowData{appWindow, {}, {}, rootVisual, compositionTarget, controller, isSupported};
    result.CompositionHost = compositionHost;
    result.ViewHost = viewHost;
    return result;
  }

  AppWindowData *App() noexcept
  {
    return Kind == WindowType::DEFAULT ? std::get_if<AppWindowData>(&Window) : nullptr;
  }

  AppWindowData const *App() const noexcept
  {
    return Kind == WindowType::DEFAULT ? std::get_if<AppWindowData>(&Window) : nullptr;
  }

  MicaAppWindowData *MicaApp() noexcept
  {
    return Kind == WindowType::MICA ? std::get_if<MicaAppWindowData>(&Window) : nullptr;
  }

  MicaAppWindowData const *MicaApp() const noexcept
  {
    return Kind == WindowType::MICA ? std::get_if<MicaAppWindowData>(&Window) : nullptr;
  }
};

} // namespace winrt::testlib::detail
