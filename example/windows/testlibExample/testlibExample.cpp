// testlibExample.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "testlibExample.h"

#include "AutolinkedNativeModules.g.h"

#include "NativeModules.h"

#include <DispatcherQueue.h>
#include <windows.ui.composition.interop.h>

#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Microsoft.UI.Windowing.h>

namespace
{

  inline auto CreateDispatcherQueueControllerForCurrentThread()
    {
        namespace abi = ABI::Windows::System;

        DispatcherQueueOptions options
        {
            sizeof(DispatcherQueueOptions),
            DQTYPE_THREAD_CURRENT,
            DQTAT_COM_NONE
        };

        winrt::Windows::System::DispatcherQueueController controller{ nullptr };
        winrt::check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(winrt::put_abi(controller))));
        return controller;
    }

  inline auto PBNM = winrt::Microsoft::ReactNative::ReactPropertyBagHelper::GetNamespace(L"ReactNative.InstanceSettings");
  inline auto CompositorProperty = winrt::Microsoft::ReactNative::ReactPropertyBagHelper::GetName(PBNM, L"Windows::UI::Composition::Compositor");

} // namespace

// A PackageProvider containing any turbo modules you define within this app project
struct CompReactPackageProvider
    : winrt::implements<CompReactPackageProvider, winrt::Microsoft::ReactNative::IReactPackageProvider> {
 public: // IReactPackageProvider
  void CreatePackage(winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept {
    AddAttributedModules(packageBuilder, true);
  }
};

// The entry point of the Win32 application
_Use_decl_annotations_ int CALLBACK WinMain(HINSTANCE instance, HINSTANCE, PSTR /* commandLine */, int showCmd) {
  // Initialize WinRT
  winrt::init_apartment(winrt::apartment_type::single_threaded);

  // Enable per monitor DPI scaling
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  // Find the path hosting the app exe file
  WCHAR appDirectory[MAX_PATH];
  GetModuleFileNameW(NULL, appDirectory, MAX_PATH);
  PathCchRemoveFileSpec(appDirectory, MAX_PATH);

  // Create a ReactNativeWin32App with the ReactNativeAppBuilder
  auto reactNativeWin32App{winrt::Microsoft::ReactNative::ReactNativeAppBuilder().Build()};

  // Configure the initial InstanceSettings for the app's ReactNativeHost
  auto settings{reactNativeWin32App.ReactNativeHost().InstanceSettings()};
  // Register any autolinked native modules
  RegisterAutolinkedNativeModulePackages(settings.PackageProviders());
  // Register any native modules defined within this app project
  settings.PackageProviders().Append(winrt::make<CompReactPackageProvider>());

#if BUNDLE
  // Load the JS bundle from a file (not Metro):
  // Set the path (on disk) where the .bundle file is located
  settings.BundleRootPath(std::wstring(L"file://").append(appDirectory).append(L"\\Bundle\\").c_str());
  // Set the name of the bundle file (without the .bundle extension)
  settings.JavaScriptBundleFile(L"index.windows");
  // Disable hot reload
  settings.UseFastRefresh(false);
#else
  // Load the JS bundle from Metro
  settings.JavaScriptBundleFile(L"index");
  // Enable hot reload
  settings.UseFastRefresh(true);
#endif
#if _DEBUG
  // For Debug builds
  // Enable Direct Debugging of JS
  settings.UseDirectDebugger(true);
  // Enable the Developer Menu
  settings.UseDeveloperSupport(true);
#else
  // For Release builds:
  // Disable Direct Debugging of JS
  settings.UseDirectDebugger(false);
  // Disable the Developer Menu
  settings.UseDeveloperSupport(false);
#endif

  // Get the AppWindow so we can configure its initial title and size
  auto appWindow{reactNativeWin32App.AppWindow()};
  appWindow.Title(L"TestlibExample");
  appWindow.Resize({1000, 600});

  // setting mica
  auto queueController = CreateDispatcherQueueControllerForCurrentThread();
  auto compositor = winrt::Windows::UI::Composition::Compositor();
  auto hwnd = winrt::Microsoft::UI::GetWindowFromWindowId(appWindow.Id());
  winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget desktopTarget{nullptr};
  auto interop = compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();
  winrt::check_hresult(interop->CreateDesktopWindowTarget(
      hwnd,
      false,
      reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget **>(winrt::put_abi(desktopTarget))));

  auto compositionTarget = desktopTarget.as<winrt::Windows::UI::Composition::CompositionTarget>();
  auto container = compositor.CreateContainerVisual();
  container.RelativeSizeAdjustment({1.0f, 1.0f});
  compositionTarget.Root(container);

  auto controller = winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController();
  bool supported = controller.SetTarget(
      winrt::Microsoft::UI::WindowId{reinterpret_cast<uint64_t>(hwnd)},
      compositionTarget);
  // end setting mica

  // save compositor
  settings.Properties().Set(CompositorProperty, compositor);
  // end save compositor

  // Get the ReactViewOptions so we can set the initial RN component to load
  auto viewOptions{reactNativeWin32App.ReactViewOptions()};
  viewOptions.ComponentName(L"TestlibExample");

  // Start the app
  reactNativeWin32App.Start();
}
