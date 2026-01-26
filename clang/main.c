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
    case CTR: {
      // Print constructor name
      char name[8];
      nick_b64_to_str(ext, name);
      printf("#%s{\n", name);

      // Print children based on arity
      u32 arity = term_arity(t);
      for (u32 i = 0; i < arity; i++) {
        print_term_indent((Term)heap[val + i], indent + 1);
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
// HVM4 Code Emission
// =============================================================================

fn void emit_hvm4_code(FILE *out, OmniEmit *emit) {
  fprintf(out, "// Generated HVM4 code from OmniLisp\n\n");

  // Emit the output buffer
  fprintf(out, "%s", emit->out);

  // Emit main entry point
  fprintf(out, "\n// Entry point\n");
  fprintf(out, "@main = @omni_run(@omni_main)\n");
}

// =============================================================================
// Runtime Initialization
// =============================================================================

fn void omni_runtime_init(void) {
  // Initialize HVM4
  hvm4_init();

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
// Reduction with FFI Interception
// =============================================================================

fn Term omni_reduce_with_ffi(Term t) {
  // Reduce to weak normal form
  Term result = wnf(t);

  // Check if it's an FFI node
  if (term_tag(result) == CTR && term_ext(result) == OMNI_NAM_FFI) {
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

  if (tag == CTR) {
    u32 val = term_val(wnf_result);
    u32 arity = term_arity(wnf_result);

    for (u32 i = 0; i < arity; i++) {
      Term child = (Term)heap[val + i];
      Term normalized = omni_normalize(child);
      heap[val + i] = (u32)normalized;
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

  OmniEmit emit;
  omni_emit_init(&emit);

  omni_compile(&emit, ast);

  FILE *out = stdout;
  if (output) {
    out = fopen(output, "w");
    if (!out) {
      fprintf(stderr, "Error: Cannot create output file '%s'\n", output);
      return 1;
    }
  }

  emit_hvm4_code(out, &emit);

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

  // For now, directly normalize the parsed term
  // In a full implementation, we would:
  // 1. Compile to HVM4
  // 2. Load into book
  // 3. Run evaluator

  Term result = omni_normalize(ast);

  printf("Result: ");
  print_term(result, 0);  // Use HVM4's print_term
  printf("\n");

  if (stats) {
    printf("\nStatistics:\n");
    printf("  Heap used: %u words\n", heap_ptr);
    printf("  Handles allocated: %u\n", omni_ffi_handle_count());
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

  Term result = omni_normalize(ast);

  // Convert result to string
  // For now, use a simple buffer approach
  char *buf = (char*)malloc(REPL_BUFFER_SIZE);
  FILE *memstream = fmemopen(buf, REPL_BUFFER_SIZE - 1, "w");
  if (!memstream) {
    free(buf);
    return strdup("Error: failed to create output stream");
  }

  // Redirect print_term to our buffer
  FILE *old_stdout = stdout;
  stdout = memstream;
  print_term(result, 0);
  stdout = old_stdout;
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

