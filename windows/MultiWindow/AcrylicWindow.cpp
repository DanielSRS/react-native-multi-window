#include "pch.h"

#include "AcrylicWindow.h"

#include <winrt/Windows.UI.Composition.Desktop.h>

#include <windows.ui.composition.interop.h>

namespace winrt {
	using namespace Microsoft::UI::Composition::SystemBackdrops;
	using namespace Windows::UI::Composition;
}

AcrylicWindow::AcrylicWindowData AcrylicWindow::applyAcrylic(const winrt::Windows::UI::Composition::Compositor& compositor, HWND window) noexcept {
	AcrylicWindowData result{};

	try {
		if (!compositor || !window) {
			return result;
		}

		auto interop = compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();

		winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget desktopTarget{ nullptr };
		winrt::check_hresult(interop->CreateDesktopWindowTarget(
			window,
			false,
			reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget**>(winrt::put_abi(desktopTarget))));

		auto root = compositor.CreateContainerVisual();
		if (!root) {
			return result;
		}
		root.RelativeSizeAdjustment({ 1.0f, 1.0f });
		desktopTarget.Root(root);

		auto compositionTarget = desktopTarget.as<winrt::Windows::UI::Composition::CompositionTarget>();

		auto controller = winrt::Microsoft::UI::Composition::SystemBackdrops::DesktopAcrylicController();
		auto configuration = winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration();
		configuration.IsInputActive(true);
		configuration.Theme(winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Default);
		//controller.SetSystemBackdropConfiguration(configuration);

		bool supported = controller.SetTarget(
			winrt::Microsoft::UI::WindowId{ reinterpret_cast<uint64_t>(window) },
			compositionTarget);

		result.rootVisual = root;
		result.compositionTarget = compositionTarget;
		result.controller = controller;
		result.configuration = configuration;
		result.isSupported = supported;
	}
	catch (...) {
		// Leave defaults in result on failure
	}
	return result;
}
