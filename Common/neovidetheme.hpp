#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace NeovideTheme
{
	inline constexpr std::uintptr_t CopyDataId = 0x4e544241;
	inline constexpr unsigned int ProcessExitedMessage = 0x83a0;
	inline constexpr unsigned int RefreshMessage = 0x83a1;
	struct Packet { std::uint32_t Version, ProcessId, Rgb, Enabled; };
	inline bool IsExecutable(std::wstring_view filename)
	{
		std::wstring name(filename);
		for (auto &character : name)
			if (character >= L'A' && character <= L'Z') character += L'a' - L'A';
		if (name == L"neovide.exe") return true;
		constexpr std::wstring_view prefix = L"neovide.previous-";
		if (name.size() != prefix.size() + 8 + 4 || !name.starts_with(prefix) || !name.ends_with(L".exe")) return false;
		for (std::size_t i = prefix.size(); i < prefix.size() + 8; ++i)
			if (name[i] < L'0' || name[i] > L'9') return false;
		return true;
	}
	constexpr bool Valid(const Packet &packet) noexcept
	{
		return packet.Version == 1 && packet.ProcessId != 0 && packet.Rgb <= 0xffffff && packet.Enabled <= 1;
	}
	struct FocusContext
	{
		std::uintptr_t LastApplication = 0;
		constexpr std::uintptr_t Observe(std::uintptr_t window, bool taskbar) noexcept
		{
			if (!taskbar) LastApplication = window;
			return LastApplication;
		}
	};
	struct Appearance { bool Tint, HideBorder; };
	constexpr Appearance Select(bool registered, bool visible, bool minimized, bool maximized) noexcept
	{
		const bool tint = registered && visible && !minimized;
		return { tint, tint && maximized };
	}
}
