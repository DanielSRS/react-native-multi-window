#include "pch.h"
#include <winrt/Microsoft.UI.Windowing.h>
#include <future>
#include <chrono>

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
  auto appWindow =
            winrt::Microsoft::UI::Windowing::AppWindow::Create();
  if (appWindow)
  {
      appWindow.Title(L"New Window form RN");
      appWindow.Show();
      return 999;
  }
  return -1 * 111; // failed to create window
 }

 double Testlib::openNewWindow() noexcept { 
  std::promise<double> creationPromise;
  auto creationFuture = creationPromise.get_future();

  m_context.UIDispatcher().Post([&]() {
    double result = _openNewWindow();
    creationPromise.set_value(result);
  });

  auto status = creationFuture.wait_for(std::chrono::seconds(5));
  if (status == std::future_status::timeout) {
    return -1 * 222; // timeout
  }
  return creationFuture.get();
 }

} // namespace winrt::testlib