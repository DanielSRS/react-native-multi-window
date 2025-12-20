#pragma once

#include <variant>
#include <optional>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include "AcrylicWindow.h"
#include "MicaWindow.h"

#if __has_include("codegen/NativeMultiWindowDataTypes.g.h")
#include "codegen/NativeMultiWindowDataTypes.g.h"
#endif

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
  using winrt::Windows::UI::Composition::Compositor;

  enum class WindowType {
    DEFAULT = 0,
    ACRYLIC = 1,
    MICA = 2,
  };

  enum class ReactWindowCreationError {
    NO_APP_WINDOW = -81273,
    NO_WINDOW_HANDLE = -85674,
    NO_REACT_NATIVE_HOST = -91234,
    NO_VIEW_HOST = -23456,
    NO_COMPOSITOR = -34567,
    NO_CONTAINER_VISUAL = -45678,
    UNKNOWN_MICA_ERROR = -56789,
    UNKNOWN_ACRYLIC_ERROR = -67890,
  };

  /*struct DefaultWindowData {
    
  };*/

  struct ReactWindow {
    WindowType type{ WindowType::DEFAULT };
    AppWindow window{ nullptr };
    EventToken changedToken{};
    EventToken destroyingToken{};
    CompositionHwndHost compositionHost{ nullptr };
    IReactViewHost viewHost{ nullptr };
    std::optional<MicaWindow::MicaWindowData> micaWindowData{};
    std::optional<AcrylicWindow::AcrylicWindowData> acrylicWindowData{};
  };

  inline WindowType ParseWindowType(double value) noexcept {
    const auto type = static_cast<int>(value);
    switch (type) {
    case 1:
      return WindowType::ACRYLIC;
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
  using EnsureCompositor = std::function<Compositor(winrt::Microsoft::ReactNative::ReactContext const&)>;

  inline EitherReactWindowCreationErrorOrReactWindow OpenReactWindow(
    winrt::Microsoft::ReactNative::ReactContext const& context,
    WindowOptions const& options,
    RemoveCallback onWindowClosed = nullptr,
    EnsureCompositor ensureThreadLocalCompositor = nullptr
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
    winrt::Microsoft::ReactNative::JSValueObject initialProps;
    if (options.initialProps.has_value()) {
      initialProps["initialProps"] = options.initialProps.value().Copy();
    }
    viewOptions.InitialProps([props = std::move(initialProps)](const winrt::Microsoft::ReactNative::IJSValueWriter& writer) noexcept {
      winrt::Microsoft::ReactNative::WriteValue(writer, props);
    });

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

    ReactWindow result;
    result.type = ParseWindowType(options.windows_WindowType);

    Compositor compositor{ nullptr };
    if (result.type != WindowType::DEFAULT) {
      if (!ensureThreadLocalCompositor) {
        appWindow.Destroy();
        return ReactWindowCreationError::NO_COMPOSITOR;
      }
      compositor = ensureThreadLocalCompositor(context);
      if (!compositor) {
        appWindow.Destroy();
        return ReactWindowCreationError::NO_COMPOSITOR;
      }
    }

    switch (result.type) {
    case WindowType::MICA: {
      auto micaResult = MicaWindow::applyMica(compositor, hwnd);
      if (!micaResult.compositionTarget || !micaResult.controller || !micaResult.isSupported) {
        appWindow.Destroy();
        return ReactWindowCreationError::UNKNOWN_MICA_ERROR;
      }
      result.micaWindowData = micaResult;
      break;
    }
    case WindowType::ACRYLIC: {
      auto acrylicResult = AcrylicWindow::applyAcrylic(compositor, hwnd);
      if (!acrylicResult.compositionTarget || !acrylicResult.controller || !acrylicResult.isSupported) {
        appWindow.Destroy();
        return ReactWindowCreationError::UNKNOWN_ACRYLIC_ERROR;
      }
      result.acrylicWindowData = acrylicResult;
      break;
    }
    default:
      break;
    }

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

    result.window = appWindow;
    result.compositionHost = compositionHost;
    result.viewHost = viewHost;
    result.changedToken = changedToken;
    result.destroyingToken = destroyingToken;

    return result;
  }

} // namespace winrt::MultiWindow
