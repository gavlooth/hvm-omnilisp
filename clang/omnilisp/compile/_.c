// OmniLisp Compiler
// Emits HVM4 source from OmniLisp AST
//
// Compilation strategy:
// - Types are erased (used for dispatch at compile time)
// - Simple patterns -> native HVM4 λ{...} matching
// - Complex patterns -> runtime interpretation
// - FFI calls -> #FFI{name, args} nodes
// - Effects -> CPS transformation

#include "../parse/_.c"

// =============================================================================
// Compiler State
// =============================================================================

typedef struct {
  FILE *out;
  char *env_names[256];
  u32 env_len;
  u32 fresh;
} OmniEmit;

fn void omni_env_init(OmniEmit *e) {
  e->env_len = 0;
  e->fresh = 0;
}

fn const char *omni_env_push(OmniEmit *e) {
  static char buf[64];
  snprintf(buf, sizeof(buf), "v%u", e->fresh++);
  if (e->env_len >= 256) {
    fprintf(stderr, "OMNI_ERROR: too many bindings in emit\n");
    exit(1);
  }
  char *dup = strdup(buf);
  if (!dup) {
    fprintf(stderr, "OMNI_ERROR: out of memory in env_push\n");
    exit(1);
  }
  e->env_names[e->env_len++] = dup;
  return e->env_names[e->env_len - 1];
}

fn void omni_env_pop(OmniEmit *e, u32 count) {
  while (count > 0 && e->env_len > 0) {
    free(e->env_names[--e->env_len]);
    count--;
  }
}

fn const char *omni_env_get(OmniEmit *e, u32 idx) {
  if (idx >= e->env_len) return "?";
  return e->env_names[e->env_len - 1 - idx];
}

// =============================================================================
// Helper Functions
// =============================================================================

fn u32 omni_ctr_arity(Term t) {
  u8 tag = term_tag(t);
  if (tag >= C00 && tag <= C16) {
    return (u32)(tag - C00);
  }
  return 0;
}

// omni_ctr_arg is defined in parse/_.c

// Nick decoder
fn const char *omni_nick_to_name(u32 nick) {
  static char buf[8];
  u32 k = nick;
  int len = 0;
  char tmp[5] = {0};
  while (k > 0 && len < 4) {
    u32 b = k & 0x3F;
    if (b == 0) tmp[len++] = '_';
    else if (b <= 26) tmp[len++] = 'a' + b - 1;
    else if (b <= 52) tmp[len++] = 'A' + b - 27;
    else if (b <= 62) tmp[len++] = '0' + b - 53;
    else tmp[len++] = '$';
    k >>= 6;
  }
  for (int i = 0; i < len; i++) buf[i] = tmp[len - 1 - i];
  buf[len] = '\0';
  return buf;
}

// =============================================================================
// Forward Declaration
// =============================================================================

fn void omni_emit_term(OmniEmit *e, Term t);

// =============================================================================
// Lambda Emission
// =============================================================================

fn void omni_emit_lambda(OmniEmit *e, Term body, int is_rec) {
  const char *name = omni_env_push(e);
  fputs("λ&", e->out);
  fputs(name, e->out);
  fputc('.', e->out);

  // Check if body is another lambda
  u8 tag = term_tag(body);
  if (tag >= C00 && tag <= C16) {
    u32 nam = term_ext(body);
    if (nam == OMNI_NAM_LAM && omni_ctr_arity(body) == 1) {
      omni_emit_lambda(e, omni_ctr_arg(body, 0), 0);
      omni_env_pop(e, 1);
      return;
    }
    if (nam == OMNI_NAM_LAMR && omni_ctr_arity(body) == 1) {
      omni_emit_lambda(e, omni_ctr_arg(body, 0), 1);
      omni_env_pop(e, 1);
      return;
    }
  }

  omni_emit_term(e, body);
  omni_env_pop(e, 1);
}

// =============================================================================
// Pattern Match Emission
// =============================================================================

// Check if pattern can use native HVM4 matching
fn int omni_pattern_is_simple(Term pattern) {
  u8 tag = term_tag(pattern);
  if (tag < C00 || tag > C16) return 0;

  u32 nam = term_ext(pattern);

  // Wildcard is simple
  if (nam == OMNI_NAM_PWLD) return 1;

  // Variable is simple (binds value)
  if (nam == OMNI_NAM_PVAR) return 1;

  // Literal is simple
  if (nam == OMNI_NAM_PLIT) return 1;

  // Constructor pattern is simple if all args are simple variables
  if (nam == OMNI_NAM_PCTR) {
    Term args = omni_ctr_arg(pattern, 1);
    Term curr = args;
    while (term_tag(curr) >= C00 && term_tag(curr) <= C16) {
      u32 curr_nam = term_ext(curr);
      if (curr_nam == OMNI_NAM_NIL) break;
      if (curr_nam == OMNI_NAM_CON) {
        Term arg_pat = omni_ctr_arg(curr, 0);
        u8 arg_tag = term_tag(arg_pat);
        if (arg_tag >= C00 && arg_tag <= C16) {
          u32 arg_nam = term_ext(arg_pat);
          // Only allow variable patterns in constructor args for native match
          if (arg_nam != OMNI_NAM_PVAR && arg_nam != OMNI_NAM_PWLD) {
            return 0;
          }
        }
        curr = omni_ctr_arg(curr, 1);
        continue;
      }
      break;
    }
    return 1;
  }

  // Guards, as-patterns, rest-patterns need runtime
  return 0;
}

fn int omni_match_needs_runtime(Term cases) {
  Term curr = cases;
  while (term_tag(curr) >= C00 && term_tag(curr) <= C16) {
    u32 nam = term_ext(curr);
    if (nam == OMNI_NAM_NIL) break;
    if (nam == OMNI_NAM_CON) {
      Term case_term = omni_ctr_arg(curr, 0);
      u8 case_tag = term_tag(case_term);
      if (case_tag >= C00 && case_tag <= C16 && term_ext(case_term) == OMNI_NAM_CASE) {
        Term pattern = omni_ctr_arg(case_term, 0);
        Term guard = omni_ctr_arg(case_term, 1);

        // Non-nil guard needs runtime
        if (term_tag(guard) >= C00 && term_tag(guard) <= C16 &&
            term_ext(guard) != OMNI_NAM_NIL) {
          return 1;
        }

        if (!omni_pattern_is_simple(pattern)) {
          return 1;
        }
      }
      curr = omni_ctr_arg(curr, 1);
      continue;
    }
    break;
  }
  return 0;
}

// Emit native HVM4 pattern match
fn void omni_emit_native_case(OmniEmit *e, Term pattern, Term body) {
  u8 tag = term_tag(pattern);
  if (tag < C00 || tag > C16) return;

  u32 nam = term_ext(pattern);

  // Wildcard: _ : body
  if (nam == OMNI_NAM_PWLD) {
    fputs("_ : λ&u. ", e->out);
    omni_emit_term(e, body);
    return;
  }

  // Variable: _ : λ&v. body (binds the value)
  if (nam == OMNI_NAM_PVAR) {
    const char *var_name = omni_env_push(e);
    fputs("_ : λ&", e->out);
    fputs(var_name, e->out);
    fputs(". ", e->out);
    omni_emit_term(e, body);
    omni_env_pop(e, 1);
    return;
  }

  // Literal: n : body
  if (nam == OMNI_NAM_PLIT) {
    Term lit = omni_ctr_arg(pattern, 0);
    omni_emit_term(e, lit);
    fputs(": ", e->out);
    omni_emit_term(e, body);
    return;
  }

  // Constructor: #TAG: λ&a. λ&b. body
  if (nam == OMNI_NAM_PCTR) {
    Term tag_term = omni_ctr_arg(pattern, 0);
    Term args = omni_ctr_arg(pattern, 1);

    u32 tag_nick = term_val(tag_term);
    const char *ctr_name = omni_nick_to_name(tag_nick);
    fprintf(e->out, "#%s: ", ctr_name);

    // Count args and push bindings
    u32 n_args = 0;
    Term curr = args;
    while (term_tag(curr) >= C00 && term_tag(curr) <= C16 && term_ext(curr) == OMNI_NAM_CON) {
      n_args++;
      curr = omni_ctr_arg(curr, 1);
    }

    // Emit lambdas for args
    for (u32 i = 0; i < n_args; i++) {
      const char *arg_name = omni_env_push(e);
      fprintf(e->out, "λ&%s. ", arg_name);
    }

    omni_emit_term(e, body);

    omni_env_pop(e, n_args);
    return;
  }
}

fn void omni_emit_native_match(OmniEmit *e, Term scrutinee, Term cases) {
  // Emit scrutinee
  fputs("λ{", e->out);

  Term curr = cases;
  int first = 1;
  while (term_tag(curr) >= C00 && term_tag(curr) <= C16) {
    u32 nam = term_ext(curr);
    if (nam == OMNI_NAM_NIL) break;
    if (nam == OMNI_NAM_CON) {
      Term case_term = omni_ctr_arg(curr, 0);
      if (term_tag(case_term) >= C00 && term_tag(case_term) <= C16 &&
          term_ext(case_term) == OMNI_NAM_CASE) {
        Term pattern = omni_ctr_arg(case_term, 0);
        Term body = omni_ctr_arg(case_term, 2);

        if (!first) fputs("; ", e->out);
        first = 0;

        omni_emit_native_case(e, pattern, body);
      }
      curr = omni_ctr_arg(curr, 1);
      continue;
    }
    break;
  }

  fputs("}(", e->out);
  omni_emit_term(e, scrutinee);
  fputc(')', e->out);
}

// =============================================================================
// Main Term Emission
// =============================================================================

fn void omni_emit_term(OmniEmit *e, Term t) {
  u8 tag = term_tag(t);

  // Raw number
  if (tag == NUM) {
    fprintf(e->out, "%u", term_val(t));
    return;
  }

  // Reference
  if (tag == REF) {
    u32 id = term_val(t);
    const char *name = (id < BOOK_CAP) ? TABLE[id] : NULL;
    if (name) {
      fprintf(e->out, "@%s", name);
    } else {
      fprintf(e->out, "@ref_%u", id);
    }
    return;
  }

  // Constructors
  if (tag >= C00 && tag <= C16) {
    u32 nam = term_ext(t);
    u32 ari = omni_ctr_arity(t);

    // Literal: #Lit{n} -> #Cst{n}
    if (nam == OMNI_NAM_LIT && ari == 1) {
      fputs("#Cst{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }

    // Fixed-point: #Fix{hi, lo, scale}
    if (nam == OMNI_NAM_FIX && ari == 3) {
      fputs("#Fix{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 2));
      fputc('}', e->out);
      return;
    }

    // Symbol
    if (nam == OMNI_NAM_SYM && ari == 1) {
      fputs("#Sym{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }

    // Variable
    if (nam == OMNI_NAM_VAR && ari == 1) {
      Term idx_term = omni_ctr_arg(t, 0);
      if (term_tag(idx_term) == NUM) {
        u32 idx = term_val(idx_term);
        const char *name = omni_env_get(e, idx);
        fputs(name, e->out);
      } else {
        fputs("#Var{", e->out);
        omni_emit_term(e, idx_term);
        fputc('}', e->out);
      }
      return;
    }

    // Lambda
    if (nam == OMNI_NAM_LAM && ari == 1) {
      omni_emit_lambda(e, omni_ctr_arg(t, 0), 0);
      return;
    }

    // Recursive lambda
    if (nam == OMNI_NAM_LAMR && ari == 1) {
      omni_emit_lambda(e, omni_ctr_arg(t, 0), 1);
      return;
    }

    // Application
    if (nam == OMNI_NAM_APP && ari == 2) {
      fputc('(', e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc(')', e->out);
      fputc('(', e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc(')', e->out);
      return;
    }

    // Multiple dispatch: Generic function definition
    // #GFun{name, methods}
    if (nam == OMNI_NAM_GFUN && ari == 2) {
      fputs("(@omni_make_generic(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // name
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // methods list
      fputs("))", e->out);
      return;
    }

    // Multiple dispatch: Method definition
    // #Meth{name, sig, impl}
    if (nam == OMNI_NAM_METH && ari == 3) {
      fputs("#Meth{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // name
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // signature (type list)
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 2));  // implementation
      fputc('}', e->out);
      return;
    }

    // Multiple dispatch: Dispatch call
    // #Disp{name, args}
    if (nam == OMNI_NAM_DISP && ari == 2) {
      fputs("(@omni_dispatch(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // name
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // args list
      fputs("))", e->out);
      return;
    }

    // Multiple dispatch: Signature
    // #Sig{types}
    if (nam == OMNI_NAM_SIG && ari == 1) {
      fputs("#Sig{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // types list
      fputc('}', e->out);
      return;
    }

    // Macro system: Syntax macro definition
    // #MSyn{name, patterns}
    if (nam == OMNI_NAM_MSYN && ari == 2) {
      fputs("(@omni_register_macro(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // name
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // patterns
      fputs("))", e->out);
      return;
    }

    // Macro system: Pattern rule
    // #MPat{pattern, template}
    if (nam == OMNI_NAM_MPAT && ari == 2) {
      fputs("#MPat{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // pattern
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // template
      fputc('}', e->out);
      return;
    }

    // Macro system: Pattern variable
    // #MVar{name}
    if (nam == OMNI_NAM_MVAR && ari == 1) {
      fputs("#MVar{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // name
      fputc('}', e->out);
      return;
    }

    // Macro system: Rest pattern
    // #MRst{name}
    if (nam == OMNI_NAM_MRST && ari == 1) {
      fputs("#MRst{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // name
      fputc('}', e->out);
      return;
    }

    // Macro system: Literal in pattern
    // #MLit{value}
    if (nam == OMNI_NAM_MLIT && ari == 1) {
      fputs("#MLit{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // value
      fputc('}', e->out);
      return;
    }

    // Macro system: Expansion call
    // #MExp{name, args}
    if (nam == OMNI_NAM_MEXP && ari == 2) {
      fputs("(@omni_expand_macro(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // name
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // args
      fputs("))", e->out);
      return;
    }

    // Module system: Module definition
    // #Modl{name, exports, body}
    if (nam == OMNI_NAM_MODL && ari == 3) {
      fputs("(@omni_register_module(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // name
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // exports
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 2));  // body
      fputs("))", e->out);
      return;
    }

    // Module system: Import
    // #Impt{module_name, bindings}
    if (nam == OMNI_NAM_IMPT && ari == 2) {
      fputs("(@omni_import_module(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // module name
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // bindings
      fputs("))", e->out);
      return;
    }

    // Module system: Export list
    // #Expt{names}
    if (nam == OMNI_NAM_EXPT && ari == 1) {
      fputs("#Expt{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // names list
      fputc('}', e->out);
      return;
    }

    // Module system: Qualified access
    // #Qual{module, name}
    if (nam == OMNI_NAM_QUAL && ari == 2) {
      fputs("(@omni_qualified_lookup(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));  // module
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));  // name
      fputs("))", e->out);
      return;
    }

    // Let (lazy binding - allows parallel evaluation)
    if (nam == OMNI_NAM_LET && ari == 2) {
      // Lazy binding: &var = expr; (no !! prefix)
      const char *var_name = omni_env_push(e);
      fprintf(e->out, "&%s = ", var_name);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs("; ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      omni_env_pop(e, 1);
      return;
    }

    // LetS (strict binding - forces eager evaluation with ^:strict)
    if (nam == OMNI_NAM_LETS && ari == 2) {
      // Strict binding: !!&var = expr; (with !! prefix)
      fputs("!!", e->out);
      const char *var_name = omni_env_push(e);
      fprintf(e->out, "&%s = ", var_name);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs("; ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      omni_env_pop(e, 1);
      return;
    }

    // Pure (purity marker from ^:pure metadata)
    // Emits #Pure{fn} - can be used for static analysis and optimization
    if (nam == OMNI_NAM_PURE && ari == 1) {
      fputs("#Pure{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }

    // If
    if (nam == OMNI_NAM_IF && ari == 3) {
      fputs("#If{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 2));
      fputc('}', e->out);
      return;
    }

    // Arithmetic
    if (nam == OMNI_NAM_ADD && ari == 2) {
      fputs("#Add{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_SUB && ari == 2) {
      fputs("#Sub{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_MUL && ari == 2) {
      fputs("#Mul{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_DIV && ari == 2) {
      fputs("#Div{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_MOD && ari == 2) {
      fputs("#Mod{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // Comparison
    if (nam == OMNI_NAM_EQL && ari == 2) {
      fputs("#Eql{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_LT && ari == 2) {
      fputs("#Lt{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_GT && ari == 2) {
      fputs("#Gt{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_LE && ari == 2) {
      fputs("#Le{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_GE && ari == 2) {
      fputs("#Ge{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // Boolean
    if (nam == OMNI_NAM_AND && ari == 2) {
      fputs("#And{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_OR && ari == 2) {
      fputs("#Or{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_NOT && ari == 1) {
      fputs("#Not{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }

    // Data structures
    if (nam == OMNI_NAM_CON && ari == 2) {
      fputs("#CON{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_NIL && ari == 0) {
      fputs("#NIL", e->out);
      return;
    }
    if (nam == OMNI_NAM_CHR && ari == 1) {
      fputs("#CHR{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_ARR && ari == 1) {
      fputs("#Arr{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_DICT && ari == 1) {
      fputs("#Dict{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_FST && ari == 1) {
      fputs("#Fst{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_SND && ari == 1) {
      fputs("#Snd{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }

    // Match expression
    if (nam == OMNI_NAM_MAT && ari == 2) {
      Term scrutinee = omni_ctr_arg(t, 0);
      Term cases = omni_ctr_arg(t, 1);

      if (omni_match_needs_runtime(cases)) {
        // Runtime interpretation
        fputs("#Mat{", e->out);
        omni_emit_term(e, scrutinee);
        fputs(", ", e->out);
        omni_emit_term(e, cases);
        fputc('}', e->out);
      } else {
        // Native HVM4 match
        omni_emit_native_match(e, scrutinee, cases);
      }
      return;
    }

    // Speculative match (^:speculate) - evaluates branches in parallel
    // Emits #MatS{scrutinee, cases} for runtime handling
    // In HVM4, the dup/superposition semantics enable parallel branch evaluation
    if (nam == OMNI_NAM_MATS && ari == 2) {
      fputs("#MatS{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // FFI
    if (nam == OMNI_NAM_FFI && ari == 2) {
      fputs("#FFI{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // Handle (effect handler)
    if (nam == OMNI_NAM_HDLE && ari == 2) {
      fputs("#Hdle{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // Perform (effect invocation)
    if (nam == OMNI_NAM_PERF && ari == 2) {
      fputs("#Perf{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // Do (sequencing)
    if (nam == OMNI_NAM_DO && ari == 2) {
      fputs("#Do{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // Quasiquote
    if (nam == OMNI_NAM_QQ && ari == 1) {
      fputs("#QQ{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_UQ && ari == 1) {
      fputs("#UQ{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_UQS && ari == 1) {
      fputs("#UQS{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }

    // Booleans
    if (nam == OMNI_NAM_TRUE && ari == 0) {
      fputs("#True", e->out);
      return;
    }
    if (nam == OMNI_NAM_FALS && ari == 0) {
      fputs("#Fals", e->out);
      return;
    }

    // Nothing
    if (nam == OMNI_NAM_NOTH && ari == 0) {
      fputs("#Noth", e->out);
      return;
    }

    // Handle (FFI)
    if (nam == OMNI_NAM_HNDL && ari == 2) {
      fputs("#Hndl{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // Case (for pattern match)
    if (nam == OMNI_NAM_CASE && ari == 3) {
      fputs("#Case{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 2));
      fputc('}', e->out);
      return;
    }

    // Patterns
    if (nam == OMNI_NAM_PCTR && ari == 2) {
      fputs("#PCtr{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_PLIT && ari == 1) {
      fputs("#PLit{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_PWLD && ari == 0) {
      fputs("#PWld", e->out);
      return;
    }
    if (nam == OMNI_NAM_PVAR && ari == 1) {
      fputs("#PVar{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }

    // Type definitions
    if (nam == OMNI_NAM_TABS && ari == 2) {
      // Abstract type: #TAbs{name, parent} -> @omni_register_abstract(name)(parent)
      fputs("(@omni_register_abstract(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputs("))", e->out);
      return;
    }
    if (nam == OMNI_NAM_TSTR && ari == 4) {
      // Struct type: #TStr{name, parent, fields, params}
      fputs("(@omni_register_struct(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 2));
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 3));
      fputs("))", e->out);
      return;
    }
    if (nam == OMNI_NAM_TENM && ari == 3) {
      // Enum type: #TEnm{name, variants, params}
      fputs("(@omni_register_enum(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 2));
      fputs("))", e->out);
      return;
    }
    if (nam == OMNI_NAM_TUNI && ari == 2) {
      // Union type: #TUni{name, types}
      fputs("(@omni_register_union(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(")(", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputs("))", e->out);
      return;
    }
    if (nam == OMNI_NAM_TFLD && ari == 2) {
      // Field descriptor: #TFld{name, type}
      fputs("#TFld{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_TVRN && ari == 2) {
      // Variant descriptor: #TVrn{name, fields}
      fputs("#TVrn{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }
    // Type annotations and type expressions
    if (nam == OMNI_NAM_TVAR && ari == 1) {
      fputs("#TVar{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputc('}', e->out);
      return;
    }
    if (nam == OMNI_NAM_TAPP && ari == 2) {
      fputs("#TApp{", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 0));
      fputs(", ", e->out);
      omni_emit_term(e, omni_ctr_arg(t, 1));
      fputc('}', e->out);
      return;
    }

    // Default: emit unknown constructor
    const char *ctr_name = omni_nick_to_name(nam);
    fprintf(e->out, "#%s", ctr_name);
    if (ari > 0) {
      fputc('{', e->out);
      for (u32 i = 0; i < ari; i++) {
        if (i > 0) fputs(", ", e->out);
        omni_emit_term(e, omni_ctr_arg(t, i));
      }
      fputc('}', e->out);
    }
    return;
  }

  // Unknown tag
  fprintf(e->out, "<?tag=%u>", tag);
}

// =============================================================================
// Public API
// =============================================================================

fn void omni_compile_emit(FILE *out, Term ast) {
  omni_names_init();
  OmniEmit e = { .out = out };
  omni_env_init(&e);
  omni_emit_term(&e, ast);
}

fn int omni_compile_with_runtime(FILE *out, Term ast, const char *runtime_path) {
  // Read and emit runtime library
  char *runtime = sys_file_read(runtime_path);
  if (!runtime) {
    fprintf(stderr, "Error: could not read runtime '%s'\n", runtime_path);
    return 1;
  }
  fputs(runtime, out);
  free(runtime);

  // Emit main entry point
  fputs("\n@main = @omni_unwrap(@omni_eval(@omni_menv_new(0)(#NIL)(#NIL))(", out);
  omni_compile_emit(out, ast);
  fputs("))\n", out);
  return 0;
}
