#include "pch.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Windows.Foundation.h>

#include "testlib.h"

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
};

inline std::vector<ReactWindowState> &ReactWindowStates() noexcept {
  static std::vector<ReactWindowState> states;
  return states;
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

inline void PruneWindowState(winrt::Microsoft::UI::Windowing::AppWindow const &window) noexcept {
  auto &states = ReactWindowStates();
  states.erase(
      std::remove_if(states.begin(), states.end(), [windowId = window.Id()](ReactWindowState const &state) {
        return state.Window && state.Window.Id() == windowId;
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

  auto viewHost = ReactCoreInjection::MakeViewHost(reactHost, viewOptions);
  if (viewHost == nullptr) {
    appWindow.Destroy();
    return static_cast<double>(kErrorCreateViewHost);
  }

  CompositionHwndHost compositionHost;
  compositionHost.ReactViewHost(viewHost);
  compositionHost.Initialize(reinterpret_cast<uint64_t>(hwnd));

  auto &states = ReactWindowStates();
  states.emplace_back(ReactWindowState{appWindow, compositionHost, viewHost});

  appWindow.Title(L"New Window from RN");

  appWindow.Destroying([](winrt::Microsoft::UI::Windowing::AppWindow const &sender,
                          winrt::Windows::Foundation::IInspectable const &) {
    PruneWindowState(sender);
  });

  appWindow.Show();

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

} // namespace winrt::testlib