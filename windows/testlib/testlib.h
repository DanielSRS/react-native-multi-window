#pragma once

#include "pch.h"
#include "resource.h"

#if __has_include("codegen/NativeTestlibDataTypes.g.h")
  #include "codegen/NativeTestlibDataTypes.g.h"
#endif
#include "codegen/NativeTestlibSpec.g.h"

#include "NativeModules.h"

namespace winrt::testlib
{

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

REACT_MODULE(Testlib)
struct Testlib
{
  using ModuleSpec = testlibCodegen::TestlibSpec;

  REACT_INIT(Initialize)
  void Initialize(React::ReactContext const &reactContext) noexcept;

  REACT_SYNC_METHOD(multiply)
  double multiply(double a, double b) noexcept;

  REACT_METHOD(openNewWindow)
  void openNewWindow(::React::ReactPromise<double> &&result) noexcept;

private:
  React::ReactContext m_context;
};

} // namespace winrt::testlib