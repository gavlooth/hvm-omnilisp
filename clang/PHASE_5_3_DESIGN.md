# Phase 5.3: REPL

**Status:** Design Phase  
**Date:** 2026-02-05

## Overview

Interactive Read-Eval-Print Loop for development and exploration.

## Features

- Multi-line input
- Tab completion
- History
- Syntax highlighting
- Pretty printing
- Help system

## Commands

```
> (+ 1 2)
3
> (define x 10)
> x
10
> :help
> :type factorial
Int -> Int / {}
> :effects fetch-data
Int -> Data / {IO, Http}
```

## Success Criteria
- [ ] Basic REPL loop
- [ ] Persistent history
- [ ] Tab completion
- [ ] Pretty printing

---

**Status:** Design complete.
