
/*
 * This file is auto-generated from a NativeModule spec file in js.
 *
 * This is a C++ Spec class that should be used with MakeTurboModuleProvider to register native modules
 * in a way that also verifies at compile time that the native module matches the interface required
 * by the TurboModule JS spec.
 */
#pragma once
// clang-format off

// #include "NativeTestlibDataTypes.g.h" before this file to use the generated type definition
#include <NativeModules.h>
#include <tuple>

namespace testlibCodegen {

inline winrt::Microsoft::ReactNative::FieldMap GetStructInfo(TestlibSpec_WindowOptions*) noexcept {
    winrt::Microsoft::ReactNative::FieldMap fieldMap {
        {L"title", &TestlibSpec_WindowOptions::title},
    };
    return fieldMap;
}

struct TestlibSpec : winrt::Microsoft::ReactNative::TurboModuleSpec {
  static constexpr auto methods = std::tuple{
      SyncMethod<double(double, double) noexcept>{0, L"multiply"},
      Method<void(TestlibSpec_WindowOptions, Promise<double>) noexcept>{1, L"openNewWindow"},
      Method<void(Promise<double>) noexcept>{2, L"openMicaWindow"},
  };

  template <class TModule>
  static constexpr void ValidateModule() noexcept {
    constexpr auto methodCheckResults = CheckMethods<TModule, TestlibSpec>();

    REACT_SHOW_METHOD_SPEC_ERRORS(
          0,
          "multiply",
          "    REACT_SYNC_METHOD(multiply) double multiply(double a, double b) noexcept { /* implementation */ }\n"
          "    REACT_SYNC_METHOD(multiply) static double multiply(double a, double b) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          1,
          "openNewWindow",
          "    REACT_METHOD(openNewWindow) void openNewWindow(TestlibSpec_WindowOptions && options, ::React::ReactPromise<double> &&result) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(openNewWindow) static void openNewWindow(TestlibSpec_WindowOptions && options, ::React::ReactPromise<double> &&result) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          2,
          "openMicaWindow",
          "    REACT_METHOD(openMicaWindow) void openMicaWindow(::React::ReactPromise<double> &&result) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(openMicaWindow) static void openMicaWindow(::React::ReactPromise<double> &&result) noexcept { /* implementation */ }\n");
  }
};

} // namespace testlibCodegen
