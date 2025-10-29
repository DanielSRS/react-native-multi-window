
/*
 * This file is auto-generated from a NativeModule spec file in js.
 *
 * This is a C++ Spec class that should be used with MakeTurboModuleProvider to register native modules
 * in a way that also verifies at compile time that the native module matches the interface required
 * by the TurboModule JS spec.
 */
#pragma once
// clang-format off


#include <NativeModules.h>
#include <tuple>

namespace testlibCodegen {

struct TestlibSpec : winrt::Microsoft::ReactNative::TurboModuleSpec {
  static constexpr auto methods = std::tuple{
      SyncMethod<double(double, double) noexcept>{0, L"multiply"},
      Method<void(Promise<double>) noexcept>{1, L"openNewWindow"},
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
          "    REACT_METHOD(openNewWindow) void openNewWindow(::React::ReactPromise<double> &&result) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(openNewWindow) static void openNewWindow(::React::ReactPromise<double> &&result) noexcept { /* implementation */ }\n");
  }
};

} // namespace testlibCodegen
