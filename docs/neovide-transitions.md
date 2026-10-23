# Codex and Neovide themes with lightweight transitions

This fork adds 180 ms Windows Composition transitions directly to ExplorerTAP. It requires no separate animator DLL, launcher or framework.

## Codex Desktop

Codex Desktop is detected automatically by its Windows package family, `OpenAI.Codex_2p2nqsd0c76g0`; its executable is currently named `ChatGPT.exe`. The separate ChatGPT application is not matched.

When Codex is focused, its monitor's taskbar uses the theme's `surface` color. The border hides only while its window is maximized or fullscreen. Taskbar focus retains the preceding app; switching to another app or minimizing Codex restores the native appearance.

The fork reads only the desktop appearance fields from `%USERPROFILE%/.codex/config.toml` (or `CODEX_HOME/config.toml`): `appearanceTheme`, `appearanceLightChromeTheme.surface` and `appearanceDarkChromeTheme.surface`. Light, dark and system modes are supported. Default surfaces are `#ffffff` and `#181818`, matching Codex 26.1016.8120.0. The resolved base theme color is used, not a screenshot of Mica/transparency. Codex may change this internal configuration format or its defaults in future versions.

An existing native directory-change watcher reloads the settings after changes, and Windows setting notifications update system mode. Duplicate queued notifications are coalesced. No polling, screenshot capture, helper process or app modification is added. Invalid/incomplete TOML keeps the last valid theme. The TOML parser is the header-only [toml++ library](https://github.com/marzer/tomlplusplus), installed by vcpkg; the focused check downloads v3.4.0 and verifies its SHA-256.

## Behavior

- Opaque solid taskbar colors crossfade over the original Windows background. Changes between opaque colors animate their color; return to the native appearance fades the overlay away.
- The border fades instead of switching its fill to a transparent brush.
- Acrylic, translucent/clear and blur retain upstream rendering semantics. Windows 10/classic taskbars retain upstream behavior.
- Disabled Windows client-area animations are respected. Native restoration after TranslucentTB exits is immediate.
- Existing window events drive state changes; no polling or animation-frame timer is added. Finite animations run in the Windows compositor.

## Connect Neovide

1. Build and run this fork following [CONTRIBUTING.md](../CONTRIBUTING.md). The Store version does not understand the new theme message.
2. Disable/remove any previous standalone taskbar animator. Only one component should control the taskbar.
3. Copy [examples/neovide-taskbar.lua](../examples/neovide-taskbar.lua) to your Neovim configuration as `lua/neovide-taskbar.lua`.
4. Add `require("neovide-taskbar").setup()` after your theme is loaded.

While at least one Neovide instance is registered, the focused Neovide's monitor uses its current `Normal.bg`. The border hides only for a maximized/fullscreen Neovide. Taskbar focus retains the preceding application context. Other apps and minimized Neovide use native Windows appearance. After the last instance unregisters or exits, the normal TranslucentTB configuration resumes. Multiple instances register their own parent PID; kernel process waits remove crashed/exited clients without polling.

Theme updates use `ColorScheme`, NvChad's `NvThemeReload`, startup and regained focus. No JSON file writes or resident helper process are needed. A missing/transparent `Normal.bg` unregisters the instance instead of inventing a background.

`:NeovideTaskbarDisable` unregisters the current instance; `:NeovideTaskbarEnable` registers it again.

## IPC

The existing hidden top-level `TTB_WorkerWindow` accepts `WM_COPYDATA` with `dwData = 0x4e544241` and exactly four 32-bit unsigned fields: version (1), Neovide parent PID, RGB (`0xRRGGBB`) and enabled (0/1). The process path must end in `neovide.exe` or a dated update backup `neovide.previous-YYYYMMDD.exe` before registration is accepted. This keeps already-open Neovide windows working when an executable is renamed during an update. Unregistration removes only that PID. Process-wait notifications carry a generation token to reject stale exit messages. The IPC is acknowledged before the renderer is called.

## Verification

Run `pwsh -NoProfile -File scripts/check-neovide.ps1`. This compiles the exact C++/WinRT transition implementation and Codex helper, and checks Codex package identity, TOML appearance fields, default/system/custom modes, malformed settings, packet validation, black backgrounds, taskbar focus context, minimize, maximize and visibility policies. It does not build the full application or prove visual rendering.

With Neovim installed, run `nvim --headless -u NONE -l Tests/neovide_ipc.lua` for the example's IPC, theme events, disable/enable commands and absence of polling timers.

The transition technique was previously exercised in the standalone animator on Windows build 26300, including theme/fullscreen transitions and idle counters. This fork needs its own full application build and runtime validation; the standalone result is not a runtime test of the fork.
