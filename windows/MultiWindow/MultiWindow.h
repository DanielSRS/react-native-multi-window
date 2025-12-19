#pragma once

#include "pch.h"
#include "resource.h"

#if __has_include("codegen/NativeMultiWindowDataTypes.g.h")
  #include "codegen/NativeMultiWindowDataTypes.g.h"
#endif
#include "codegen/NativeMultiWindowSpec.g.h"

#include "NativeModules.h"
#include "ReactWindow.h"
#include "map"

namespace winrt::MultiWindow
{

  using JSValueObject = winrt::Microsoft::ReactNative::JSValueObject;

// See https://microsoft.github.io/react-native-windows/docs/native-platform for help writing native modules

REACT_MODULE(MultiWindow)
struct MultiWindow
{
  using ModuleSpec = MultiWindowCodegen::MultiWindowSpec;
  using WindowOptions = MultiWindowCodegen::MultiWindowSpec_WindowOptions;
  using ReactPromiseDouble = ::React::ReactPromise<double>;

  REACT_INIT(Initialize)
  void Initialize(React::ReactContext const &reactContext) noexcept;

  REACT_METHOD(openNewWindow)
  void openNewWindow(WindowOptions&& options, ReactPromiseDouble&& result) noexcept;

  REACT_SYNC_METHOD(closeWindowBy)
  double closeWindowBy(double id) noexcept;

 private:
  void EmitLogEvent(JSValueObject payload) noexcept;
    void EmitWindowEvent(JSValueObject payload) noexcept;
  void RemoveWindow(winrt::Microsoft::UI::Windowing::AppWindow const& window) noexcept;

  React::ReactContext m_context;
  std::map<uintptr_t, ReactWindow> m_openWindows;
};

} // namespace winrt::MultiWindow
