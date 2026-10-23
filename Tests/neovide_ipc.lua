-- nvim --headless -u NONE -l Tests/neovide_ipc.lua
local original_ffi = require("ffi")
local original_neovide, original_notify = vim.g.neovide, vim.notify_once
local packets, warnings = {}, 0
local available = true
local timers = #vim.fn.timer_info()
package.loaded.ffi = {
  typeof = function() error("undeclared") end,
  cdef = function() end,
  new = function(name) return name:match("^TTB_") and { [0] = {} } or {} end,
  cast = function(_, value) return value end,
  sizeof = function() return 16 end,
  load = function()
    return {
      FindWindowExW = function(parent, _, class)
        assert(parent == nil, "target the existing top-level TTB worker")
        local chars = {}
        for i = 0, 15 do if class[i] then chars[#chars + 1] = string.char(class[i]) end end
        assert(table.concat(chars) == "TTB_WorkerWindow")
        return available and 1 or nil
      end,
      SendMessageTimeoutW = function(_, message, _, copy, _, _, result)
        assert(message == 0x4a and copy[0].dwData == 0x4e544241 and copy[0].cbData == 16)
        local packet = copy[0].lpData[0]
        packets[#packets + 1] = { rgb = packet.rgb, enabled = packet.enabled, pid = packet.pid }
        result[0] = 1
        return 1
      end,
    }
  end,
}
vim.g.neovide = true
vim.notify_once = function() warnings = warnings + 1 end
local function settle() vim.wait(25, function() return false end, 5) end
local ok, err = pcall(function()
  vim.api.nvim_set_hl(0, "Normal", { bg = "#1f1f28" })
  dofile("examples/neovide-taskbar.lua").setup()
  settle()
  assert(packets[#packets].rgb == 0x1f1f28 and packets[#packets].enabled == 1)
  assert(packets[#packets].pid == vim.uv.os_getppid())
  vim.api.nvim_set_hl(0, "Normal", { bg = "#000000" })
  vim.api.nvim_exec_autocmds("User", { pattern = "NvThemeReload" })
  settle()
  assert(packets[#packets].rgb == 0 and packets[#packets].enabled == 1)
  vim.cmd.NeovideTaskbarDisable()
  vim.api.nvim_exec_autocmds("FocusGained", {})
  settle()
  assert(packets[#packets].enabled == 0)
  vim.cmd.NeovideTaskbarEnable()
  assert(packets[#packets].enabled == 1)
  vim.api.nvim_set_hl(0, "Normal", { bg = "NONE" })
  vim.api.nvim_exec_autocmds("ColorScheme", {})
  settle()
  assert(packets[#packets].enabled == 0)
  available = false
  vim.api.nvim_exec_autocmds("FocusGained", {})
  settle()
  assert(warnings == 1)
  assert(#vim.fn.timer_info() == timers, "no periodic polling")
end)
vim.api.nvim_del_augroup_by_name("TranslucentTBNeovide")
vim.api.nvim_del_user_command("NeovideTaskbarDisable")
vim.api.nvim_del_user_command("NeovideTaskbarEnable")
package.loaded.ffi = original_ffi
vim.g.neovide, vim.notify_once = original_neovide, original_notify
assert(ok, err)
print("Event-only TranslucentTB Neovide IPC checks passed")
