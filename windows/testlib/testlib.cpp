#include "pch.h"
#include <winrt/Microsoft.UI.Windowing.h>
#include <utility>

#include "testlib.h"

namespace winrt::testlib
{

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

void Testlib::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
}

double Testlib::multiply(double a, double b) noexcept {
  return a * b;
}

double _openNewWindow() noexcept {
  auto appWindow = winrt::Microsoft::UI::Windowing::AppWindow::Create();
  if (appWindow) {
    appWindow.Title(L"New Window from RN");
    appWindow.Show();
    return 999;
  }
  return -111; // failed to create window
}

void Testlib::openNewWindow(::React::ReactPromise<double> &&promise) noexcept {
  auto dispatcher = m_context.UIDispatcher();

  if (dispatcher && dispatcher.HasThreadAccess()) {
    promise.Resolve(_openNewWindow());
    return;
  }

  if (dispatcher) {
    dispatcher.Post([promise = std::move(promise)]() mutable {
      promise.Resolve(_openNewWindow());
    });
    return;
  }

  promise.Reject(L"no_dispatcher. UIDispatcher is not available.");
}

} // namespace winrt::testlib