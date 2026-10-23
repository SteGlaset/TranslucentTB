#include <cassert>
#include <cstdlib>
#include "../Common/neovidetheme.hpp"
#include "../Common/codextheme.hpp"
#include "../ExplorerTAP/taskbartransition.hpp"

int main(int argc, char **argv)
{
	if (argc == 3 && std::string_view(argv[1]) == "--codex-window")
		assert(CodexTheme::IsWindow(reinterpret_cast<HWND>(std::strtoull(argv[2], nullptr, 10))));
	assert(CodexTheme::IsPackage(L"OpenAI.Codex_2p2nqsd0c76g0"));
	assert(!CodexTheme::IsPackage(L"OpenAI.ChatGPT-Desktop_2p2nqsd0c76g0"));
	assert(!CodexTheme::IsWindow(nullptr));
	const auto defaults = CodexTheme::Parse("");
	assert(defaults && defaults->Resolve(true) == 0x181818 && defaults->Resolve(false) == 0xffffff);
	const auto custom = CodexTheme::Parse(R"(
[desktop]
appearanceTheme = 'dark'
appearanceLightChromeTheme = { surface = "#aBcD12" }
[desktop.appearanceDarkChromeTheme]
surface = "#000000" # black is valid
)");
	assert(custom && custom->Resolve(false) == 0);
	assert(custom->Light == 0xabcd12);
	assert(!CodexTheme::Parse("[desktop\nappearanceTheme="));
	assert(!CodexTheme::Color("#12345678"));
	assert(!CodexTheme::Color("#12gg34"));
	const auto light = CodexTheme::Parse("[desktop]\nappearanceTheme = 'light'");
	assert(light && light->Resolve(true) == 0xffffff);
	const auto system = CodexTheme::Parse(R"(
[desktop]
appearanceTheme = "system"
appearanceLightChromeTheme.surface = "#f0e0d0"
appearanceDarkChromeTheme.surface = "#102030"
)");
	assert(system && system->Resolve(false) == 0xf0e0d0 && system->Resolve(true) == 0x102030);
	using namespace NeovideTheme;
	assert(IsExecutable(L"neovide.exe"));
	assert(IsExecutable(L"NEOVIDE.EXE"));
	assert(IsExecutable(L"neovide.previous-20261023.exe"));
	assert(!IsExecutable(L"neovide.previous-unrelated.exe"));
	assert(!IsExecutable(L"neovide.previous-20261023.exe.bad"));
	assert(!IsExecutable(L"ChatGPT.exe"));
	static_assert(sizeof(Packet) == 16);
	assert(Valid({ 1, 123, 0, 1 })); // black is a valid theme
	assert(!Valid({ 2, 123, 0, 1 }));
	assert(!Valid({ 1, 0, 0, 1 }));
	assert(!Valid({ 1, 123, 0x1000000, 1 }));
	assert(!Valid({ 1, 123, 0, 2 }));
	FocusContext context;
	assert(context.Observe(10, false) == 10);
	assert(context.Observe(20, true) == 10);
	assert(context.Observe(30, false) == 30);
	assert(context.Observe(20, true) == 30);
	assert(Select(true, true, false, false).Tint);
	assert(!Select(true, true, false, false).HideBorder);
	assert(Select(true, true, false, true).HideBorder);
	assert(!Select(true, true, true, true).Tint);
	assert(!Select(false, true, false, true).Tint);
	assert(!Select(true, false, false, true).Tint);
	TaskbarTransition transition; // compile the exact C++/WinRT implementation
	transition.Hide();
	transition.Reset();
}
