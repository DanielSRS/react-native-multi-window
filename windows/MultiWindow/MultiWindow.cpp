#include "pch.h"

#include "MultiWindow.h"

namespace winrt::MultiWindow
{

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

void MultiWindow::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
}

double MultiWindow::multiply(double a, double b) noexcept {
  return a * b;
}

} // namespace winrt::MultiWindow