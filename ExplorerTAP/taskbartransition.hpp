#pragma once
#include <chrono>
#include <cstdint>
#include <optional>
#include <windows.h>
#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>

// Finite compositor animations only. No timers, rendering loop or idle callbacks.
class TaskbarTransition
{
private:
	using Visual = winrt::Windows::UI::Composition::Visual;
	using Preview = winrt::Windows::UI::Xaml::Hosting::ElementCompositionPreview;
	winrt::Windows::UI::Xaml::Shapes::Shape m_Control{ nullptr };
	winrt::Windows::UI::Composition::SpriteVisual m_Overlay{ nullptr };
	winrt::Windows::UI::Composition::CompositionColorBrush m_Brush{ nullptr };
	std::optional<std::uint32_t> m_Color;
	bool m_Visible = false;

public:
	static bool AnimationsEnabled() noexcept
	{
		BOOL enabled = TRUE;
		SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &enabled, 0);
		return enabled != FALSE;
	}
	static void Opacity(const Visual &visual, float target)
	{
		if (!AnimationsEnabled())
		{
			visual.StopAnimation(L"Opacity");
			visual.Opacity(target);
			return;
		}
		auto animation = visual.Compositor().CreateScalarKeyFrameAnimation();
		animation.Duration(std::chrono::milliseconds(180));
		animation.InsertExpressionKeyFrame(0, L"this.StartingValue");
		animation.InsertKeyFrame(1, target);
		animation.StopBehavior(winrt::Windows::UI::Composition::AnimationStopBehavior::SetToFinalValue);
		visual.StartAnimation(L"Opacity", animation);
	}
	bool SolidColor(const winrt::Windows::UI::Xaml::Shapes::Shape &control,
		const winrt::Windows::UI::Xaml::Media::Brush &original, winrt::Windows::UI::Color color)
	{
		if (color.A != 255) return false; // preserve existing clear/acrylic/blur semantics
		if (!m_Overlay)
		{
			if (Preview::GetElementChildVisual(control)) return false;
			auto compositor = Preview::GetElementVisual(control).Compositor();
			m_Control = control;
			m_Overlay = compositor.CreateSpriteVisual();
			m_Brush = compositor.CreateColorBrush();
			m_Overlay.Brush(m_Brush);
			m_Overlay.RelativeSizeAdjustment({ 1, 1 });
			m_Overlay.Opacity(0);
			Preview::SetElementChildVisual(control, m_Overlay);
		}
		const auto rgb = (static_cast<std::uint32_t>(color.R) << 16) | (static_cast<std::uint32_t>(color.G) << 8) | color.B;
		if (m_Visible && m_Color == rgb) return true;
		if (!m_Visible)
		{
			control.Fill(original);
			m_Brush.StopAnimation(L"Color");
			m_Brush.Color(color);
			Opacity(m_Overlay, 1);
		}
		else if (AnimationsEnabled())
		{
			auto animation = m_Brush.Compositor().CreateColorKeyFrameAnimation();
			animation.Duration(std::chrono::milliseconds(180));
			animation.InsertExpressionKeyFrame(0, L"this.StartingValue");
			animation.InsertKeyFrame(1, color);
			animation.StopBehavior(winrt::Windows::UI::Composition::AnimationStopBehavior::SetToFinalValue);
			m_Brush.StartAnimation(L"Color", animation);
		}
		else
		{
			m_Brush.StopAnimation(L"Color");
			m_Brush.Color(color);
		}
		m_Color = rgb;
		m_Visible = true;
		return true;
	}
	void Hide()
	{
		if (m_Overlay && m_Visible) Opacity(m_Overlay, 0);
		m_Visible = false;
	}
	void Reset()
	{
		if (!m_Overlay) return;
		m_Overlay.StopAnimation(L"Opacity");
		m_Brush.StopAnimation(L"Color");
		m_Overlay.Opacity(0);
		if (Preview::GetElementChildVisual(m_Control) == m_Overlay) Preview::SetElementChildVisual(m_Control, nullptr);
		m_Overlay = nullptr;
		m_Brush = nullptr;
		m_Control = nullptr;
		m_Color.reset();
		m_Visible = false;
	}
};
