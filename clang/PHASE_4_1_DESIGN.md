# Phase 4.1: Module System

**Status:** Design Phase  
**Date:** 2026-02-05  
**Depends on:** Phase 3 (Effects System)

## Overview

The module system provides namespace separation, code organization, and controlled visibility of definitions. It enables:
- Large-scale code organization
- Reusable library creation
- Explicit dependency management
- Effect tracking across module boundaries
- Separate compilation (future)

## Motivation

**Without modules:**
```scheme
;; Everything in global namespace
(define user-count 0)
(define db-count 0)  ; Name collision risk!

(define get-count [] user-count)
```

**With modules:**
```scheme
;; user.omni
(module user
  (export get-count set-count user-count)
  
  (define user-count 0)
  (define get-count [] user-count)
  (define set-count [n] (set! user-count n)))

;; db.omni
(module db
  (export get-count)
  
  (define db-count 0)
  (define get-count [] db-count))

;; main.omni
(import user)
(import db :as database)

(user/get-count)      ;; Uses user module
(database/get-count)  ;; Uses db module
```

## Module Syntax

### Module Declaration

```scheme
(module name
  (export symbol1 symbol2 ...)
  (import module1 module2 ...)
  
  ;; Module body
  (define ...)
  (deftype ...)
  ...)
```

### Import Forms

```scheme
;; Import all exported symbols
(import module-name)

;; Qualified import (prefix)
(import module-name :as prefix)

;; Selective import
(import module-name :only [sym1 sym2])

;; Hiding import
(import module-name :hiding [sym1 sym2])

;; Renaming import
(import module-name :rename [(old-name new-name)])
```

### Export Forms

```scheme
;; Export specific symbols
(export sym1 sym2 ...)

;; Export all definitions
(export :all)

;; Export with renaming
(export (sym1 :as external-name))

;; Re-export from imported module
(export :from other-module sym1 sym2)
```

## Module Structure

### File Organization

```
project/
├── core/
│   ├── list.omni      ;; (module core.list ...)
│   ├── string.omni    ;; (module core.string ...)
│   └── io.omni        ;; (module core.io ...)
├── utils/
│   ├── parser.omni    ;; (module utils.parser ...)
│   └── format.omni    ;; (module utils.format ...)
└── main.omni          ;; (module main ...)
```

### Module Naming

Hierarchical dot-notation:
- `core.list`
- `core.string.regex`
- `utils.parser.json`

## Module Examples

### Example 1: Simple Module

```scheme
;; math.omni
(module math
  (export add multiply square)
  
  (define add [x] [y]
    (+ x y))
  
  (define multiply [x] [y]
    (* x y))
  
  (define square [x]
    (multiply x x)))

;; Usage
(import math)
(math/square 5)  ;; → 25
```

### Example 2: Qualified Import

```scheme
;; main.omni
(import math :as m)
(import core.string :as str)

(m/add 1 2)              ;; → 3
(str/concat "hello" " world")
```

### Example 3: Selective Import

```scheme
(import math :only [add multiply])

(add 1 2)       ;; OK
(multiply 3 4)  ;; OK
(square 5)      ;; ERROR: not imported
```

### Example 4: Private Definitions

```scheme
(module crypto
  (export encrypt decrypt)
  
  ;; Private helper (not exported)
  (define generate-key [seed]
    (hash seed 12345))
  
  ;; Public API
  (define encrypt [text] [seed]
    (let [key (generate-key seed)]
      (xor-cipher text key)))
  
  (define decrypt [cipher] [seed]
    (let [key (generate-key seed)]
      (xor-cipher cipher key))))
```

### Example 5: Module with Effects

```scheme
(module database
  (export connect query close)
  
  ;; Effect-tracked functions
  (define connect [config] : Config -> Connection / {IO}
    (perform io-connect config))
  
  (define query [conn] [sql] : Connection -> SQL -> ResultSet / {IO, DB}
    (perform db-query conn sql))
  
  (define close [conn] : Connection -> Unit / {IO}
    (perform io-close conn)))

;; Type signatures preserved across module boundary
```

## Module Resolution

### Search Path

```
1. Current directory
2. Project root
3. Standard library path
4. Environment OMNILISP_PATH
```

### Resolution Algorithm

```
resolve_module("core.list"):
  1. Try: ./core/list.omni
  2. Try: ./core/list/main.omni
  3. Try: $OMNISPATH/core/list.omni
  4. Try: $OMNISPATH/core/list/main.omni
  5. Error: module not found
```

## Module Compilation

### Dependency Graph

```scheme
;; main.omni depends on utils, core
;; utils.omni depends on core
;; core.omni has no dependencies

Compilation order:
  1. core.omni
  2. utils.omni
  3. main.omni
```

### Separate Compilation

Each module compiles to:
- **Interface file** (.omi): Exported types and effects
- **Implementation file** (.omc): Compiled code

```
math.omni → math.omi + math.omc
```

Interface file example:
```scheme
;; math.omi
(interface math
  (export 
    (add : Int -> Int -> Int / {})
    (multiply : Int -> Int -> Int / {})
    (square : Int -> Int / {})))
```

## Effect Tracking Across Modules

### Effect Propagation

```scheme
;; user.omni
(module user
  (export get-user save-user)
  
  (define get-user [id] : Int -> User / {DB}
    (perform db-fetch "users" id))
  
  (define save-user [user] : User -> Unit / {DB}
    (perform db-save "users" user)))

;; app.omni
(import user)

;; Effects visible through imports
(define update-email [id] [email] : Int -> String -> Unit / {DB}
  (let [u (user/get-user id)]
    (user/save-user (assoc u :email email))))
```

### Effect Abstraction

```scheme
;; api.omni
(module api
  (export make-request)
  
  ;; Internal effect
  (define-effect HttpInternal
    (raw-get [String] Response))
  
  ;; Public API hides internal effect
  (define make-request [url]
    (handle
      (perform HttpInternal/raw-get url)
      (HttpInternal/raw-get [url k]
        (perform Http/get url)  ; Public effect
        (k result)))))

;; Effect signature: String -> Response / {Http}
;; HttpInternal is hidden
```

## Standard Library Organization

```
omnilisp.core/
├── list          ;; List operations
├── string        ;; String manipulation
├── io            ;; I/O operations
├── math          ;; Mathematical functions
├── option        ;; Option type
├── result        ;; Result type
└── prelude       ;; Auto-imported basics

omnilisp.data/
├── map           ;; Persistent maps
├── set           ;; Persistent sets
├── vector        ;; Persistent vectors
└── queue         ;; Queue structures

omnilisp.concurrent/
├── async         ;; Async operations
├── channel       ;; CSP channels
└── promise       ;; Promise abstraction

omnilisp.test/
├── check         ;; QuickCheck-style testing
└── unit          ;; Unit test framework
```

## Implementation Strategy

### Phase 4.1.1: Module Parser

```c
typedef struct Module {
    char *name;
    char **exports;       // Exported symbol names
    size_t export_count;
    char **imports;       // Imported module names
    size_t import_count;
    Term *body;           // Module body expressions
    EffectRow *effects;   // Module-level effects
} Module;

// Parse module definition
Module *parse_module(const char *source);
```

### Phase 4.1.2: Module Registry

```c
typedef struct ModuleRegistry {
    Module **modules;
    size_t count;
    size_t capacity;
} ModuleRegistry;

// Register module
void register_module(ModuleRegistry *reg, Module *mod);

// Lookup module
Module *find_module(ModuleRegistry *reg, const char *name);

// Resolve module reference
Module *resolve_module(ModuleRegistry *reg, 
                      const char *name,
                      const char *search_path);
```

### Phase 4.1.3: Symbol Resolution

```c
typedef struct QualifiedName {
    char *module;   // NULL for current module
    char *symbol;
} QualifiedName;

// Resolve qualified reference
Term *resolve_qualified(QualifiedName *qname, 
                       Module *current_module,
                       ModuleRegistry *registry);

// Check symbol visibility
bool is_exported(Module *mod, const char *symbol);
```

### Phase 4.1.4: Dependency Analysis

```c
typedef struct DependencyGraph {
    Module **nodes;
    bool **edges;         // Adjacency matrix
    size_t node_count;
} DependencyGraph;

// Build dependency graph
DependencyGraph *build_dep_graph(ModuleRegistry *registry);

// Topological sort for compilation order
Module **compilation_order(DependencyGraph *graph);

// Detect circular dependencies
bool has_circular_deps(DependencyGraph *graph);
```

## Success Criteria

- [ ] Module declaration syntax
- [ ] Import/export forms
- [ ] Module registry and lookup
- [ ] Qualified name resolution
- [ ] Dependency graph construction
- [ ] Topological sort for compilation
- [ ] Effect tracking across modules
- [ ] Circular dependency detection
- [ ] Private definition enforcement
- [ ] Standard library organization

## Next Phase

**Phase 4.2: Macro System** will implement:
- Compile-time code generation
- Hygenic macro expansion
- Pattern-based transformations

---

**Implementation Approach:** Design for HVM module system with separate compilation. C interpreter could use simpler runtime loading.
