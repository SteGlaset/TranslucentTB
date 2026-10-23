-- Save as lua/neovide-taskbar.lua; call require("neovide-taskbar").setup().
-- Requires this fork of TranslucentTB to be running. No helper process or timer.
local M = {}
function M.setup()
  if not vim.g.neovide or vim.fn.has("win32") == 0 then return end
  local ffi = require("ffi")
  if not pcall(ffi.typeof, "TTB_NeovidePacket") then
    ffi.cdef([[
      typedef struct { uint32_t version, pid, rgb, enabled; } TTB_NeovidePacket;
      typedef struct { uintptr_t dwData; uint32_t cbData; const void *lpData; } TTB_NeovideCopyData;
      void *FindWindowExW(void *, void *, const uint16_t *, const uint16_t *);
      intptr_t SendMessageTimeoutW(void *, unsigned int, uintptr_t, intptr_t, unsigned int, unsigned int, uintptr_t *);
    ]])
  end
  local user32 = ffi.load("user32")
  local name = "TTB_WorkerWindow"
  local wide = ffi.new("uint16_t[?]", #name + 1)
  for i = 1, #name do wide[i - 1] = name:byte(i) end
  local enabled = true
  local function publish(leaving)
    local window = user32.FindWindowExW(nil, nil, wide, nil)
    if window == nil then return false end
    local bg = vim.api.nvim_get_hl(0, { name = "Normal", link = false }).bg
    local packet = ffi.new("TTB_NeovidePacket[1]")
    packet[0].version, packet[0].pid, packet[0].rgb = 1, vim.uv.os_getppid(), bg or 0
    packet[0].enabled = not leaving and bg ~= nil and 1 or 0
    local copy = ffi.new("TTB_NeovideCopyData[1]")
    copy[0].dwData, copy[0].cbData, copy[0].lpData = 0x4e544241, ffi.sizeof(packet), packet
    local result = ffi.new("uintptr_t[1]")
    return user32.SendMessageTimeoutW(window, 0x4a, 0, ffi.cast("intptr_t", copy), 2, 100, result) ~= 0 and result[0] ~= 0
  end
  local function sync()
    if enabled and not publish(false) then
      vim.notify_once("Neovide taskbar: start the TranslucentTB fork with Neovide integration", vim.log.levels.WARN)
    end
  end
  local group = vim.api.nvim_create_augroup("TranslucentTBNeovide", { clear = true })
  vim.api.nvim_create_autocmd({ "ColorScheme", "FocusGained" }, {
    group = group, callback = function() vim.schedule(sync) end,
  })
  vim.api.nvim_create_autocmd("User", {
    group = group, pattern = "NvThemeReload", callback = function() vim.schedule(sync) end,
  })
  vim.api.nvim_create_autocmd("VimLeavePre", {
    group = group, callback = function() enabled = false; publish(true) end,
  })
  vim.api.nvim_create_user_command("NeovideTaskbarDisable", function() enabled = false; publish(true) end, { force = true })
  vim.api.nvim_create_user_command("NeovideTaskbarEnable", function() enabled = true; sync() end, { force = true })
  vim.schedule(sync)
end
return M
