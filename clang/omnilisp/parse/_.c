// OmniLisp Parser
// Parses OmniLisp syntax into HVM4 AST terms
//
// Character Calculus:
//   {} - Types (Kind domain)
//   [] - Slots/data (Data domain)
//   () - Execution (Flow domain)
//   ^: - Metadata

#include "../nick/omnilisp.c"

// =============================================================================
// Parser State and Helpers
// =============================================================================

// Bind stack for de Bruijn indexing
#define OMNI_BIND_MAX 16384
static u32 OMNI_BINDS[OMNI_BIND_MAX];
static u32 OMNI_BINDS_LEN = 0;

fn void omni_bind_push(u32 sym) {
  if (OMNI_BINDS_LEN >= OMNI_BIND_MAX) {
    fprintf(stderr, "OMNI_ERROR: too many binders\n");
    exit(1);
  }
  OMNI_BINDS[OMNI_BINDS_LEN++] = sym;
}

fn void omni_bind_pop(u32 count) {
  while (count > 0 && OMNI_BINDS_LEN > 0) {
    OMNI_BINDS_LEN--;
    count--;
  }
}

fn int omni_bind_lookup(u32 sym, u32 *out_idx) {
  for (int i = (int)OMNI_BINDS_LEN - 1; i >= 0; i--) {
    if (OMNI_BINDS[i] == sym) {
      *out_idx = (u32)(OMNI_BINDS_LEN - 1 - (u32)i);
      return 1;
    }
  }
  return 0;
}

// Skip Lisp-style comments (;)
fn void omni_skip_comment(PState *s) {
  while (!parse_at_end(s) && parse_peek(s) != '\n') {
    parse_advance(s);
  }
}

// Skip whitespace and comments
fn void omni_skip(PState *s) {
  while (!parse_at_end(s)) {
    char c = parse_peek(s);
    if (parse_is_space(c)) {
      parse_advance(s);
      continue;
    }
    if (c == ';') {
      omni_skip_comment(s);
      continue;
    }
    break;
  }
}

fn int omni_is_delim(char c) {
  return c == '(' || c == ')' || c == '[' || c == ']' ||
         c == '{' || c == '}' || c == ';' || c == '\'' ||
         c == '`' || c == ',' || c == '"' || c == '#' ||
         parse_is_space(c) || c == '\0';
}

// Match a string exactly
fn int omni_match(PState *s, const char *str) {
  omni_skip(s);
  u32 len = strlen(str);
  if (s->pos + len > s->len) return 0;
  if (memcmp(s->src + s->pos, str, len) != 0) return 0;
  // Check it's not part of a longer token
  if (!omni_is_delim(s->src[s->pos + len])) return 0;
  for (u32 i = 0; i < len; i++) parse_advance(s);
  omni_skip(s);
  return 1;
}

// Match a character
fn int omni_match_char(PState *s, char c) {
  omni_skip(s);
  if (parse_peek(s) == c) {
    parse_advance(s);
    omni_skip(s);
    return 1;
  }
  return 0;
}

// Expect a character, error if not found
fn void omni_expect_char(PState *s, char c) {
  omni_skip(s);
  if (parse_peek(s) != c) {
    parse_error(s, (char[2]){c, 0}, parse_peek(s));
  }
  parse_advance(s);
  omni_skip(s);
}

// =============================================================================
// Symbol Parsing
// =============================================================================

// Parse a symbol token, returning start position and length
fn int omni_parse_symbol_raw(PState *s, u32 *out_start, u32 *out_len) {
  omni_skip(s);
  char c = parse_peek(s);

  // Can't start with delimiter or digit
  if (omni_is_delim(c) || isdigit(c)) return 0;

  // Can't start with : (colon-quoted symbol) or ^ (metadata)
  if (c == ':' || c == '^') return 0;

  u32 start = s->pos;
  while (!parse_at_end(s)) {
    c = parse_peek(s);
    if (omni_is_delim(c) || c == ':' || c == '^') break;
    parse_advance(s);
  }

  u32 len = s->pos - start;
  if (len == 0) return 0;

  *out_start = start;
  *out_len = len;
  return 1;
}

// Get nick value from symbol in source
fn u32 omni_symbol_nick(PState *s, u32 start, u32 len) {
  char buf[5] = {0};
  u32 copy_len = len > 4 ? 4 : len;
  memcpy(buf, s->src + start, copy_len);
  return omni_nick(buf);
}

// Check if symbol matches a literal
fn int omni_symbol_is(PState *s, u32 start, u32 len, const char *lit) {
  u32 lit_len = strlen(lit);
  if (len != lit_len) return 0;
  return memcmp(s->src + start, lit, len) == 0;
}

// =============================================================================
// Term Constructors
// =============================================================================

fn Term omni_ctr0(u32 nam) {
  return term_new_ctr(nam, 0, NULL);
}

fn Term omni_ctr1(u32 nam, Term a) {
  Term args[1] = {a};
  return term_new_ctr(nam, 1, args);
}

fn Term omni_ctr2(u32 nam, Term a, Term b) {
  Term args[2] = {a, b};
  return term_new_ctr(nam, 2, args);
}

fn Term omni_ctr3(u32 nam, Term a, Term b, Term c) {
  Term args[3] = {a, b, c};
  return term_new_ctr(nam, 3, args);
}

fn Term omni_ctr4(u32 nam, Term a, Term b, Term c, Term d) {
  Term args[4] = {a, b, c, d};
  return term_new_ctr(nam, 4, args);
}

fn Term omni_ctr5(u32 nam, Term a, Term b, Term c, Term d, Term e) {
  Term args[5] = {a, b, c, d, e};
  return term_new_ctr(nam, 5, args);
}

// Build a list from an array of terms
fn Term omni_list(Term *items, u32 count) {
  Term result = omni_ctr0(OMNI_NAM_NIL);
  for (int i = (int)count - 1; i >= 0; i--) {
    result = omni_ctr2(OMNI_NAM_CON, items[i], result);
  }
  return result;
}

// AST constructors
fn Term omni_lit(u32 n) {
  return omni_ctr1(OMNI_NAM_LIT, term_new_num(n));
}

fn Term omni_sym(u32 sym_id) {
  return omni_ctr1(OMNI_NAM_SYM, term_new_num(sym_id));
}

fn Term omni_var(u32 idx) {
  return omni_ctr1(OMNI_NAM_VAR, term_new_num(idx));
}

fn Term omni_lam(Term body) {
  return omni_ctr1(OMNI_NAM_LAM, body);
}

fn Term omni_lamr(Term body) {
  return omni_ctr1(OMNI_NAM_LAMR, body);
}

fn Term omni_app(Term fn, Term arg) {
  return omni_ctr2(OMNI_NAM_APP, fn, arg);
}

fn Term omni_let(Term val, Term body) {
  return omni_ctr2(OMNI_NAM_LET, val, body);
}

fn Term omni_if(Term cond, Term then_br, Term else_br) {
  return omni_ctr3(OMNI_NAM_IF, cond, then_br, else_br);
}

fn Term omni_add(Term a, Term b) { return omni_ctr2(OMNI_NAM_ADD, a, b); }
fn Term omni_sub(Term a, Term b) { return omni_ctr2(OMNI_NAM_SUB, a, b); }
fn Term omni_mul(Term a, Term b) { return omni_ctr2(OMNI_NAM_MUL, a, b); }
fn Term omni_div(Term a, Term b) { return omni_ctr2(OMNI_NAM_DIV, a, b); }
fn Term omni_mod(Term a, Term b) { return omni_ctr2(OMNI_NAM_MOD, a, b); }
fn Term omni_eql(Term a, Term b) { return omni_ctr2(OMNI_NAM_EQL, a, b); }
fn Term omni_lt(Term a, Term b)  { return omni_ctr2(OMNI_NAM_LT, a, b); }
fn Term omni_gt(Term a, Term b)  { return omni_ctr2(OMNI_NAM_GT, a, b); }
fn Term omni_le(Term a, Term b)  { return omni_ctr2(OMNI_NAM_LE, a, b); }
fn Term omni_ge(Term a, Term b)  { return omni_ctr2(OMNI_NAM_GE, a, b); }
fn Term omni_and(Term a, Term b) { return omni_ctr2(OMNI_NAM_AND, a, b); }
fn Term omni_or(Term a, Term b)  { return omni_ctr2(OMNI_NAM_OR, a, b); }
fn Term omni_not(Term a)         { return omni_ctr1(OMNI_NAM_NOT, a); }

fn Term omni_cons(Term h, Term t) { return omni_ctr2(OMNI_NAM_CON, h, t); }
fn Term omni_nil(void)            { return omni_ctr0(OMNI_NAM_NIL); }
fn Term omni_chr(u32 c)           { return omni_ctr1(OMNI_NAM_CHR, term_new_num(c)); }

fn Term omni_nothing(void) { return omni_ctr0(OMNI_NAM_NOTH); }
fn Term omni_true(void)    { return omni_ctr0(OMNI_NAM_TRUE); }
fn Term omni_false(void)   { return omni_ctr0(OMNI_NAM_FALS); }

// Fixed-point number: value * 10^(-scale)
fn Term omni_fix(int64_t value, u32 scale) {
  u32 hi = (u32)(((u64)value) >> 32);
  u32 lo = (u32)(value & 0xFFFFFFFF);
  Term args[3];
  args[0] = term_new_num(hi);
  args[1] = term_new_num(lo);
  args[2] = term_new_num(scale);
  return term_new_ctr(OMNI_NAM_FIX, 3, args);
}

// Pattern constructors
fn Term omni_pat_wildcard(void) {
  return omni_ctr0(OMNI_NAM_PWLD);
}

fn Term omni_pat_var(u32 idx) {
  return omni_ctr1(OMNI_NAM_PVAR, term_new_num(idx));
}

fn Term omni_pat_lit(Term val) {
  return omni_ctr1(OMNI_NAM_PLIT, val);
}

fn Term omni_pat_ctr(u32 tag_nick, Term args) {
  return omni_ctr2(OMNI_NAM_PCTR, term_new_num(tag_nick), args);
}

fn Term omni_case(Term pattern, Term guard, Term body) {
  return omni_ctr3(OMNI_NAM_CASE, pattern, guard, body);
}

fn Term omni_match(Term scrutinee, Term cases) {
  return omni_ctr2(OMNI_NAM_MAT, scrutinee, cases);
}

// FFI
fn Term omni_ffi(Term name, Term args) {
  return omni_ctr2(OMNI_NAM_FFI, name, args);
}

// Handle for effect - #Hdle{handlers, body}
fn Term omni_handle(Term handlers, Term body) {
  return omni_ctr2(OMNI_NAM_HDLE, handlers, body);
}

fn Term omni_perform(Term tag, Term payload) {
  return omni_ctr2(OMNI_NAM_PERF, tag, payload);
}

// Delimited continuations
fn Term omni_reset(Term body) {
  return omni_ctr1(OMNI_NAM_PRMT, body);
}

fn Term omni_control(u32 k_idx, Term body) {
  return omni_ctr2(OMNI_NAM_CTRL, term_new_num(k_idx), body);
}

fn Term omni_yield(Term val) {
  return omni_ctr1(OMNI_NAM_YLD, val);
}

// =============================================================================
// Forward Declarations
// =============================================================================

fn Term parse_omni_expr(PState *s);
fn Term parse_omni_type(PState *s);
fn Term parse_omni_pattern(PState *s);

// =============================================================================
// Number Parsing
// =============================================================================

fn int omni_parse_number(PState *s, Term *out) {
  omni_skip(s);

  char c = parse_peek(s);
  int negative = 0;

  // Check for sign
  if (c == '-' || c == '+') {
    negative = (c == '-');
    // Peek ahead to see if it's followed by digit
    if (s->pos + 1 < s->len && isdigit(s->src[s->pos + 1])) {
      parse_advance(s);
    } else {
      return 0;  // It's an operator, not a number
    }
  }

  if (!isdigit(parse_peek(s)) && parse_peek(s) != '.') {
    return 0;
  }

  // Parse integer part
  int64_t int_part = 0;
  while (!parse_at_end(s) && isdigit(parse_peek(s))) {
    c = parse_peek(s);
    int_part = int_part * 10 + (int64_t)(c - '0');
    parse_advance(s);
  }

  // Check for decimal point or exponent
  c = parse_peek(s);
  if (c != '.' && c != 'e' && c != 'E') {
    // Plain integer
    if (negative) int_part = -int_part;
    if (int_part >= 0 && int_part <= 0xFFFFFFFF) {
      *out = omni_lit((u32)int_part);
    } else {
      *out = omni_fix(int_part, 0);
    }
    omni_skip(s);
    return 1;
  }

  // Parse fractional part
  int64_t frac_part = 0;
  u32 frac_digits = 0;
  if (c == '.') {
    parse_advance(s);
    while (!parse_at_end(s) && isdigit(parse_peek(s))) {
      c = parse_peek(s);
      frac_part = frac_part * 10 + (int64_t)(c - '0');
      frac_digits++;
      parse_advance(s);
    }
  }

  // Parse exponent
  int32_t exp = 0;
  c = parse_peek(s);
  if (c == 'e' || c == 'E') {
    parse_advance(s);
    int exp_neg = 0;
    c = parse_peek(s);
    if (c == '-') {
      exp_neg = 1;
      parse_advance(s);
    } else if (c == '+') {
      parse_advance(s);
    }
    while (!parse_at_end(s) && isdigit(parse_peek(s))) {
      c = parse_peek(s);
      exp = exp * 10 + (int32_t)(c - '0');
      parse_advance(s);
    }
    if (exp_neg) exp = -exp;
  }

  // Calculate final value and scale
  int64_t mantissa = int_part;
  for (u32 i = 0; i < frac_digits; i++) {
    mantissa *= 10;
  }
  mantissa += frac_part;

  int32_t final_scale = (int32_t)frac_digits - exp;
  if (final_scale < 0) {
    for (int32_t i = 0; i < -final_scale; i++) {
      mantissa *= 10;
    }
    final_scale = 0;
  }

  if (negative) mantissa = -mantissa;

  *out = omni_fix(mantissa, (u32)final_scale);
  omni_skip(s);
  return 1;
}

// =============================================================================
// String Parsing
// =============================================================================

fn Term parse_omni_string(PState *s) {
  omni_expect_char(s, '"');

  Term result = omni_nil();
  Term *tail = &result;

  while (!parse_at_end(s) && parse_peek(s) != '"') {
    u32 c;
    if (parse_peek(s) == '\\') {
      parse_advance(s);
      char esc = parse_peek(s);
      parse_advance(s);
      switch (esc) {
        case 'n': c = '\n'; break;
        case 't': c = '\t'; break;
        case 'r': c = '\r'; break;
        case '"': c = '"'; break;
        case '\\': c = '\\'; break;
        case 'x': {
          // Hex escape
          char h1 = parse_peek(s); parse_advance(s);
          char h2 = parse_peek(s); parse_advance(s);
          int d1 = isdigit(h1) ? h1 - '0' : (tolower(h1) - 'a' + 10);
          int d2 = isdigit(h2) ? h2 - '0' : (tolower(h2) - 'a' + 10);
          c = (u32)(d1 * 16 + d2);
          break;
        }
        default: c = (u32)esc;
      }
    } else {
      // UTF-8 decoding
      u32 pos = s->pos;
      c = parse_utf8(s);
      if (c == 0 && pos == s->pos) {
        c = (u32)parse_peek(s);
        parse_advance(s);
      }
    }

    // Append character
    Term chr = omni_chr(c);
    Term new_cell = omni_cons(chr, omni_nil());
    *tail = new_cell;
    tail = &HEAP[term_val(new_cell) + 1];  // tail of cons
  }

  omni_expect_char(s, '"');
  return result;
}

// =============================================================================
// Character Literal Parsing
// =============================================================================

fn Term parse_omni_char(PState *s) {
  // #\c or #\newline or #\xNN
  omni_expect_char(s, '#');
  omni_expect_char(s, '\\');

  // Check for named characters
  if (omni_match(s, "newline")) return omni_chr('\n');
  if (omni_match(s, "space")) return omni_chr(' ');
  if (omni_match(s, "tab")) return omni_chr('\t');

  // Hex character
  if (parse_peek(s) == 'x') {
    parse_advance(s);
    char h1 = parse_peek(s); parse_advance(s);
    char h2 = parse_peek(s); parse_advance(s);
    int d1 = isdigit(h1) ? h1 - '0' : (tolower(h1) - 'a' + 10);
    int d2 = isdigit(h2) ? h2 - '0' : (tolower(h2) - 'a' + 10);
    u32 c = (u32)(d1 * 16 + d2);
    omni_skip(s);
    return omni_chr(c);
  }

  // Single character (UTF-8)
  u32 c = parse_utf8(s);
  if (c == 0) {
    c = (u32)parse_peek(s);
    parse_advance(s);
  }
  omni_skip(s);
  return omni_chr(c);
}

// =============================================================================
// Type Parsing  {Type} or {literal} for value types
// =============================================================================

fn Term parse_omni_type(PState *s) {
  omni_expect_char(s, '{');
  omni_skip(s);

  char c = parse_peek(s);

  // Check for value type literals: {3}, {true}, {"hello"}, {#\a}

  // Number literal: {3}, {-42}, {1.5}
  if (isdigit(c) || ((c == '-' || c == '+') && s->pos + 1 < s->len && isdigit(s->src[s->pos + 1]))) {
    Term num;
    if (omni_parse_number(s, &num)) {
      omni_expect_char(s, '}');
      return omni_ctr1(OMNI_NAM_VTYP, num);
    }
  }

  // String literal: {"hello"}
  if (c == '"') {
    Term str = parse_omni_string(s);
    omni_expect_char(s, '}');
    return omni_ctr1(OMNI_NAM_VTYP, str);
  }

  // Character literal: {#\a}, {#\newline}
  if (c == '#' && s->pos + 1 < s->len && s->src[s->pos + 1] == '\\') {
    Term chr = parse_omni_char(s);
    omni_expect_char(s, '}');
    return omni_ctr1(OMNI_NAM_VTYP, chr);
  }

  // Parse type expression (symbol)
  u32 sym_start, sym_len;
  if (omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
    u32 nick = omni_symbol_nick(s, sym_start, sym_len);

    // Check for boolean literals: {true}, {false}
    if (omni_symbol_is(s, sym_start, sym_len, "true")) {
      omni_expect_char(s, '}');
      return omni_ctr1(OMNI_NAM_VTYP, omni_true());
    }
    if (omni_symbol_is(s, sym_start, sym_len, "false")) {
      omni_expect_char(s, '}');
      return omni_ctr1(OMNI_NAM_VTYP, omni_false());
    }

    // Check for nothing: {nothing}
    if (omni_symbol_is(s, sym_start, sym_len, "nothing")) {
      omni_expect_char(s, '}');
      return omni_ctr1(OMNI_NAM_VTYP, omni_nothing());
    }

    // Type application: {List T}
    Term type_args = omni_nil();
    Term *tail = &type_args;
    while (parse_peek(s) != '}' && !parse_at_end(s)) {
      Term arg;
      if (parse_peek(s) == '{') {
        arg = parse_omni_type(s);
      } else {
        u32 arg_start, arg_len;
        if (omni_parse_symbol_raw(s, &arg_start, &arg_len)) {
          u32 arg_nick = omni_symbol_nick(s, arg_start, arg_len);
          arg = omni_ctr1(OMNI_NAM_TVAR, term_new_num(arg_nick));
        } else {
          parse_error(s, "type argument", parse_peek(s));
          arg = omni_nil();
        }
      }
      Term cell = omni_cons(arg, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }

    omni_expect_char(s, '}');

    if (term_ext(type_args) == OMNI_NAM_NIL) {
      // Simple type
      return omni_ctr1(OMNI_NAM_TVAR, term_new_num(nick));
    } else {
      // Type application
      Term base = omni_ctr1(OMNI_NAM_TVAR, term_new_num(nick));
      return omni_ctr2(OMNI_NAM_TAPP, base, type_args);
    }
  }

  omni_expect_char(s, '}');
  return omni_nil();
}

// =============================================================================
// Slot Parsing [name {Type}?] or [[pattern] {Type}?] for destructuring
// =============================================================================

typedef struct {
  u32 name_nick;      // Name nick (for simple slots)
  Term pattern;       // Pattern (for destructuring slots), NIL if simple
  Term type;          // OMNI_NAM_NIL if no type
  int is_destruct;    // 1 if destructuring pattern, 0 if simple name
} OmniSlot;

fn int parse_omni_slot(PState *s, OmniSlot *out) {
  omni_skip(s);
  if (parse_peek(s) != '[') return 0;

  omni_expect_char(s, '[');

  // Check if this is a destructuring pattern (nested [])
  if (parse_peek(s) == '[') {
    // Destructuring pattern: [[a b] {Type}?]
    out->pattern = parse_omni_pattern(s);
    out->is_destruct = 1;
    out->name_nick = 0;  // Not used for destructuring
  } else {
    // Simple name: [name {Type}?]
    u32 sym_start, sym_len;
    if (!omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
      parse_error(s, "parameter name or pattern", parse_peek(s));
      return 0;
    }
    out->name_nick = omni_symbol_nick(s, sym_start, sym_len);
    out->pattern = omni_nil();
    out->is_destruct = 0;
  }

  // Optional type
  if (parse_peek(s) == '{') {
    out->type = parse_omni_type(s);
  } else {
    out->type = omni_nil();
  }

  omni_expect_char(s, ']');
  return 1;
}

// =============================================================================
// Macro Pattern and Template Parsing
// =============================================================================

// Forward declaration for expression parsing
fn Term parse_omni_expr(PState *s);

// Parse a macro pattern (the LHS of a macro rule)
// Patterns can contain:
// - Literal symbols (match exactly)
// - Pattern variables (capture values)
// - ... for rest patterns
// - Nested lists
fn Term parse_omni_macro_pattern(PState *s) {
  omni_skip(s);
  char c = parse_peek(s);

  // List pattern: (sym arg1 arg2 ...)
  if (c == '(') {
    omni_expect_char(s, '(');

    Term elems = omni_nil();
    Term *tail = &elems;

    while (parse_peek(s) != ')' && !parse_at_end(s)) {
      // Check for ... (ellipsis for rest)
      if (parse_peek(s) == '.' && s->pos + 2 < s->len &&
          s->src[s->pos + 1] == '.' && s->src[s->pos + 2] == '.') {
        parse_advance(s);
        parse_advance(s);
        parse_advance(s);
        omni_skip(s);

        // Get the variable name after ...
        u32 var_start, var_len;
        if (omni_parse_symbol_raw(s, &var_start, &var_len)) {
          u32 var_nick = omni_symbol_nick(s, var_start, var_len);
          Term rest = omni_ctr1(OMNI_NAM_MRST, term_new_num(var_nick));
          Term cell = omni_cons(rest, omni_nil());
          *tail = cell;
          tail = &HEAP[term_val(cell) + 1];
        } else {
          // Ellipsis without name - mark previous element as repeating
          // For simplicity, just create a marker
          Term rest = omni_ctr1(OMNI_NAM_MRST, term_new_num(0));
          Term cell = omni_cons(rest, omni_nil());
          *tail = cell;
          tail = &HEAP[term_val(cell) + 1];
        }
        continue;
      }

      Term elem = parse_omni_macro_pattern(s);
      Term cell = omni_cons(elem, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }

    omni_expect_char(s, ')');
    return elems;
  }

  // Array pattern: [...]
  if (c == '[') {
    omni_expect_char(s, '[');

    Term elems = omni_nil();
    Term *tail = &elems;

    while (parse_peek(s) != ']' && !parse_at_end(s)) {
      // Check for ... (ellipsis)
      if (parse_peek(s) == '.' && s->pos + 2 < s->len &&
          s->src[s->pos + 1] == '.' && s->src[s->pos + 2] == '.') {
        parse_advance(s);
        parse_advance(s);
        parse_advance(s);
        omni_skip(s);

        u32 var_start, var_len;
        if (omni_parse_symbol_raw(s, &var_start, &var_len)) {
          u32 var_nick = omni_symbol_nick(s, var_start, var_len);
          Term rest = omni_ctr1(OMNI_NAM_MRST, term_new_num(var_nick));
          Term cell = omni_cons(rest, omni_nil());
          *tail = cell;
          tail = &HEAP[term_val(cell) + 1];
        }
        continue;
      }

      Term elem = parse_omni_macro_pattern(s);
      Term cell = omni_cons(elem, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }

    omni_expect_char(s, ']');
    // Wrap in array marker
    return omni_ctr1(OMNI_NAM_ARR, elems);
  }

  // Symbol: either literal or pattern variable
  u32 sym_start, sym_len;
  if (omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
    u32 sym_nick = omni_symbol_nick(s, sym_start, sym_len);

    // Convention: lowercase symbols are pattern variables, uppercase are literals
    char first = s->src[sym_start];
    if (first >= 'a' && first <= 'z') {
      // Pattern variable
      return omni_ctr1(OMNI_NAM_MVAR, term_new_num(sym_nick));
    } else {
      // Literal (must match exactly)
      return omni_ctr1(OMNI_NAM_MLIT, term_new_num(sym_nick));
    }
  }

  // Literal number
  if (isdigit(c) || (c == '-' && isdigit(s->src[s->pos + 1]))) {
    // Parse number and wrap as literal
    u32 start = s->pos;
    if (c == '-') parse_advance(s);
    while (isdigit(parse_peek(s))) parse_advance(s);
    u32 len = s->pos - start;
    char buf[32] = {0};
    memcpy(buf, s->src + start, len < 31 ? len : 31);
    i32 val = atoi(buf);
    return omni_ctr1(OMNI_NAM_MLIT, omni_lit(val));
  }

  // Fallback: just return nil
  return omni_nil();
}

// Parse a macro template (the RHS of a macro rule)
// Templates use the same syntax as patterns, but:
// - Pattern variables are substituted
// - ... causes repetition of preceding form
fn Term parse_omni_macro_template(PState *s) {
  // For templates, we reuse the pattern parser
  // The distinction is in expansion, not parsing
  return parse_omni_macro_pattern(s);
}

// =============================================================================
// Grammar Pattern Parsing (Pika)
// =============================================================================

// Forward declaration
fn Term parse_omni_grammar_seq(PState *s);

// Parse a single grammar element
fn Term parse_omni_grammar_atom(PState *s) {
  omni_skip(s);
  char c = parse_peek(s);

  // String literal: "..."
  if (c == '"') {
    parse_advance(s);  // skip opening "
    Term chars = omni_nil();
    Term *tail = &chars;
    while (!parse_at_end(s) && parse_peek(s) != '"') {
      char ch = parse_peek(s);
      if (ch == '\\' && s->pos + 1 < s->len) {
        parse_advance(s);
        ch = parse_peek(s);
        switch (ch) {
          case 'n': ch = '\n'; break;
          case 't': ch = '\t'; break;
          case 'r': ch = '\r'; break;
          case '\\': ch = '\\'; break;
          case '"': ch = '"'; break;
        }
      }
      Term cell = omni_cons(omni_chr(ch), omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
      parse_advance(s);
    }
    if (parse_peek(s) == '"') parse_advance(s);
    return omni_ctr1(OMNI_NAM_GSTR, chars);
  }

  // Character class: [abc] or [^abc] or [a-z]
  if (c == '[') {
    parse_advance(s);
    int negated = 0;
    if (parse_peek(s) == '^') {
      negated = 1;
      parse_advance(s);
    }
    Term chars = omni_nil();
    Term *tail = &chars;
    while (!parse_at_end(s) && parse_peek(s) != ']') {
      char ch = parse_peek(s);
      if (ch == '\\' && s->pos + 1 < s->len) {
        parse_advance(s);
        ch = parse_peek(s);
      }
      // Check for range a-z
      if (s->pos + 2 < s->len && s->src[s->pos + 1] == '-' && s->src[s->pos + 2] != ']') {
        char start = ch;
        parse_advance(s); // ch
        parse_advance(s); // -
        char end = parse_peek(s);
        // Add all chars in range
        for (char i = start; i <= end; i++) {
          Term cell = omni_cons(omni_chr(i), omni_nil());
          *tail = cell;
          tail = &HEAP[term_val(cell) + 1];
        }
        parse_advance(s);
      } else {
        Term cell = omni_cons(omni_chr(ch), omni_nil());
        *tail = cell;
        tail = &HEAP[term_val(cell) + 1];
        parse_advance(s);
      }
    }
    if (parse_peek(s) == ']') parse_advance(s);
    return omni_ctr2(OMNI_NAM_GCHR, chars, term_new_num(negated));
  }

  // Any character: .
  if (c == '.') {
    parse_advance(s);
    return omni_ctr0(OMNI_NAM_GANY);
  }

  // Group: (...)
  if (c == '(') {
    parse_advance(s);
    Term inner = parse_omni_grammar_seq(s);
    omni_expect_char(s, ')');
    return inner;
  }

  // Not predicate: !
  if (c == '!') {
    parse_advance(s);
    Term inner = parse_omni_grammar_atom(s);
    return omni_ctr1(OMNI_NAM_GNOT, inner);
  }

  // And predicate: &
  if (c == '&') {
    parse_advance(s);
    Term inner = parse_omni_grammar_atom(s);
    return omni_ctr1(OMNI_NAM_GAND, inner);
  }

  // Rule reference: identifier
  u32 sym_start, sym_len;
  if (omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
    u32 sym_nick = omni_symbol_nick(s, sym_start, sym_len);
    return omni_ctr1(OMNI_NAM_GREF, term_new_num(sym_nick));
  }

  return omni_nil();
}

// Parse atom with optional suffix (?, *, +)
fn Term parse_omni_grammar_suffix(PState *s) {
  Term atom = parse_omni_grammar_atom(s);
  omni_skip(s);

  char c = parse_peek(s);
  if (c == '?') {
    parse_advance(s);
    return omni_ctr1(OMNI_NAM_GOPT, atom);
  }
  if (c == '*') {
    parse_advance(s);
    return omni_ctr1(OMNI_NAM_GSTA, atom);
  }
  if (c == '+') {
    parse_advance(s);
    return omni_ctr1(OMNI_NAM_GPLS, atom);
  }
  return atom;
}

// Parse a sequence of atoms
fn Term parse_omni_grammar_seq(PState *s) {
  omni_skip(s);

  Term items = omni_nil();
  Term *tail = &items;

  while (!parse_at_end(s)) {
    char c = parse_peek(s);
    // Stop at sequence terminators
    if (c == ')' || c == ']' || c == '|' || c == '/') break;
    // Stop at action arrow: → (UTF-8) or ->
    if (s->pos + 2 < s->len &&
        (u8)s->src[s->pos] == 0xE2 &&
        (u8)s->src[s->pos + 1] == 0x86 &&
        (u8)s->src[s->pos + 2] == 0x92) break;
    if (c == '-' && s->pos + 1 < s->len && s->src[s->pos + 1] == '>') break;

    Term item = parse_omni_grammar_suffix(s);
    if (term_tag(item) == T_NIL) break;

    Term cell = omni_cons(item, omni_nil());
    *tail = cell;
    tail = &HEAP[term_val(cell) + 1];
    omni_skip(s);
  }

  // Single item: return as-is
  // Multiple items: wrap in GSeq
  Term first = items;
  if (term_tag(first) == T_NIL) return omni_nil();
  Term second = HEAP[term_val(first) + 1];
  if (term_tag(second) == T_NIL) {
    return HEAP[term_val(first)];  // Single item
  }
  return omni_ctr1(OMNI_NAM_GSEQ, items);
}

// Parse alternation (| or / for Pika grammar compatibility)
fn Term parse_omni_grammar_pattern(PState *s) {
  omni_skip(s);

  Term first = parse_omni_grammar_seq(s);
  omni_skip(s);

  char c = parse_peek(s);
  if (c != '|' && c != '/') {
    return first;
  }

  // Build list of alternatives
  Term alts = omni_nil();
  Term *tail = &alts;

  Term cell1 = omni_cons(first, omni_nil());
  *tail = cell1;
  tail = &HEAP[term_val(cell1) + 1];

  while (parse_peek(s) == '|' || parse_peek(s) == '/') {
    parse_advance(s);  // skip | or /
    omni_skip(s);
    Term alt = parse_omni_grammar_seq(s);
    Term cell = omni_cons(alt, omni_nil());
    *tail = cell;
    tail = &HEAP[term_val(cell) + 1];
    omni_skip(s);
  }

  return omni_ctr1(OMNI_NAM_GALT, alts);
}

// =============================================================================
// Pattern Parsing
// =============================================================================

// Forward declaration for recursive pattern parsing
fn Term parse_omni_pattern_base(PState *s);

// Main pattern parser - handles 'as' suffix for any pattern
fn Term parse_omni_pattern(PState *s) {
  Term pattern = parse_omni_pattern_base(s);

  // Check for 'as' suffix: pattern as name
  // Works for any pattern type: [x y] as pair, Point(x y) as p, x as val
  omni_skip(s);
  if (omni_match(s, "as")) {
    u32 name_start, name_len;
    if (omni_parse_symbol_raw(s, &name_start, &name_len)) {
      u32 name_nick = omni_symbol_nick(s, name_start, name_len);
      omni_bind_push(name_nick);
      return omni_ctr2(OMNI_NAM_PAS, term_new_num(name_nick), pattern);
    }
  }

  return pattern;
}

// Base pattern parser (without 'as' handling)
fn Term parse_omni_pattern_base(PState *s) {
  omni_skip(s);

  char c = parse_peek(s);

  // Wildcard: _
  if (c == '_' && omni_is_delim(s->src[s->pos + 1])) {
    parse_advance(s);
    omni_skip(s);
    return omni_pat_wildcard();
  }

  // Array pattern: [x y z] or [h .. t] or [h & rest]
  if (c == '[') {
    omni_expect_char(s, '[');

    Term patterns = omni_nil();
    Term *tail = &patterns;

    while (parse_peek(s) != ']' && !parse_at_end(s)) {
      // Check for spread pattern: .. rest
      if (parse_peek(s) == '.' && s->pos + 1 < s->len && s->src[s->pos + 1] == '.') {
        s->pos += 2;  // skip ..
        omni_skip(s);
        u32 rest_start, rest_len;
        if (omni_parse_symbol_raw(s, &rest_start, &rest_len)) {
          u32 rest_nick = omni_symbol_nick(s, rest_start, rest_len);
          omni_bind_push(rest_nick);
          Term rest_pat = omni_ctr1(OMNI_NAM_SPRD, term_new_num(rest_nick));
          Term cell = omni_cons(rest_pat, omni_nil());
          *tail = cell;
          tail = &HEAP[term_val(cell) + 1];
        }
        break;
      }

      // Legacy: & rest (also supported)
      if (parse_peek(s) == '&') {
        parse_advance(s);
        omni_skip(s);
        u32 rest_start, rest_len;
        if (omni_parse_symbol_raw(s, &rest_start, &rest_len)) {
          u32 rest_nick = omni_symbol_nick(s, rest_start, rest_len);
          omni_bind_push(rest_nick);
          Term rest_pat = omni_ctr1(OMNI_NAM_PRST, term_new_num(0));
          Term cell = omni_cons(rest_pat, omni_nil());
          *tail = cell;
          tail = &HEAP[term_val(cell) + 1];
        }
        break;
      }

      Term pat = parse_omni_pattern(s);  // Recursive, allows nested 'as'
      Term cell = omni_cons(pat, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }

    omni_expect_char(s, ']');

    // Array pattern constructor
    u32 arr_nick = omni_nick("Arr");
    return omni_pat_ctr(arr_nick, patterns);
  }

  // Literal pattern (number)
  Term num_out;
  if (omni_parse_number(s, &num_out)) {
    return omni_pat_lit(num_out);
  }

  // String literal pattern
  if (c == '"') {
    Term str = parse_omni_string(s);
    return omni_pat_lit(str);
  }

  // Constructor or variable pattern
  u32 sym_start, sym_len;
  if (omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
    u32 nick = omni_symbol_nick(s, sym_start, sym_len);

    // Check for reserved words
    if (omni_symbol_is(s, sym_start, sym_len, "true")) {
      return omni_pat_lit(omni_true());
    }
    if (omni_symbol_is(s, sym_start, sym_len, "false")) {
      return omni_pat_lit(omni_false());
    }
    if (omni_symbol_is(s, sym_start, sym_len, "nothing")) {
      return omni_pat_lit(omni_nothing());
    }

    // Uppercase = constructor pattern
    if (isupper(s->src[sym_start])) {
      // Check for arguments: Point(x y) or Point x y until delimiter
      if (parse_peek(s) == '(') {
        omni_expect_char(s, '(');
        Term args = omni_nil();
        Term *tail = &args;
        while (parse_peek(s) != ')' && !parse_at_end(s)) {
          Term arg = parse_omni_pattern(s);
          Term cell = omni_cons(arg, omni_nil());
          *tail = cell;
          tail = &HEAP[term_val(cell) + 1];
        }
        omni_expect_char(s, ')');
        return omni_pat_ctr(nick, args);
      }
      // Zero-arg constructor
      return omni_pat_ctr(nick, omni_nil());
    }

    // Variable pattern - bind it
    omni_bind_push(nick);
    u32 idx = OMNI_BINDS_LEN - 1;
    return omni_pat_var(idx);
  }

  parse_error(s, "pattern", c);
  return omni_pat_wildcard();
}

// =============================================================================
// Match Clause Parsing: [pattern result] or [pattern :when guard result]
// =============================================================================

fn Term parse_omni_match_clause(PState *s) {
  omni_expect_char(s, '[');

  u32 binds_before = OMNI_BINDS_LEN;

  // Parse pattern
  Term pattern = parse_omni_pattern(s);

  // Check for guard
  Term guard = omni_nil();
  if (parse_peek(s) == ':') {
    parse_advance(s);
    if (omni_match(s, "when")) {
      guard = parse_omni_expr(s);
    } else {
      // Put the colon back (it was a colon-quoted symbol)
      s->pos--;
    }
  }

  // Parse body
  Term body = parse_omni_expr(s);

  omni_expect_char(s, ']');

  // Pop bindings from pattern
  omni_bind_pop(OMNI_BINDS_LEN - binds_before);

  return omni_case(pattern, guard, body);
}

// =============================================================================
// Expression Parsing
// =============================================================================

fn Term parse_omni_atom(PState *s) {
  omni_skip(s);
  char c = parse_peek(s);

  // Quote: 'expr
  if (c == '\'') {
    parse_advance(s);
    Term quoted = parse_omni_atom(s);
    // Plain quote - wraps in #Cod{} which evaluates to itself
    return omni_ctr1(OMNI_NAM_COD, quoted);
  }

  // Quasiquote: `expr
  if (c == '`') {
    parse_advance(s);
    Term quoted = parse_omni_atom(s);
    // Quasiquote - allows unquoting inside
    return omni_ctr1(OMNI_NAM_QQ, quoted);
  }

  // Unquote: ,expr or ,@expr
  if (c == ',') {
    parse_advance(s);
    if (parse_peek(s) == '@') {
      parse_advance(s);
      Term unquoted = parse_omni_atom(s);
      return omni_ctr1(OMNI_NAM_UQS, unquoted);
    }
    Term unquoted = parse_omni_atom(s);
    return omni_ctr1(OMNI_NAM_UQ, unquoted);
  }

  // Colon-quoted symbol: :name
  if (c == ':') {
    parse_advance(s);
    u32 sym_start, sym_len;
    if (omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
      u32 nick = omni_symbol_nick(s, sym_start, sym_len);
      return omni_sym(nick);
    }
    parse_error(s, "symbol after :", parse_peek(s));
    return omni_nil();
  }

  // Character literal: #\c
  if (c == '#' && s->pos + 1 < s->len && s->src[s->pos + 1] == '\\') {
    return parse_omni_char(s);
  }

  // Value-to-type (singleton type): #val expr
  if (c == '#' && s->pos + 3 < s->len &&
      s->src[s->pos + 1] == 'v' &&
      s->src[s->pos + 2] == 'a' &&
      s->src[s->pos + 3] == 'l' &&
      omni_is_delim(s->src[s->pos + 4])) {
    parse_advance(s);  // skip #
    parse_advance(s);  // skip v
    parse_advance(s);  // skip a
    parse_advance(s);  // skip l
    omni_skip(s);
    Term value = parse_omni_expr(s);
    return omni_ctr1(OMNI_NAM_VTYP, value);
  }

  // Dictionary literal: #{...}
  if (c == '#' && s->pos + 1 < s->len && s->src[s->pos + 1] == '{') {
    parse_advance(s);  // skip #
    omni_expect_char(s, '{');
    Term pairs = omni_nil();
    Term *tail = &pairs;
    while (parse_peek(s) != '}' && !parse_at_end(s)) {
      Term key = parse_omni_expr(s);
      Term val = parse_omni_expr(s);
      Term pair = omni_cons(key, val);
      Term cell = omni_cons(pair, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }
    omni_expect_char(s, '}');
    return omni_ctr1(OMNI_NAM_DICT, pairs);
  }

  // Regex literal: #r"pattern" or #r"pattern"flags
  if (c == '#' && s->pos + 1 < s->len && s->src[s->pos + 1] == 'r') {
    parse_advance(s);  // skip #
    parse_advance(s);  // skip r
    omni_skip(s);

    // Expect opening quote
    if (parse_peek(s) != '"') {
      parse_error(s, "\"", parse_peek(s));
    }
    parse_advance(s);  // skip "

    // Parse regex pattern (with escape handling)
    Term pattern = omni_nil();
    Term *tail = &pattern;
    while (!parse_at_end(s) && parse_peek(s) != '"') {
      char ch = parse_peek(s);
      if (ch == '\\' && s->pos + 1 < s->len) {
        // Keep escape sequences for regex
        parse_advance(s);
        char esc = parse_peek(s);
        // Add backslash and escaped char
        Term bs_cell = omni_cons(omni_chr('\\'), omni_nil());
        *tail = bs_cell;
        tail = &HEAP[term_val(bs_cell) + 1];
        Term esc_cell = omni_cons(omni_chr(esc), omni_nil());
        *tail = esc_cell;
        tail = &HEAP[term_val(esc_cell) + 1];
        parse_advance(s);
      } else {
        Term cell = omni_cons(omni_chr(ch), omni_nil());
        *tail = cell;
        tail = &HEAP[term_val(cell) + 1];
        parse_advance(s);
      }
    }
    if (parse_peek(s) != '"') {
      parse_error(s, "\"", parse_peek(s));
    }
    parse_advance(s);  // skip closing "

    // Parse optional flags (i, g, m, s, etc.)
    Term flags = omni_nil();
    Term *flags_tail = &flags;
    while (!parse_at_end(s) && !omni_is_delim(parse_peek(s))) {
      char flag = parse_peek(s);
      Term flag_cell = omni_cons(omni_chr(flag), omni_nil());
      *flags_tail = flag_cell;
      flags_tail = &HEAP[term_val(flag_cell) + 1];
      parse_advance(s);
    }

    return omni_ctr2(OMNI_NAM_REGX, pattern, flags);
  }

  // Format string: #fmt"Hello $name" - interpolated string
  if (c == '#' && s->pos + 3 < s->len &&
      s->src[s->pos + 1] == 'f' &&
      s->src[s->pos + 2] == 'm' &&
      s->src[s->pos + 3] == 't') {
    parse_advance(s);  // skip #
    parse_advance(s);  // skip f
    parse_advance(s);  // skip m
    parse_advance(s);  // skip t
    omni_skip(s);

    // Expect opening quote
    if (parse_peek(s) != '"') {
      parse_error(s, "\"", parse_peek(s));
    }
    parse_advance(s);  // skip "

    // Parse format string, building list of parts
    Term parts = omni_nil();
    Term *parts_tail = &parts;

    // Current literal being accumulated
    Term lit_chars = omni_nil();
    Term *lit_tail = &lit_chars;

    while (!parse_at_end(s) && parse_peek(s) != '"') {
      char ch = parse_peek(s);

      if (ch == '$') {
        // Flush any accumulated literal
        if (term_tag(lit_chars) != term_tag(omni_nil()) ||
            term_val(lit_chars) != term_val(omni_nil())) {
          Term lit_part = omni_ctr1(OMNI_NAM_FLIT, lit_chars);
          Term cell = omni_cons(lit_part, omni_nil());
          *parts_tail = cell;
          parts_tail = &HEAP[term_val(cell) + 1];
          lit_chars = omni_nil();
          lit_tail = &lit_chars;
        }

        parse_advance(s);  // skip $

        // Check for ${expr} or $name
        if (parse_peek(s) == '{') {
          // ${expr} - full expression interpolation
          parse_advance(s);  // skip {
          Term expr = parse_omni_expr(s);
          omni_expect_char(s, '}');
          Term exp_part = omni_ctr1(OMNI_NAM_FEXP, expr);
          Term cell = omni_cons(exp_part, omni_nil());
          *parts_tail = cell;
          parts_tail = &HEAP[term_val(cell) + 1];
        } else {
          // $name - simple variable interpolation
          u32 sym_start, sym_len;
          if (omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
            u32 sym_nick = omni_symbol_nick(s, sym_start, sym_len);
            // Look up variable in binding stack
            u32 idx;
            Term var_ref;
            if (omni_bind_lookup(sym_nick, &idx)) {
              var_ref = omni_var(idx);
            } else {
              // Free variable - use symbol
              var_ref = omni_sym(sym_nick);
            }
            Term exp_part = omni_ctr1(OMNI_NAM_FEXP, var_ref);
            Term cell = omni_cons(exp_part, omni_nil());
            *parts_tail = cell;
            parts_tail = &HEAP[term_val(cell) + 1];
          } else {
            // Just a lone $, treat as literal
            Term chr_cell = omni_cons(omni_chr('$'), omni_nil());
            *lit_tail = chr_cell;
            lit_tail = &HEAP[term_val(chr_cell) + 1];
          }
        }
      } else if (ch == '\\' && s->pos + 1 < s->len) {
        // Escape sequence
        parse_advance(s);
        char esc = parse_peek(s);
        parse_advance(s);
        u32 ec;
        switch (esc) {
          case 'n': ec = '\n'; break;
          case 't': ec = '\t'; break;
          case 'r': ec = '\r'; break;
          case '"': ec = '"'; break;
          case '\\': ec = '\\'; break;
          case '$': ec = '$'; break;  // Escape $ literally
          default: ec = (u32)esc;
        }
        Term chr_cell = omni_cons(omni_chr(ec), omni_nil());
        *lit_tail = chr_cell;
        lit_tail = &HEAP[term_val(chr_cell) + 1];
      } else {
        // Regular character
        u32 cp = parse_utf8(s);
        if (cp == 0 && parse_peek(s) != '\0') {
          cp = (u32)parse_peek(s);
          parse_advance(s);
        }
        Term chr_cell = omni_cons(omni_chr(cp), omni_nil());
        *lit_tail = chr_cell;
        lit_tail = &HEAP[term_val(chr_cell) + 1];
      }
    }

    // Flush final literal if any
    if (term_tag(lit_chars) != term_tag(omni_nil()) ||
        term_val(lit_chars) != term_val(omni_nil())) {
      Term lit_part = omni_ctr1(OMNI_NAM_FLIT, lit_chars);
      Term cell = omni_cons(lit_part, omni_nil());
      *parts_tail = cell;
      parts_tail = &HEAP[term_val(cell) + 1];
    }

    if (parse_peek(s) != '"') {
      parse_error(s, "\"", parse_peek(s));
    }
    parse_advance(s);  // skip closing "

    return omni_ctr1(OMNI_NAM_FMTS, parts);
  }

  // Set literal: #set{1 2 3}
  if (c == '#' && s->pos + 3 < s->len &&
      s->src[s->pos + 1] == 's' &&
      s->src[s->pos + 2] == 'e' &&
      s->src[s->pos + 3] == 't' &&
      s->pos + 4 < s->len && s->src[s->pos + 4] == '{') {
    parse_advance(s);  // skip #
    parse_advance(s);  // skip s
    parse_advance(s);  // skip e
    parse_advance(s);  // skip t
    omni_expect_char(s, '{');

    Term elements = omni_nil();
    Term *tail = &elements;
    while (parse_peek(s) != '}' && !parse_at_end(s)) {
      Term elem = parse_omni_expr(s);
      Term cell = omni_cons(elem, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }
    omni_expect_char(s, '}');
    return omni_ctr1(OMNI_NAM_SET, elements);
  }

  // String: "..."
  if (c == '"') {
    return parse_omni_string(s);
  }

  // Type annotation: {Type}
  if (c == '{') {
    return parse_omni_type(s);
  }

  // Array literal: [...]
  if (c == '[') {
    omni_expect_char(s, '[');
    Term items = omni_nil();
    Term *tail = &items;
    while (parse_peek(s) != ']' && !parse_at_end(s)) {
      Term item = parse_omni_expr(s);
      Term cell = omni_cons(item, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }
    omni_expect_char(s, ']');
    return omni_ctr1(OMNI_NAM_ARR, items);
  }

  // List/S-expression: (...)
  if (c == '(') {
    return parse_omni_expr(s);
  }

  // Number
  Term num_out;
  if (omni_parse_number(s, &num_out)) {
    return num_out;
  }

  // Symbol/variable
  u32 sym_start, sym_len;
  if (omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
    // Check special values
    if (omni_symbol_is(s, sym_start, sym_len, "true")) return omni_true();
    if (omni_symbol_is(s, sym_start, sym_len, "false")) return omni_false();
    if (omni_symbol_is(s, sym_start, sym_len, "nothing")) return omni_nothing();
    if (omni_symbol_is(s, sym_start, sym_len, "nil")) return omni_nil();

    // Note: _ in expression context is treated as a regular symbol.
    // In pattern context, _ is a wildcard (handled by parse_omni_pattern).
    // For pipe argument positioning, use flip/partial instead of placeholder.

    u32 nick = omni_symbol_nick(s, sym_start, sym_len);

    // Check if bound variable
    u32 idx;
    if (omni_bind_lookup(nick, &idx)) {
      return omni_var(idx);
    }

    // Check for reference (@name in book)
    char name_buf[256];
    u32 copy_len = sym_len < 255 ? sym_len : 255;
    memcpy(name_buf, s->src + sym_start, copy_len);
    name_buf[copy_len] = '\0';
    u32 ref_id = table_find(name_buf, copy_len);
    if (BOOK[ref_id] != 0) {
      return term_new_ref(ref_id);
    }

    // Free variable - treat as symbol (will be resolved at runtime)
    return omni_sym(nick);
  }

  parse_error(s, "expression", c);
  return omni_nil();
}

// =============================================================================
// S-Expression Parsing (special forms)
// =============================================================================

fn Term parse_omni_sexp(PState *s) {
  omni_expect_char(s, '(');

  omni_skip(s);

  // Empty list
  if (parse_peek(s) == ')') {
    parse_advance(s);
    omni_skip(s);
    return omni_nil();
  }

  // Get the head symbol
  u32 sym_start, sym_len;
  if (!omni_parse_symbol_raw(s, &sym_start, &sym_len)) {
    // Not a special form, parse as application
    s->pos = sym_start;  // Rewind
    Term fn = parse_omni_expr(s);
    while (parse_peek(s) != ')' && !parse_at_end(s)) {
      Term arg = parse_omni_expr(s);
      fn = omni_app(fn, arg);
    }
    omni_expect_char(s, ')');
    return fn;
  }

  // ============ SPECIAL FORMS ============

  // define: (define name [slots...] body) or (define name value)
  //         (define {abstract Name})
  //         (define {struct Name} [field {Type}]...)
  //         (define {enum Name} Variant1 Variant2 ...)
  if (omni_symbol_is(s, sym_start, sym_len, "define")) {
    omni_skip(s);

    // Check for type definition: (define {kind Name} ...)
    if (parse_peek(s) == '{') {
      omni_expect_char(s, '{');

      // Parse type kind: abstract, struct, enum, or just a type name
      u32 kind_start, kind_len;
      if (!omni_parse_symbol_raw(s, &kind_start, &kind_len)) {
        parse_error(s, "type kind or name", parse_peek(s));
        return omni_nil();
      }

      // Check for abstract type: (define {abstract Name})
      if (omni_symbol_is(s, kind_start, kind_len, "abstract")) {
        u32 type_start, type_len;
        if (!omni_parse_symbol_raw(s, &type_start, &type_len)) {
          parse_error(s, "abstract type name", parse_peek(s));
          return omni_nil();
        }
        u32 type_nick = omni_symbol_nick(s, type_start, type_len);
        omni_expect_char(s, '}');

        // Parse optional parent: ^:parent {ParentType}
        Term parent = omni_nil();
        // Check for parent in metadata before the type definition
        // For now, no parent support in abstract types directly

        omni_expect_char(s, ')');

        // Return #TAbs{name, parent}
        Term result = omni_ctr2(OMNI_NAM_TABS, term_new_num(type_nick), parent);

        // Register type in book
        char type_name[256];
        u32 copy_len = type_len < 255 ? type_len : 255;
        memcpy(type_name, s->src + type_start, copy_len);
        type_name[copy_len] = '\0';
        u32 def_id = table_find(type_name, copy_len);
        BOOK[def_id] = result;

        return result;
      }

      // Check for struct type: (define {struct Name} [field {Type}]...)
      if (omni_symbol_is(s, kind_start, kind_len, "struct")) {
        // Parse struct name (possibly parametric: [Name T])
        u32 type_nick;
        Term type_params = omni_nil();

        if (parse_peek(s) == '[') {
          // Parametric struct: {struct [Name T U]}
          omni_expect_char(s, '[');
          u32 type_start, type_len;
          if (!omni_parse_symbol_raw(s, &type_start, &type_len)) {
            parse_error(s, "struct name", parse_peek(s));
            return omni_nil();
          }
          type_nick = omni_symbol_nick(s, type_start, type_len);

          // Parse type parameters (with optional variance annotations)
          Term *params_tail = &type_params;
          while (parse_peek(s) != ']' && !parse_at_end(s)) {
            // Check for variance annotation: ^:covar or ^:contravar
            int variance = 0;  // 0 = invariant, 1 = covariant, -1 = contravariant
            if (parse_peek(s) == '^') {
              u32 saved_var_pos = s->pos;
              parse_advance(s);  // skip ^
              if (parse_peek(s) == ':') {
                parse_advance(s);  // skip :
                u32 var_start, var_len;
                if (omni_parse_symbol_raw(s, &var_start, &var_len)) {
                  if (omni_symbol_is(s, var_start, var_len, "covar")) {
                    variance = 1;
                  } else if (omni_symbol_is(s, var_start, var_len, "contravar")) {
                    variance = -1;
                  } else {
                    // Unknown metadata, restore position
                    s->pos = saved_var_pos;
                  }
                } else {
                  s->pos = saved_var_pos;
                }
              } else {
                s->pos = saved_var_pos;
              }
            }

            u32 param_start, param_len;
            if (omni_parse_symbol_raw(s, &param_start, &param_len)) {
              u32 param_nick = omni_symbol_nick(s, param_start, param_len);
              Term param = omni_ctr1(OMNI_NAM_TVAR, term_new_num(param_nick));

              // Wrap with variance marker if specified
              if (variance == 1) {
                param = omni_ctr1(OMNI_NAM_COVR, param);  // #Covr{#TVar{T}}
              } else if (variance == -1) {
                param = omni_ctr1(OMNI_NAM_CNVR, param);  // #Cnvr{#TVar{T}}
              }

              Term cell = omni_cons(param, omni_nil());
              *params_tail = cell;
              params_tail = &HEAP[term_val(cell) + 1];
            } else {
              break;
            }
          }
          omni_expect_char(s, ']');
        } else {
          // Simple struct name
          u32 type_start, type_len;
          if (!omni_parse_symbol_raw(s, &type_start, &type_len)) {
            parse_error(s, "struct name", parse_peek(s));
            return omni_nil();
          }
          type_nick = omni_symbol_nick(s, type_start, type_len);
        }

        omni_expect_char(s, '}');

        // Parse fields: [field {Type}]...
        Term fields = omni_nil();
        Term *fields_tail = &fields;
        while (parse_peek(s) == '[') {
          OmniSlot slot;
          if (parse_omni_slot(s, &slot)) {
            // Create field descriptor: #TFld{name, type}
            Term field = omni_ctr2(OMNI_NAM_TFLD, term_new_num(slot.name_nick), slot.type);
            Term cell = omni_cons(field, omni_nil());
            *fields_tail = cell;
            fields_tail = &HEAP[term_val(cell) + 1];
          } else {
            break;
          }
        }

        omni_expect_char(s, ')');

        // Return #TStr{name, parent, fields, params}
        // For now: #TStr{name, NIL (parent), fields, params}
        Term args[4];
        args[0] = term_new_num(type_nick);
        args[1] = omni_nil();  // parent
        args[2] = fields;
        args[3] = type_params;
        Term result = term_new_ctr(OMNI_NAM_TSTR, 4, args);

        return result;
      }

      // Check for enum type: (define {enum Name} Variant1 (Variant2 [field {Type}]) ...)
      if (omni_symbol_is(s, kind_start, kind_len, "enum")) {
        // Parse enum name (possibly parametric)
        u32 type_nick;
        Term type_params = omni_nil();

        if (parse_peek(s) == '[') {
          // Parametric enum: {enum [Option T]}
          omni_expect_char(s, '[');
          u32 type_start, type_len;
          if (!omni_parse_symbol_raw(s, &type_start, &type_len)) {
            parse_error(s, "enum name", parse_peek(s));
            return omni_nil();
          }
          type_nick = omni_symbol_nick(s, type_start, type_len);

          // Parse type parameters (with optional variance annotations)
          Term *params_tail = &type_params;
          while (parse_peek(s) != ']' && !parse_at_end(s)) {
            // Check for variance annotation: ^:covar or ^:contravar
            int variance = 0;  // 0 = invariant, 1 = covariant, -1 = contravariant
            if (parse_peek(s) == '^') {
              u32 saved_var_pos = s->pos;
              parse_advance(s);  // skip ^
              if (parse_peek(s) == ':') {
                parse_advance(s);  // skip :
                u32 var_start, var_len;
                if (omni_parse_symbol_raw(s, &var_start, &var_len)) {
                  if (omni_symbol_is(s, var_start, var_len, "covar")) {
                    variance = 1;
                  } else if (omni_symbol_is(s, var_start, var_len, "contravar")) {
                    variance = -1;
                  } else {
                    s->pos = saved_var_pos;
                  }
                } else {
                  s->pos = saved_var_pos;
                }
              } else {
                s->pos = saved_var_pos;
              }
            }

            u32 param_start, param_len;
            if (omni_parse_symbol_raw(s, &param_start, &param_len)) {
              u32 param_nick = omni_symbol_nick(s, param_start, param_len);
              Term param = omni_ctr1(OMNI_NAM_TVAR, term_new_num(param_nick));

              // Wrap with variance marker if specified
              if (variance == 1) {
                param = omni_ctr1(OMNI_NAM_COVR, param);
              } else if (variance == -1) {
                param = omni_ctr1(OMNI_NAM_CNVR, param);
              }

              Term cell = omni_cons(param, omni_nil());
              *params_tail = cell;
              params_tail = &HEAP[term_val(cell) + 1];
            } else {
              break;
            }
          }
          omni_expect_char(s, ']');
        } else {
          // Simple enum name
          u32 type_start, type_len;
          if (!omni_parse_symbol_raw(s, &type_start, &type_len)) {
            parse_error(s, "enum name", parse_peek(s));
            return omni_nil();
          }
          type_nick = omni_symbol_nick(s, type_start, type_len);
        }

        omni_expect_char(s, '}');

        // Parse variants: Variant1 (Variant2 [field {Type}]) ...
        Term variants = omni_nil();
        Term *variants_tail = &variants;
        while (parse_peek(s) != ')' && !parse_at_end(s)) {
          u32 var_nick;
          Term var_fields = omni_nil();

          if (parse_peek(s) == '(') {
            // Variant with fields: (VariantName [field {Type}]...)
            omni_expect_char(s, '(');
            u32 var_start, var_len;
            if (!omni_parse_symbol_raw(s, &var_start, &var_len)) {
              parse_error(s, "variant name", parse_peek(s));
              break;
            }
            var_nick = omni_symbol_nick(s, var_start, var_len);

            // Parse variant fields
            Term *var_fields_tail = &var_fields;
            while (parse_peek(s) == '[') {
              OmniSlot slot;
              if (parse_omni_slot(s, &slot)) {
                Term field = omni_ctr2(OMNI_NAM_TFLD, term_new_num(slot.name_nick), slot.type);
                Term cell = omni_cons(field, omni_nil());
                *var_fields_tail = cell;
                var_fields_tail = &HEAP[term_val(cell) + 1];
              } else {
                break;
              }
            }
            omni_expect_char(s, ')');
          } else {
            // Simple variant (no fields)
            u32 var_start, var_len;
            if (!omni_parse_symbol_raw(s, &var_start, &var_len)) {
              break;
            }
            var_nick = omni_symbol_nick(s, var_start, var_len);
          }

          // Create variant descriptor: #TVrn{name, fields}
          Term variant = omni_ctr2(OMNI_NAM_TVRN, term_new_num(var_nick), var_fields);
          Term cell = omni_cons(variant, omni_nil());
          *variants_tail = cell;
          variants_tail = &HEAP[term_val(cell) + 1];
        }

        omni_expect_char(s, ')');

        // Return #TEnm{name, variants, params}
        Term args[3];
        args[0] = term_new_num(type_nick);
        args[1] = variants;
        args[2] = type_params;
        Term result = term_new_ctr(OMNI_NAM_TENM, 3, args);

        return result;
      }

      // Check for effect type: (define {effect Name} [op [args] {RetType}]...)
      // Effect declarations define algebraic effect types with operations
      if (omni_symbol_is(s, kind_start, kind_len, "effect")) {
        // Parse effect name
        u32 effect_start, effect_len;
        if (!omni_parse_symbol_raw(s, &effect_start, &effect_len)) {
          parse_error(s, "effect name", parse_peek(s));
          return omni_nil();
        }
        u32 effect_nick = omni_symbol_nick(s, effect_start, effect_len);

        omni_expect_char(s, '}');

        // Parse effect operations: [op-name [arg-slots...] {RetType}]...
        Term operations = omni_nil();
        Term *ops_tail = &operations;

        while (parse_peek(s) == '[') {
          omni_expect_char(s, '[');

          // Parse operation name
          u32 op_start, op_len;
          if (!omni_parse_symbol_raw(s, &op_start, &op_len)) {
            parse_error(s, "effect operation name", parse_peek(s));
            return omni_nil();
          }
          u32 op_nick = omni_symbol_nick(s, op_start, op_len);

          // Parse operation parameters: [arg {Type}]...
          Term params = omni_nil();
          Term *params_tail = &params;

          while (parse_peek(s) == '[') {
            OmniSlot slot;
            if (parse_omni_slot(s, &slot)) {
              // Create parameter: #Slot{name, type}
              Term param = omni_ctr2(OMNI_NAM_SLOT, term_new_num(slot.name_nick), slot.type);
              Term cell = omni_cons(param, omni_nil());
              *params_tail = cell;
              params_tail = &HEAP[term_val(cell) + 1];
            } else {
              break;
            }
          }

          // Parse return type: {RetType}
          Term ret_type = omni_nil();
          if (parse_peek(s) == '{') {
            ret_type = parse_omni_type(s);
          }

          omni_expect_char(s, ']');

          // Create operation: #TEOp{name, params, ret_type}
          Term args[3];
          args[0] = term_new_num(op_nick);
          args[1] = params;
          args[2] = ret_type;
          Term op = term_new_ctr(OMNI_NAM_TEOP, 3, args);

          Term cell = omni_cons(op, omni_nil());
          *ops_tail = cell;
          ops_tail = &HEAP[term_val(cell) + 1];
        }

        omni_expect_char(s, ')');

        // Return #TEff{name, operations}
        Term result = omni_ctr2(OMNI_NAM_TEFF, term_new_num(effect_nick), operations);

        // Register effect in book
        char effect_name[256];
        u32 copy_len = effect_len < 255 ? effect_len : 255;
        memcpy(effect_name, s->src + effect_start, copy_len);
        effect_name[copy_len] = '\0';
        u32 def_id = table_find(effect_name, copy_len);
        BOOK[def_id] = result;

        return result;
      }

      // Not a special type keyword - must be a type annotation form
      // This would be like (define {TypeName} ...) for union or alias
      // For now, treat as union if followed by (union [...])
      u32 type_nick = omni_symbol_nick(s, kind_start, kind_len);
      omni_expect_char(s, '}');

      // Check for union definition: (define {Name} (union [{Type1} {Type2}]))
      if (parse_peek(s) == '(') {
        u32 saved_pos = s->pos;
        omni_expect_char(s, '(');
        u32 inner_start, inner_len;
        if (omni_parse_symbol_raw(s, &inner_start, &inner_len) &&
            omni_symbol_is(s, inner_start, inner_len, "union")) {
          // Parse union types
          omni_expect_char(s, '[');
          Term types = omni_nil();
          Term *types_tail = &types;
          while (parse_peek(s) != ']' && !parse_at_end(s)) {
            Term t = parse_omni_type(s);
            Term cell = omni_cons(t, omni_nil());
            *types_tail = cell;
            types_tail = &HEAP[term_val(cell) + 1];
          }
          omni_expect_char(s, ']');
          omni_expect_char(s, ')');
          omni_expect_char(s, ')');

          // Return #TUni{name, types}
          return omni_ctr2(OMNI_NAM_TUNI, term_new_num(type_nick), types);
        }
        // Not a union, restore position
        s->pos = saved_pos;
      }

      parse_error(s, "type definition body", parse_peek(s));
      return omni_nil();
    }

    // Check for ^:where type constraints: (define ^:where [T {Number}] name ...)
    // Parses: ^:where [TypeVar {Bound}]...
    Term type_constraints = omni_nil();
    if (parse_peek(s) == '^') {
      u32 saved_meta_pos = s->pos;
      parse_advance(s);  // skip ^
      if (parse_peek(s) == ':') {
        parse_advance(s);  // skip :
        u32 meta_start, meta_len;
        if (omni_parse_symbol_raw(s, &meta_start, &meta_len) &&
            omni_symbol_is(s, meta_start, meta_len, "where")) {
          // Parse constraint list: [T {Bound}] [U {Bound2}] ...
          Term *constraints_tail = &type_constraints;
          while (parse_peek(s) == '[') {
            omni_expect_char(s, '[');

            // Parse type variable name
            u32 tvar_start, tvar_len;
            if (!omni_parse_symbol_raw(s, &tvar_start, &tvar_len)) {
              parse_error(s, "type variable name in ^:where", parse_peek(s));
              break;
            }
            u32 tvar_nick = omni_symbol_nick(s, tvar_start, tvar_len);

            // Parse bound type: {Number}
            Term bound = omni_nil();
            if (parse_peek(s) == '{') {
              bound = parse_omni_type(s);
            }

            omni_expect_char(s, ']');

            // Create constraint: #TWhr{tvar_nick, bound}
            Term constraint = omni_ctr2(OMNI_NAM_TWHR, term_new_num(tvar_nick), bound);
            Term cell = omni_cons(constraint, omni_nil());
            *constraints_tail = cell;
            constraints_tail = &HEAP[term_val(cell) + 1];
          }
          omni_skip(s);
        } else {
          // Not ^:where, restore position
          s->pos = saved_meta_pos;
        }
      } else {
        // Not ^:something, restore position
        s->pos = saved_meta_pos;
      }
    }

    // Check for syntax macro definition: (define [syntax name] ...patterns...)
    if (parse_peek(s) == '[') {
      u32 saved_pos = s->pos;
      omni_expect_char(s, '[');

      u32 kw_start, kw_len;
      if (omni_parse_symbol_raw(s, &kw_start, &kw_len) &&
          omni_symbol_is(s, kw_start, kw_len, "syntax")) {

        // Parse macro name
        u32 mac_start, mac_len;
        if (!omni_parse_symbol_raw(s, &mac_start, &mac_len)) {
          parse_error(s, "syntax macro name", parse_peek(s));
          return omni_nil();
        }
        u32 mac_nick = omni_symbol_nick(s, mac_start, mac_len);

        omni_expect_char(s, ']');

        // Parse patterns: [pattern template] ...
        Term patterns = omni_nil();
        Term *patterns_tail = &patterns;

        while (parse_peek(s) == '[') {
          omni_expect_char(s, '[');

          // Parse pattern (the left side, to be matched)
          Term pattern = parse_omni_macro_pattern(s);

          // Parse template (the right side, to be expanded)
          Term template = parse_omni_macro_template(s);

          omni_expect_char(s, ']');

          // Create pattern entry: #MPat{pattern, template}
          Term pat_entry = omni_ctr2(OMNI_NAM_MPAT, pattern, template);
          Term cell = omni_cons(pat_entry, omni_nil());
          *patterns_tail = cell;
          patterns_tail = &HEAP[term_val(cell) + 1];
        }

        omni_expect_char(s, ')');

        // Create macro definition: #MSyn{name, patterns}
        Term macro = omni_ctr2(OMNI_NAM_MSYN, term_new_num(mac_nick), patterns);

        // Register in macro table
        char mac_name[256];
        u32 copy_len = mac_len < 255 ? mac_len : 255;
        memcpy(mac_name, s->src + mac_start, copy_len);
        mac_name[copy_len] = '\0';
        u32 def_id = table_find(mac_name, copy_len);
        BOOK[def_id] = macro;

        return macro;
      }

      // Check for grammar definition: (define [grammar name] ...rules...)
      if (omni_parse_symbol_raw(s, &kw_start, &kw_len) &&
          omni_symbol_is(s, kw_start, kw_len, "grammar")) {

        // Parse grammar name
        u32 gram_start, gram_len;
        if (!omni_parse_symbol_raw(s, &gram_start, &gram_len)) {
          parse_error(s, "grammar name", parse_peek(s));
          return omni_nil();
        }
        u32 gram_nick = omni_symbol_nick(s, gram_start, gram_len);

        omni_expect_char(s, ']');

        // Parse rules in two supported formats:
        // 1. Bracket syntax: [rule-name pattern]
        // 2. Pika DSL syntax: rule-name := pattern → action
        Term rules = omni_nil();
        Term *rules_tail = &rules;

        while (parse_peek(s) != ')' && !parse_at_end(s)) {
          omni_skip(s);
          if (parse_peek(s) == ')') break;

          u32 rule_start, rule_len;

          // Check for bracket syntax: [rule-name pattern]
          if (parse_peek(s) == '[') {
            omni_expect_char(s, '[');

            // Parse rule name
            if (!omni_parse_symbol_raw(s, &rule_start, &rule_len)) {
              parse_error(s, "rule name", parse_peek(s));
              return omni_nil();
            }
            u32 rule_nick = omni_symbol_nick(s, rule_start, rule_len);

            // Parse pattern expression
            Term pattern = parse_omni_grammar_pattern(s);

            omni_expect_char(s, ']');

            // Create rule: #Rule{name, pattern}
            Term rule_entry = omni_ctr2(OMNI_NAM_RULE, term_new_num(rule_nick), pattern);
            Term cell = omni_cons(rule_entry, omni_nil());
            *rules_tail = cell;
            rules_tail = &HEAP[term_val(cell) + 1];
          }
          // Check for Pika DSL syntax: rule-name := pattern [→ action]
          else if (omni_parse_symbol_raw(s, &rule_start, &rule_len)) {
            u32 rule_nick = omni_symbol_nick(s, rule_start, rule_len);

            omni_skip(s);
            // Expect :=
            if (parse_peek(s) == ':' && s->pos + 1 < s->len && s->src[s->pos + 1] == '=') {
              parse_advance(s);  // skip :
              parse_advance(s);  // skip =
              omni_skip(s);

              // Parse pattern (until → or newline or next rule)
              // Note: using / as alternation delimiter in patterns
              Term pattern = parse_omni_grammar_pattern(s);
              omni_skip(s);

              // Check for action: → (Unicode U+2192) or -> (ASCII)
              Term action = omni_nil();
              // Check for Unicode → (UTF-8: 0xE2 0x86 0x92)
              if (s->pos + 2 < s->len &&
                  (u8)s->src[s->pos] == 0xE2 &&
                  (u8)s->src[s->pos + 1] == 0x86 &&
                  (u8)s->src[s->pos + 2] == 0x92) {
                parse_advance(s);  // skip UTF-8 byte 1
                parse_advance(s);  // skip UTF-8 byte 2
                parse_advance(s);  // skip UTF-8 byte 3
                omni_skip(s);
                action = parse_omni_expr(s);
              }
              // Check for ASCII ->
              else if (parse_peek(s) == '-' && s->pos + 1 < s->len && s->src[s->pos + 1] == '>') {
                parse_advance(s);  // skip -
                parse_advance(s);  // skip >
                omni_skip(s);
                action = parse_omni_expr(s);
              }

              // Create rule with optional action
              Term rule_pattern = pattern;
              if (term_tag(action) != term_tag(omni_nil()) ||
                  term_val(action) != term_val(omni_nil())) {
                // Wrap pattern with action: #GAct{pattern, action}
                rule_pattern = omni_ctr2(OMNI_NAM_GACT, pattern, action);
              }
              Term rule_entry = omni_ctr2(OMNI_NAM_RULE, term_new_num(rule_nick), rule_pattern);
              Term cell = omni_cons(rule_entry, omni_nil());
              *rules_tail = cell;
              rules_tail = &HEAP[term_val(cell) + 1];
            } else {
              // Not a valid rule definition - error or break
              parse_error(s, ":= after rule name", parse_peek(s));
              return omni_nil();
            }
          } else {
            break;  // No more rules
          }
        }

        omni_expect_char(s, ')');

        // Create grammar definition: #Gram{name, rules}
        Term grammar = omni_ctr2(OMNI_NAM_GRAM, term_new_num(gram_nick), rules);

        // Register in grammar table
        char gram_name[256];
        u32 copy_len = gram_len < 255 ? gram_len : 255;
        memcpy(gram_name, s->src + gram_start, copy_len);
        gram_name[copy_len] = '\0';
        u32 def_id = table_find(gram_name, copy_len);
        BOOK[def_id] = grammar;

        return grammar;
      }

      // Not a syntax or grammar definition, restore position
      s->pos = saved_pos;
    }

    // Regular define: (define name ...) or (define name [slots...] body)
    u32 name_start, name_len;
    if (!omni_parse_symbol_raw(s, &name_start, &name_len)) {
      parse_error(s, "definition name", parse_peek(s));
      return omni_nil();
    }

    char def_name[256];
    u32 copy_len = name_len < 255 ? name_len : 255;
    memcpy(def_name, s->src + name_start, copy_len);
    def_name[copy_len] = '\0';
    u32 name_nick = omni_symbol_nick(s, name_start, name_len);

    // Check for slots (function definition)
    OmniSlot slots[64];
    u32 slot_count = 0;
    int has_typed_params = 0;
    while (parse_peek(s) == '[' && slot_count < 64) {
      if (!parse_omni_slot(s, &slots[slot_count])) break;
      // Check if this slot has a type annotation
      if (term_ext(slots[slot_count].type) != OMNI_NAM_NIL) {
        has_typed_params = 1;
      }
      slot_count++;
    }

    // Parse return type if present
    Term ret_type = omni_nil();
    if (parse_peek(s) == '{') {
      ret_type = parse_omni_type(s);
    }

    // Parse effect row signature: ^:effects [{Error} {Ask}]
    Term effect_row = omni_nil();
    if (parse_peek(s) == '^') {
      u32 saved_eff_pos = s->pos;
      parse_advance(s);  // skip ^
      if (parse_peek(s) == ':') {
        parse_advance(s);  // skip :
        u32 meta_start, meta_len;
        if (omni_parse_symbol_raw(s, &meta_start, &meta_len) &&
            omni_symbol_is(s, meta_start, meta_len, "effects")) {
          // Parse effect list: [{Type1} {Type2}...]
          omni_expect_char(s, '[');
          Term *effects_tail = &effect_row;
          while (parse_peek(s) == '{') {
            Term eff_type = parse_omni_type(s);
            Term cell = omni_cons(eff_type, omni_nil());
            *effects_tail = cell;
            effects_tail = &HEAP[term_val(cell) + 1];
          }
          omni_expect_char(s, ']');
          // Wrap in effect row node: #ERws{effects}
          effect_row = omni_ctr1(OMNI_NAM_ERWS, effect_row);
          omni_skip(s);
        } else {
          // Not ^:effects, restore position
          s->pos = saved_eff_pos;
        }
      } else {
        // Not ^:something, restore position
        s->pos = saved_eff_pos;
      }
    }

    // Parse ^:require precondition(s): ^:require predicate
    // Multiple ^:require can be specified
    // Desugars to (perform require predicate) at function entry
    Term requires = omni_nil();
    Term *requires_tail = &requires;
    while (parse_peek(s) == '^') {
      u32 saved_req_pos = s->pos;
      parse_advance(s);  // skip ^
      if (parse_peek(s) == ':') {
        parse_advance(s);  // skip :
        u32 meta_start, meta_len;
        if (omni_parse_symbol_raw(s, &meta_start, &meta_len) &&
            omni_symbol_is(s, meta_start, meta_len, "require")) {
          // Parse the predicate expression
          Term predicate = parse_omni_expr(s);
          // Create #Reqr{predicate} node
          Term req = omni_ctr1(OMNI_NAM_REQR, predicate);
          Term cell = omni_cons(req, omni_nil());
          *requires_tail = cell;
          requires_tail = &HEAP[term_val(cell) + 1];
          omni_skip(s);
        } else {
          // Not ^:require, restore position and break
          s->pos = saved_req_pos;
          break;
        }
      } else {
        // Not ^:something, restore position and break
        s->pos = saved_req_pos;
        break;
      }
    }

    // Parse ^:ensure postcondition(s): ^:ensure predicate
    // Multiple ^:ensure can be specified
    // Desugars to (perform ensure predicate) wrapping the result
    // Note: 'result' is bound to the function's return value in the predicate
    Term ensures = omni_nil();
    Term *ensures_tail = &ensures;
    while (parse_peek(s) == '^') {
      u32 saved_ens_pos = s->pos;
      parse_advance(s);  // skip ^
      if (parse_peek(s) == ':') {
        parse_advance(s);  // skip :
        u32 meta_start, meta_len;
        if (omni_parse_symbol_raw(s, &meta_start, &meta_len) &&
            omni_symbol_is(s, meta_start, meta_len, "ensure")) {
          // Parse the predicate expression
          Term predicate = parse_omni_expr(s);
          // Create #Ensr{predicate} node
          Term ens = omni_ctr1(OMNI_NAM_ENSR, predicate);
          Term cell = omni_cons(ens, omni_nil());
          *ensures_tail = cell;
          ensures_tail = &HEAP[term_val(cell) + 1];
          omni_skip(s);
        } else {
          // Not ^:ensure, restore position and break
          s->pos = saved_ens_pos;
          break;
        }
      } else {
        // Not ^:something, restore position and break
        s->pos = saved_ens_pos;
        break;
      }
    }

    // Push parameter bindings
    for (u32 i = 0; i < slot_count; i++) {
      omni_bind_push(slots[i].name_nick);
    }

    // Parse body
    Term body = parse_omni_expr(s);

    omni_expect_char(s, ')');

    // Pop bindings
    omni_bind_pop(slot_count);

    // Desugar ^:require and ^:ensure to effect calls
    // ^:require P1 ^:require P2 ... body
    // becomes: (do (perform require P1) (perform require P2) ... body)
    if (term_ext(requires) != OMNI_NAM_NIL) {
      // Build a do-block with all require performs followed by body
      Term do_exprs = omni_nil();
      Term *do_tail = &do_exprs;

      // Add performs for each require
      Term req_cur = requires;
      while (term_ext(req_cur) != OMNI_NAM_NIL) {
        // Extract #Reqr{predicate} from list
        Term req_node = omni_ctr_arg(req_cur, 0);  // car
        Term predicate = omni_ctr_arg(req_node, 0);  // predicate from #Reqr{predicate}

        // Create: (perform require predicate)
        Term require_tag = omni_sym(omni_nick("reqr"));
        Term perform_require = omni_perform(require_tag, predicate);

        Term cell = omni_cons(perform_require, omni_nil());
        *do_tail = cell;
        do_tail = &HEAP[term_val(cell) + 1];

        req_cur = omni_ctr_arg(req_cur, 1);  // cdr
      }

      // Add body as last expression
      Term cell = omni_cons(body, omni_nil());
      *do_tail = cell;

      // Wrap in do-block: #Do{exprs}
      body = omni_ctr1(OMNI_NAM_DO, do_exprs);
    }

    // ^:ensure Q1 ^:ensure Q2 ... body
    // becomes: (let [__result body] (do (perform ensure Q1) (perform ensure Q2) ... __result))
    if (term_ext(ensures) != OMNI_NAM_NIL) {
      // Push binding for __result
      u32 result_nick = omni_nick("rslt");  // internal binding for result
      omni_bind_push(result_nick);

      // Build a do-block with all ensure performs followed by __result ref
      Term do_exprs = omni_nil();
      Term *do_tail = &do_exprs;

      // Add performs for each ensure
      Term ens_cur = ensures;
      while (term_ext(ens_cur) != OMNI_NAM_NIL) {
        // Extract #Ensr{predicate} from list
        Term ens_node = omni_ctr_arg(ens_cur, 0);  // car
        Term predicate = omni_ctr_arg(ens_node, 0);  // predicate from #Ensr{predicate}

        // Create: (perform ensure predicate)
        Term ensure_tag = omni_sym(omni_nick("ensr"));
        Term perform_ensure = omni_perform(ensure_tag, predicate);

        Term cell = omni_cons(perform_ensure, omni_nil());
        *do_tail = cell;
        do_tail = &HEAP[term_val(cell) + 1];

        ens_cur = omni_ctr_arg(ens_cur, 1);  // cdr
      }

      // Add __result reference as last expression (de Bruijn index 0)
      Term result_ref = omni_var(0);
      Term cell = omni_cons(result_ref, omni_nil());
      *do_tail = cell;

      // do-block: #Do{exprs}
      Term do_block = omni_ctr1(OMNI_NAM_DO, do_exprs);

      // Pop the result binding
      omni_bind_pop(1);

      // Wrap in let: (let [__result body] do_block)
      // #Let{body, lambda_over_do_block}
      Term let_body = omni_lam(do_block);  // lambda wrapping do_block
      body = omni_ctr2(OMNI_NAM_LET, body, let_body);
    }

    // Wrap body in lambdas
    for (int i = (int)slot_count - 1; i >= 0; i--) {
      body = omni_lam(body);
    }

    // Check if this is a typed function (for multiple dispatch)
    if (has_typed_params && slot_count > 0) {
      // Build signature: list of types
      Term sig = omni_nil();
      for (int i = (int)slot_count - 1; i >= 0; i--) {
        Term slot_type = slots[i].type;
        // If no type, use Any
        if (term_ext(slot_type) == OMNI_NAM_NIL) {
          slot_type = omni_sym(omni_nick("Any"));
        }
        sig = omni_cons(slot_type, sig);
      }

      // Create method: #Meth{name, sig, impl, constraints, effects}
      // constraints is a list of #TWhr{tvar, bound} from ^:where
      // effects is #ERws{effect_types} from ^:effects or NIL
      Term meth = omni_ctr5(OMNI_NAM_METH,
                            term_new_num(name_nick),
                            sig,
                            body,
                            type_constraints,
                            effect_row);

      // Register in book as method entry
      // The runtime will accumulate multiple methods for same name
      u32 def_id = table_find(def_name, copy_len);

      // If this is the first method, wrap in GFun; otherwise append to existing
      Term existing = BOOK[def_id];
      if (term_tag(existing) == CTR && term_ext(existing) == OMNI_NAM_GFUN) {
        // Append method to existing generic function
        Term methods = omni_ctr_arg(existing, 1);
        Term new_methods = omni_cons(meth, methods);
        BOOK[def_id] = omni_ctr2(OMNI_NAM_GFUN, term_new_num(name_nick), new_methods);
      } else {
        // Create new generic function with this method
        Term methods = omni_cons(meth, omni_nil());
        BOOK[def_id] = omni_ctr2(OMNI_NAM_GFUN, term_new_num(name_nick), methods);
      }

      return meth;
    }

    // Regular (untyped) definition
    u32 def_id = table_find(def_name, copy_len);
    BOOK[def_id] = body;

    return body;
  }

  // module: (module Name (export sym1 sym2 ...) body...)
  if (omni_symbol_is(s, sym_start, sym_len, "module")) {
    // Parse module name
    u32 mod_start, mod_len;
    if (!omni_parse_symbol_raw(s, &mod_start, &mod_len)) {
      parse_error(s, "module name", parse_peek(s));
      return omni_nil();
    }
    u32 mod_nick = omni_symbol_nick(s, mod_start, mod_len);

    // Parse export list: (export sym1 sym2 ...)
    Term exports = omni_nil();
    if (parse_peek(s) == '(') {
      u32 saved_pos = s->pos;
      omni_expect_char(s, '(');

      u32 kw_start, kw_len;
      if (omni_parse_symbol_raw(s, &kw_start, &kw_len) &&
          omni_symbol_is(s, kw_start, kw_len, "export")) {

        Term *exports_tail = &exports;
        while (parse_peek(s) != ')' && !parse_at_end(s)) {
          u32 exp_start, exp_len;
          if (!omni_parse_symbol_raw(s, &exp_start, &exp_len)) break;
          u32 exp_nick = omni_symbol_nick(s, exp_start, exp_len);
          Term cell = omni_cons(term_new_num(exp_nick), omni_nil());
          *exports_tail = cell;
          exports_tail = &HEAP[term_val(cell) + 1];
        }
        omni_expect_char(s, ')');
      } else {
        // Not an export, restore and treat as body
        s->pos = saved_pos;
      }
    }

    // Parse module body (sequence of definitions)
    Term body = omni_nil();
    Term *body_tail = &body;
    while (parse_peek(s) != ')' && !parse_at_end(s)) {
      Term expr = parse_omni_expr(s);
      Term cell = omni_cons(expr, omni_nil());
      *body_tail = cell;
      body_tail = &HEAP[term_val(cell) + 1];
    }

    omni_expect_char(s, ')');

    // Create module: #Modl{name, exports, body}
    Term mod = omni_ctr3(OMNI_NAM_MODL,
                         term_new_num(mod_nick),
                         omni_ctr1(OMNI_NAM_EXPT, exports),
                         body);

    // Register module in book
    char mod_name[256];
    u32 copy_len = mod_len < 255 ? mod_len : 255;
    memcpy(mod_name, s->src + mod_start, copy_len);
    mod_name[copy_len] = '\0';
    u32 def_id = table_find(mod_name, copy_len);
    BOOK[def_id] = mod;

    return mod;
  }

  // import: (import ModuleName) or (import ModuleName (only sym1 sym2))
  if (omni_symbol_is(s, sym_start, sym_len, "import")) {
    // Parse module name
    u32 mod_start, mod_len;
    if (!omni_parse_symbol_raw(s, &mod_start, &mod_len)) {
      parse_error(s, "module name to import", parse_peek(s));
      return omni_nil();
    }
    u32 mod_nick = omni_symbol_nick(s, mod_start, mod_len);

    // Parse optional binding specification
    Term bindings = omni_nil();  // nil means import all exports

    if (parse_peek(s) == '(') {
      omni_expect_char(s, '(');

      u32 kw_start, kw_len;
      if (omni_parse_symbol_raw(s, &kw_start, &kw_len)) {
        // (only sym1 sym2 ...) - selective import
        if (omni_symbol_is(s, kw_start, kw_len, "only")) {
          Term *bindings_tail = &bindings;
          while (parse_peek(s) != ')' && !parse_at_end(s)) {
            u32 bind_start, bind_len;
            if (!omni_parse_symbol_raw(s, &bind_start, &bind_len)) break;
            u32 bind_nick = omni_symbol_nick(s, bind_start, bind_len);
            Term cell = omni_cons(term_new_num(bind_nick), omni_nil());
            *bindings_tail = cell;
            bindings_tail = &HEAP[term_val(cell) + 1];
          }
        }
        // (as alias) - import with prefix
        else if (omni_symbol_is(s, kw_start, kw_len, "as")) {
          u32 alias_start, alias_len;
          if (omni_parse_symbol_raw(s, &alias_start, &alias_len)) {
            u32 alias_nick = omni_symbol_nick(s, alias_start, alias_len);
            bindings = omni_ctr2(OMNI_NAM_SYM, term_new_num(alias_nick), omni_nil());
          }
        }
      }
      omni_expect_char(s, ')');
    }

    omni_expect_char(s, ')');

    // Create import: #Impt{module_name, bindings}
    return omni_ctr2(OMNI_NAM_IMPT, term_new_num(mod_nick), bindings);
  }

  // let: (let [name value]... body) - regular let (parallel by default)
  // let: (let [[a b] value]... body) - destructuring let
  // let: (let loop [i 0] [sum 0] body) - named let (Scheme-style, parallel)
  // let: (let ^:seq loop [i 0] [sum 0] body) - named let (sequential)
  // let: (let ^:seq [x 1] [y (+ x 1)] body) - regular let (sequential)
  if (omni_symbol_is(s, sym_start, sym_len, "let")) {
    // Check for ^:seq metadata for sequential evaluation
    int is_sequential = 0;
    omni_skip(s);
    if (parse_peek(s) == '^') {
      parse_advance(s);  // skip ^
      if (parse_peek(s) == ':') {
        parse_advance(s);  // skip :
        u32 meta_start, meta_len;
        if (omni_parse_symbol_raw(s, &meta_start, &meta_len)) {
          if (omni_symbol_is(s, meta_start, meta_len, "seq")) {
            is_sequential = 1;
          }
          // Could add other metadata like ^:parallel (explicit parallel) here
        }
      }
    }

    // Check for named let: (let name [bindings...] body)
    // Named let allows recursive calls to 'name' within body
    u32 loop_name_nick = 0;
    int is_named_let = 0;

    // Peek ahead to see if next token is a symbol (named let) or '[' (regular let)
    omni_skip(s);
    if (parse_peek(s) != '[') {
      // Could be named let - try to parse a symbol
      u32 name_start, name_len;
      if (omni_parse_symbol_raw(s, &name_start, &name_len)) {
        loop_name_nick = omni_symbol_nick(s, name_start, name_len);
        is_named_let = 1;
        // Push the loop name so it's visible in the body (for recursive calls)
        omni_bind_push(loop_name_nick);
      }
    }

    // Collect all bindings
    typedef struct {
      Term pattern;       // Pattern (or nil for simple binding)
      u32 name_nick;      // Name nick (for simple binding)
      Term value;         // Value expression
      int is_destruct;    // 1 if destructuring, 0 if simple
    } LetBinding;
    LetBinding bindings[64];
    u32 binding_count = 0;

    // Parse bindings
    while (parse_peek(s) == '[' && binding_count < 64) {
      omni_expect_char(s, '[');

      // Check if this is a destructuring pattern (starts with [ for array)
      if (parse_peek(s) == '[') {
        // Destructuring pattern: [[a b] value]
        Term pattern = parse_omni_pattern(s);
        bindings[binding_count].pattern = pattern;
        bindings[binding_count].is_destruct = 1;
      } else {
        // Simple binding: [name value] or [name {Type} value]
        u32 bind_start, bind_len;
        if (!omni_parse_symbol_raw(s, &bind_start, &bind_len)) {
          parse_error(s, "binding name or pattern", parse_peek(s));
          break;
        }
        u32 bind_nick = omni_symbol_nick(s, bind_start, bind_len);
        bindings[binding_count].name_nick = bind_nick;
        bindings[binding_count].is_destruct = 0;

        // Optional type
        if (parse_peek(s) == '{') {
          parse_omni_type(s);  // Ignore type for now
        }

        // Push binding for simple case
        omni_bind_push(bind_nick);
      }

      // Parse value
      Term val = parse_omni_expr(s);
      bindings[binding_count].value = val;

      omni_expect_char(s, ']');
      binding_count++;
    }

    // Parse body
    Term body = parse_omni_expr(s);

    omni_expect_char(s, ')');

    // Pop simple bindings (destructuring bindings are popped by pattern parser)
    for (int i = (int)binding_count - 1; i >= 0; i--) {
      if (!bindings[i].is_destruct) {
        omni_bind_pop(1);
      }
    }

    // Pop the loop name if this is a named let
    if (is_named_let) {
      omni_bind_pop(1);
    }

    if (is_named_let) {
      // Named let: (let loop [i 0] [sum 0] body)
      // Default (parallel): #NLet{name_nick, init_values, body}
      // With ^:seq:         #NLetS{name_nick, init_values, body}
      // The body can call (loop new_i new_sum) to recurse

      // Build list of initial values
      Term init_values = omni_nil();
      for (int i = (int)binding_count - 1; i >= 0; i--) {
        init_values = omni_pair(bindings[i].value, init_values);
      }

      // Wrap body in lambdas for each binding (innermost to outermost)
      // This creates the function that 'loop' refers to
      Term loop_body = body;
      for (int i = (int)binding_count - 1; i >= 0; i--) {
        if (bindings[i].is_destruct) {
          loop_body = omni_ctr2(OMNI_NAM_DLAM, bindings[i].pattern, loop_body);
        } else {
          loop_body = omni_lam(loop_body);
        }
      }

      // Use sequential or parallel constructor based on ^:seq metadata
      u32 nlet_tag = is_sequential ? OMNI_NAM_NLETS : OMNI_NAM_NLET;
      return omni_ctr3(nlet_tag, term_new_num(loop_name_nick), init_values, loop_body);
    } else {
      // Regular let: construct let chain from innermost to outermost
      // Note: Regular let also respects ^:seq for sequential binding evaluation
      // but current implementation is already sequential (let* semantics)
      Term result = body;
      for (int i = (int)binding_count - 1; i >= 0; i--) {
        if (bindings[i].is_destruct) {
          // Destructuring let: #DLet{pattern, value, body}
          result = omni_ctr3(OMNI_NAM_DLET, bindings[i].pattern, bindings[i].value, result);
        } else {
          // Simple let: #Let{value, body}
          result = omni_let(bindings[i].value, result);
        }
      }

      return result;
    }
  }

  // lambda/fn/λ: (fn [x] [y] body) or (fn [[a b]] body) for destructuring
  // λ is U+03BB, encoded as 0xCE 0xBB in UTF-8
  int is_lambda = omni_symbol_is(s, sym_start, sym_len, "lambda") ||
                  omni_symbol_is(s, sym_start, sym_len, "fn") ||
                  (sym_len == 2 &&
                   (u8)s->src[sym_start] == 0xCE &&
                   (u8)s->src[sym_start + 1] == 0xBB);
  if (is_lambda) {

    OmniSlot slots[64];
    u32 slot_count = 0;
    while (parse_peek(s) == '[' && slot_count < 64) {
      if (!parse_omni_slot(s, &slots[slot_count])) break;
      slot_count++;
    }

    // Optional return type
    if (parse_peek(s) == '{') {
      parse_omni_type(s);  // Ignore for now
    }

    // Push bindings for simple slots (destructuring bindings are already pushed by pattern parser)
    for (u32 i = 0; i < slot_count; i++) {
      if (!slots[i].is_destruct) {
        omni_bind_push(slots[i].name_nick);
      }
    }

    // Parse body
    Term body = parse_omni_expr(s);

    omni_expect_char(s, ')');

    // Pop simple bindings (destructuring bindings are popped by pattern parser)
    for (u32 i = 0; i < slot_count; i++) {
      if (!slots[i].is_destruct) {
        omni_bind_pop(1);
      }
    }

    // Wrap in lambdas (from innermost to outermost)
    for (int i = (int)slot_count - 1; i >= 0; i--) {
      if (slots[i].is_destruct) {
        // Destructuring lambda: #DLam{pattern, body}
        // This will match the argument against the pattern and bind variables
        body = omni_ctr2(OMNI_NAM_DLAM, slots[i].pattern, body);
      } else {
        // Simple lambda: #Lam{body}
        body = omni_lam(body);
      }
    }

    return body;
  }

  // if: (if cond then else) - SYNTACTIC SUGAR for match
  // Desugars to: (match cond [true] then [_] else)
  // This makes 'match' the single source of truth for conditional logic
  if (omni_symbol_is(s, sym_start, sym_len, "if")) {
    Term cond = parse_omni_expr(s);
    Term then_br = parse_omni_expr(s);
    Term else_br = omni_nothing();
    if (parse_peek(s) != ')') {
      else_br = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');

    // Desugar to match: (match cond [true] then [_] else)
    Term true_pat = omni_pat_lit(omni_true());   // #PLit{#True{}}
    Term true_case = omni_case(true_pat, omni_nil(), then_br);

    Term wild_pat = omni_pat_wildcard();         // #PWld{}
    Term else_case = omni_case(wild_pat, omni_nil(), else_br);

    Term cases = omni_cons(true_case, omni_cons(else_case, omni_nil()));
    return omni_match(cond, cases);
  }

  // match: (match expr [pattern body]...)
  if (omni_symbol_is(s, sym_start, sym_len, "match")) {
    Term scrutinee = parse_omni_expr(s);

    Term cases = omni_nil();
    Term *tail = &cases;

    while (parse_peek(s) == '[') {
      Term clause = parse_omni_match_clause(s);
      Term cell = omni_cons(clause, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }

    omni_expect_char(s, ')');
    return omni_match(scrutinee, cases);
  }

  // handle: (handle body [effect-name [args] handler-body]...)
  if (omni_symbol_is(s, sym_start, sym_len, "handle")) {
    Term body = parse_omni_expr(s);

    Term handlers = omni_nil();
    Term *tail = &handlers;

    while (parse_peek(s) == '[') {
      omni_expect_char(s, '[');

      // Effect name
      u32 eff_start, eff_len;
      omni_parse_symbol_raw(s, &eff_start, &eff_len);
      u32 eff_nick = omni_symbol_nick(s, eff_start, eff_len);

      // Handler args
      omni_expect_char(s, '[');
      u32 handler_binds = 0;
      while (parse_peek(s) != ']') {
        u32 arg_start, arg_len;
        if (omni_parse_symbol_raw(s, &arg_start, &arg_len)) {
          u32 arg_nick = omni_symbol_nick(s, arg_start, arg_len);
          omni_bind_push(arg_nick);
          handler_binds++;
        }
      }
      omni_expect_char(s, ']');

      // Handler body
      Term handler_body = parse_omni_expr(s);

      omni_expect_char(s, ']');

      omni_bind_pop(handler_binds);

      // Wrap in lambdas
      for (u32 i = 0; i < handler_binds; i++) {
        handler_body = omni_lam(handler_body);
      }

      Term handler = omni_ctr2(OMNI_NAM_HDEF, term_new_num(eff_nick), handler_body);
      Term cell = omni_cons(handler, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }

    omni_expect_char(s, ')');
    return omni_handle(handlers, body);
  }

  // reset: (reset body) - delimited continuation boundary
  if (omni_symbol_is(s, sym_start, sym_len, "reset")) {
    Term body = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_reset(body);
  }

  // control: (control k body) - capture continuation
  if (omni_symbol_is(s, sym_start, sym_len, "control")) {
    // Parse continuation variable name
    u32 k_start, k_len;
    if (!omni_parse_symbol_raw(s, &k_start, &k_len)) {
      s->error = "control requires continuation variable name";
      return omni_nothing();
    }
    u32 k_nick = omni_symbol_nick(s, k_start, k_len);
    omni_bind_push(k_nick);

    // Parse body
    Term body = parse_omni_expr(s);
    omni_expect_char(s, ')');

    omni_bind_pop(1);

    // k is bound at index 0 (de Bruijn)
    return omni_control(0, body);
  }

  // shift: (shift k body) - alias for control (Scheme-style)
  if (omni_symbol_is(s, sym_start, sym_len, "shift")) {
    u32 k_start, k_len;
    if (!omni_parse_symbol_raw(s, &k_start, &k_len)) {
      s->error = "shift requires continuation variable name";
      return omni_nothing();
    }
    u32 k_nick = omni_symbol_nick(s, k_start, k_len);
    omni_bind_push(k_nick);

    Term body = parse_omni_expr(s);
    omni_expect_char(s, ')');

    omni_bind_pop(1);
    return omni_control(0, body);
  }

  // yield: (yield val) - yield from fiber
  if (omni_symbol_is(s, sym_start, sym_len, "yield")) {
    Term val = omni_nothing();
    if (parse_peek(s) != ')') {
      val = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_yield(val);
  }

  // NOTE: loop/recur removed - use recursion or effects for iteration

  // |> pipe operator: (|> value fn1 fn2 ...) - thread value through functions
  // Desugars at parse time: (|> 5 inc square) → (square (inc 5))
  if (omni_symbol_is(s, sym_start, sym_len, "|>")) {
    // First arg is the initial value
    Term result = parse_omni_expr(s);

    // Chain functions left-to-right
    while (parse_peek(s) != ')' && !parse_at_end(s)) {
      Term fn = parse_omni_expr(s);
      result = omni_app(fn, result);
    }

    omni_expect_char(s, ')');
    return result;
  }

  // apply: (apply fn args-list) - apply function to list of arguments
  if (omni_symbol_is(s, sym_start, sym_len, "apply")) {
    Term fn = parse_omni_expr(s);
    Term args_list = parse_omni_expr(s);
    omni_expect_char(s, ')');

    // Create apply: #Appl{fn, args}
    return omni_ctr2(OMNI_NAM_APPL, fn, args_list);
  }

  // curry: (curry fn) or (curry fn arity) - convert multi-arg function to curried form
  // ((curry f) a b c) where f takes 3 args → f(a)(b)(c)
  // (curry f 2) - explicitly curry the 2-arg version for multi-arity functions
  if (omni_symbol_is(s, sym_start, sym_len, "curry")) {
    Term fn = parse_omni_expr(s);
    Term arity = omni_nil();  // nil means infer/max arity
    if (parse_peek(s) != ')') {
      arity = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_CURY, fn, arity);
  }

  // flip: (flip fn) - swap first two arguments
  // ((flip f) a b) → (f b a)
  if (omni_symbol_is(s, sym_start, sym_len, "flip")) {
    Term fn = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_FLIP, fn);
  }

  // rotate: (rotate fn) - cycle arguments left (first arg moves to end)
  // ((rotate f) a b c) → (f b c a)
  if (omni_symbol_is(s, sym_start, sym_len, "rotate")) {
    Term fn = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_ROTR, fn);
  }

  // comp: (comp fn1 fn2 ...) - compose functions (right to left)
  if (omni_symbol_is(s, sym_start, sym_len, "comp")) {
    Term fns = omni_nil();
    Term *tail = &fns;

    while (parse_peek(s) != ')') {
      Term fn = parse_omni_expr(s);
      Term cell = omni_cons(fn, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }

    omni_expect_char(s, ')');

    // Create compose: #Comp{fns}
    return omni_ctr1(OMNI_NAM_COMP, fns);
  }

  // ffi: (ffi "lib" "func" args...)
  if (omni_symbol_is(s, sym_start, sym_len, "ffi")) {
    Term lib_name = parse_omni_expr(s);
    Term func_name = parse_omni_expr(s);

    Term args = omni_nil();
    Term *tail = &args;
    while (parse_peek(s) != ')') {
      Term arg = parse_omni_expr(s);
      Term cell = omni_cons(arg, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }
    omni_expect_char(s, ')');

    // FFI name is library + function combined
    return omni_ffi(func_name, args);
  }

  // Arithmetic operators
  if (omni_symbol_is(s, sym_start, sym_len, "+")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_add(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "-")) {
    Term a = parse_omni_expr(s);
    if (parse_peek(s) == ')') {
      // Unary minus
      omni_expect_char(s, ')');
      return omni_sub(omni_lit(0), a);
    }
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_sub(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "*")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_mul(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "/")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_div(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "mod")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_mod(a, b);
  }

  // Comparison operators
  if (omni_symbol_is(s, sym_start, sym_len, "=")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_eql(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "<")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_lt(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, ">")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_gt(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "<=")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_le(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, ">=")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ge(a, b);
  }

  // Boolean operators
  if (omni_symbol_is(s, sym_start, sym_len, "and")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_and(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "or")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_or(a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "not")) {
    Term a = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_not(a);
  }

  // List operations
  if (omni_symbol_is(s, sym_start, sym_len, "list")) {
    Term items = omni_nil();
    Term *tail = &items;
    while (parse_peek(s) != ')') {
      Term item = parse_omni_expr(s);
      Term cell = omni_cons(item, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }
    omni_expect_char(s, ')');
    return items;
  }
  if (omni_symbol_is(s, sym_start, sym_len, "cons")) {
    Term h = parse_omni_expr(s);
    Term t = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_cons(h, t);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "first") ||
      omni_symbol_is(s, sym_start, sym_len, "car")) {
    Term lst = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_FST, lst);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "rest") ||
      omni_symbol_is(s, sym_start, sym_len, "cdr")) {
    Term lst = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_SND, lst);
  }

  // begin/do: (begin e1 e2 ...) - sequencing
  if (omni_symbol_is(s, sym_start, sym_len, "begin") ||
      omni_symbol_is(s, sym_start, sym_len, "do")) {
    Term result = omni_nothing();
    while (parse_peek(s) != ')') {
      Term expr = parse_omni_expr(s);
      if (term_ext(result) == OMNI_NAM_NOTH) {
        result = expr;
      } else {
        result = omni_ctr2(OMNI_NAM_DO, result, expr);
      }
    }
    omni_expect_char(s, ')');
    return result;
  }

  // when: (when cond body...) - SYNTACTIC SUGAR for match
  // Desugars to: (match cond [true] body [_] nothing)
  if (omni_symbol_is(s, sym_start, sym_len, "when")) {
    Term cond_expr = parse_omni_expr(s);

    // Parse body expressions as a sequence
    Term body = omni_nothing();
    while (parse_peek(s) != ')') {
      Term expr = parse_omni_expr(s);
      if (term_ext(body) == OMNI_NAM_NOTH) {
        body = expr;
      } else {
        body = omni_ctr2(OMNI_NAM_DO, body, expr);
      }
    }
    omni_expect_char(s, ')');

    // Desugar to match: (match cond [true] body [_] nothing)
    Term true_pat = omni_pat_lit(omni_true());
    Term true_case = omni_case(true_pat, omni_nil(), body);
    Term wild_pat = omni_pat_wildcard();
    Term else_case = omni_case(wild_pat, omni_nil(), omni_nothing());
    Term cases = omni_cons(true_case, omni_cons(else_case, omni_nil()));
    return omni_match(cond_expr, cases);
  }

  // unless: (unless cond body...) - SYNTACTIC SUGAR for match
  // Desugars to: (match cond [true] nothing [_] body)
  if (omni_symbol_is(s, sym_start, sym_len, "unless")) {
    Term cond_expr = parse_omni_expr(s);

    // Parse body expressions as a sequence
    Term body = omni_nothing();
    while (parse_peek(s) != ')') {
      Term expr = parse_omni_expr(s);
      if (term_ext(body) == OMNI_NAM_NOTH) {
        body = expr;
      } else {
        body = omni_ctr2(OMNI_NAM_DO, body, expr);
      }
    }
    omni_expect_char(s, ')');

    // Desugar to match: (match cond [true] nothing [_] body)
    Term true_pat = omni_pat_lit(omni_true());
    Term true_case = omni_case(true_pat, omni_nil(), omni_nothing());
    Term wild_pat = omni_pat_wildcard();
    Term else_case = omni_case(wild_pat, omni_nil(), body);
    Term cases = omni_cons(true_case, omni_cons(else_case, omni_nil()));
    return omni_match(cond_expr, cases);
  }

  // NOTE: cond/case removed - use match or nested if

  // get: (get coll key) or (get coll key default)
  if (omni_symbol_is(s, sym_start, sym_len, "get")) {
    Term coll = parse_omni_expr(s);
    Term key = parse_omni_expr(s);
    Term default_val = omni_nothing();
    if (parse_peek(s) != ')') {
      default_val = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_GET, coll, key, default_val);
  }

  // put: (put coll key val) - functional update
  if (omni_symbol_is(s, sym_start, sym_len, "put") ||
      omni_symbol_is(s, sym_start, sym_len, "assoc")) {
    Term coll = parse_omni_expr(s);
    Term key = parse_omni_expr(s);
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_PUT, coll, key, val);
  }

  // update: (update coll key fn) - apply fn to value at key
  if (omni_symbol_is(s, sym_start, sym_len, "update")) {
    Term coll = parse_omni_expr(s);
    Term key = parse_omni_expr(s);
    Term fn = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_UPDT, coll, key, fn);
  }

  // get-in: (get-in coll [k1 k2 ...]) or (get-in coll [k1 k2 ...] default)
  if (omni_symbol_is(s, sym_start, sym_len, "get-in")) {
    Term coll = parse_omni_expr(s);
    Term path = parse_omni_expr(s);  // Should be an array/list of keys
    Term default_val = omni_nothing();
    if (parse_peek(s) != ')') {
      default_val = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_GTIN, coll, path, default_val);
  }

  // assoc-in: (assoc-in coll [k1 k2 ...] val)
  if (omni_symbol_is(s, sym_start, sym_len, "assoc-in")) {
    Term coll = parse_omni_expr(s);
    Term path = parse_omni_expr(s);
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_ASIN, coll, path, val);
  }

  // update-in: (update-in coll [k1 k2 ...] fn)
  if (omni_symbol_is(s, sym_start, sym_len, "update-in")) {
    Term coll = parse_omni_expr(s);
    Term path = parse_omni_expr(s);
    Term fn = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_UPIN, coll, path, fn);
  }

  // ==========================================================================
  // Mutation primitives (side-effecting, use with ^:mutable bindings)
  // ==========================================================================

  // set!: (set! var val) - mutate a variable binding
  if (omni_symbol_is(s, sym_start, sym_len, "set!")) {
    // Parse variable name
    u32 var_start, var_len;
    if (!omni_parse_symbol_raw(s, &var_start, &var_len)) {
      parse_error(s, "variable name for set!", parse_peek(s));
      return omni_nothing();
    }
    u32 var_nick = omni_symbol_nick(s, var_start, var_len);
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_SETB, term_new_num(var_nick), val);
  }

  // put!: (put! coll key val) - mutate collection in place
  if (omni_symbol_is(s, sym_start, sym_len, "put!")) {
    Term coll = parse_omni_expr(s);
    Term key = parse_omni_expr(s);
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_PUTB, coll, key, val);
  }

  // update!: (update! coll key fn) - mutate collection via function
  if (omni_symbol_is(s, sym_start, sym_len, "update!")) {
    Term coll = parse_omni_expr(s);
    Term key = parse_omni_expr(s);
    Term fn = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_UPDTB, coll, key, fn);
  }

  // dissoc: (dissoc coll key) - remove key from collection
  if (omni_symbol_is(s, sym_start, sym_len, "dissoc")) {
    Term coll = parse_omni_expr(s);
    Term key = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Use PUT with nothing as value to represent removal
    return omni_ctr3(OMNI_NAM_PUT, coll, key, omni_ctr1(OMNI_NAM_NOTH, omni_lit(1)));
  }

  // range: (range end) or (range start end) or (range start end step)
  if (omni_symbol_is(s, sym_start, sym_len, "range")) {
    Term arg1 = parse_omni_expr(s);
    Term start, end, step;
    if (parse_peek(s) == ')') {
      // (range end) - start=0, step=1
      start = omni_lit(0);
      end = arg1;
      step = omni_lit(1);
    } else {
      Term arg2 = parse_omni_expr(s);
      if (parse_peek(s) == ')') {
        // (range start end) - step=1
        start = arg1;
        end = arg2;
        step = omni_lit(1);
      } else {
        // (range start end step)
        start = arg1;
        end = arg2;
        step = parse_omni_expr(s);
      }
    }
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_RANG, start, end, step);
  }

  // iter-map: (iter-map fn iter) - lazy map over iterator
  if (omni_symbol_is(s, sym_start, sym_len, "iter-map")) {
    Term fn = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IMAP, iter, fn);
  }

  // iter-filter: (iter-filter pred iter) - lazy filter over iterator
  if (omni_symbol_is(s, sym_start, sym_len, "iter-filter")) {
    Term pred = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IFLT, iter, pred);
  }

  // take: (take n iter) - take first n elements
  if (omni_symbol_is(s, sym_start, sym_len, "take")) {
    Term n = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_ITKN, iter, n);
  }

  // drop: (drop n iter) - drop first n elements
  if (omni_symbol_is(s, sym_start, sym_len, "drop")) {
    Term n = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IDRP, iter, n);
  }

  // collect-list: (collect-list iter) - realize iterator into list
  if (omni_symbol_is(s, sym_start, sym_len, "collect-list") ||
      omni_symbol_is(s, sym_start, sym_len, "into-list")) {
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_APPL, omni_sym(omni_nick("coll")), iter);
  }

  // collect-array: (collect-array iter) - realize iterator into array
  if (omni_symbol_is(s, sym_start, sym_len, "collect-array") ||
      omni_symbol_is(s, sym_start, sym_len, "into-array")) {
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_APPL, omni_sym(omni_nick("cola")), iter);
  }

  // iterate: (iterate fn init) - infinite lazy sequence
  if (omni_symbol_is(s, sym_start, sym_len, "iterate")) {
    Term fn = parse_omni_expr(s);
    Term init = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_ITER, init, fn);
  }

  // repeat: (repeat val) or (repeat n val) - repeated value
  if (omni_symbol_is(s, sym_start, sym_len, "repeat")) {
    Term arg1 = parse_omni_expr(s);
    if (parse_peek(s) == ')') {
      // Infinite repeat
      omni_expect_char(s, ')');
      return omni_ctr2(OMNI_NAM_ITER, arg1, omni_ctr1(OMNI_NAM_SLOT, omni_lit(0)));
    } else {
      // Repeat n times
      Term val = parse_omni_expr(s);
      omni_expect_char(s, ')');
      return omni_ctr2(OMNI_NAM_ITKN, omni_ctr2(OMNI_NAM_ITER, val, omni_ctr1(OMNI_NAM_SLOT, omni_lit(0))), arg1);
    }
  }

  // cycle: (cycle coll) - infinite cycle through collection
  if (omni_symbol_is(s, sym_start, sym_len, "cycle")) {
    Term coll = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_ITER, coll);
  }

  // iter-zip: (iter-zip iter1 iter2 ...) - zip multiple iterators
  if (omni_symbol_is(s, sym_start, sym_len, "iter-zip") ||
      omni_symbol_is(s, sym_start, sym_len, "zip")) {
    Term iters = omni_nil();
    Term *tail = &iters;
    while (parse_peek(s) != ')') {
      Term iter = parse_omni_expr(s);
      Term cell = omni_cons(iter, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_IZIP, iters);
  }

  // iter-chain: (iter-chain iter1 iter2 ...) - concatenate iterators
  if (omni_symbol_is(s, sym_start, sym_len, "iter-chain") ||
      omni_symbol_is(s, sym_start, sym_len, "chain")) {
    Term iters = omni_nil();
    Term *tail = &iters;
    while (parse_peek(s) != ')') {
      Term iter = parse_omni_expr(s);
      Term cell = omni_cons(iter, omni_nil());
      *tail = cell;
      tail = &HEAP[term_val(cell) + 1];
    }
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_ICHN, iters);
  }

  // iter-enumerate: (iter-enumerate iter) - add indices (0, val), (1, val), ...
  if (omni_symbol_is(s, sym_start, sym_len, "iter-enumerate") ||
      omni_symbol_is(s, sym_start, sym_len, "enumerate")) {
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_IENM, iter);
  }

  // iter-take-while: (iter-take-while pred iter) - take while predicate is true
  if (omni_symbol_is(s, sym_start, sym_len, "iter-take-while") ||
      omni_symbol_is(s, sym_start, sym_len, "take-while")) {
    Term pred = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_ITKW, iter, pred);
  }

  // iter-drop-while: (iter-drop-while pred iter) - drop while predicate is true
  if (omni_symbol_is(s, sym_start, sym_len, "iter-drop-while") ||
      omni_symbol_is(s, sym_start, sym_len, "drop-while")) {
    Term pred = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IDRW, iter, pred);
  }

  // iter-fold: (iter-fold fn init iter) - fold/reduce iterator
  if (omni_symbol_is(s, sym_start, sym_len, "iter-fold") ||
      omni_symbol_is(s, sym_start, sym_len, "fold") ||
      omni_symbol_is(s, sym_start, sym_len, "reduce")) {
    Term fn = parse_omni_expr(s);
    Term init = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_IFLD, iter, init, fn);
  }

  // iter-find: (iter-find pred iter) - find first matching element
  if (omni_symbol_is(s, sym_start, sym_len, "iter-find") ||
      omni_symbol_is(s, sym_start, sym_len, "find")) {
    Term pred = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IFND, iter, pred);
  }

  // iter-any?: (iter-any? pred iter) - true if any element matches
  if (omni_symbol_is(s, sym_start, sym_len, "iter-any?") ||
      omni_symbol_is(s, sym_start, sym_len, "any?")) {
    Term pred = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IANY, iter, pred);
  }

  // iter-all?: (iter-all? pred iter) - true if all elements match
  if (omni_symbol_is(s, sym_start, sym_len, "iter-all?") ||
      omni_symbol_is(s, sym_start, sym_len, "all?")) {
    Term pred = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IALL, iter, pred);
  }

  // iter-nth: (iter-nth n iter) - get nth element
  if (omni_symbol_is(s, sym_start, sym_len, "iter-nth") ||
      omni_symbol_is(s, sym_start, sym_len, "nth")) {
    Term n = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_INTH, iter, n);
  }

  // iter-flat-map: (iter-flat-map fn iter) - map then flatten
  if (omni_symbol_is(s, sym_start, sym_len, "iter-flat-map") ||
      omni_symbol_is(s, sym_start, sym_len, "flat-map")) {
    Term fn = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IFMP, iter, fn);
  }

  // iter-step-by: (iter-step-by n iter) - take every nth element
  if (omni_symbol_is(s, sym_start, sym_len, "iter-step-by") ||
      omni_symbol_is(s, sym_start, sym_len, "step-by")) {
    Term n = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_ISTP, iter, n);
  }

  // iter-chunks: (iter-chunks n iter) - group into chunks of n
  if (omni_symbol_is(s, sym_start, sym_len, "iter-chunks") ||
      omni_symbol_is(s, sym_start, sym_len, "chunks")) {
    Term n = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_ICHK, iter, n);
  }

  // iter-windows: (iter-windows n iter) - sliding windows of size n
  if (omni_symbol_is(s, sym_start, sym_len, "iter-windows") ||
      omni_symbol_is(s, sym_start, sym_len, "windows")) {
    Term n = parse_omni_expr(s);
    Term iter = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_IWIN, iter, n);
  }

  // Math functions
  if (omni_symbol_is(s, sym_start, sym_len, "sqrt")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_SQRT, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "pow")) {
    Term base = parse_omni_expr(s);
    Term exp = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_POW, base, exp);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "exp")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MEXP, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "log")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MLOG, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "sin")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MSIN, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "cos")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MCOS, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "tan")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MTAN, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "asin")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MASN, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "acos")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MACS, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "atan")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MATN, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "atan2")) {
    Term y = parse_omni_expr(s);
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_MATN, y, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "abs")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MABS, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "floor")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_FLOR, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "ceil")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MCEI, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "round")) {
    Term x = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_ROND, x);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "random")) {
    omni_expect_char(s, ')');
    return omni_ctr0(OMNI_NAM_RAND);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "min")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // min(a, b) = if a < b then a else b
    return omni_if(omni_lt(a, b), a, b);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "max")) {
    Term a = parse_omni_expr(s);
    Term b = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // max(a, b) = if a > b then a else b
    return omni_if(omni_gt(a, b), a, b);
  }

  // I/O operations
  if (omni_symbol_is(s, sym_start, sym_len, "read-file") ||
      omni_symbol_is(s, sym_start, sym_len, "slurp")) {
    Term path = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_RDFL, path);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "write-file") ||
      omni_symbol_is(s, sym_start, sym_len, "spit")) {
    Term path = parse_omni_expr(s);
    Term content = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_WRFL, path, content);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "append-file")) {
    Term path = parse_omni_expr(s);
    Term content = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_APFL, path, content);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "read-lines")) {
    Term path = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_RDLN, path);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "print")) {
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_PRNT, val);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "println")) {
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_PRNL, val);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "read-line")) {
    omni_expect_char(s, ')');
    return omni_ctr0(OMNI_NAM_RDLN2);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "getenv") ||
      omni_symbol_is(s, sym_start, sym_len, "env")) {
    Term name = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_GTEV, name);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "setenv")) {
    Term name = parse_omni_expr(s);
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_STEV, name, val);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "file-exists?") ||
      omni_symbol_is(s, sym_start, sym_len, "exists?")) {
    Term path = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_EXST, path);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "dir?") ||
      omni_symbol_is(s, sym_start, sym_len, "directory?")) {
    Term path = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_ISDR, path);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "mkdir") ||
      omni_symbol_is(s, sym_start, sym_len, "make-dir")) {
    Term path = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_MKDR, path);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "list-dir") ||
      omni_symbol_is(s, sym_start, sym_len, "ls")) {
    Term path = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_LSDR, path);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "delete-file") ||
      omni_symbol_is(s, sym_start, sym_len, "rm")) {
    Term path = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DLFL, path);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "rename-file") ||
      omni_symbol_is(s, sym_start, sym_len, "mv")) {
    Term from = parse_omni_expr(s);
    Term to = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_RNFL, from, to);
  }
  if (omni_symbol_is(s, sym_start, sym_len, "copy-file") ||
      omni_symbol_is(s, sym_start, sym_len, "cp")) {
    Term from = parse_omni_expr(s);
    Term to = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_CPFL, from, to);
  }

  // ==========================================================================
  // JSON Operations
  // ==========================================================================

  // json-parse: (json-parse str)
  if (omni_symbol_is(s, sym_start, sym_len, "json-parse")) {
    Term str = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_JPRS, str);
  }

  // json-stringify / json-encode: (json-stringify val)
  if (omni_symbol_is(s, sym_start, sym_len, "json-stringify") ||
      omni_symbol_is(s, sym_start, sym_len, "json-encode") ||
      omni_symbol_is(s, sym_start, sym_len, "to-json")) {
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_JSTR, val);
  }

  // json-get: (json-get json key) - shorthand for (get (json-parse str) key)
  if (omni_symbol_is(s, sym_start, sym_len, "json-get")) {
    Term json = parse_omni_expr(s);
    Term key = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Equivalent to (get json key)
    return omni_ctr2(OMNI_NAM_GET, json, key);
  }

  // json-get-in: (json-get-in json path) - deep access
  if (omni_symbol_is(s, sym_start, sym_len, "json-get-in")) {
    Term json = parse_omni_expr(s);
    Term path = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Equivalent to (get-in json path)
    return omni_ctr2(OMNI_NAM_GTIN, json, path);
  }

  // ==========================================================================
  // Regex Operations (Pika-based pattern matching)
  // ==========================================================================

  // re-match: (re-match pattern str) - match entire string against pattern
  // Pattern is compiled to Pika grammar for matching
  if (omni_symbol_is(s, sym_start, sym_len, "re-match")) {
    Term pattern = parse_omni_expr(s);
    Term str = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_RMAT, pattern, str);
  }

  // re-find: (re-find pattern str) - find first match in string
  if (omni_symbol_is(s, sym_start, sym_len, "re-find")) {
    Term pattern = parse_omni_expr(s);
    Term str = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_RFND, pattern, str);
  }

  // re-find-all: (re-find-all pattern str) - find all matches
  if (omni_symbol_is(s, sym_start, sym_len, "re-find-all")) {
    Term pattern = parse_omni_expr(s);
    Term str = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_RFNA, pattern, str);
  }

  // re-replace: (re-replace pattern str replacement) - replace matches
  if (omni_symbol_is(s, sym_start, sym_len, "re-replace")) {
    Term pattern = parse_omni_expr(s);
    Term str = parse_omni_expr(s);
    Term replacement = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_RRPL, pattern, str, replacement);
  }

  // re-split: (re-split pattern str) - split string by pattern
  if (omni_symbol_is(s, sym_start, sym_len, "re-split")) {
    Term pattern = parse_omni_expr(s);
    Term str = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_RSPL, pattern, str);
  }

  // re-groups: (re-groups match) - get capture groups from match result
  if (omni_symbol_is(s, sym_start, sym_len, "re-groups")) {
    Term match = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_RGRP, match);
  }

  // ==========================================================================
  // DateTime Operations
  // ==========================================================================

  // datetime-now / now: (datetime-now)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-now") ||
      omni_symbol_is(s, sym_start, sym_len, "now")) {
    omni_expect_char(s, ')');
    return omni_ctr0(OMNI_NAM_DTNW);
  }

  // datetime-parse: (datetime-parse str) or (datetime-parse str format)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-parse") ||
      omni_symbol_is(s, sym_start, sym_len, "parse-datetime")) {
    Term str = parse_omni_expr(s);
    Term fmt = omni_nothing();
    if (parse_peek(s) != ')') {
      fmt = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_DTPR, str, fmt);
  }

  // datetime-format: (datetime-format dt format)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-format") ||
      omni_symbol_is(s, sym_start, sym_len, "format-datetime")) {
    Term dt = parse_omni_expr(s);
    Term fmt = omni_nothing();
    if (parse_peek(s) != ')') {
      fmt = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_DTFM, dt, fmt);
  }

  // datetime-add: (datetime-add dt duration)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-add") ||
      omni_symbol_is(s, sym_start, sym_len, "dt+")) {
    Term dt = parse_omni_expr(s);
    Term dur = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_DTAD, dt, dur);
  }

  // datetime-sub: (datetime-sub dt duration)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-sub") ||
      omni_symbol_is(s, sym_start, sym_len, "dt-")) {
    Term dt = parse_omni_expr(s);
    Term dur = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_DTSB, dt, dur);
  }

  // datetime-diff: (datetime-diff dt1 dt2)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-diff") ||
      omni_symbol_is(s, sym_start, sym_len, "dt-diff")) {
    Term dt1 = parse_omni_expr(s);
    Term dt2 = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_DTDF, dt1, dt2);
  }

  // datetime-year: (datetime-year dt)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-year") ||
      omni_symbol_is(s, sym_start, sym_len, "year")) {
    Term dt = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DTYR, dt);
  }

  // datetime-month: (datetime-month dt)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-month") ||
      omni_symbol_is(s, sym_start, sym_len, "month")) {
    Term dt = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DTMO, dt);
  }

  // datetime-day: (datetime-day dt)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-day") ||
      omni_symbol_is(s, sym_start, sym_len, "day")) {
    Term dt = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DTDY, dt);
  }

  // datetime-hour: (datetime-hour dt)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-hour") ||
      omni_symbol_is(s, sym_start, sym_len, "hour")) {
    Term dt = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DTHR, dt);
  }

  // datetime-minute: (datetime-minute dt)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-minute") ||
      omni_symbol_is(s, sym_start, sym_len, "minute")) {
    Term dt = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DTMI, dt);
  }

  // datetime-second: (datetime-second dt)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-second") ||
      omni_symbol_is(s, sym_start, sym_len, "second")) {
    Term dt = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DTSC, dt);
  }

  // datetime-timestamp / timestamp: (datetime-timestamp dt)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-timestamp") ||
      omni_symbol_is(s, sym_start, sym_len, "timestamp")) {
    Term dt = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DTTS, dt);
  }

  // datetime-from-timestamp / from-timestamp: (datetime-from-timestamp ts)
  if (omni_symbol_is(s, sym_start, sym_len, "datetime-from-timestamp") ||
      omni_symbol_is(s, sym_start, sym_len, "from-timestamp")) {
    Term ts = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DTFT, ts);
  }

  // duration: (duration secs) or (duration secs nsecs)
  if (omni_symbol_is(s, sym_start, sym_len, "duration")) {
    Term secs = parse_omni_expr(s);
    Term nsecs = omni_int(0);
    if (parse_peek(s) != ')') {
      nsecs = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_DUR, secs, nsecs);
  }

  // days: (days n) - shorthand for (duration (* n 86400))
  if (omni_symbol_is(s, sym_start, sym_len, "days")) {
    Term n = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Create duration with n * 86400 seconds
    Term secs_per_day = omni_int(86400);
    Term mul = omni_ctr2(OMNI_NAM_MUL, n, secs_per_day);
    return omni_ctr2(OMNI_NAM_DUR, mul, omni_int(0));
  }

  // hours: (hours n) - shorthand for (duration (* n 3600))
  if (omni_symbol_is(s, sym_start, sym_len, "hours")) {
    Term n = parse_omni_expr(s);
    omni_expect_char(s, ')');
    Term secs_per_hour = omni_int(3600);
    Term mul = omni_ctr2(OMNI_NAM_MUL, n, secs_per_hour);
    return omni_ctr2(OMNI_NAM_DUR, mul, omni_int(0));
  }

  // minutes: (minutes n) - shorthand for (duration (* n 60))
  if (omni_symbol_is(s, sym_start, sym_len, "minutes")) {
    Term n = parse_omni_expr(s);
    omni_expect_char(s, ')');
    Term secs_per_min = omni_int(60);
    Term mul = omni_ctr2(OMNI_NAM_MUL, n, secs_per_min);
    return omni_ctr2(OMNI_NAM_DUR, mul, omni_int(0));
  }

  // seconds: (seconds n) - shorthand for (duration n)
  if (omni_symbol_is(s, sym_start, sym_len, "seconds")) {
    Term n = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_DUR, n, omni_int(0));
  }

  // ==========================================================================
  // Tower/Meta-programming Operations
  // ==========================================================================

  // lift: (lift expr) - move expression to meta-level
  if (omni_symbol_is(s, sym_start, sym_len, "lift")) {
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_LIFT, expr);
  }

  // run: (run expr) - execute meta-level code and get result
  if (omni_symbol_is(s, sym_start, sym_len, "run")) {
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_RUN, expr);
  }

  // EM (eval-meta): (EM expr) - jump to parent meta-level
  if (omni_symbol_is(s, sym_start, sym_len, "EM") ||
      omni_symbol_is(s, sym_start, sym_len, "eval-meta")) {
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_EM, expr);
  }

  // clambda: (clambda [params...] body) - compiled/staged lambda
  if (omni_symbol_is(s, sym_start, sym_len, "clambda") ||
      omni_symbol_is(s, sym_start, sym_len, "staged-fn")) {
    // Parse parameter list
    Term params = omni_nil();
    if (parse_peek(s) == '[') {
      parse_advance(s);  // consume '['
      while (parse_peek(s) != ']' && !parse_at_end(s)) {
        parse_skip_whitespace(s);
        if (parse_peek(s) == ']') break;
        u32 p_start = s->pos;
        u32 p_len = parse_symbol_length(s);
        if (p_len > 0) {
          u32 p_nick = omni_symbol_nick(s, p_start, p_len);
          s->pos += p_len;
          params = omni_ctr2(OMNI_NAM_CON, omni_sym(p_nick), params);
        }
        parse_skip_whitespace(s);
      }
      omni_expect_char(s, ']');
      params = omni_reverse_list(params);
    }
    Term body = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Create compiled lambda: #CLam{params, body}
    return omni_ctr2(OMNI_NAM_CLAM, params, body);
  }

  // stage: (stage level expr) - stage expression at specific level
  if (omni_symbol_is(s, sym_start, sym_len, "stage")) {
    Term level = parse_omni_expr(s);
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_STAG, level, expr);
  }

  // splice: (splice expr) - splice code into current stage
  if (omni_symbol_is(s, sym_start, sym_len, "splice") ||
      omni_symbol_is(s, sym_start, sym_len, "~")) {
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_SPLI, expr);
  }

  // reflect: (reflect val) - turn value into code representation
  if (omni_symbol_is(s, sym_start, sym_len, "reflect")) {
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_REFL, val);
  }

  // reify: (reify code) - turn code into value (execute)
  if (omni_symbol_is(s, sym_start, sym_len, "reify")) {
    Term code = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_REIF, code);
  }

  // meta-level: (meta-level) - get current meta-level number
  if (omni_symbol_is(s, sym_start, sym_len, "meta-level")) {
    omni_expect_char(s, ')');
    return omni_ctr0(OMNI_NAM_MLVL);
  }

  // with-meta-env: (with-meta-env env expr) - evaluate with specific meta-environment
  if (omni_symbol_is(s, sym_start, sym_len, "with-meta-env")) {
    Term env = parse_omni_expr(s);
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Use lazy parent for meta-environment access
    return omni_ctr2(OMNI_NAM_LPAR, env, expr);
  }

  // quote: (quote expr)
  if (omni_symbol_is(s, sym_start, sym_len, "quote")) {
    Term quoted = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_QQ, quoted);
  }

  // ==========================================================================
  // Developer Tools
  // ==========================================================================

  // inspect: (inspect val) or (inspect val depth) - examine value structure
  if (omni_symbol_is(s, sym_start, sym_len, "inspect")) {
    Term val = parse_omni_expr(s);
    Term depth = omni_int(3);  // Default depth
    if (parse_peek(s) != ')') {
      depth = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_INSP, val, depth);
  }

  // type-of: (type-of val) - get runtime type
  if (omni_symbol_is(s, sym_start, sym_len, "type-of")) {
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_TYOF, val);
  }

  // doc: (doc symbol) - get documentation
  if (omni_symbol_is(s, sym_start, sym_len, "doc")) {
    Term sym = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DOC, sym);
  }

  // trace: (trace label val) - debug logging, returns val
  if (omni_symbol_is(s, sym_start, sym_len, "trace")) {
    Term label = parse_omni_expr(s);
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_TRCE, label, val);
  }

  // time: (time expr) - measure execution time
  if (omni_symbol_is(s, sym_start, sym_len, "time")) {
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_TIME, expr);
  }

  // expand: (expand expr) - macro expansion
  if (omni_symbol_is(s, sym_start, sym_len, "expand")) {
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_EXPD, expr);
  }

  // expand-1: (expand-1 expr) - single-step macro expansion
  if (omni_symbol_is(s, sym_start, sym_len, "expand-1")) {
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Use same constructor with flag
    return omni_ctr2(OMNI_NAM_EXPD, expr, omni_int(1));
  }

  // debug: (debug expr) - debug breakpoint (for REPL integration)
  if (omni_symbol_is(s, sym_start, sym_len, "debug")) {
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_DBUG, expr);
  }

  // pprint: (pprint val) - pretty print value
  if (omni_symbol_is(s, sym_start, sym_len, "pprint")) {
    Term val = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_PRTY, val);
  }

  // source: (source fn) - get source code of function
  if (omni_symbol_is(s, sym_start, sym_len, "source")) {
    Term fn = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_SRCE, fn);
  }

  // profile: (profile label expr) - profiling wrapper
  if (omni_symbol_is(s, sym_start, sym_len, "profile")) {
    Term label = parse_omni_expr(s);
    Term expr = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_PROF, label, expr);
  }

  // assert: (assert condition) or (assert condition message)
  if (omni_symbol_is(s, sym_start, sym_len, "assert")) {
    Term cond = parse_omni_expr(s);
    Term msg = omni_nothing();
    if (parse_peek(s) != ')') {
      msg = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    // Assert is implemented as runtime check
    return omni_ctr2(OMNI_NAM_DBUG, cond, msg);
  }

  // ==========================================================================
  // Networking Operations (FFI-backed)
  // ==========================================================================

  // tcp-connect: (tcp-connect host port) -> socket handle
  if (omni_symbol_is(s, sym_start, sym_len, "tcp-connect")) {
    Term host = parse_omni_expr(s);
    Term port = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_TCPC, host, port);
  }

  // tcp-listen: (tcp-listen port) -> server socket handle
  if (omni_symbol_is(s, sym_start, sym_len, "tcp-listen")) {
    Term port = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_TCPL, port);
  }

  // tcp-accept: (tcp-accept server-sock) -> client socket handle
  if (omni_symbol_is(s, sym_start, sym_len, "tcp-accept")) {
    Term server_sock = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_TCPA, server_sock);
  }

  // tcp-send: (tcp-send sock data) -> bytes sent
  if (omni_symbol_is(s, sym_start, sym_len, "tcp-send")) {
    Term sock = parse_omni_expr(s);
    Term data = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_TCPS, sock, data);
  }

  // tcp-recv: (tcp-recv sock max-len) -> data or nothing
  if (omni_symbol_is(s, sym_start, sym_len, "tcp-recv")) {
    Term sock = parse_omni_expr(s);
    Term max_len = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_TCPR, sock, max_len);
  }

  // udp-socket: (udp-socket) -> socket handle
  if (omni_symbol_is(s, sym_start, sym_len, "udp-socket")) {
    omni_expect_char(s, ')');
    return omni_ctr0(OMNI_NAM_UDPC);
  }

  // udp-bind: (udp-bind sock port) -> boolean success
  if (omni_symbol_is(s, sym_start, sym_len, "udp-bind")) {
    Term sock = parse_omni_expr(s);
    Term port = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_UDPB, sock, port);
  }

  // udp-send-to: (udp-send-to sock host port data) -> bytes sent
  if (omni_symbol_is(s, sym_start, sym_len, "udp-send-to")) {
    Term sock = parse_omni_expr(s);
    Term host = parse_omni_expr(s);
    Term port = parse_omni_expr(s);
    Term data = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_UDPS, sock, omni_cons(host, omni_cons(port, omni_cons(data, omni_nil()))));
  }

  // udp-recv-from: (udp-recv-from sock max-len) -> [data host port] or nothing
  if (omni_symbol_is(s, sym_start, sym_len, "udp-recv-from")) {
    Term sock = parse_omni_expr(s);
    Term max_len = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_UDPR, sock, max_len);
  }

  // socket-close: (socket-close sock) -> nothing
  if (omni_symbol_is(s, sym_start, sym_len, "socket-close")) {
    Term sock = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_SCLS, sock);
  }

  // http-get: (http-get url) -> response
  if (omni_symbol_is(s, sym_start, sym_len, "http-get")) {
    Term url = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr1(OMNI_NAM_HGET, url);
  }

  // http-post: (http-post url body) -> response
  if (omni_symbol_is(s, sym_start, sym_len, "http-post")) {
    Term url = parse_omni_expr(s);
    Term body = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr2(OMNI_NAM_HPOS, url, body);
  }

  // http-request: (http-request method url headers body) -> response
  if (omni_symbol_is(s, sym_start, sym_len, "http-request")) {
    Term method = parse_omni_expr(s);
    Term url = parse_omni_expr(s);
    Term headers = parse_omni_expr(s);
    Term body = parse_omni_expr(s);
    omni_expect_char(s, ')');
    return omni_ctr3(OMNI_NAM_HTTP, method, omni_cons(url, omni_cons(headers, omni_cons(body, omni_nil()))));
  }

  // require: (require predicate) - inline precondition check
  // Desugars to: (perform require predicate)
  // Used for contract-based programming within function bodies
  if (omni_symbol_is(s, sym_start, sym_len, "require")) {
    Term predicate = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Create: (perform require predicate)
    Term require_tag = omni_sym(omni_nick("reqr"));
    return omni_perform(require_tag, predicate);
  }

  // ensure: (ensure predicate) - inline postcondition check
  // Desugars to: (perform ensure predicate)
  // Typically used at the end of function bodies
  if (omni_symbol_is(s, sym_start, sym_len, "ensure")) {
    Term predicate = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Create: (perform ensure predicate)
    Term ensure_tag = omni_sym(omni_nick("ensr"));
    return omni_perform(ensure_tag, predicate);
  }

  // prove: (prove goal) - request automatic proof from handler
  // Desugars to: (perform prove goal)
  // Used to request that the proof handler prove a goal
  if (omni_symbol_is(s, sym_start, sym_len, "prove")) {
    Term goal = parse_omni_expr(s);
    omni_expect_char(s, ')');
    // Create: (perform prove goal)
    Term prove_tag = omni_sym(omni_nick("prov"));
    return omni_perform(prove_tag, goal);
  }

  // perform: (perform effect-tag payload)
  if (omni_symbol_is(s, sym_start, sym_len, "perform")) {
    Term tag = parse_omni_expr(s);
    Term payload = omni_nothing();
    if (parse_peek(s) != ')') {
      payload = parse_omni_expr(s);
    }
    omni_expect_char(s, ')');
    return omni_perform(tag, payload);
  }

  // Default: function application
  u32 fn_nick = omni_symbol_nick(s, sym_start, sym_len);

  // Look up function
  char fn_name[256];
  u32 fn_len = sym_len < 255 ? sym_len : 255;
  memcpy(fn_name, s->src + sym_start, fn_len);
  fn_name[fn_len] = '\0';

  Term fn;
  u32 fn_id = table_find(fn_name, fn_len);
  if (BOOK[fn_id] != 0) {
    fn = term_new_ref(fn_id);
  } else {
    // Check if bound variable
    u32 idx;
    if (omni_bind_lookup(fn_nick, &idx)) {
      fn = omni_var(idx);
    } else {
      // Unknown - symbol reference
      fn = omni_sym(fn_nick);
    }
  }

  // Parse arguments
  while (parse_peek(s) != ')' && !parse_at_end(s)) {
    Term arg = parse_omni_expr(s);
    fn = omni_app(fn, arg);
  }

  omni_expect_char(s, ')');
  return fn;
}

// =============================================================================
// Main Expression Parser
// =============================================================================

fn Term parse_omni_expr(PState *s) {
  omni_skip(s);

  if (parse_peek(s) == '(') {
    return parse_omni_sexp(s);
  }

  return parse_omni_atom(s);
}

// =============================================================================
// Top-Level Parser
// =============================================================================

fn Term parse_omnilisp(PState *s) {
  omni_names_init();
  OMNI_BINDS_LEN = 0;

  Term result = omni_nil();
  Term *tail = &result;

  while (!parse_at_end(s)) {
    omni_skip(s);
    if (parse_at_end(s)) break;

    Term expr = parse_omni_expr(s);
    Term cell = omni_cons(expr, omni_nil());
    *tail = cell;
    tail = &HEAP[term_val(cell) + 1];
  }

  // If single expression, return it directly
  if (term_ext(result) == OMNI_NAM_CON) {
    Term head = HEAP[term_val(result)];
    Term rest = HEAP[term_val(result) + 1];
    if (term_ext(rest) == OMNI_NAM_NIL) {
      return head;
    }
  }

  // Multiple expressions: wrap in begin
  return result;
}

// =============================================================================
// Public Parser API (OmniParse wrapper)
// =============================================================================

// Parser state that main.c expects
typedef struct {
  const char *source;  // Source code
  u32 pos;             // Current position
  u32 len;             // Source length
  u32 line;            // Current line number
  u32 col;             // Current column number
  const char *error;   // Error message (NULL if no error)
} OmniParse;

// Initialize parser state
fn void omni_parse_init(OmniParse *parse, const char *source) {
  parse->source = source;
  parse->pos = 0;
  parse->len = strlen(source);
  parse->line = 1;
  parse->col = 1;
  parse->error = NULL;
}

// Main parse entry point - uses recursive descent parser
// TODO: Switch to Pika parser when fully tested
fn Term omni_parse(OmniParse *parse) {
  // Initialize names
  omni_names_init();

  // Create PState for internal parsing
  PState s;
  s.src = parse->source;
  s.len = parse->len;
  s.pos = 0;
  s.line = 1;
  s.col = 1;

  // Parse using recursive descent parser
  Term result = parse_omnilisp(&s);

  // Update parse state with final position
  parse->pos = s.pos;
  parse->line = s.line;
  parse->col = s.col;

  return result;
}

// Parse single expression
fn Term omni_parse_expr(OmniParse *parse) {
  omni_names_init();

  PState s;
  s.src = parse->source;
  s.len = parse->len;
  s.pos = parse->pos;
  s.line = parse->line;
  s.col = parse->col;

  OMNI_BINDS_LEN = 0;
  Term result = parse_omni_expr(&s);

  parse->pos = s.pos;
  parse->line = s.line;
  parse->col = s.col;

  return result;
}

// =============================================================================
// Pika Parser Integration (Alternative)
// =============================================================================

// Define OMNI_USE_PIKA to use Pika parser instead of recursive descent
#ifdef OMNI_USE_PIKA
#include "../pika/omni_pika.c"

// Override the parser to use Pika
fn Term omni_parse_pika(OmniParse *parse) {
  return omni_pika_read(parse->source);
}

fn Term omni_parse_expr_pika(OmniParse *parse) {
  return omni_pika_read_expr(parse->source + parse->pos);
}

#endif /* OMNI_USE_PIKA */
