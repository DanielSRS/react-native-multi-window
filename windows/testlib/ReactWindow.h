#pragma once

#include <memory>
#include <utility>
#include <variant>

#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Microsoft.UI.Windowing.h>

struct MicaWindow;

namespace winrt::testlib::detail
{

struct ReactWindow
{
  enum class Type
  {
    AppWindow,
    MicaWindow,
  };

  struct AppWindowData
  {
    winrt::Microsoft::UI::Windowing::AppWindow Window{nullptr};
    winrt::event_token ChangedToken{};
    winrt::event_token DestroyingToken{};
  };

  struct MicaWindowData
  {
    std::unique_ptr<::MicaWindow> Window{};
  };

  using WindowVariant = std::variant<std::monostate, AppWindowData, MicaWindowData>;

  Type Kind{Type::AppWindow};
  WindowVariant Window{std::monostate{}};
  winrt::Microsoft::ReactNative::CompositionHwndHost CompositionHost{nullptr};
  winrt::Microsoft::ReactNative::IReactViewHost ViewHost{nullptr};

  static ReactWindow CreateAppWindow(
      winrt::Microsoft::UI::Windowing::AppWindow const &appWindow,
      winrt::Microsoft::ReactNative::CompositionHwndHost const &compositionHost,
      winrt::Microsoft::ReactNative::IReactViewHost const &viewHost)
  {
    ReactWindow result;
    result.Kind = Type::AppWindow;
    result.Window = AppWindowData{appWindow};
    result.CompositionHost = compositionHost;
    result.ViewHost = viewHost;
    return result;
  }

  static ReactWindow CreateMicaWindow(std::unique_ptr<::MicaWindow> window) noexcept
  {
    ReactWindow result;
    result.Kind = Type::MicaWindow;
    result.Window = MicaWindowData{std::move(window)};
    return result;
  }

  AppWindowData *App() noexcept
  {
    return Kind == Type::AppWindow ? std::get_if<AppWindowData>(&Window) : nullptr;
  }

  AppWindowData const *App() const noexcept
  {
    return Kind == Type::AppWindow ? std::get_if<AppWindowData>(&Window) : nullptr;
  }

  MicaWindowData *Mica() noexcept
  {
    return Kind == Type::MicaWindow ? std::get_if<MicaWindowData>(&Window) : nullptr;
  }

  MicaWindowData const *Mica() const noexcept
  {
    return Kind == Type::MicaWindow ? std::get_if<MicaWindowData>(&Window) : nullptr;
  }
};

} // namespace winrt::testlib::detail
