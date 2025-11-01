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

enum class WindowType
{
  Default = 0,
  Mica = 1,
  DefaultWithMica = 2,
};

inline WindowType ParseWindowType(double value) noexcept {
  const auto type = static_cast<int>(value);
  switch (type) {
  case 1:
    return WindowType::Mica;
  case 2:
    return WindowType::DefaultWithMica;
  default:
    return WindowType::Default;
  }
}

inline winrt::Windows::UI::Composition::Compositor EnsureThreadLocalCompositor() {
  using winrt::Windows::System::DispatcherQueue;
  if (DispatcherQueue::GetForCurrentThread() == nullptr) {
    thread_local winrt::Windows::System::DispatcherQueueController controller{nullptr};
    if (!controller) {
      controller = Utilities::CreateDispatcherQueueControllerForCurrentThread();
    }
  }

  thread_local winrt::Windows::UI::Composition::Compositor compositor{nullptr};
  if (!compositor) {
    compositor = winrt::Windows::UI::Composition::Compositor();
  }
  return compositor;
}

inline std::vector<ReactWindow> &ReactWindows() noexcept {
  static std::vector<ReactWindow> windows;
  return windows;
}

namespace mica
{
namespace
{

constexpr wchar_t kWindowTitle[] = L"Hello, Mica!";

inline winrt::Windows::UI::Composition::Compositor &SharedCompositor() noexcept {
  static winrt::Windows::UI::Composition::Compositor compositor{nullptr};
  return compositor;
}

inline void EnsureDispatcherQueueController() {
  // Composition APIs require a dispatcher queue on the owning thread.
  using winrt::Windows::System::DispatcherQueue;
  if (DispatcherQueue::GetForCurrentThread() != nullptr) {
    return;
  }

  thread_local winrt::Windows::System::DispatcherQueueController controller{nullptr};
  if (!controller) {
    controller = Utilities::CreateDispatcherQueueControllerForCurrentThread();
  }
}

inline winrt::Windows::UI::Composition::Compositor EnsureCompositor() {
  EnsureDispatcherQueueController();

  auto &compositor = SharedCompositor();
  if (!compositor) {
    compositor = winrt::Windows::UI::Composition::Compositor();
  }
  return compositor;
}

} // namespace

void Untrack(MicaWindow *window) noexcept {
  if (!window) {
    return;
  }

  auto &windows = ReactWindows();
  windows.erase(
      std::remove_if(
          windows.begin(),
          windows.end(),
          [window](ReactWindow &entry) {
            auto mica = entry.Mica();
            if (!mica || mica->Window.get() != window) {
              return false;
            }

            mica->Window.reset();
            return true;
          }),
      windows.end());
}

double Open(winrt::Microsoft::ReactNative::ReactContext const &context) noexcept {
  try {
    auto reactHost = winrt::Microsoft::ReactNative::ReactNativeHost::FromContext(context.Handle());
    if (reactHost == nullptr) {
      return -2.0; // no ReactNativeHost available
    }

    auto compositor = EnsureCompositor();
    if (!compositor) {
      return -3.0; // failed to create compositor instance
    }


  auto window = std::make_unique<MicaWindow>(compositor, kWindowTitle);
  ReactWindows().push_back(ReactWindow::CreateMicaWindow(std::move(window)));
    return 1.0;
  } catch (winrt::hresult_error const &error) {
    return static_cast<double>(error.code());
  } catch (...) {
    return -5.0; // window creation failed with unexpected exception
  }
}

} // namespace mica

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

  if (windowType == WindowType::DefaultWithMica) {
    try {
      auto compositor = EnsureThreadLocalCompositor();
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
  if (windowType == WindowType::DefaultWithMica) {
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

void Testlib::openMicaWindow(::React::ReactPromise<double> &&promise) noexcept {
  auto dispatcher = m_context.UIDispatcher();

  auto fulfill = [context = m_context](::React::ReactPromise<double> &&innerPromise) mutable {
    auto result = detail::mica::Open(context);
    innerPromise.Resolve(result);
  };

  if (dispatcher && dispatcher.HasThreadAccess()) {
    fulfill(std::move(promise));
    return;
  }

  if (dispatcher) {
    auto context = m_context;
    dispatcher.Post([promise = std::move(promise), context]() mutable {
      auto result = detail::mica::Open(context);
      promise.Resolve(result);
    });
    return;
  }
  // Mica window creation not implemented yet
  promise.Reject(L"Mica window creation is not implemented.");
}

} // namespace winrt::testlib