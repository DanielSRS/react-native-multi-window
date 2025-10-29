#include "pch.h"

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

double Testlib::openNewWindow() noexcept { 
  return -1 * 123456789; // not implemented yet
 }

} // namespace winrt::testlib