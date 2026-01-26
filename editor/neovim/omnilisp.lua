-- OmniLisp Neovim Integration
-- Simple REPL connection over TCP socket
--
-- Usage:
--   1. Start OmniLisp server: omnilisp -S 5555
--   2. In neovim: :lua require('omnilisp').connect()
--   3. Evaluate: <leader>ee (current expression) or <leader>eb (buffer)
--
-- Or add to your init.lua:
--   require('omnilisp').setup({ port = 5555, auto_connect = true })

local M = {}

M.config = {
  host = 'localhost',
  port = 5555,
  timeout = 5000,  -- ms
}

M.state = {
  connected = false,
  channel = nil,
}

-- Connect to OmniLisp server
function M.connect()
  if M.state.connected then
    vim.notify("OmniLisp: Already connected", vim.log.levels.INFO)
    return true
  end

  local addr = string.format("tcp:%s:%d", M.config.host, M.config.port)

  local ok, channel = pcall(vim.fn.sockconnect, 'tcp', M.config.host .. ':' .. M.config.port, {
    on_data = function(_, data, _)
      M.handle_response(data)
    end,
  })

  if not ok or channel == 0 then
    vim.notify("OmniLisp: Failed to connect to " .. addr, vim.log.levels.ERROR)
    return false
  end

  M.state.channel = channel
  M.state.connected = true
  vim.notify("OmniLisp: Connected to " .. addr, vim.log.levels.INFO)
  return true
end

-- Disconnect from server
function M.disconnect()
  if M.state.channel then
    vim.fn.chanclose(M.state.channel)
    M.state.channel = nil
    M.state.connected = false
    vim.notify("OmniLisp: Disconnected", vim.log.levels.INFO)
  end
end

-- Response buffer for accumulating multi-line responses
M.response_buffer = ""

-- Handle response from server
function M.handle_response(data)
  for _, chunk in ipairs(data) do
    M.response_buffer = M.response_buffer .. chunk
  end

  -- Check for end-of-response marker (null + newline)
  local null_pos = M.response_buffer:find("\0\n")
  if null_pos then
    local result = M.response_buffer:sub(1, null_pos - 1)
    M.response_buffer = M.response_buffer:sub(null_pos + 2)

    -- Display result
    M.show_result(result)
  end
end

-- Show result in floating window or virtual text
function M.show_result(result)
  -- Option 1: Echo (simple)
  -- vim.api.nvim_echo({{result, "Normal"}}, true, {})

  -- Option 2: Floating window
  local buf = vim.api.nvim_create_buf(false, true)
  local lines = vim.split(result, "\n")
  vim.api.nvim_buf_set_lines(buf, 0, -1, false, lines)

  local width = 0
  for _, line in ipairs(lines) do
    width = math.max(width, #line)
  end
  width = math.min(width + 2, 80)
  local height = math.min(#lines, 10)

  local win = vim.api.nvim_open_win(buf, false, {
    relative = 'cursor',
    row = 1,
    col = 0,
    width = width,
    height = height,
    style = 'minimal',
    border = 'rounded',
  })

  -- Auto-close after delay
  vim.defer_fn(function()
    if vim.api.nvim_win_is_valid(win) then
      vim.api.nvim_win_close(win, true)
    end
  end, 3000)
end

-- Send expression to server
function M.send(expr)
  if not M.state.connected then
    if not M.connect() then
      return
    end
  end

  -- Send expression with newline
  vim.fn.chansend(M.state.channel, expr .. "\n")
end

-- Evaluate current s-expression under cursor
function M.eval_expr()
  -- Simple: get current line
  -- TODO: Proper s-expression detection with paredit-style navigation
  local line = vim.api.nvim_get_current_line()
  M.send(line)
end

-- Evaluate visual selection
function M.eval_visual()
  local start_pos = vim.fn.getpos("'<")
  local end_pos = vim.fn.getpos("'>")
  local lines = vim.api.nvim_buf_get_lines(0, start_pos[2] - 1, end_pos[2], false)

  if #lines == 0 then return end

  -- Trim selection within lines
  if #lines == 1 then
    lines[1] = lines[1]:sub(start_pos[3], end_pos[3])
  else
    lines[1] = lines[1]:sub(start_pos[3])
    lines[#lines] = lines[#lines]:sub(1, end_pos[3])
  end

  local expr = table.concat(lines, "\n")
  M.send(expr)
end

-- Evaluate entire buffer
function M.eval_buffer()
  local lines = vim.api.nvim_buf_get_lines(0, 0, -1, false)
  local expr = table.concat(lines, "\n")
  M.send(expr)
end

-- Ping server (keepalive check)
function M.ping()
  M.send(":ping")
end

-- Setup function
function M.setup(opts)
  opts = opts or {}
  M.config = vim.tbl_deep_extend("force", M.config, opts)

  -- Set up keymaps
  vim.keymap.set('n', '<leader>oc', M.connect, { desc = 'OmniLisp: Connect' })
  vim.keymap.set('n', '<leader>od', M.disconnect, { desc = 'OmniLisp: Disconnect' })
  vim.keymap.set('n', '<leader>ee', M.eval_expr, { desc = 'OmniLisp: Eval line' })
  vim.keymap.set('v', '<leader>ee', M.eval_visual, { desc = 'OmniLisp: Eval selection' })
  vim.keymap.set('n', '<leader>eb', M.eval_buffer, { desc = 'OmniLisp: Eval buffer' })

  -- Auto-connect if configured
  if opts.auto_connect then
    vim.defer_fn(function()
      M.connect()
    end, 100)
  end

  -- Create user commands
  vim.api.nvim_create_user_command('OmniConnect', M.connect, {})
  vim.api.nvim_create_user_command('OmniDisconnect', M.disconnect, {})
  vim.api.nvim_create_user_command('OmniEval', function(args)
    M.send(args.args)
  end, { nargs = '+' })

  vim.notify("OmniLisp: Plugin loaded. Use :OmniConnect to connect.", vim.log.levels.INFO)
end

return M
