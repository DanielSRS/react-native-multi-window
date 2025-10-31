#include "pch.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <DispatcherQueue.h>

#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.System.h>
#include <winrt/Microsoft.ReactNative.Composition.h>

#include "testlib.h"
#include "MicaWindow.h"
#include "Utilities.h"

namespace winrt::testlib
{

namespace detail
{

constexpr int kErrorCreateWindow = -111;
constexpr int kErrorLookupWindowHandle = -112;
constexpr int kErrorReactNativeHostUnavailable = -113;
constexpr int kErrorCreateViewHost = -114;

// Keep references alive for as long as the secondary window exists.
struct ReactWindowState
{
  winrt::Microsoft::UI::Windowing::AppWindow Window{nullptr};
  winrt::Microsoft::ReactNative::CompositionHwndHost CompositionHost{nullptr};
  winrt::Microsoft::ReactNative::IReactViewHost ViewHost{nullptr};
  winrt::event_token WindowChangedToken{};
  winrt::event_token WindowDestroyingToken{};
};

inline std::vector<ReactWindowState> &ReactWindowStates() noexcept {
  static std::vector<ReactWindowState> states;
  return states;
}

inline std::vector<std::unique_ptr<MicaWindow>> &MicaWindows() noexcept {
  static std::vector<std::unique_ptr<MicaWindow>> windows;
  return windows;
}

inline void EnsureDispatcherQueueController() {
  using winrt::Windows::System::DispatcherQueue;
  using winrt::Windows::System::DispatcherQueueController;

  if (DispatcherQueue::GetForCurrentThread() != nullptr) {
    return;
  }

  thread_local DispatcherQueueController controller{nullptr};
  if (!controller) {
    controller = Utilities::CreateDispatcherQueueControllerForCurrentThread();
  }
}

inline ReactWindowState *FindWindowState(winrt::Microsoft::UI::WindowId const &windowId) noexcept {
  auto &states = ReactWindowStates();
  auto it = std::find_if(states.begin(), states.end(), [&windowId](ReactWindowState const &candidate) {
    return candidate.Window && candidate.Window.Id() == windowId;
  });
  return it == states.end() ? nullptr : &(*it);
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
  default:
    return L"Failed to create a React window.";
  }
}

inline void UpdateCompositionHostSize(ReactWindowState &state) noexcept {
  if (!state.Window || !state.CompositionHost) {
    return;
  }

  state.CompositionHost.TranslateMessage(WM_WINDOWPOSCHANGED, 0, 0);
}

inline void PruneWindowState(winrt::Microsoft::UI::Windowing::AppWindow const &window) noexcept {
  auto &states = ReactWindowStates();
  states.erase(
      std::remove_if(states.begin(), states.end(), [windowId = window.Id()](ReactWindowState &state) {
        if (!(state.Window && state.Window.Id() == windowId)) {
          return false;
        }

        if (state.WindowChangedToken.value != 0) {
          state.Window.Changed(state.WindowChangedToken);
        }
        if (state.WindowDestroyingToken.value != 0) {
          state.Window.Destroying(state.WindowDestroyingToken);
        }

        if (state.ViewHost) {
          try {
            auto unloadAction = state.ViewHost.UnloadViewInstance();
            if (unloadAction) {
              unloadAction.Completed([](auto &&, auto &&) {});
            }
          } catch (...) {
          }
        }

        state.Window = nullptr;
        state.CompositionHost = nullptr;
        state.ViewHost = nullptr;
        state.WindowChangedToken = {};
        state.WindowDestroyingToken = {};

        return true;
      }),
      states.end());
}

inline double OpenReactWindow(winrt::Microsoft::ReactNative::ReactContext const &context) noexcept {
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

  auto &states = ReactWindowStates();
  states.emplace_back();
  auto &storedState = states.back();
  storedState.Window = appWindow;
  storedState.CompositionHost = compositionHost;
  storedState.ViewHost = viewHost;

  storedState.WindowChangedToken = appWindow.Changed([](
                                             winrt::Microsoft::UI::Windowing::AppWindow const &sender,
                                             winrt::Microsoft::UI::Windowing::AppWindowChangedEventArgs const &args) {
    if (args.DidSizeChange() || args.DidPresenterChange()) {
      if (auto statePtr = FindWindowState(sender.Id())) {
        UpdateCompositionHostSize(*statePtr);
      }
    }
  });

  storedState.WindowDestroyingToken = appWindow.Destroying([](
      winrt::Microsoft::UI::Windowing::AppWindow const &sender, winrt::Windows::Foundation::IInspectable const &) {
    PruneWindowState(sender);
  });

  appWindow.Title(L"New Window from RN");
  appWindow.Show();

  UpdateCompositionHostSize(storedState);

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

void Testlib::openNewWindow(::React::ReactPromise<double> &&promise) noexcept {
  auto dispatcher = m_context.UIDispatcher();

  auto fulfill = [context = m_context](::React::ReactPromise<double> &&innerPromise) mutable {
    auto result = detail::OpenReactWindow(context);
    if (result >= 0) {
      innerPromise.Resolve(result);
    } else {
      innerPromise.Reject(detail::ErrorMessageFor(static_cast<int>(result)));
    }
  };

  if (dispatcher && dispatcher.HasThreadAccess()) {
    fulfill(std::move(promise));
    return;
  }

  if (dispatcher) {
    auto context = m_context;
    dispatcher.Post([promise = std::move(promise), context]() mutable {
      auto result = detail::OpenReactWindow(context);
      if (result >= 0) {
        promise.Resolve(result);
      } else {
        promise.Reject(detail::ErrorMessageFor(static_cast<int>(result)));
      }
    });
    return;
  }

  promise.Reject(L"no_dispatcher. UIDispatcher is not available.");
}

double _openMicaWindow(winrt::Microsoft::ReactNative::ReactContext const &context) noexcept {
  try {
    auto reactHost = winrt::Microsoft::ReactNative::ReactNativeHost::FromContext(context.Handle());
    if (reactHost == nullptr) {
      return -2.0; // no ReactNativeHost available
    }

    detail::EnsureDispatcherQueueController();

    static winrt::Windows::UI::Composition::Compositor sharedCompositor{nullptr};
    if (!sharedCompositor) {
      sharedCompositor = winrt::Windows::UI::Composition::Compositor();
    }
    if (!sharedCompositor) {
      return -3.0; // failed to create compositor instance
    }

    MicaWindow::RegisterWindowClass();

    auto &windows = detail::MicaWindows();
    windows.push_back(std::make_unique<MicaWindow>(sharedCompositor, L"Hello, Mica!"));

    return 1.0;
  } catch (winrt::hresult_error const &error) {
    return static_cast<double>(error.code());
  } catch (...) {
    return -5.0; // window creation failed with unexpected exception
  }
}

void Testlib::openMicaWindow(::React::ReactPromise<double> &&promise) noexcept {
  auto dispatcher = m_context.UIDispatcher();

  auto fulfill = [context = m_context](::React::ReactPromise<double> &&innerPromise) mutable {
    auto result = _openMicaWindow(context);
    innerPromise.Resolve(result);
  };

  if (dispatcher && dispatcher.HasThreadAccess()) {
    fulfill(std::move(promise));
    return;
  }

  if (dispatcher) {
    auto context = m_context;
    dispatcher.Post([promise = std::move(promise), context]() mutable {
      auto result = _openMicaWindow(context);
      promise.Resolve(result);
    });
    return;
  }
  // Mica window creation not implemented yet
  promise.Reject(L"Mica window creation is not implemented.");
}

} // namespace winrt::testlib