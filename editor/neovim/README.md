# OmniLisp Neovim Integration

Send OmniLisp expressions to a running server and display results inline.

## Quick Start

1. **Start the OmniLisp server:**
   ```bash
   omnilisp -S 5555
   ```

2. **Add to your neovim config:**
   ```lua
   -- Option A: Direct require (if in runtimepath)
   require('omnilisp').setup({
     port = 5555,
     auto_connect = true,
   })

   -- Option B: Manual path
   vim.opt.runtimepath:append('/path/to/hvm-omnilisp/editor/neovim')
   require('omnilisp').setup()
   ```

3. **Use the keybindings:**
   - `<leader>oc` - Connect to server
   - `<leader>od` - Disconnect
   - `<leader>ee` - Evaluate current line (normal mode)
   - `<leader>ee` - Evaluate selection (visual mode)
   - `<leader>eb` - Evaluate entire buffer

## Commands

- `:OmniConnect` - Connect to server
- `:OmniDisconnect` - Disconnect
- `:OmniEval (+ 1 2)` - Evaluate expression directly

## Protocol

The server uses a simple line-based protocol:

```
Client: <expression>\n
Server: <result>\x00\n
```

The null byte (`\x00`) followed by newline marks the end of a response,
allowing multi-line results.

## Testing with netcat

```bash
# Terminal 1: Start server
omnilisp -S 5555

# Terminal 2: Test with netcat
echo '(+ 1 2)' | nc localhost 5555
# Output: 3

# Interactive session
nc localhost 5555
(+ 1 2)
3
(* 6 7)
42
```

## Configuration

```lua
require('omnilisp').setup({
  host = 'localhost',  -- Server host
  port = 5555,         -- Server port
  auto_connect = true, -- Connect on startup
})
```

## Troubleshooting

**Connection refused:**
- Make sure `omnilisp -S 5555` is running
- Check the port number matches

**No response:**
- Check the server terminal for errors
- Try `:ping` to test the connection

**Result not showing:**
- The floating window auto-closes after 3 seconds
- Check `:messages` for any errors
