
/*
 * This file is auto-generated from a NativeModule spec file in js.
 *
 * This is a C++ Spec class that should be used with MakeTurboModuleProvider to register native modules
 * in a way that also verifies at compile time that the native module matches the interface required
 * by the TurboModule JS spec.
 */
#pragma once
// clang-format off

// #include "NativeMultiWindowDataTypes.g.h" before this file to use the generated type definition
#include <NativeModules.h>
#include <tuple>

namespace MultiWindowCodegen {

inline winrt::Microsoft::ReactNative::FieldMap GetStructInfo(MultiWindowSpec_WindowOptions*) noexcept {
    winrt::Microsoft::ReactNative::FieldMap fieldMap {
        {L"title", &MultiWindowSpec_WindowOptions::title},
        {L"componentName", &MultiWindowSpec_WindowOptions::componentName},
        {L"windows_WindowType", &MultiWindowSpec_WindowOptions::windows_WindowType},
    };
    return fieldMap;
}

struct MultiWindowSpec : winrt::Microsoft::ReactNative::TurboModuleSpec {
  static constexpr auto methods = std::tuple{
      Method<void(MultiWindowSpec_WindowOptions, Promise<double>) noexcept>{0, L"openNewWindow"},
      SyncMethod<double(double) noexcept>{1, L"closeWindowBy"},
  };

  template <class TModule>
  static constexpr void ValidateModule() noexcept {
    constexpr auto methodCheckResults = CheckMethods<TModule, MultiWindowSpec>();

    REACT_SHOW_METHOD_SPEC_ERRORS(
          0,
          "openNewWindow",
          "    REACT_METHOD(openNewWindow) void openNewWindow(MultiWindowSpec_WindowOptions && options, ::React::ReactPromise<double> &&result) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(openNewWindow) static void openNewWindow(MultiWindowSpec_WindowOptions && options, ::React::ReactPromise<double> &&result) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          1,
          "closeWindowBy",
          "    REACT_SYNC_METHOD(closeWindowBy) double closeWindowBy(double id) noexcept { /* implementation */ }\n"
          "    REACT_SYNC_METHOD(closeWindowBy) static double closeWindowBy(double id) noexcept { /* implementation */ }\n");
  }
};

} // namespace MultiWindowCodegen
