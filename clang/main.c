// OmniLisp Main Entry Point
// Command-line interface for parsing, compiling, and running OmniLisp code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>

// Socket support
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

// Include HVM4 runtime first
#include "../hvm4/clang/hvm4.c"

// Include OmniLisp components
#include "omnilisp/nick/omnilisp.c"
#include "omnilisp/ffi/handle.c"
#include "omnilisp/ffi/thread_pool.c"
#include "omnilisp/parse/_.c"
#include "omnilisp/compile/_.c"

// =============================================================================
// Command Line Options
// =============================================================================

typedef struct {
  int help;
  int version;
  int parse_only;      // -p: Just parse and print AST
  int compile_only;    // -c: Compile but don't run
  int debug;           // -d: Debug mode
  int stats;           // -s: Show statistics
  int collapse;        // -C: Collapse limit
  int eval_mode;       // -e: Evaluate expression directly
  int interactive;     // -i: Interactive REPL
  int server_port;     // -S: Socket server port (0 = disabled)
  const char *file;    // Input file
  const char *expr;    // Expression to evaluate
  const char *output;  // -o: Output file
} OmniOptions;

// Global flag for graceful shutdown
static volatile int g_running = 1;

fn void signal_handler(int sig) {
  (void)sig;
  g_running = 0;
}

fn void print_usage(const char *prog) {
  printf("OmniLisp - A Lisp dialect for HVM4\n");
  printf("\n");
  printf("Usage: %s [options] [file.ol]\n", prog);
  printf("       %s -e \"(+ 1 2)\"\n", prog);
  printf("       %s -i                    (interactive REPL)\n", prog);
  printf("       %s -S 5555               (socket server)\n", prog);
  printf("\n");
  printf("Options:\n");
  printf("  -h, --help        Show this help message\n");
  printf("  -v, --version     Show version information\n");
  printf("  -p, --parse       Parse only (print AST)\n");
  printf("  -c, --compile     Compile only (emit HVM4)\n");
  printf("  -e, --eval EXPR   Evaluate expression\n");
  printf("  -i, --interactive Interactive REPL\n");
  printf("  -S, --server PORT Start socket server on PORT\n");
  printf("  -o, --output FILE Output file for compilation\n");
  printf("  -d, --debug       Enable debug output\n");
  printf("  -s, --stats       Show execution statistics\n");
  printf("  -C, --collapse N  Set collapse limit (default: 10)\n");
  printf("\n");
  printf("Examples:\n");
  printf("  %s program.ol           Run OmniLisp program\n", prog);
  printf("  %s -e \"(+ 1 2)\"         Evaluate expression\n", prog);
  printf("  %s -i                   Start interactive REPL\n", prog);
  printf("  %s -S 5555              Start server on port 5555\n", prog);
  printf("  %s -c -o out.hvm4 in.ol Compile to HVM4\n", prog);
  printf("  %s -p program.ol        Show parse tree\n", prog);
  printf("\n");
  printf("Socket Protocol (for editor integration):\n");
  printf("  Send: expression followed by newline\n");
  printf("  Recv: result followed by \\x00\\n (null + newline)\n");
  printf("  Test: echo '(+ 1 2)' | nc localhost 5555\n");
  printf("\n");
}

fn void print_version(void) {
  printf("OmniLisp version 0.1.0\n");
  printf("Built on HVM4 runtime\n");
}

fn OmniOptions parse_options(int argc, char *argv[]) {
  OmniOptions opts = {0};
  opts.collapse = 10;

  static struct option long_options[] = {
    {"help",        no_argument,       0, 'h'},
    {"version",     no_argument,       0, 'v'},
    {"parse",       no_argument,       0, 'p'},
    {"compile",     no_argument,       0, 'c'},
    {"eval",        required_argument, 0, 'e'},
    {"interactive", no_argument,       0, 'i'},
    {"server",      required_argument, 0, 'S'},
    {"output",      required_argument, 0, 'o'},
    {"debug",       no_argument,       0, 'd'},
    {"stats",       no_argument,       0, 's'},
    {"collapse",    required_argument, 0, 'C'},
    {0, 0, 0, 0}
  };

  int opt;
  int opt_index = 0;

  while ((opt = getopt_long(argc, argv, "hvpce:iS:o:dsC:", long_options, &opt_index)) != -1) {
    switch (opt) {
      case 'h': opts.help = 1; break;
      case 'v': opts.version = 1; break;
      case 'p': opts.parse_only = 1; break;
      case 'c': opts.compile_only = 1; break;
      case 'e': opts.eval_mode = 1; opts.expr = optarg; break;
      case 'i': opts.interactive = 1; break;
      case 'S': opts.server_port = atoi(optarg); break;
      case 'o': opts.output = optarg; break;
      case 'd': opts.debug = 1; break;
      case 's': opts.stats = 1; break;
      case 'C': opts.collapse = atoi(optarg); break;
      default: opts.help = 1; break;
    }
  }

  if (optind < argc) {
    opts.file = argv[optind];
  }

  return opts;
}

// =============================================================================
// File Reading
// =============================================================================

fn char* read_file(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Error: Cannot open file '%s'\n", path);
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buf = (char*)malloc(size + 1);
  if (!buf) {
    fclose(f);
    return NULL;
  }

  size_t read = fread(buf, 1, size, f);
  buf[read] = '\0';
  fclose(f);

  return buf;
}

// =============================================================================
// AST Printing
// =============================================================================

fn void print_term_indent(Term t, int indent);

fn void print_indent(int n) {
  for (int i = 0; i < n; i++) printf("  ");
}

fn void print_term_indent(Term t, int indent) {
  u32 tag = term_tag(t);
  u32 ext = term_ext(t);
  u32 val = term_val(t);

  print_indent(indent);

  switch (tag) {
    case C00: case C01: case C02: case C03: case C04:
    case C05: case C06: case C07: case C08: case C09:
    case C10: case C11: case C12: case C13: case C14:
    case C15: case C16: {
      // Print constructor name
      char name[16];
      nick_to_str(ext, name, sizeof(name));
      printf("#%s{\n", name);

      // Print children based on arity (tag - C00)
      u32 arity = tag - C00;
      for (u32 i = 0; i < arity; i++) {
        print_term_indent(HEAP[val + i], indent + 1);
        if (i < arity - 1) printf(",");
        printf("\n");
      }

      print_indent(indent);
      printf("}");
      break;
    }

    case NUM:
      printf("%u", val);
      break;

    case LAM:
      printf("λ. ...");
      break;

    case VAR:
      printf("v%u", val);
      break;

    case REF:
      printf("@%u", ext);
      break;

    default:
      printf("<%u:%u:%u>", tag, ext, val);
      break;
  }
}

fn void print_ast(Term t) {
  print_term_indent(t, 0);
  printf("\n");
}

// =============================================================================
// Runtime Initialization
// =============================================================================

// Track if runtime.hvm4 has been loaded
static int g_runtime_loaded = 0;

// Load runtime.hvm4 into the HVM4 book
// NOTE: Currently disabled - runtime.hvm4 has syntax incompatible with HVM4 parser.
//       Using direct C interpreter instead.
fn int omni_load_runtime(void) {
  // Skip loading runtime.hvm4 - use direct C interpreter
  // The runtime.hvm4 syntax uses features HVM4 parser doesn't support:
  // - !!& bindings inside match arms
  // - Complex lambda patterns
  return 1;
}

fn void omni_runtime_init(void) {
  // Initialize HVM4 runtime
  thread_set_count(1);
  wnf_set_tid(0);

  // Allocate global memory
  BOOK  = calloc(BOOK_CAP, sizeof(u32));
  HEAP  = calloc(HEAP_CAP, sizeof(Term));
  TABLE = calloc(BOOK_CAP, sizeof(char*));

  if (!BOOK || !HEAP || !TABLE) {
    fprintf(stderr, "Error: Memory allocation failed\n");
    exit(1);
  }
  heap_init_slices();

  // Initialize OmniLisp names
  omni_names_init();

  // Initialize FFI
  omni_ffi_handle_init();
  omni_ffi_register_stdlib();
}

fn void omni_runtime_cleanup(void) {
  // Cleanup FFI
  omni_ffi_pool_shutdown();
  omni_ffi_handle_cleanup();
}

// =============================================================================
// Direct OmniLisp Interpreter
// =============================================================================

// Environment for direct evaluation (prefixed with interp_ to avoid conflict)
#define OMNI_INTERP_MAX 256
static Term OMNI_INTERP_ENV[OMNI_INTERP_MAX];
static u32 OMNI_INTERP_LEN = 0;

fn void omni_interp_push(Term val) {
  if (OMNI_INTERP_LEN >= OMNI_INTERP_MAX) {
    fprintf(stderr, "Error: environment overflow\n");
    exit(1);
  }
  OMNI_INTERP_ENV[OMNI_INTERP_LEN++] = val;
}

// Effect handler stack
// Each entry is a list of #HDef{tag, handler_fn}
#define OMNI_HANDLER_STACK_MAX 64
static Term OMNI_HANDLER_STACK[OMNI_HANDLER_STACK_MAX];
static u32 OMNI_HANDLER_STACK_LEN = 0;

fn void omni_handler_push(Term handlers) {
  if (OMNI_HANDLER_STACK_LEN >= OMNI_HANDLER_STACK_MAX) {
    fprintf(stderr, "Error: handler stack overflow\n");
    exit(1);
  }
  OMNI_HANDLER_STACK[OMNI_HANDLER_STACK_LEN++] = handlers;
}

fn void omni_handler_pop(void) {
  if (OMNI_HANDLER_STACK_LEN > 0) {
    OMNI_HANDLER_STACK_LEN--;
  }
}

// Find a handler for the given effect tag
// Returns the handler function, or 0 if not found
fn Term omni_find_handler(u32 effect_tag) {
  for (int i = (int)OMNI_HANDLER_STACK_LEN - 1; i >= 0; i--) {
    Term handlers = OMNI_HANDLER_STACK[i];
    // Iterate through handler list
    while (term_tag(handlers) == C02 && term_ext(handlers) == OMNI_NAM_CON) {
      u32 loc = term_val(handlers);
      Term handler_def = HEAP[loc];      // #HDef{tag, fn}
      handlers = HEAP[loc + 1];          // rest of list

      if (term_tag(handler_def) == C02 && term_ext(handler_def) == OMNI_NAM_HDEF) {
        u32 def_loc = term_val(handler_def);
        Term tag_term = HEAP[def_loc];
        Term handler_fn = HEAP[def_loc + 1];

        if (term_tag(tag_term) == NUM && term_val(tag_term) == effect_tag) {
          return handler_fn;
        }
      }
    }
  }
  return 0;  // Not found
}

fn void omni_interp_pop(u32 count) {
  if (count > OMNI_INTERP_LEN) count = OMNI_INTERP_LEN;
  OMNI_INTERP_LEN -= count;
}

fn Term omni_interp_lookup(u32 idx) {
  if (idx >= OMNI_INTERP_LEN) {
    return term_new_num(0);  // Error: unbound variable
  }
  return OMNI_INTERP_ENV[OMNI_INTERP_LEN - 1 - idx];
}

// Forward declaration
fn Term omni_interpret(Term t);

// Apply a closure to an argument
fn Term omni_apply(Term func, Term arg) {
  u32 tag = term_tag(func);
  u32 ext = term_ext(func);

  // Check for closure: #Clo{env_list, body}
  if (tag == C02 && ext == OMNI_NAM_CLO) {
    u32 loc = term_val(func);
    Term env_list = HEAP[loc];
    Term body = HEAP[loc + 1];

    // Save current environment length
    u32 saved_len = OMNI_INTERP_LEN;

    // Restore captured environment
    u32 captured_count = 0;
    Term cur = env_list;
    while (term_tag(cur) == C02 && term_ext(cur) == OMNI_NAM_CON) {
      u32 cloc = term_val(cur);
      omni_interp_push(HEAP[cloc]);  // head
      cur = HEAP[cloc + 1];  // tail
      captured_count++;
    }

    // Push new argument
    omni_interp_push(arg);

    // Evaluate body
    Term result = omni_interpret(body);

    // Restore environment
    OMNI_INTERP_LEN = saved_len;
    return result;
  }

  // Check for lambda: #Lam{body}
  if (tag == C01 && ext == OMNI_NAM_LAM) {
    u32 loc = term_val(func);
    Term body = HEAP[loc];
    omni_interp_push(arg);
    Term result = omni_interpret(body);
    omni_interp_pop(1);
    return result;
  }

  // Check for recursive closure: #CloR{env_marker, lam_body}
  // lam_body is #Lam{actual_body} - we need to unwrap and apply
  if (tag == C02 && ext == OMNI_NAM_CLOR) {
    u32 loc = term_val(func);
    Term lam_body = HEAP[loc + 1];

    // The lam_body should be #Lam{body}
    if (term_tag(lam_body) == C01 && term_ext(lam_body) == OMNI_NAM_LAM) {
      u32 lam_loc = term_val(lam_body);
      Term body = HEAP[lam_loc];  // The actual body

      // Save current env
      u32 saved_len = OMNI_INTERP_LEN;

      // Push self-reference then argument
      omni_interp_push(func);  // Self for recursion (loop variable)
      omni_interp_push(arg);   // The actual argument (n)
      Term result = omni_interpret(body);

      // Restore env
      OMNI_INTERP_LEN = saved_len;
      return result;
    }

    // Fallback: interpret body directly
    omni_interp_push(func);
    omni_interp_push(arg);
    Term result = omni_interpret(lam_body);
    omni_interp_pop(2);
    return result;
  }

  // Not a function - return as-is (partial application not supported yet)
  return term_new_app(func, arg);
}

// Direct interpreter for OmniLisp AST
fn Term omni_interpret(Term t) {
  u32 tag = term_tag(t);
  u32 ext = term_ext(t);
  u32 val = term_val(t);

  // Already a number (primitive value)
  if (tag == NUM) {
    return t;
  }

  // REF - reference to a definition in BOOK
  // BOOK[id] stores heap location, actual term is in HEAP[loc]
  // Note: REF stores the definition ID in ext field, not val
  if (tag == REF) {
    u32 def_id = ext;  // REF stores id in ext, not val
    u32 loc = BOOK[def_id];
    if (loc != 0) {
      Term def = HEAP[loc];
      return omni_interpret(def);
    }
    // Undefined reference - return as-is
    return t;
  }

  // Not a constructor - normalize via HVM
  if (tag < C00 || tag > C16) {
    return wnf(t);
  }

  // #Lit{n} - literal
  if (ext == OMNI_NAM_LIT) {
    return HEAP[val];  // Unwrap to the number
  }

  // #Var{idx} - variable lookup
  if (ext == OMNI_NAM_VAR) {
    u32 idx = term_val(HEAP[val]);
    return omni_interp_lookup(idx);
  }

  // #Sym{table_id} - symbol lookup (for forward references)
  // When define parses its body before the name is in BOOK, it becomes #Sym
  // At runtime, look up BOOK[table_id] directly
  if (ext == OMNI_NAM_SYM) {
    u32 table_id = term_val(HEAP[val]);
    u32 loc = BOOK[table_id];
    if (loc != 0) {
      Term def = HEAP[loc];
      // For top-level definitions (lambdas), return them directly without
      // interpreting, to avoid capturing the current call stack into a closure.
      // The caller (apply) will handle the lambda appropriately.
      return def;
    }
    // Symbol not found in BOOK - return as-is
    return t;
  }

  // #Lam{body} - create closure capturing environment
  if (ext == OMNI_NAM_LAM) {
    if (OMNI_INTERP_LEN > 0) {
      // Capture current environment as a list (in order, oldest first)
      Term env_list = omni_nil();
      for (int i = OMNI_INTERP_LEN - 1; i >= 0; i--) {
        env_list = omni_cons(OMNI_INTERP_ENV[i], env_list);
      }
      // Create closure: #Clo{env_list, body}
      Term args[2];
      args[0] = env_list;
      args[1] = HEAP[val];  // body
      return term_new_ctr(OMNI_NAM_CLO, 2, args);
    }
    return t;
  }

  // #LamR{body} - recursive lambda
  if (ext == OMNI_NAM_LAMR) {
    // Return as recursive closure
    Term args[2];
    args[0] = term_new_num(OMNI_INTERP_LEN);  // Env marker
    args[1] = HEAP[val];  // Body
    return term_new_ctr(OMNI_NAM_CLOR, 2, args);
  }

  // #App{fn, arg} - application
  if (ext == OMNI_NAM_APP) {
    Term func = omni_interpret(HEAP[val]);
    Term arg = omni_interpret(HEAP[val + 1]);
    return omni_apply(func, arg);
  }

  // #Let{val, body} - let binding (lazy semantics, but C interp evaluates eagerly)
  // #LetS{val, body} - strict let binding (^:strict metadata)
  if (ext == OMNI_NAM_LET || ext == OMNI_NAM_LETS) {
    Term let_val = omni_interpret(HEAP[val]);
    omni_interp_push(let_val);
    Term result = omni_interpret(HEAP[val + 1]);
    omni_interp_pop(1);
    return result;
  }

  // #Pure{fn} - purity marker (^:pure metadata)
  // Indicates function has no side effects - used for static analysis
  // At runtime, just unwrap and return the inner function
  if (ext == OMNI_NAM_PURE) {
    return omni_interpret(HEAP[val]);
  }

  // #NLet{name_nick, init_values, loop_body} - named let (recursive)
  // Named let: (let loop [i 0] [sum 0] body) where body can call (loop new_i new_sum)
  if (ext == OMNI_NAM_NLET || ext == OMNI_NAM_NLETS) {
    // Term name_nick = HEAP[val];  // Not directly used - recursion via closure
    Term init_values = HEAP[val + 1];
    Term loop_body = HEAP[val + 2];  // Already wrapped in lambdas

    // Create a recursive closure for the loop
    // The loop_body is #Lam{#Lam{...body...}} for each binding
    // We need to make this self-referential

    // First, evaluate initial values
    Term args[16];
    u32 arg_count = 0;
    Term cur = init_values;
    while (term_tag(cur) == C02 && term_ext(cur) == OMNI_NAM_CON && arg_count < 16) {
      u32 cloc = term_val(cur);
      args[arg_count++] = omni_interpret(HEAP[cloc]);
      cur = HEAP[cloc + 1];
    }

    // Create recursive closure: wrap loop_body to capture itself
    Term clo_args[2];
    clo_args[0] = omni_nil();  // Empty env initially
    clo_args[1] = loop_body;
    Term loop_closure = term_new_ctr(OMNI_NAM_CLOR, 2, clo_args);

    // Push the loop closure as the first binding (for self-reference)
    omni_interp_push(loop_closure);

    // Apply initial arguments
    Term result = loop_closure;
    for (u32 i = 0; i < arg_count; i++) {
      result = omni_apply(result, args[i]);
    }

    omni_interp_pop(1);
    return result;
  }

  // #Fst{pair} - first element (car)
  if (ext == OMNI_NAM_FST) {
    Term pair = omni_interpret(HEAP[val]);
    if (term_tag(pair) == C02 && (term_ext(pair) == OMNI_NAM_CON || term_ext(pair) == NAM_CON)) {
      return HEAP[term_val(pair)];
    }
    return t;
  }

  // #Snd{pair} - second element (cdr)
  if (ext == OMNI_NAM_SND) {
    Term pair = omni_interpret(HEAP[val]);
    if (term_tag(pair) == C02 && (term_ext(pair) == OMNI_NAM_CON || term_ext(pair) == NAM_CON)) {
      return HEAP[term_val(pair) + 1];
    }
    return t;
  }

  // #If{cond, then, else} - conditional
  if (ext == OMNI_NAM_IF) {
    Term cond = omni_interpret(HEAP[val]);
    u32 cond_val = term_tag(cond) == NUM ? term_val(cond) : 0;
    if (cond_val != 0) {
      return omni_interpret(HEAP[val + 1]);
    } else {
      return omni_interpret(HEAP[val + 2]);
    }
  }

  // #Add{a, b} - addition
  if (ext == OMNI_NAM_ADD) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) + term_val(b));
    }
    return t;
  }

  // #Sub{a, b} - subtraction
  if (ext == OMNI_NAM_SUB) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) - term_val(b));
    }
    return t;
  }

  // #Mul{a, b} - multiplication
  if (ext == OMNI_NAM_MUL) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) * term_val(b));
    }
    return t;
  }

  // #Div{a, b} - division
  if (ext == OMNI_NAM_DIV) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      u32 bv = term_val(b);
      if (bv == 0) return term_new_num(0);
      return term_new_num(term_val(a) / bv);
    }
    return t;
  }

  // #Mod{a, b} - modulo
  if (ext == OMNI_NAM_MOD) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      u32 bv = term_val(b);
      if (bv == 0) return term_new_num(0);
      return term_new_num(term_val(a) % bv);
    }
    return t;
  }

  // #Eql{a, b} - equality
  if (ext == OMNI_NAM_EQL) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) == term_val(b) ? 1 : 0);
    }
    return t;
  }

  // #Neq{a, b} - not equal
  if (ext == OMNI_NAM_NEQ) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) != term_val(b) ? 1 : 0);
    }
    return t;
  }

  // #Lt{a, b} - less than
  if (ext == OMNI_NAM_LT) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) < term_val(b) ? 1 : 0);
    }
    return t;
  }

  // #Gt{a, b} - greater than
  if (ext == OMNI_NAM_GT) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) > term_val(b) ? 1 : 0);
    }
    return t;
  }

  // #Le{a, b} - less than or equal
  if (ext == OMNI_NAM_LE) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) <= term_val(b) ? 1 : 0);
    }
    return t;
  }

  // #Ge{a, b} - greater than or equal
  if (ext == OMNI_NAM_GE) {
    Term a = omni_interpret(HEAP[val]);
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(a) == NUM && term_tag(b) == NUM) {
      return term_new_num(term_val(a) >= term_val(b) ? 1 : 0);
    }
    return t;
  }

  // #And{a, b} - logical and
  if (ext == OMNI_NAM_AND) {
    Term a = omni_interpret(HEAP[val]);
    if (term_tag(a) == NUM && term_val(a) == 0) {
      return term_new_num(0);
    }
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(b) == NUM && term_val(b) == 0) {
      return term_new_num(0);
    }
    return term_new_num(1);
  }

  // #Or{a, b} - logical or
  if (ext == OMNI_NAM_OR) {
    Term a = omni_interpret(HEAP[val]);
    if (term_tag(a) == NUM && term_val(a) != 0) {
      return term_new_num(1);
    }
    Term b = omni_interpret(HEAP[val + 1]);
    if (term_tag(b) == NUM && term_val(b) != 0) {
      return term_new_num(1);
    }
    return term_new_num(0);
  }

  // #Not{a} - logical not
  if (ext == OMNI_NAM_NOT) {
    Term a = omni_interpret(HEAP[val]);
    if (term_tag(a) == NUM) {
      return term_new_num(term_val(a) == 0 ? 1 : 0);
    }
    return t;
  }

  // #IntP{value} - int? predicate
  if (ext == OMNI_NAM_INTP) {
    Term value = omni_interpret(HEAP[val]);
    // Check if it's a number (NUM tag)
    return term_new_num(term_tag(value) == NUM ? 1 : 0);
  }

  // #LstP{value} - list? predicate
  if (ext == OMNI_NAM_LSTP) {
    Term value = omni_interpret(HEAP[val]);
    u32 t_tag = term_tag(value);
    u32 t_ext = term_ext(value);
    // List is either nil or cons
    int is_list = (t_tag == C00 && (t_ext == OMNI_NAM_NIL || t_ext == NAM_NIL)) ||
                  (t_tag == C02 && (t_ext == OMNI_NAM_CON || t_ext == NAM_CON));
    return term_new_num(is_list ? 1 : 0);
  }

  // #NilP{value} - nil? predicate
  if (ext == OMNI_NAM_NILP) {
    Term value = omni_interpret(HEAP[val]);
    u32 t_tag = term_tag(value);
    u32 t_ext = term_ext(value);
    int is_nil = (t_tag == C00 && (t_ext == OMNI_NAM_NIL || t_ext == NAM_NIL));
    return term_new_num(is_nil ? 1 : 0);
  }

  // #NumP{value} - number? predicate
  if (ext == OMNI_NAM_NUMP) {
    Term value = omni_interpret(HEAP[val]);
    // Check if it's a number or fixed-point
    int is_num = term_tag(value) == NUM ||
                 (term_tag(value) == C03 && term_ext(value) == OMNI_NAM_FIX);
    return term_new_num(is_num ? 1 : 0);
  }

  // #Do{first, rest} - sequence
  if (ext == OMNI_NAM_DO) {
    omni_interpret(HEAP[val]);  // Evaluate first for side effects
    return omni_interpret(HEAP[val + 1]);
  }

  // #Hdle{handlers, body} - install effect handlers
  if (ext == OMNI_NAM_HDLE) {
    Term handlers = HEAP[val];
    Term body = HEAP[val + 1];

    // Push handlers onto stack
    omni_handler_push(handlers);

    // Evaluate body
    Term result = omni_interpret(body);

    // Pop handlers
    omni_handler_pop();

    return result;
  }

  // #Perf{tag, payload} - perform an effect
  if (ext == OMNI_NAM_PERF) {
    Term tag_term = HEAP[val];
    Term payload = omni_interpret(HEAP[val + 1]);

    // Get effect tag (nick value)
    u32 effect_tag = 0;
    if (term_tag(tag_term) == NUM) {
      effect_tag = term_val(tag_term);
    } else if (term_tag(tag_term) == C01 && term_ext(tag_term) == OMNI_NAM_SYM) {
      // Symbol - get nick from the symbol
      effect_tag = term_val(HEAP[term_val(tag_term)]);
    }

    // Find handler
    Term handler_fn = omni_find_handler(effect_tag);
    if (handler_fn == 0) {
      fprintf(stderr, "Error: unhandled effect\n");
      return term_new_num(0);
    }

    // For simple one-shot effects, resume is identity function
    // Create a resume closure that just returns its argument
    // #Lam{#Var{0}} is identity
    u32 resume_loc = heap_alloc(1);
    u32 var_loc = heap_alloc(1);
    HEAP[var_loc] = term_new_num(0);  // idx=0
    HEAP[resume_loc] = term_new_ctr(OMNI_NAM_VAR, 1, &HEAP[var_loc]);
    Term resume_fn = term_new_ctr(OMNI_NAM_LAM, 1, &HEAP[resume_loc]);

    // Apply handler to payload, then to resume
    Term partial = omni_apply(handler_fn, payload);
    Term result = omni_apply(partial, resume_fn);

    return result;
  }

  // #CON{h, t} - cons cell, evaluate elements
  if (ext == OMNI_NAM_CON || ext == NAM_CON) {
    Term h = omni_interpret(HEAP[val]);
    Term t_tail = omni_interpret(HEAP[val + 1]);
    Term args[2] = {h, t_tail};
    return term_new_ctr(ext, 2, args);
  }

  // #NIL - nil
  if (ext == OMNI_NAM_NIL || ext == NAM_NIL) {
    return t;
  }

  // #Mat{scrutinee, cases} - pattern matching
  // #MatS{scrutinee, cases} - speculative pattern matching (^:speculate)
  // In C interpreter, speculative match behaves like regular match
  // (parallelism is achieved in HVM4 compilation)
  if (ext == OMNI_NAM_MAT || ext == OMNI_NAM_MATS) {
    Term scrutinee = omni_interpret(HEAP[val]);
    Term cases = HEAP[val + 1];

    // Iterate through cases
    while (term_tag(cases) == C02 && term_ext(cases) == OMNI_NAM_CON) {
      u32 cases_loc = term_val(cases);
      Term case_term = HEAP[cases_loc];
      cases = HEAP[cases_loc + 1];

      // Each case is #Case{pattern, guard, body}
      if (term_tag(case_term) == C03 && term_ext(case_term) == OMNI_NAM_CASE) {
        u32 case_loc = term_val(case_term);
        Term pattern = HEAP[case_loc];
        // Term guard = HEAP[case_loc + 1];  // TODO: handle guards
        Term body = HEAP[case_loc + 2];

        u32 pat_tag = term_tag(pattern);
        u32 pat_ext = term_ext(pattern);

        // #PWld{} - wildcard pattern (always matches)
        if (pat_tag == C00 && pat_ext == OMNI_NAM_PWLD) {
          // Check guard if present
          Term guard = HEAP[case_loc + 1];
          if (term_tag(guard) != C00 || term_ext(guard) != OMNI_NAM_NIL) {
            Term guard_result = omni_interpret(guard);
            // Guard must be truthy (non-zero number or #True)
            if (term_tag(guard_result) == NUM && term_val(guard_result) == 0) continue;
            if (term_tag(guard_result) == C00 && term_ext(guard_result) == OMNI_NAM_FALS) continue;
          }
          return omni_interpret(body);
        }

        // #PLit{value} - literal pattern
        if (pat_tag == C01 && pat_ext == OMNI_NAM_PLIT) {
          u32 pat_loc = term_val(pattern);
          Term lit_val = HEAP[pat_loc];

          // Unwrap #Lit{n} if present
          if (term_tag(lit_val) == C01 && term_ext(lit_val) == OMNI_NAM_LIT) {
            lit_val = HEAP[term_val(lit_val)];
          }

          // Check if it's #True{} and scrutinee is truthy
          if (term_tag(lit_val) == C00 && term_ext(lit_val) == OMNI_NAM_TRUE) {
            // Match if scrutinee is truthy (non-zero number or #True)
            if (term_tag(scrutinee) == NUM && term_val(scrutinee) != 0) {
              return omni_interpret(body);
            }
            if (term_tag(scrutinee) == C00 && term_ext(scrutinee) == OMNI_NAM_TRUE) {
              return omni_interpret(body);
            }
            continue;  // Try next case
          }

          // Check if it's #Fals{} and scrutinee is falsy
          if (term_tag(lit_val) == C00 && term_ext(lit_val) == OMNI_NAM_FALS) {
            if (term_tag(scrutinee) == NUM && term_val(scrutinee) == 0) {
              return omni_interpret(body);
            }
            if (term_tag(scrutinee) == C00 && term_ext(scrutinee) == OMNI_NAM_FALS) {
              return omni_interpret(body);
            }
            continue;
          }

          // Check numeric literal match
          if (term_tag(scrutinee) == NUM && term_tag(lit_val) == NUM) {
            if (term_val(scrutinee) == term_val(lit_val)) {
              return omni_interpret(body);
            }
            continue;
          }

          // Check nil/empty list match: () pattern
          u32 lv_tag = term_tag(lit_val);
          u32 lv_ext = term_ext(lit_val);
          if (lv_tag == C00 && (lv_ext == OMNI_NAM_NIL || lv_ext == NAM_NIL)) {
            u32 s_tag = term_tag(scrutinee);
            u32 s_ext = term_ext(scrutinee);
            if (s_tag == C00 && (s_ext == OMNI_NAM_NIL || s_ext == NAM_NIL)) {
              return omni_interpret(body);
            }
            continue;
          }
        }

        // #PVar{idx} - variable pattern (binds and matches anything)
        if (pat_tag == C01 && pat_ext == OMNI_NAM_PVAR) {
          omni_interp_push(scrutinee);
          // Check guard if present
          Term guard = HEAP[case_loc + 1];
          if (term_tag(guard) != C00 || term_ext(guard) != OMNI_NAM_NIL) {
            Term guard_result = omni_interpret(guard);
            // Guard must be truthy
            if (term_tag(guard_result) == NUM && term_val(guard_result) == 0) {
              omni_interp_pop(1);
              continue;
            }
            if (term_tag(guard_result) == C00 && term_ext(guard_result) == OMNI_NAM_FALS) {
              omni_interp_pop(1);
              continue;
            }
          }
          Term result = omni_interpret(body);
          omni_interp_pop(1);
          return result;
        }

        // #PCtr{tag_nick, field_patterns} - constructor pattern
        if (pat_tag == C02 && pat_ext == OMNI_NAM_PCTR) {
          u32 pat_val = term_val(pattern);
          u32 ctr_nick = term_val(HEAP[pat_val]);  // Constructor name as nick
          Term field_pats = HEAP[pat_val + 1];     // List of field patterns

          // Check if scrutinee is a constructor
          u32 scr_tag = term_tag(scrutinee);
          if (scr_tag < C00 || scr_tag > C16) continue;  // Not a constructor

          u32 scr_nick = term_ext(scrutinee);
          if (scr_nick != ctr_nick) continue;  // Constructor name mismatch

          u32 scr_arity = scr_tag - C00;
          u32 scr_loc = term_val(scrutinee);

          // Match field patterns and collect bindings
          u32 bindings_count = 0;
          Term bindings[16];  // Max 16 bindings per pattern
          int match_ok = 1;
          u32 field_idx = 0;

          while (term_tag(field_pats) == C02 && term_ext(field_pats) == OMNI_NAM_CON) {
            if (field_idx >= scr_arity) { match_ok = 0; break; }

            u32 fp_loc = term_val(field_pats);
            Term field_pat = HEAP[fp_loc];
            field_pats = HEAP[fp_loc + 1];

            Term field_val = HEAP[scr_loc + field_idx];

            u32 fp_tag = term_tag(field_pat);
            u32 fp_ext = term_ext(field_pat);

            // #PWld{} - wildcard matches anything
            if (fp_tag == C00 && fp_ext == OMNI_NAM_PWLD) {
              // Match, no binding
            }
            // #PVar{} - variable pattern, bind value
            else if (fp_tag == C01 && fp_ext == OMNI_NAM_PVAR) {
              if (bindings_count < 16) {
                bindings[bindings_count++] = field_val;
              }
            }
            // #PLit{val} - literal pattern, must match exactly
            else if (fp_tag == C01 && fp_ext == OMNI_NAM_PLIT) {
              Term lit_val = HEAP[term_val(field_pat)];
              if (term_tag(field_val) != term_tag(lit_val) ||
                  term_val(field_val) != term_val(lit_val)) {
                match_ok = 0;
                break;
              }
            }
            // Other patterns - not yet supported in nested position
            else {
              match_ok = 0;
              break;
            }

            field_idx++;
          }

          // Check arity match (all fields consumed)
          if (field_idx != scr_arity) match_ok = 0;

          if (match_ok) {
            // Push bindings for guard and body evaluation
            for (u32 i = 0; i < bindings_count; i++) {
              omni_interp_push(bindings[i]);
            }
            // Check guard if present
            Term guard = HEAP[case_loc + 1];
            if (term_tag(guard) != C00 || term_ext(guard) != OMNI_NAM_NIL) {
              Term guard_result = omni_interpret(guard);
              if (term_tag(guard_result) == NUM && term_val(guard_result) == 0) {
                omni_interp_pop(bindings_count);
                continue;
              }
              if (term_tag(guard_result) == C00 && term_ext(guard_result) == OMNI_NAM_FALS) {
                omni_interp_pop(bindings_count);
                continue;
              }
            }
            Term result = omni_interpret(body);
            omni_interp_pop(bindings_count);
            return result;
          }
          // Pattern didn't match, try next case
          continue;
        }

        // #PLst{elements} - list/array pattern
        if (pat_tag == C01 && pat_ext == OMNI_NAM_PLST) {
          u32 pat_loc = term_val(pattern);
          Term pats = HEAP[pat_loc];  // List of element patterns

          // Check for empty pattern ()
          u32 pats_tag = term_tag(pats);
          u32 pats_ext = term_ext(pats);
          if (pats_tag == C00 && (pats_ext == OMNI_NAM_NIL || pats_ext == NAM_NIL)) {
            // Empty pattern - match only nil/empty list
            u32 scr_tag = term_tag(scrutinee);
            u32 scr_ext = term_ext(scrutinee);
            if (scr_tag == C00 && (scr_ext == OMNI_NAM_NIL || scr_ext == NAM_NIL)) {
              return omni_interpret(body);
            }
            continue;  // Not empty, try next case
          }

          // Match list elements
          Term cur_scr = scrutinee;
          Term cur_pats = pats;
          u32 bindings_count = 0;
          Term bindings[16];
          int match_ok = 1;

          while (match_ok) {
            u32 cp_tag = term_tag(cur_pats);
            u32 cp_ext = term_ext(cur_pats);

            // End of pattern list
            if (cp_tag == C00 && (cp_ext == OMNI_NAM_NIL || cp_ext == NAM_NIL)) {
              // Pattern exhausted - scrutinee must also be nil
              u32 cs_tag = term_tag(cur_scr);
              u32 cs_ext = term_ext(cur_scr);
              if (!(cs_tag == C00 && (cs_ext == OMNI_NAM_NIL || cs_ext == NAM_NIL))) {
                match_ok = 0;  // Scrutinee has more elements
              }
              break;
            }

            // Pattern is a cons cell
            if (cp_tag == C02 && (cp_ext == OMNI_NAM_CON || cp_ext == NAM_CON)) {
              u32 cp_loc = term_val(cur_pats);
              Term elem_pat = HEAP[cp_loc];
              cur_pats = HEAP[cp_loc + 1];

              u32 ep_tag = term_tag(elem_pat);
              u32 ep_ext = term_ext(elem_pat);

              // Check for spread pattern #Sprd{name}
              if (ep_tag == C01 && ep_ext == OMNI_NAM_SPRD) {
                // Spread - bind rest of list to this variable
                if (bindings_count < 16) {
                  bindings[bindings_count++] = cur_scr;
                }
                // Spread consumes remaining elements
                break;
              }

              // Regular element pattern - scrutinee must be cons
              u32 cs_tag = term_tag(cur_scr);
              u32 cs_ext = term_ext(cur_scr);
              if (!(cs_tag == C02 && (cs_ext == OMNI_NAM_CON || cs_ext == NAM_CON))) {
                match_ok = 0;  // Scrutinee is not a cons cell
                break;
              }

              u32 cs_loc = term_val(cur_scr);
              Term scr_head = HEAP[cs_loc];
              cur_scr = HEAP[cs_loc + 1];

              // Match element pattern against head
              // #PWld{} - wildcard
              if (ep_tag == C00 && ep_ext == OMNI_NAM_PWLD) {
                // Match, no binding
              }
              // #PVar{} - variable
              else if (ep_tag == C01 && ep_ext == OMNI_NAM_PVAR) {
                if (bindings_count < 16) {
                  bindings[bindings_count++] = scr_head;
                }
              }
              // #PLit{val} - literal
              else if (ep_tag == C01 && ep_ext == OMNI_NAM_PLIT) {
                Term lit_val = HEAP[term_val(elem_pat)];
                if (term_tag(lit_val) == C01 && term_ext(lit_val) == OMNI_NAM_LIT) {
                  lit_val = HEAP[term_val(lit_val)];
                }
                if (term_tag(scr_head) != term_tag(lit_val) ||
                    term_val(scr_head) != term_val(lit_val)) {
                  match_ok = 0;
                }
              }
              else {
                // Unsupported nested pattern
                match_ok = 0;
              }
            } else {
              // Pattern list not properly formed
              match_ok = 0;
              break;
            }
          }

          if (match_ok) {
            for (u32 i = 0; i < bindings_count; i++) {
              omni_interp_push(bindings[i]);
            }
            // Check guard if present
            Term guard = HEAP[case_loc + 1];
            if (term_tag(guard) != C00 || term_ext(guard) != OMNI_NAM_NIL) {
              Term guard_result = omni_interpret(guard);
              if (term_tag(guard_result) == NUM && term_val(guard_result) == 0) {
                omni_interp_pop(bindings_count);
                continue;
              }
              if (term_tag(guard_result) == C00 && term_ext(guard_result) == OMNI_NAM_FALS) {
                omni_interp_pop(bindings_count);
                continue;
              }
            }
            Term result = omni_interpret(body);
            omni_interp_pop(bindings_count);
            return result;
          }
          continue;
        }
      }
    }

    // No match found - return nil or error
    return omni_nil();
  }

  // For other constructors, evaluate children
  u32 arity = tag - C00;
  for (u32 i = 0; i < arity; i++) {
    HEAP[val + i] = omni_interpret(HEAP[val + i]);
  }
  return t;
}

// =============================================================================
// Reduction with FFI Interception
// =============================================================================

fn Term omni_reduce_with_ffi(Term t) {
  // Reduce to weak normal form
  Term result = wnf(t);

  // Check if it's an FFI node (#FFI{name, args} has 2 args = C02)
  if (term_tag(result) == C02 && term_ext(result) == OMNI_NAM_FFI) {
    // Dispatch FFI call
    result = omni_ffi_dispatch(result);
  }

  return result;
}

// Full normalization with FFI
fn Term omni_normalize(Term t) {
  Term wnf_result = omni_reduce_with_ffi(t);

  // Continue normalization if needed
  u32 tag = term_tag(wnf_result);

  // Check if it's a constructor (C00-C16)
  if (tag >= C00 && tag <= C16) {
    u32 val = term_val(wnf_result);
    u32 arity = tag - C00;

    for (u32 i = 0; i < arity; i++) {
      Term child = HEAP[val + i];
      Term normalized = omni_normalize(child);
      HEAP[val + i] = normalized;
    }
  }

  return wnf_result;
}

// =============================================================================
// Main Entry Points
// =============================================================================

fn int run_parse_only(const char *source, int debug) {
  OmniParse parse;
  omni_parse_init(&parse, source);

  Term ast = omni_parse(&parse);

  if (parse.error) {
    fprintf(stderr, "Parse error at line %u, col %u: %s\n",
            parse.line, parse.col, parse.error);
    return 1;
  }

  print_ast(ast);
  return 0;
}

fn int run_compile_only(const char *source, const char *output, int debug) {
  OmniParse parse;
  omni_parse_init(&parse, source);

  Term ast = omni_parse(&parse);

  if (parse.error) {
    fprintf(stderr, "Parse error at line %u, col %u: %s\n",
            parse.line, parse.col, parse.error);
    return 1;
  }

  FILE *out = stdout;
  if (output) {
    out = fopen(output, "w");
    if (!out) {
      fprintf(stderr, "Error: Cannot create output file '%s'\n", output);
      return 1;
    }
  }

  // Emit HVM4 code
  fprintf(out, "// Generated HVM4 code from OmniLisp\n\n");
  omni_compile_emit(out, ast);
  fprintf(out, "\n// Entry point\n");
  fprintf(out, "@main = @omni_run(@omni_main)\n");

  if (output) {
    fclose(out);
    printf("Compiled to: %s\n", output);
  }

  return 0;
}

fn int run_evaluate(const char *source, int collapse_limit, int stats, int debug) {
  OmniParse parse;
  omni_parse_init(&parse, source);

  Term ast = omni_parse(&parse);

  if (parse.error) {
    fprintf(stderr, "Parse error at line %u, col %u: %s\n",
            parse.line, parse.col, parse.error);
    return 1;
  }

  if (debug) {
    printf("AST:\n");
    print_ast(ast);
    printf("\nEvaluating...\n\n");
  }

  // Load runtime.hvm4 if not already loaded
  int runtime_err = omni_load_runtime();

  Term result;

  if (runtime_err == 0 && g_runtime_loaded) {
    // Use the HVM4-based interpreter from runtime.hvm4

    // Look up @omni_eval and @omni_menv_empty
    u32 eval_id = table_find("omni_eval", 9);
    u32 menv_id = table_find("omni_menv_empty", 15);

    if (BOOK[eval_id] == 0 || BOOK[menv_id] == 0) {
      fprintf(stderr, "Error: runtime.hvm4 missing required definitions\n");
      return 1;
    }

    // Build: @omni_eval(@omni_menv_empty)(ast)
    Term eval_ref = term_new_ref(eval_id);
    Term menv_ref = term_new_ref(menv_id);

    // @omni_eval(@omni_menv_empty)
    Term eval_with_menv = term_new_app(eval_ref, menv_ref);

    // @omni_eval(@omni_menv_empty)(ast)
    Term eval_expr = term_new_app(eval_with_menv, ast);

    // Evaluate to normal form
    result = eval_normalize(eval_expr);
  } else {
    // Fallback: use direct C interpreter
    result = omni_interpret(ast);
  }

  printf("Result: ");
  print_term(result);
  printf("\n");

  if (stats) {
    printf("\nStatistics:\n");
    printf("  Handles allocated: %u\n", omni_ffi_handle_count());
    printf("  Interactions: %llu\n", (unsigned long long)wnf_itrs_total());
  }

  return 0;
}

// =============================================================================
// REPL - Interactive Read-Eval-Print Loop
// =============================================================================

#define REPL_BUFFER_SIZE 4096

// Evaluate a single expression and write result to output
// Returns result string (caller must free) or NULL on error
fn char* eval_to_string(const char *source, int debug) {
  OmniParse parse;
  omni_parse_init(&parse, source);

  Term ast = omni_parse(&parse);

  if (parse.error) {
    char *err = (char*)malloc(256);
    snprintf(err, 256, "Parse error at line %u, col %u: %s",
             parse.line, parse.col, parse.error);
    return err;
  }

  // Load runtime if not loaded
  omni_load_runtime();

  Term result;

  if (g_runtime_loaded) {
    // Use the HVM4 interpreter
    u32 eval_id = table_find("omni_eval", 9);
    u32 menv_id = table_find("omni_menv_empty", 15);

    if (BOOK[eval_id] != 0 && BOOK[menv_id] != 0) {
      Term eval_ref = term_new_ref(eval_id);
      Term menv_ref = term_new_ref(menv_id);
      Term eval_with_menv = term_new_app(eval_ref, menv_ref);
      Term eval_expr = term_new_app(eval_with_menv, ast);
      result = eval_normalize(eval_expr);
    } else {
      result = omni_interpret(ast);
    }
  } else {
    result = omni_interpret(ast);
  }

  // Convert result to string
  char *buf = (char*)malloc(REPL_BUFFER_SIZE);
  FILE *memstream = fmemopen(buf, REPL_BUFFER_SIZE - 1, "w");
  if (!memstream) {
    free(buf);
    return strdup("Error: failed to create output stream");
  }

  print_term_ex(memstream, result);
  fclose(memstream);

  return buf;
}

fn int run_repl(int debug) {
  char input[REPL_BUFFER_SIZE];
  char *line;

  printf("OmniLisp REPL v0.1.0\n");
  printf("Type expressions to evaluate. Ctrl+D to exit.\n\n");

  while (g_running) {
    printf("λ> ");
    fflush(stdout);

    line = fgets(input, REPL_BUFFER_SIZE, stdin);
    if (!line) {
      // EOF (Ctrl+D)
      printf("\nBye!\n");
      break;
    }

    // Skip empty lines
    size_t len = strlen(line);
    if (len == 0 || (len == 1 && line[0] == '\n')) {
      continue;
    }

    // Remove trailing newline
    if (len > 0 && line[len-1] == '\n') {
      line[len-1] = '\0';
    }

    // Special commands
    if (strcmp(line, ":q") == 0 || strcmp(line, ":quit") == 0) {
      printf("Bye!\n");
      break;
    }
    if (strcmp(line, ":h") == 0 || strcmp(line, ":help") == 0) {
      printf("Commands:\n");
      printf("  :q, :quit   Exit REPL\n");
      printf("  :h, :help   Show this help\n");
      printf("  :c, :clear  Clear screen\n");
      printf("\n");
      continue;
    }
    if (strcmp(line, ":c") == 0 || strcmp(line, ":clear") == 0) {
      printf("\033[2J\033[H");  // ANSI clear screen
      continue;
    }

    // Evaluate expression
    char *result = eval_to_string(line, debug);
    if (result) {
      printf("%s\n", result);
      free(result);
    }
  }

  return 0;
}

// =============================================================================
// Socket Server - For Editor Integration (neovim, etc.)
// =============================================================================
//
// Protocol:
//   Client sends: <expression>\n
//   Server sends: <result>\x00\n  (null byte marks end of result)
//
// The null byte allows multi-line results while still having a clear delimiter.
// Neovim/editors can read until they see \x00\n to know the response is complete.

fn int handle_client(int client_fd, int debug) {
  char buffer[REPL_BUFFER_SIZE];
  ssize_t bytes_read;

  while (g_running) {
    bytes_read = recv(client_fd, buffer, REPL_BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
      // Client disconnected or error
      break;
    }

    buffer[bytes_read] = '\0';

    // Remove trailing newline
    if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
      buffer[bytes_read - 1] = '\0';
    }

    // Skip empty input
    if (strlen(buffer) == 0) {
      continue;
    }

    // Special: :ping for keepalive
    if (strcmp(buffer, ":ping") == 0) {
      send(client_fd, "pong\0\n", 6, 0);
      continue;
    }

    // Evaluate and send result
    char *result = eval_to_string(buffer, debug);
    if (result) {
      send(client_fd, result, strlen(result), 0);
      free(result);
    } else {
      send(client_fd, "Error: evaluation failed", 24, 0);
    }

    // Send delimiter: null byte + newline
    send(client_fd, "\0\n", 2, 0);
  }

  close(client_fd);
  return 0;
}

fn int run_server(int port, int debug) {
  int server_fd, client_fd;
  struct sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket failed");
    return 1;
  }

  // Allow address reuse
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt failed");
    close(server_fd);
    return 1;
  }

  // Bind to port
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(server_fd);
    return 1;
  }

  // Listen
  if (listen(server_fd, 3) < 0) {
    perror("listen failed");
    close(server_fd);
    return 1;
  }

  printf("OmniLisp server listening on port %d\n", port);
  printf("Connect with: nc localhost %d\n", port);
  printf("Or from neovim: :lua vim.fn.sockconnect('tcp', 'localhost:%d')\n", port);
  printf("Press Ctrl+C to stop.\n\n");

  // Setup signal handler for graceful shutdown
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  while (g_running) {
    // Use poll to allow checking g_running periodically
    struct pollfd pfd = {server_fd, POLLIN, 0};
    int poll_result = poll(&pfd, 1, 1000);  // 1 second timeout

    if (poll_result < 0) {
      if (errno == EINTR) continue;  // Interrupted by signal
      perror("poll failed");
      break;
    }

    if (poll_result == 0) {
      continue;  // Timeout, check g_running and loop
    }

    // Accept connection
    client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_fd < 0) {
      if (errno == EINTR) continue;
      perror("accept failed");
      continue;
    }

    printf("Client connected from %s:%d\n",
           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    // Handle client (single-threaded for simplicity)
    handle_client(client_fd, debug);

    printf("Client disconnected\n");
  }

  printf("\nShutting down server...\n");
  close(server_fd);
  return 0;
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char *argv[]) {
  OmniOptions opts = parse_options(argc, argv);

  if (opts.help) {
    print_usage(argv[0]);
    return 0;
  }

  if (opts.version) {
    print_version();
    return 0;
  }

  // Initialize runtime
  omni_runtime_init();

  int result = 0;

  if (opts.server_port > 0) {
    // Socket server mode
    result = run_server(opts.server_port, opts.debug);
  } else if (opts.interactive) {
    // Interactive REPL mode
    result = run_repl(opts.debug);
  } else if (opts.eval_mode && opts.expr) {
    // Evaluate expression
    if (opts.parse_only) {
      result = run_parse_only(opts.expr, opts.debug);
    } else if (opts.compile_only) {
      result = run_compile_only(opts.expr, opts.output, opts.debug);
    } else {
      result = run_evaluate(opts.expr, opts.collapse, opts.stats, opts.debug);
    }
  } else if (opts.file) {
    // Process file
    char *source = read_file(opts.file);
    if (!source) {
      result = 1;
    } else {
      if (opts.parse_only) {
        result = run_parse_only(source, opts.debug);
      } else if (opts.compile_only) {
        result = run_compile_only(source, opts.output, opts.debug);
      } else {
        result = run_evaluate(source, opts.collapse, opts.stats, opts.debug);
      }
      free(source);
    }
  } else {
    // No input - start interactive REPL by default
    result = run_repl(opts.debug);
  }

  // Cleanup
  omni_runtime_cleanup();

  return result;
}

