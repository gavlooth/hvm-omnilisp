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

// FFI dispatch function pointer - set up after includes
// Uses void* to avoid type dependency issues with include order
static void* (*omni_ffi_dispatch_fn)(void*) = 0;

// Counter for debugging
static unsigned long omni_hook_calls = 0;
static unsigned long omni_ffi_tag_counts[20] = {0};  // Count by tag C00-C16

// FFI dispatch hook - called from wnf when encountering C02 constructors
// Uses unsigned long (same size as Term/u64) to avoid type issues
static inline unsigned long omni_ffi_dispatch_hook_wrapper(unsigned long t) {
  omni_hook_calls++;

  // Count by tag (C00-C16 are tags 24-40)
  // Tag at bits 56-62, ext at bits 32-55
  unsigned char tag = (t >> 56) & 0x7F;
  if (tag >= 24 && tag <= 40) {
    omni_ffi_tag_counts[tag - 24]++;
  }

  if (omni_ffi_dispatch_fn) {
    return (unsigned long)omni_ffi_dispatch_fn((void*)t);
  }
  return t;
}

// Enable FFI dispatch hook in HVM4 wnf
#define OMNI_FFI_DISPATCH_HOOK omni_ffi_dispatch_hook_wrapper

// Include HVM4 runtime first
#include "../hvm4/clang/hvm4.c"

// Include OmniLisp components
#include "omnilisp/nick/omnilisp.c"
#include "omnilisp/ffi/handle.c"
#include "omnilisp/ffi/thread_pool.c"
#include "omnilisp/parse/_.c"
#include "omnilisp/compile/_.c"

// =============================================================================
// FFI Dispatch Hook Implementation
// =============================================================================

// Debug flag for FFI dispatch
static int omni_ffi_debug = 0;  // Disabled by default

// Count how many C02 nodes we see
static unsigned long omni_c02_count = 0;

// Track unique ext values seen
static u32 unique_exts[1000];
static u32 unique_ext_counts[1000];
static u32 unique_ext_count = 0;

static void track_ext(u32 ext) {
  for (u32 i = 0; i < unique_ext_count; i++) {
    if (unique_exts[i] == ext) {
      unique_ext_counts[i]++;
      return;
    }
  }
  if (unique_ext_count < 1000) {
    unique_exts[unique_ext_count] = ext;
    unique_ext_counts[unique_ext_count] = 1;
    unique_ext_count++;
  }
}

// Counter for all constructor entries
static unsigned long omni_ctr_enter_count = 0;

// Tag counts for all constructors
static unsigned long omni_tag_counts[17] = {0};  // C00-C16
static unsigned long omni_ext_per_tag[17][100];  // Up to 100 unique exts per tag
static unsigned long omni_ext_count_per_tag[17][100];
static unsigned int omni_unique_ext_per_tag[17];

static void track_constructor(u8 tag, u32 ext) {
  int idx = tag - C00;
  if (idx < 0 || idx > 16) return;
  omni_tag_counts[idx]++;

  // Track unique exts for this tag
  for (unsigned int i = 0; i < omni_unique_ext_per_tag[idx]; i++) {
    if (omni_ext_per_tag[idx][i] == ext) {
      omni_ext_count_per_tag[idx][i]++;
      return;
    }
  }
  if (omni_unique_ext_per_tag[idx] < 100) {
    int n = omni_unique_ext_per_tag[idx]++;
    omni_ext_per_tag[idx][n] = ext;
    omni_ext_count_per_tag[idx][n] = 1;
  }
}

// Actual FFI dispatch function (now that types are available)
static void* omni_ffi_dispatch_impl(void* term_ptr) {
  Term t = (Term)term_ptr;
  u8 tag = term_tag(t);

  // Track ALL constructor entries (C00-C16, tags 24-40)
  if (tag >= C00 && tag <= C16) {
    omni_ctr_enter_count++;
    u32 ext = term_ext(t);
    track_constructor(tag, ext);

    // Track C02 specifically for unique ext summary
    if (tag == C02) {
      omni_c02_count++;
      track_ext(ext);
    }

    // Log first 20 constructor entries
    if (omni_ffi_debug && omni_ctr_enter_count <= 20) {
      fprintf(stderr, "[CTR ENTER] C%02d ext=%u%s\n", tag - C00, ext,
              ext == OMNI_NAM_FFI ? " <-- FFI!" : "");
    }
  }

  // Check if this is an FFI node: C02 with ext == OMNI_NAM_FFI
  if (tag == C02 && term_ext(t) == OMNI_NAM_FFI) {
    if (omni_ffi_debug) {
      fprintf(stderr, "[FFI] Dispatching FFI node\n");
    }
    return (void*)omni_ffi_dispatch(t);
  }
  return term_ptr;
}

// Initialize the FFI dispatch hook
static void omni_ffi_hook_init(void) {
  omni_ffi_dispatch_fn = omni_ffi_dispatch_impl;
}

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
  int hvm4_print;      // -T: Use HVM4's print_term for output
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
    {"term-print",  no_argument,       0, 'T'},
    {0, 0, 0, 0}
  };

  int opt;
  int opt_index = 0;

  while ((opt = getopt_long(argc, argv, "hvpce:iS:o:dsC:T", long_options, &opt_index)) != -1) {
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
      case 'T': opts.hvm4_print = 1; break;
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
// User-Friendly Value Printing for REPL
// =============================================================================

fn void omni_print_value_to(FILE *out, Term t);

fn void omni_print_list_to(FILE *out, Term t) {
  fprintf(out, "(");
  int first = 1;
  while (term_tag(t) == C02 && term_ext(t) == OMNI_NAM_CON) {
    u32 loc = term_val(t);
    if (!first) fprintf(out, " ");
    first = 0;
    omni_print_value_to(out, HEAP[loc]);
    t = HEAP[loc + 1];
  }
  // Check for improper list
  if (!(term_tag(t) == C00 && term_ext(t) == OMNI_NAM_NIL)) {
    fprintf(out, " . ");
    omni_print_value_to(out, t);
  }
  fprintf(out, ")");
}

fn void omni_print_value_to(FILE *out, Term t) {
  u32 tag = term_tag(t);
  u32 ext = term_ext(t);
  u32 val = term_val(t);

  // Raw number
  if (tag == NUM) {
    fprintf(out, "%u", val);
    return;
  }

  // Constructors
  if (tag >= C00 && tag <= C16) {
    // #Cst{n} or #Lit{n} - integer literal
    if (ext == OMNI_NAM_CST || ext == OMNI_NAM_LIT) {
      Term inner = HEAP[val];
      if (term_tag(inner) == NUM) {
        fprintf(out, "%u", term_val(inner));
      } else {
        omni_print_value_to(out, inner);
      }
      return;
    }

    // #True{} - boolean true
    if (ext == OMNI_NAM_TRUE) {
      fprintf(out, "true");
      return;
    }

    // #Fals{} - boolean false
    if (ext == OMNI_NAM_FALS) {
      fprintf(out, "false");
      return;
    }

    // #Noth{} - nothing
    if (ext == OMNI_NAM_NOTH) {
      fprintf(out, "nothing");
      return;
    }

    // #NIL{} - empty list
    if (ext == OMNI_NAM_NIL) {
      fprintf(out, "()");
      return;
    }

    // #CON{h, t} - cons cell (list)
    if (ext == OMNI_NAM_CON) {
      omni_print_list_to(out, t);
      return;
    }

    // #Sym{nick} - symbol
    if (ext == OMNI_NAM_SYM) {
      char name[64];
      Term nick_term = HEAP[val];
      u32 nick = term_tag(nick_term) == NUM ? term_val(nick_term) : term_ext(nick_term);
      nick_to_str(nick, name, sizeof(name));
      fprintf(out, "%s", name);
      return;
    }

    // #Str{...} - string (nick-encoded)
    if (ext == OMNI_NAM_STR) {
      Term str_nick = HEAP[val];
      if (term_tag(str_nick) == NUM) {
        char str[256];
        nick_to_str(term_val(str_nick), str, sizeof(str));
        fprintf(out, "\"%s\"", str);
      } else {
        fprintf(out, "\"...\"");
      }
      return;
    }

    // #Lam{body} - lambda
    if (ext == OMNI_NAM_LAM) {
      fprintf(out, "<lambda>");
      return;
    }

    // #Clo{env, body} - closure
    if (ext == OMNI_NAM_CLO) {
      fprintf(out, "<closure>");
      return;
    }

    // #CloR{marker, body} - recursive closure
    if (ext == OMNI_NAM_CLOR) {
      fprintf(out, "<function>");
      return;
    }

    // #Meth{...} - method
    if (ext == OMNI_NAM_METH) {
      fprintf(out, "<method>");
      return;
    }

    // #GFun{name, methods} - generic function
    if (ext == OMNI_NAM_GFUN) {
      fprintf(out, "<function>");
      return;
    }

    // #Prnt{msg} - print result (show the value)
    if (ext == OMNI_NAM_PRNT || ext == OMNI_NAM_PRNL) {
      omni_print_value_to(out, HEAP[val]);
      return;
    }

    // #Err{msg} - error
    if (ext == OMNI_NAM_ERR) {
      fprintf(out, "Error: ");
      omni_print_value_to(out, HEAP[val]);
      return;
    }

    // Default: show as #Name{...}
    char name[16];
    nick_to_str(ext, name, sizeof(name));
    fprintf(out, "#%s", name);
    if (tag > C00) {
      fprintf(out, "{");
      u32 arity = tag - C00;
      for (u32 i = 0; i < arity && i < 3; i++) {
        if (i > 0) fprintf(out, ", ");
        omni_print_value_to(out, HEAP[val + i]);
      }
      if (arity > 3) fprintf(out, ", ...");
      fprintf(out, "}");
    }
    return;
  }

  // References
  if (tag == REF) {
    char *name = TABLE[ext];
    if (name) {
      fprintf(out, "%s", name);
    } else {
      fprintf(out, "@%u", ext);
    }
    return;
  }

  // Variables
  if (tag == VAR) {
    fprintf(out, "v%u", val);
    return;
  }

  // Default
  fprintf(out, "<%u:%u:%u>", tag, ext, val);
}

fn void omni_print_value(Term t) {
  omni_print_value_to(stdout, t);
}

// =============================================================================
// Runtime Initialization
// =============================================================================

// Track if runtime.hvm4 has been loaded
static int g_runtime_loaded = 0;

// Helper to find runtime file in multiple locations
fn char* omni_find_runtime_file(const char *filename) {
  static char path_buf[512];

  // Try 1: Current directory (lib/filename)
  snprintf(path_buf, sizeof(path_buf), "lib/%s", filename);
  if (access(path_buf, R_OK) == 0) return path_buf;

  // Try 2: Parent directory (../lib/filename) - when running from clang/
  snprintf(path_buf, sizeof(path_buf), "../lib/%s", filename);
  if (access(path_buf, R_OK) == 0) return path_buf;

  // Try 3: Executable directory (for installed builds)
  // Use /proc/self/exe on Linux to get executable path
  char exe_path[256];
  ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
  if (len > 0) {
    exe_path[len] = '\0';
    // Find last / and truncate to get directory
    char *last_slash = strrchr(exe_path, '/');
    if (last_slash) {
      *last_slash = '\0';
      snprintf(path_buf, sizeof(path_buf), "%s/../lib/%s", exe_path, filename);
      if (access(path_buf, R_OK) == 0) return path_buf;
    }
  }

  return NULL;
}

// Load runtime.hvm4 into the HVM4 book
// Loads lib/prelude.hvm4 and lib/runtime.hvm4 definitions
fn int omni_load_runtime(void) {
  if (g_runtime_loaded) return 0;  // Already loaded

  // Find and load prelude.hvm4
  char *prelude_path = omni_find_runtime_file("prelude.hvm4");
  if (!prelude_path) {
    fprintf(stderr, "Error: Could not find lib/prelude.hvm4 (required for evaluation)\n");
    return 1;
  }

  char *prelude_src = sys_file_read(prelude_path);
  if (!prelude_src) {
    fprintf(stderr, "Error: Could not read %s\n", prelude_path);
    return 1;
  }

  PState ps = {
    .file = prelude_path,
    .src  = prelude_src,
    .pos  = 0,
    .len  = (u32)strlen(prelude_src),
    .line = 1,
    .col  = 1
  };
  parse_def(&ps);
  free(prelude_src);

  // Find and load runtime.hvm4
  char *runtime_path = omni_find_runtime_file("runtime.hvm4");
  if (!runtime_path) {
    fprintf(stderr, "Error: Could not find lib/runtime.hvm4 (required for evaluation)\n");
    return 1;
  }

  char *runtime_src = sys_file_read(runtime_path);
  if (!runtime_src) {
    fprintf(stderr, "Error: Could not read %s\n", runtime_path);
    return 1;
  }

  PState rs = {
    .file = runtime_path,
    .src  = runtime_src,
    .pos  = 0,
    .len  = (u32)strlen(runtime_src),
    .line = 1,
    .col  = 1
  };
  parse_def(&rs);
  free(runtime_src);

  // Verify critical functions are loaded
  u32 eval_id = table_find("omni_eval", 9);
  u32 menv_id = table_find("omni_menv_empty", 15);

  if (BOOK[eval_id] == 0 || BOOK[menv_id] == 0) {
    fprintf(stderr, "Error: runtime.hvm4 missing required definitions (@omni_eval, @omni_menv_empty)\n");
    return 1;
  }

  g_runtime_loaded = 1;
  return 0;
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

  // Initialize FFI dispatch hook (must be after names init)
  omni_ffi_hook_init();
}

fn void omni_runtime_cleanup(void) {
  // Cleanup FFI
  omni_ffi_pool_shutdown();
  omni_ffi_handle_cleanup();
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

fn int run_evaluate(const char *source, int collapse_limit, int stats, int debug, int hvm4_print) {
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

    // Evaluate to strong normal form
    result = omni_normalize(eval_expr);
  } else {
    // Runtime is required - no fallback interpreter
    fprintf(stderr, "Error: runtime.hvm4 failed to load - cannot evaluate\n");
    return 1;
  }

  // Debug: show how many times the FFI hook was called
  if (omni_ffi_debug) {
    fprintf(stderr, "[DEBUG] FFI hook called %lu times\n", omni_hook_calls);
    fprintf(stderr, "[DEBUG] Total constructor enters: %lu\n", omni_ctr_enter_count);
    // Show tag distribution with ext breakdown
    fprintf(stderr, "[DEBUG] Constructor tag breakdown:\n");
    for (int i = 0; i <= 16; i++) {
      if (omni_tag_counts[i] > 0) {
        fprintf(stderr, "  C%02d: %lu entries, %u unique exts\n", i, omni_tag_counts[i], omni_unique_ext_per_tag[i]);
        // Show top exts for this tag
        for (unsigned int j = 0; j < omni_unique_ext_per_tag[i] && j < 5; j++) {
          fprintf(stderr, "    ext=%lu (count=%lu)%s\n",
                  (unsigned long)omni_ext_per_tag[i][j],
                  (unsigned long)omni_ext_count_per_tag[i][j],
                  omni_ext_per_tag[i][j] == OMNI_NAM_FFI ? " <-- FFI!" : "");
        }
      }
    }
    // Also check if FFI ext appears in C02
    fprintf(stderr, "[DEBUG] Checking for FFI (ext=%u) in C02: ", OMNI_NAM_FFI);
    int found = 0;
    for (unsigned int j = 0; j < omni_unique_ext_per_tag[2]; j++) {
      if (omni_ext_per_tag[2][j] == OMNI_NAM_FFI) {
        fprintf(stderr, "FOUND! (count=%lu)\n", (unsigned long)omni_ext_count_per_tag[2][j]);
        found = 1;
        break;
      }
    }
    if (!found) fprintf(stderr, "NOT FOUND\n");
  }

  printf("Result: ");
  if (hvm4_print) {
    // Use HVM4's print_term for coverage testing
    print_term(result);
  } else {
    omni_print_value(result);
  }
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

  if (!g_runtime_loaded) {
    return strdup("Error: runtime.hvm4 failed to load");
  }

  // Use the HVM4 interpreter
  u32 eval_id = table_find("omni_eval", 9);
  u32 menv_id = table_find("omni_menv_empty", 15);

  if (BOOK[eval_id] == 0 || BOOK[menv_id] == 0) {
    return strdup("Error: runtime.hvm4 missing required definitions");
  }

  Term eval_ref = term_new_ref(eval_id);
  Term menv_ref = term_new_ref(menv_id);
  Term eval_with_menv = term_new_app(eval_ref, menv_ref);
  Term eval_expr = term_new_app(eval_with_menv, ast);
  result = omni_normalize(eval_expr);

  // Convert result to string using user-friendly printer
  char *buf = (char*)malloc(REPL_BUFFER_SIZE);
  FILE *memstream = fmemopen(buf, REPL_BUFFER_SIZE - 1, "w");
  if (!memstream) {
    free(buf);
    return strdup("Error: failed to create output stream");
  }

  omni_print_value_to(memstream, result);
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
      result = run_evaluate(opts.expr, opts.collapse, opts.stats, opts.debug, opts.hvm4_print);
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
        result = run_evaluate(source, opts.collapse, opts.stats, opts.debug, opts.hvm4_print);
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

