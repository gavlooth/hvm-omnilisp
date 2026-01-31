# OmniLisp Quickstart Guide

Get up and running with OmniLisp in minutes. This guide covers installation, the REPL, editor integration, and your first programs.

## Installation

### Prerequisites

- C compiler (gcc or clang)
- Make
- HVM4 runtime (included as submodule)

### Build

```bash
git clone https://github.com/your-org/hvm-omnilisp.git
cd hvm-omnilisp
make
```

### Verify

```bash
./omnilisp -e "(+ 1 2)"
# Output: 3
```

---

## Running OmniLisp

### Command Line Options

```
Usage: omnilisp [options] [file.ol]

Options:
  -e EXPR       Evaluate expression directly
  -i            Interactive REPL
  -S PORT       Start socket server (for editors)
  -p            Parse only (show AST)
  -c            Compile only (emit HVM4)
  -o FILE       Output file for compilation
  -d            Debug mode
  -h            Show help
```

### Quick Examples

```bash
# Evaluate an expression
./omnilisp -e "(* 6 7)"

# Run a file
./omnilisp program.ol

# Start interactive REPL
./omnilisp -i

# Start socket server for editor integration
./omnilisp -S 5555
```

---

## Interactive REPL

Start the REPL:

```bash
./omnilisp -i
```

You'll see:

```
OmniLisp REPL v0.1.0
Type expressions to evaluate. Ctrl+D to exit.

λ>
```

### Basic Usage

```lisp
λ> (+ 1 2)
3

λ> (define square [x] (* x x))
#<function square>

λ> (square 5)
25

λ> (map square '(1 2 3 4 5))
(1 4 9 16 25)
```

### REPL Commands

| Command | Action |
|---------|--------|
| `:q` or `:quit` | Exit the REPL |
| `:h` or `:help` | Show help |
| `:c` or `:clear` | Clear screen |
| `Ctrl+D` | Exit (EOF) |

### Multi-line Input

The REPL reads complete s-expressions. If you start a parenthesis, continue until it's balanced:

```lisp
λ> (define factorial
     0  1
     n  (* n (factorial (- n 1))))
#<function factorial>

λ> (factorial 10)
3628800
```

---

## Socket Server (nREPL-like)

For editor integration, OmniLisp provides a TCP socket server.

### Starting the Server

```bash
./omnilisp -S 5555
```

Output:

```
OmniLisp server listening on port 5555
Connect with: nc localhost 5555
Press Ctrl+C to stop.
```

### Protocol

Simple text-based protocol:
- **Send**: Expression followed by newline (`\n`)
- **Receive**: Result followed by null byte and newline (`\x00\n`)

The null byte delimiter allows multi-line results.

### Testing with netcat

```bash
# In another terminal:
echo '(+ 1 2)' | nc localhost 5555
# Output: 3

# Interactive session:
nc localhost 5555
(+ 10 20)
30
(map (lambda [x] (* x x)) '(1 2 3))
(1 4 9)
^C
```

### Special Commands

| Command | Response |
|---------|----------|
| `:ping` | `pong` (keepalive check) |

---

## Editor Integration

### Neovim

OmniLisp includes a Neovim plugin for interactive development.

#### Setup

1. Copy the plugin to your Neovim config:

```bash
mkdir -p ~/.config/nvim/lua
cp editor/neovim/omnilisp.lua ~/.config/nvim/lua/
```

2. Add to your `init.lua`:

```lua
require('omnilisp').setup({
  port = 5555,
  auto_connect = false,  -- set to true to connect on startup
})
```

#### Usage

1. Start the OmniLisp server in a terminal:

```bash
./omnilisp -S 5555
```

2. In Neovim, connect:

```vim
:OmniConnect
```

#### Keybindings

| Key | Mode | Action |
|-----|------|--------|
| `<leader>oc` | Normal | Connect to server |
| `<leader>od` | Normal | Disconnect |
| `<leader>ee` | Normal | Evaluate current line |
| `<leader>ee` | Visual | Evaluate selection |
| `<leader>eb` | Normal | Evaluate entire buffer |

#### Commands

```vim
:OmniConnect       " Connect to server
:OmniDisconnect    " Disconnect
:OmniEval (+ 1 2)  " Evaluate expression
```

#### Result Display

Results appear in a floating window near the cursor, auto-closing after 3 seconds.

### VS Code / Other Editors

Use the socket server with any editor that supports TCP connections:

```
Host: localhost
Port: 5555
Protocol: Send expression + newline, read until null byte
```

---

## Your First Program

### hello.ol

```lisp
;; hello.ol - Your first OmniLisp program

;; Print a greeting
(println "Hello, OmniLisp!")

;; Define a function
(define greet [name]
  (str "Hello, " name "!"))

;; Use it
(println (greet "World"))

;; Work with lists
(define nums '(1 2 3 4 5))
(println (str "Sum: " (foldl + 0 nums)))
(println (str "Squares: " (map (lambda [x] (* x x)) nums)))
```

Run it:

```bash
./omnilisp hello.ol
```

Output:

```
Hello, OmniLisp!
Hello, World!
Sum: 15
Squares: (1 4 9 16 25)
```

---

## Quick Syntax Reference

### Functions

```lisp
;; Define a function (curried parameters)
(define add [x] [y] (+ x y))

;; With types
(define add [x {Int}] [y {Int}] {Int} (+ x y))

;; Call
(add 3 5)  ;; -> 8

;; Partial application
(define add5 (add 5))
(add5 10)  ;; -> 15
```

### Pattern Matching

```lisp
;; Basic match (flat pattern-result pairs)
(match value
  0           "zero"
  1           "one"
  x & (> x 0) "positive"
  _           "other")

;; Pattern-based function
(define length
  ()       0
  (_ .. t) (+ 1 (length t)))

;; Or patterns
(match day
  (or "Saturday" "Sunday") "weekend"
  _                        "weekday")
```

### Collections

```lisp
;; Lists (linked, quoted)
'(1 2 3 4 5)

;; Arrays (indexed, brackets)
[1 2 3 4 5]

;; Dicts (hash maps)
#{"name" "Alice" "age" 30}
```

### Effects

```lisp
;; Handle effects instead of try/catch
(handle
  (if (< x 0)
      (perform fail "negative")
      (sqrt x))
  (fail [msg resume]
    (println msg)
    0))
```

### Named Let (Iteration)

```lisp
;; Sum 0 to 99
(let loop [[i 0] [sum 0]]
  (if (>= i 100)
      sum
      (loop (+ i 1) (+ sum i))))
;; -> 4950
```

---

## Common Patterns

### Map/Filter/Fold

```lisp
;; Transform
(map (lambda [x] (* x x)) '(1 2 3 4 5))
;; -> (1 4 9 16 25)

;; Filter
(filter even? '(1 2 3 4 5 6 7 8))
;; -> (2 4 6 8)

;; Reduce
(foldl + 0 '(1 2 3 4 5))
;; -> 15
```

### FizzBuzz

```lisp
(define fizzbuzz [n]
  (match n
    _ & (= (mod n 15) 0) "FizzBuzz"
    _ & (= (mod n 3) 0)  "Fizz"
    _ & (= (mod n 5) 0)  "Buzz"
    _                     (int->str n)))

(map fizzbuzz (range 1 16))
```

### Factorial

```lisp
(define factorial
  0  1
  n  (* n (factorial (- n 1))))

(factorial 10)  ;; -> 3628800
```

### Fibonacci

```lisp
(define fib
  0  0
  1  1
  n  (+ (fib (- n 1)) (fib (- n 2))))

(fib 10)  ;; -> 55
```

### Quicksort

```lisp
(define quicksort
  ()             '()
  (pivot .. rest)
    (append (quicksort (filter (lambda [x] (< x pivot)) rest))
            (cons pivot (quicksort (filter (lambda [x] (>= x pivot)) rest)))))

(quicksort '(3 1 4 1 5 9 2 6))
;; -> (1 1 2 3 4 5 6 9)
```

---

## Troubleshooting

### REPL won't start

```bash
# Check if compiled
ls -la omnilisp

# Rebuild
make clean && make
```

### Socket server connection refused

```bash
# Check if server is running
pgrep -f "omnilisp -S"

# Check port in use
lsof -i :5555

# Try different port
./omnilisp -S 5556
```

### Neovim can't connect

```lua
-- Check connection status
:lua print(require('omnilisp').state.connected)

-- Manual connect with verbose output
:lua require('omnilisp').connect()

-- Check if server is reachable
:!nc -z localhost 5555 && echo "OK" || echo "FAIL"
```

### Parse errors

```bash
# Use parse-only mode to debug
./omnilisp -p -e "(your expression)"

# Enable debug output
./omnilisp -d -e "(your expression)"
```

---

## Next Steps

- **[Getting Started](GETTING_STARTED.md)** - Detailed introduction
- **[Pattern Matching Guide](PATTERN_MATCHING_GUIDE.md)** - Deep dive into patterns
- **[Effects and Handlers](EFFECTS_AND_HANDLERS.md)** - Algebraic effects
- **[Standard Library](STANDARD_LIBRARY.md)** - All functions
- **[Quick Reference](QUICK_REFERENCE.md)** - Language cheat sheet

---

## Cheat Sheet

```
┌─────────────────────────────────────────────────────────────┐
│ OMNILISP QUICKSTART                                         │
├─────────────────────────────────────────────────────────────┤
│ RUN            │ omnilisp file.ol                           │
│ EVAL           │ omnilisp -e "(+ 1 2)"                      │
│ REPL           │ omnilisp -i                                │
│ SERVER         │ omnilisp -S 5555                           │
├─────────────────────────────────────────────────────────────┤
│ BRACKETS       │ () = execute  [] = data/slots  {} = types │
├─────────────────────────────────────────────────────────────┤
│ DEFINE         │ (define name [x] [y] body)                 │
│ LAMBDA         │ (lambda [x] body)                          │
│ LET            │ (let [x 1] [y 2] body)                     │
│ MATCH          │ (match val pat1 res1 pat2 res2)            │
│ GUARD          │ pattern & condition                        │
├─────────────────────────────────────────────────────────────┤
│ LIST           │ '(1 2 3)                                   │
│ ARRAY          │ [1 2 3]                                    │
│ DICT           │ #{"k" "v"}                                 │
├─────────────────────────────────────────────────────────────┤
│ REPL COMMANDS  │ :q quit  :h help  :c clear                 │
│ NEOVIM         │ <leader>ee eval  <leader>eb buffer         │
└─────────────────────────────────────────────────────────────┘
```
