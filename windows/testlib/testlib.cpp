#include "pch.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <DispatcherQueue.h>

#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.System.h>
#include <winrt/Microsoft.ReactNative.Composition.h>

#include "testlib.h"
#include "MicaWindow.h"
#include "ReactWindow.h"
#include "Utilities.h"

namespace winrt::testlib
{

namespace detail
{

constexpr int kErrorCreateWindow = -111;
constexpr int kErrorLookupWindowHandle = -112;
constexpr int kErrorReactNativeHostUnavailable = -113;
constexpr int kErrorCreateViewHost = -114;
constexpr int kErrorEnableMica = -115;



inline std::vector<ReactWindow> &ReactWindows() noexcept {
  static std::vector<ReactWindow> windows;
  return windows;
}

inline ReactWindow *FindWindow(winrt::Microsoft::UI::WindowId const &windowId) noexcept {
  auto &windows = ReactWindows();
  auto it = std::find_if(windows.begin(), windows.end(), [&windowId](ReactWindow const &candidate) {
    if (auto app = candidate.App()) {
      if (app->Window && app->Window.Id() == windowId) {
        return true;
      }
    }

    if (auto micaApp = candidate.MicaApp()) {
      return micaApp->Window && micaApp->Window.Id() == windowId;
    }

    return false;
  });
  return it == windows.end() ? nullptr : &(*it);
}

inline ReactWindow *FindWindow(winrt::Microsoft::UI::Windowing::AppWindow const &window) noexcept {
  return FindWindow(window.Id());
}

inline const wchar_t *ErrorMessageFor(int errorCode) noexcept {
  switch (errorCode) {
  case kErrorCreateWindow:
    return L"Failed to create AppWindow.";
  case kErrorLookupWindowHandle:
    return L"Failed to acquire native window handle.";
  case kErrorReactNativeHostUnavailable:
    return L"ReactNativeHost is not available.";
  case kErrorCreateViewHost:
    return L"Failed to create React view host for the window.";
  case kErrorEnableMica:
    return L"Failed to enable Mica on the window.";
  default:
    return L"Failed to create a React window.";
  }
}

inline void UpdateCompositionHostSize(ReactWindow &window) noexcept {
  if (!window.CompositionHost) {
    return;
  }

  window.CompositionHost.TranslateMessage(WM_WINDOWPOSCHANGED, 0, 0);
}

inline void RemoveWindow(winrt::Microsoft::UI::Windowing::AppWindow const &window) noexcept {
  auto &windows = ReactWindows();
  windows.erase(
      std::remove_if(windows.begin(), windows.end(), [windowId = window.Id()](ReactWindow &entry) {
        auto detachTokens = [windowId](auto &data) {
          if (data.Window && data.Window.Id() == windowId) {
            if (data.ChangedToken.value != 0) {
              data.Window.Changed(data.ChangedToken);
            }
            if (data.DestroyingToken.value != 0) {
              data.Window.Destroying(data.DestroyingToken);
            }
            return true;
          }
          return false;
        };

        bool matched = false;
        if (auto app = entry.App()) {
          matched = detachTokens(*app);
        } else if (auto micaApp = entry.MicaApp()) {
          matched = detachTokens(*micaApp);
          if (matched) {
            micaApp->RootVisual = nullptr;
            micaApp->CompositionTarget = nullptr;
            micaApp->Controller = nullptr;
            micaApp->IsSupported = false;
          }
        }

        if (!matched) {
          return false;
        }

        if (entry.ViewHost) {
          try {
            auto unloadAction = entry.ViewHost.UnloadViewInstance();
            if (unloadAction) {
              unloadAction.Completed([](auto &&, auto &&) {});
            }
          } catch (...) {
          }
        }

        entry.CompositionHost = nullptr;
        entry.ViewHost = nullptr;
        entry.Window = std::monostate{};

        return true;
      }),
      windows.end());
}

inline double OpenReactWindow(
    winrt::Microsoft::ReactNative::ReactContext const &context,
    testlibCodegen::TestlibSpec_WindowOptions const &options) noexcept {
  using winrt::Microsoft::ReactNative::CompositionHwndHost;
  using winrt::Microsoft::ReactNative::ReactCoreInjection;
  using winrt::Microsoft::ReactNative::ReactNativeHost;
  using winrt::Microsoft::ReactNative::ReactViewOptions;

  auto appWindow = winrt::Microsoft::UI::Windowing::AppWindow::Create();
  if (!appWindow) {
    return static_cast<double>(kErrorCreateWindow);
  }

  const auto windowId = appWindow.Id();
  const auto hwnd = winrt::Microsoft::UI::GetWindowFromWindowId(windowId);
  if (!hwnd) {
    appWindow.Destroy();
    return static_cast<double>(kErrorLookupWindowHandle);
  }

  const auto windowType = ParseWindowType(options.windows_WindowType);
  winrt::Windows::UI::Composition::Visual micaRoot{nullptr};
  winrt::Windows::UI::Composition::CompositionTarget micaCompositionTarget{nullptr};
  winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController micaController{nullptr};
  bool micaSupported = false;

  auto reactHost = ReactNativeHost::FromContext(context.Handle());
  if (reactHost == nullptr) {
    appWindow.Destroy();
    return static_cast<double>(kErrorReactNativeHostUnavailable);
  }

  ReactViewOptions viewOptions;
  viewOptions.ComponentName(L"TestlibExample");

  auto instanceSettings = reactHost.InstanceSettings();
  auto properties = instanceSettings.Properties();
  auto previousWindowId = ReactCoreInjection::GetTopLevelWindowId(properties);
  ReactCoreInjection::SetTopLevelWindowId(properties, reinterpret_cast<uint64_t>(hwnd));

  winrt::Microsoft::ReactNative::IReactViewHost viewHost{nullptr};
  try {
    viewHost = ReactCoreInjection::MakeViewHost(reactHost, viewOptions);
  } catch (...) {
    viewHost = nullptr;
  }

  ReactCoreInjection::SetTopLevelWindowId(properties, previousWindowId);

  if (viewHost == nullptr) {
    appWindow.Destroy();
    return static_cast<double>(kErrorCreateViewHost);
  }

  CompositionHwndHost compositionHost;
  compositionHost.ReactViewHost(viewHost);
  compositionHost.Initialize(reinterpret_cast<uint64_t>(hwnd));

  if (windowType == WindowType::MICA) {
    try {
  auto compositor = Utilities::EnsureThreadLocalCompositor(context);
      auto micaResult = MicaWindow::applyMica(compositor, hwnd);

      if (!micaResult.Target || !micaResult.Controller) {
        throw winrt::hresult_error(E_FAIL);
      }

      // bool backdropTargetSet = true;
      // if (auto supportsBackdrop =
      //         micaResult.Target.try_as<winrt::Microsoft::UI::Composition::ICompositionSupportsSystemBackdrop>()) {
      //   backdropTargetSet = micaResult.Controller.AddSystemBackdropTarget(supportsBackdrop);
      // }

      micaCompositionTarget = micaResult.Target;
      micaRoot = micaResult.Root ? micaResult.Root : micaCompositionTarget.Root();
      micaController = micaResult.Controller;
      micaSupported = micaResult.IsSupported; // && backdropTargetSet;

      if (!micaSupported) {
        throw winrt::hresult_error(E_FAIL);
      }
    } catch (...) {
      appWindow.Destroy();
      return static_cast<double>(kErrorEnableMica);
    }
  }

  auto &windows = ReactWindows();
  if (windowType == WindowType::MICA) {
     windows.push_back(ReactWindow::CreateMicaAppWindow(
       appWindow,
       micaRoot,
       micaCompositionTarget,
       micaController,
       micaSupported,
       compositionHost,
       viewHost));
  } else {
    windows.push_back(ReactWindow::CreateAppWindow(appWindow, compositionHost, viewHost));
  }
  auto &storedWindow = windows.back();

  auto changedToken = appWindow.Changed([](
      winrt::Microsoft::UI::Windowing::AppWindow const &sender,
      winrt::Microsoft::UI::Windowing::AppWindowChangedEventArgs const &args) {
    if (args.DidSizeChange() || args.DidPresenterChange()) {
      if (auto windowPtr = FindWindow(sender.Id())) {
        UpdateCompositionHostSize(*windowPtr);
      }
    }
  });

  auto destroyingToken = appWindow.Destroying([](
      winrt::Microsoft::UI::Windowing::AppWindow const &sender, winrt::Windows::Foundation::IInspectable const &) {
    RemoveWindow(sender);
  });

  if (auto appData = storedWindow.App()) {
    appData->ChangedToken = changedToken;
    appData->DestroyingToken = destroyingToken;
  } else if (auto micaAppData = storedWindow.MicaApp()) {
    micaAppData->ChangedToken = changedToken;
    micaAppData->DestroyingToken = destroyingToken;
  }

  if (!options.title.empty()) {
    appWindow.Title(winrt::to_hstring(options.title));
  } else {
    appWindow.Title(L"New Window from RN");
  }
  appWindow.Show();

  UpdateCompositionHostSize(storedWindow);

  return static_cast<double>(windowId.Value);
}

} // namespace detail

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

void Testlib::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
}

double Testlib::multiply(double a, double b) noexcept {
  return a * b;
}

void Testlib::openNewWindow(WindowOptions && options, ::React::ReactPromise<double> &&promise) noexcept {
  auto dispatcher = m_context.UIDispatcher();

  auto fulfill = [context = m_context](
                     WindowOptions opts,
                     ::React::ReactPromise<double> &&innerPromise) mutable {
    auto result = detail::OpenReactWindow(context, opts);
    if (result >= 0) {
      innerPromise.Resolve(result);
    } else {
      innerPromise.Reject(detail::ErrorMessageFor(static_cast<int>(result)));
    }
  };

  if (dispatcher && dispatcher.HasThreadAccess()) {
    fulfill(std::move(options), std::move(promise));
    return;
  }

  if (dispatcher) {
    dispatcher.Post([fulfill,
                     opts = std::move(options),
                     promise = std::move(promise)]() mutable {
      fulfill(std::move(opts), std::move(promise));
    });
    return;
  }

  promise.Reject(L"no_dispatcher. UIDispatcher is not available.");
}

} // namespace winrt::testlib