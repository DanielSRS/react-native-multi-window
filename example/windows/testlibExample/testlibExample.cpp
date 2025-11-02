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

struct PrimaryWindowMicaState
{
  bool TryInitialize(winrt::Microsoft::UI::Windowing::AppWindow const &appWindow) noexcept
  {
    const auto hwnd = winrt::Microsoft::UI::GetWindowFromWindowId(appWindow.Id());
    if (!hwnd)
    {
      return false;
    }

    if (m_enabled && hwnd == m_hwnd && m_compositionTarget)
    {
      return true;
    }

    try
    {
      if (!EnsureDispatcherQueue() || !EnsureCompositor())
      {
        return false;
      }

      winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget desktopTarget{nullptr};
      auto interop = m_compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();
      if (!interop)
      {
        return false;
      }

      winrt::check_hresult(interop->CreateDesktopWindowTarget(
          hwnd,
          false,
          reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget **>(winrt::put_abi(desktopTarget))));

      auto compositionTarget = desktopTarget.as<winrt::Windows::UI::Composition::CompositionTarget>();
      auto previousRoot = compositionTarget.Root();

      auto container = m_compositor.CreateContainerVisual();
      if (!container)
      {
        return false;
      }
      container.RelativeSizeAdjustment({1.0f, 1.0f});

      if (previousRoot)
      {
        // Keep the existing content by making it a child of our container.
        container.Children().InsertAtTop(previousRoot);
      }

      compositionTarget.Root(container);

      auto controller = winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController();
      bool supported = false;

      try
      {
        supported = controller.SetTarget(
            winrt::Microsoft::UI::WindowId{reinterpret_cast<uint64_t>(hwnd)},
            compositionTarget);
      }
      catch (...)
      {
        supported = false;
      }

      if (!supported)
      {
        compositionTarget.Root(previousRoot);
        return false;
      }

      m_hwnd = hwnd;
      m_compositionTarget = compositionTarget;
      m_root = container;
      m_originalRoot = previousRoot;
      m_controller = controller;
      m_enabled = true;
      return true;
    }
    catch (...)
    {
      Reset();
      return false;
    }
  }

  void Reset() noexcept
  {
    if (m_compositionTarget)
    {
      try
      {
        m_compositionTarget.Root(m_originalRoot);
      }
      catch (...)
      {
      }
    }

    m_controller = nullptr;
    m_root = nullptr;
    m_compositionTarget = nullptr;
    m_originalRoot = nullptr;
    m_hwnd = nullptr;
    m_enabled = false;
  }

private:
  bool EnsureDispatcherQueue() noexcept
  {
    using winrt::Windows::System::DispatcherQueue;
    if (DispatcherQueue::GetForCurrentThread())
    {
      return true;
    }

    if (!m_dispatcherQueueController)
    {
      DispatcherQueueOptions options{
          sizeof(DispatcherQueueOptions),
          DQTYPE_THREAD_CURRENT,
          DQTAT_COM_NONE};

      winrt::check_hresult(CreateDispatcherQueueController(
          options,
          reinterpret_cast<ABI::Windows::System::IDispatcherQueueController **>(
              winrt::put_abi(m_dispatcherQueueController))));
    }

    return static_cast<bool>(m_dispatcherQueueController);
  }

  bool EnsureCompositor() noexcept
  {
    if (!m_compositor)
    {
      m_compositor = winrt::Windows::UI::Composition::Compositor();
    }
    return static_cast<bool>(m_compositor);
  }

  winrt::Windows::System::DispatcherQueueController m_dispatcherQueueController{nullptr};
  winrt::Windows::UI::Composition::Compositor m_compositor{nullptr};
  winrt::Windows::UI::Composition::CompositionTarget m_compositionTarget{nullptr};
  winrt::Windows::UI::Composition::ContainerVisual m_root{nullptr}; // Keeps the wrapper container alive.
  winrt::Windows::UI::Composition::Visual m_originalRoot{nullptr};
  winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController m_controller{nullptr};
  HWND m_hwnd{nullptr};
  bool m_enabled{false};
};

PrimaryWindowMicaState &PrimaryMicaState() noexcept
{
  static PrimaryWindowMicaState state;
  return state;
}

bool TryEnablePrimaryWindowMica(winrt::Microsoft::UI::Windowing::AppWindow const &appWindow) noexcept
{
  return PrimaryMicaState().TryInitialize(appWindow);
}

struct PrimaryMicaBootstrapper
{
  void Ensure(winrt::Microsoft::UI::Windowing::AppWindow const &appWindow) noexcept
  {
    if (TryEnablePrimaryWindowMica(appWindow))
    {
      if (m_subscribed)
      {
        appWindow.Changed(m_changedToken);
        m_subscribed = false;
      }
      return;
    }

    if (m_subscribed)
    {
      return;
    }

    auto self = this;
    m_changedToken = appWindow.Changed([self](auto const &sender, auto const & /*args*/) {
      if (TryEnablePrimaryWindowMica(sender))
      {
        sender.Changed(self->m_changedToken);
        self->m_subscribed = false;
      }
    });
    m_subscribed = true;
  }

private:
  winrt::event_token m_changedToken{};
  bool m_subscribed{false};
};

PrimaryMicaBootstrapper &PrimaryMica() noexcept
{
  static PrimaryMicaBootstrapper bootstrapper;
  return bootstrapper;
}

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

  PrimaryMica().Ensure(appWindow);

  // Get the ReactViewOptions so we can set the initial RN component to load
  auto viewOptions{reactNativeWin32App.ReactViewOptions()};
  viewOptions.ComponentName(L"TestlibExample");

  // Start the app
  reactNativeWin32App.Start();
}
