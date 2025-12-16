#pragma once

#include <variant>
#include <optional>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>

#if __has_include("codegen/NativeMultiWindowDataTypes.g.h")
#include "codegen/NativeMultiWindowDataTypes.g.h"
#endif

// ReactWindow
namespace winrt::MultiWindow {

  using AppWindow = winrt::Microsoft::UI::Windowing::AppWindow;
  using EventToken = winrt::event_token;
  using Visual = winrt::Windows::UI::Composition::Visual;
  using CompositionTarget = winrt::Windows::UI::Composition::CompositionTarget;
  using MicaController = winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController;
  using CompositionHwndHost = winrt::Microsoft::ReactNative::CompositionHwndHost;
  using IReactViewHost = winrt::Microsoft::ReactNative::IReactViewHost;
  using WindowOptions = MultiWindowCodegen::MultiWindowSpec_WindowOptions;
  using winrt::Microsoft::ReactNative::ReactNativeHost;
  using winrt::Microsoft::ReactNative::ReactViewOptions;
  using winrt::Microsoft::ReactNative::ReactCoreInjection;

  enum class WindowType {
    DEFAULT = 0,
    //ACRYLIC = 1,
    MICA = 2,
  };

  enum class ReactWindowCreationError {
    NO_APP_WINDOW = -81273,
    NO_WINDOW_HANDLE = -85674,
    NO_REACT_NATIVE_HOST = -91234,
    NO_VIEW_HOST = -23456,
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

  inline void UpdateCompositionHostSize(CompositionHwndHost compositionHost) noexcept {
    if (!compositionHost) {
      return;
    }

    compositionHost.TranslateMessage(WM_WINDOWPOSCHANGED, 0, 0);
  }

  using EitherReactWindowCreationErrorOrReactWindow = std::variant<ReactWindowCreationError, ReactWindow>;
  using RemoveCallback = std::function<void(winrt::Microsoft::UI::Windowing::AppWindow const&)>;

  inline EitherReactWindowCreationErrorOrReactWindow OpenReactWindow(
    winrt::Microsoft::ReactNative::ReactContext const& context,
    WindowOptions const& options,
    RemoveCallback onWindowClosed = nullptr
  ) {
    auto appWindow = AppWindow::Create();
    if (!appWindow) {
      return ReactWindowCreationError::NO_APP_WINDOW;
    }

    const auto windowId = appWindow.Id();
    const auto hwnd = winrt::Microsoft::UI::GetWindowFromWindowId(windowId);
    if (!hwnd) {
      appWindow.Destroy();
      return ReactWindowCreationError::NO_WINDOW_HANDLE;
    }

    auto reactHost = ReactNativeHost::FromContext(context.Handle());
    if (reactHost == nullptr) {
      appWindow.Destroy();
      return ReactWindowCreationError::NO_REACT_NATIVE_HOST;
    }

    ReactViewOptions viewOptions;
    viewOptions.ComponentName(winrt::to_hstring(options.componentName));

    auto instanceSettings = reactHost.InstanceSettings();
    auto properties = instanceSettings.Properties();
    auto previousWindowId = ReactCoreInjection::GetTopLevelWindowId(properties);
    ReactCoreInjection::SetTopLevelWindowId(properties, reinterpret_cast<uint64_t>(hwnd));

    winrt::Microsoft::ReactNative::IReactViewHost viewHost{ nullptr };
    try {
      viewHost = ReactCoreInjection::MakeViewHost(reactHost, viewOptions);
    }
    catch (...) {
      viewHost = nullptr;
    }

    ReactCoreInjection::SetTopLevelWindowId(properties, previousWindowId);

    if (viewHost == nullptr) {
      appWindow.Destroy();
      return ReactWindowCreationError::NO_VIEW_HOST;
    }

    CompositionHwndHost compositionHost;
    compositionHost.ReactViewHost(viewHost);
    compositionHost.Initialize(reinterpret_cast<uint64_t>(hwnd));

    appWindow.Title(winrt::to_hstring(options.title));

    auto changedToken = appWindow.Changed([cmp = compositionHost](
      AppWindow const& sender,
      winrt::Microsoft::UI::Windowing::AppWindowChangedEventArgs const& args) {
        if (args.DidSizeChange() || args.DidPresenterChange()) {
          UpdateCompositionHostSize(cmp);
        }
      });

    auto destroyingToken = appWindow.Destroying([onRemove = onWindowClosed](
      winrt::Microsoft::UI::Windowing::AppWindow const& sender, winrt::Windows::Foundation::IInspectable const&) {
        if (onRemove) {
          onRemove(sender);
        }
      });

    ReactWindow result;
    result.type = WindowType::DEFAULT;
    result.window = appWindow;
    result.compositionHost = compositionHost;
    result.viewHost = viewHost;
    result.changedToken = changedToken;
    result.destroyingToken = destroyingToken;

    return result;
  }

} // namespace winrt::MultiWindow
