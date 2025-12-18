#pragma once

#include <winrt/Windows.UI.Composition.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>

struct AcrylicWindow {
	struct AcrylicWindowData {
		winrt::Windows::UI::Composition::Visual rootVisual{ nullptr };
		winrt::Windows::UI::Composition::CompositionTarget compositionTarget{ nullptr };
		winrt::Microsoft::UI::Composition::SystemBackdrops::DesktopAcrylicController controller{ nullptr };
		winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration configuration{ nullptr };
		bool isSupported{ false };
	};

	static AcrylicWindowData applyAcrylic(const winrt::Windows::UI::Composition::Compositor& compositor, HWND window) noexcept;
};
