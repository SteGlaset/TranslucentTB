#pragma once
#include <charconv>
#include <cstdint>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#define TOML_ENABLE_FORMATTERS 0
#ifdef _MSC_VER
#pragma warning(push)
// toml++ 3.4 uses whitespace in literal-operator declarations, deprecated by C++23.
#pragma warning(disable: 5311)
#endif
#include <toml++/toml.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <windows.h>
#include <appmodel.h>

namespace CodexTheme
{
	inline constexpr unsigned int RefreshMessage = 0x83a2;
	struct Settings
	{
		std::string Mode = "system";
		std::uint32_t Light = 0xffffff, Dark = 0x181818;
		std::uint32_t Resolve(bool systemDark) const noexcept
		{
			return Mode == "dark" || (Mode == "system" && systemDark) ? Dark : Light;
		}
	};
	inline std::optional<std::uint32_t> Color(std::string_view value)
	{
		if (value.size() != 7 || value.front() != '#') return std::nullopt;
		std::uint32_t rgb{};
		const auto result = std::from_chars(value.data() + 1, value.data() + value.size(), rgb, 16);
		if (result.ec != std::errc{} || result.ptr != value.data() + value.size()) return std::nullopt;
		return rgb;
	}
	inline std::optional<Settings> Parse(std::string_view text)
	{
		try
		{
			const auto config = toml::parse(text);
			const auto desktop = config["desktop"];
			Settings settings;
			if (const auto mode = desktop["appearanceTheme"].value<std::string>(); mode && (*mode == "system" || *mode == "light" || *mode == "dark"))
				settings.Mode = *mode;
			if (const auto surface = desktop["appearanceLightChromeTheme"]["surface"].value<std::string>())
				if (auto color = Color(*surface)) settings.Light = *color;
			if (const auto surface = desktop["appearanceDarkChromeTheme"]["surface"].value<std::string>())
				if (auto color = Color(*surface)) settings.Dark = *color;
			return settings;
		}
		catch (const toml::parse_error&) { return std::nullopt; }
	}
	inline bool IsPackage(std::wstring_view family) noexcept
	{
		return family == L"OpenAI.Codex_2p2nqsd0c76g0";
	}
	inline bool IsWindow(HWND window) noexcept
	{
		DWORD pid{};
		GetWindowThreadProcessId(window, &pid);
		const HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		if (!process) return false;
		wchar_t family[128]{};
		UINT32 length = static_cast<UINT32>(std::size(family));
		const LONG result = GetPackageFamilyName(process, &length, family);
		CloseHandle(process);
		return result == ERROR_SUCCESS && IsPackage(family);
	}
}
