#pragma once

#include "pch.h"
#include "resource.h"

#if __has_include("codegen/NativeMultiWindowDataTypes.g.h")
  #include "codegen/NativeMultiWindowDataTypes.g.h"
#endif
#include "codegen/NativeMultiWindowSpec.g.h"

#include "NativeModules.h"

namespace winrt::MultiWindow
{

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

REACT_MODULE(MultiWindow)
struct MultiWindow
{
  using ModuleSpec = MultiWindowCodegen::MultiWindowSpec;

  REACT_INIT(Initialize)
  void Initialize(React::ReactContext const &reactContext) noexcept;

  REACT_SYNC_METHOD(multiply)
  double multiply(double a, double b) noexcept;

 private:
  void EmitLogEvent(winrt::Microsoft::ReactNative::JSValueObject payload) noexcept;
  winrt::Microsoft::ReactNative::JSValueObject CreateMultiplyPayload(double a, double b) noexcept;

  React::ReactContext m_context;
};

} // namespace winrt::MultiWindow