/*
 * OmniLisp Grammar Definition for Pika Parser
 *
 * Defines the PEG grammar for OmniLisp and semantic actions
 * that generate HVM4 terms.
 *
 * Character Calculus:
 *   () - Execution/evaluation
 *   [] - Slots/parameters (no eval)
 *   {} - Types/constraints
 *   ^  - Metadata prefix
 */

#include "pika_core.c"
#include "../nick/omnilisp.c"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* ============== Grammar Rule IDs ============== */

enum {
    R_EPSILON,          /* Matches empty string */

    /* Whitespace */
    R_CHAR_SPACE, R_CHAR_TAB, R_CHAR_NL, R_CHAR_CR,
    R_SPACE,            /* Single whitespace char */
    R_WS,               /* Optional whitespace sequence */
    R_SEMICOLON,        /* ; */
    R_COMMENT_CHAR,     /* Any char except newline */
    R_COMMENT_INNER,    /* Zero or more non-newline chars */
    R_COMMENT,          /* ; to end of line */
    R_WS_OR_COMMENT,    /* Whitespace or comment */
    R_SKIP,             /* Zero or more whitespace/comments */

    /* Digits and Numbers */
    R_DIGIT, R_DIGIT_NZ, R_DIGITS,
    R_DOT,              /* . (decimal point) */
    R_SIGN,             /* Optional +/- */
    R_INT,              /* Integer literal */
    R_FRAC,             /* Fractional part .123 */
    R_FLOAT,            /* Float literal */
    R_NUMBER,           /* Int or float */

    /* Identifiers/Symbols */
    R_ALPHA_LOWER,      /* a-z */
    R_ALPHA_UPPER,      /* A-Z */
    R_ALPHA,            /* a-z | A-Z */
    R_SYM_PLUS,         /* + */
    R_SYM_MINUS,        /* - */
    R_SYM_STAR,         /* * */
    R_SYM_SLASH,        /* / */
    R_SYM_EQ,           /* = */
    R_SYM_LT,           /* < */
    R_SYM_GT,           /* > */
    R_SYM_BANG,         /* ! */
    R_SYM_QMARK,        /* ? */
    R_SYM_UNDER,        /* _ */
    R_SYM_AT,           /* @ */
    R_SYM_PERCENT,      /* % */
    R_SYM_AMP,          /* & */
    R_SYM_SPECIAL,      /* ALT of all special chars */
    R_SYM_INIT,         /* Symbol start char (alpha | special) */
    R_SYM_CHAR,         /* Symbol continuation char (alpha | digit | special) */
    R_SYM_CONT,         /* Zero or more continuation chars */
    R_SYM,              /* Symbol */
    R_COLON_SYM,        /* :name = 'name = (quote name) */

    /* Delimiters */
    R_LPAREN, R_RPAREN, /* ( ) */
    R_LBRACKET, R_RBRACKET, /* [ ] */
    R_LBRACE, R_RBRACE, /* { } */
    R_HASHBRACE,        /* #{ */
    R_CARET,            /* ^ */
    R_DOTDOT,           /* .. (spread) */
    R_COLON,            /* : */
    R_COLONWHEN,        /* :when */

    /* String */
    R_DQUOTE,           /* " */
    R_BACKSLASH,        /* \ */
    R_ESC_N,            /* n (newline escape) */
    R_ESC_T,            /* t (tab escape) */
    R_ESC_R,            /* r (carriage return escape) */
    R_ESC_QUOTE,        /* " (escaped quote) */
    R_ESC_BSLASH,       /* \ (escaped backslash) */
    R_ESC_CHAR,         /* ALT of escape chars after backslash */
    R_ESCAPE_SEQ,       /* \ followed by escape char */
    R_STRING_REGULAR,   /* Non-special char */
    R_STRING_CHAR,      /* Regular char or escape sequence */
    R_STRING_INNER,     /* String contents (zero or more) */
    R_STRING,           /* "..." */

    /* Character literal */
    R_CHAR_BACKSLASH,   /* \ for char literals */
    R_CHAR_NEWLINE,     /* "newline" */
    R_CHAR_TAB_NAME,    /* "tab" */
    R_CHAR_SPACE_NAME,  /* "space" */
    R_CHAR_RETURN,      /* "return" */
    R_CHAR_NAMED,       /* ALT of named chars */
    R_CHAR_SIMPLE,      /* Single char after backslash */
    R_CHAR_LITERAL,     /* \c or \newline etc */

    /* Enhanced number parsing */
    R_OPT_SIGN,         /* Optional +/- */
    R_OPT_INT,          /* Optional integer */
    R_SIGNED_INT,       /* +123 or -456 */
    R_FLOAT_FULL,       /* 3.14, -2.5 */
    R_FLOAT_LEAD,       /* .5, -.25 */
    R_FLOAT_TRAIL,      /* 3., -5. */
    R_ANY_FLOAT,        /* ALT of all float forms */

    /* Quote/quasiquote */
    R_QUOTE_CHAR,       /* ' */
    R_QUASIQUOTE_CHAR,  /* ` */
    R_UNQUOTE_CHAR,     /* , */
    R_UNQUOTE_SPLICE,   /* ,@ */
    R_QUOTED,           /* 'expr, `expr, ,expr, ,@expr */

    /* Path expressions */
    R_PATH_SEGMENT,     /* Symbol or array index */
    R_PATH_TAIL_ITEM,   /* .segment */
    R_PATH_TAIL,        /* One or more .segment */
    R_PATH_ROOT,        /* Root of path (int or sym) */
    R_PATH,             /* foo.bar.baz */

    /* Set literal */
    R_HASHSET,          /* #set */
    R_SET,              /* #set{...} */

    /* Named char literal with semantic action */
    R_HASH,             /* # */
    R_NAMED_CHAR,       /* #\newline etc */

    /* Format strings */
    R_HASH_FMT,         /* #fmt */
    R_HASH_CLF,         /* #clf */
    R_FMT_STRING,       /* #fmt"..." */
    R_CLF_STRING,       /* #clf"..." */

    /* Hash-val reader */
    R_HASH_VAL,         /* #val <atom> */

    /* Kind splice */
    R_HASHKIND,         /* #kind */
    R_KIND_SPLICE,      /* {#kind expr} */

    /* String char improvement */
    R_NOT_DQUOTE,       /* NOT lookahead for " */
    R_ANY_CHAR,         /* Any single character */

    /* Expression types */
    R_EXPR,             /* Top-level expression */
    R_ATOM,             /* Number, symbol, string, char */

    /* Lists (execution) */
    R_LIST_INNER,       /* Content of list */
    R_LIST,             /* ( ... ) */

    /* Slots/Arrays (data) */
    R_SLOT_INNER,       /* Content of slot */
    R_SLOT,             /* [ ... ] */

    /* Types */
    R_TYPE_INNER,       /* Content of type */
    R_TYPE,             /* { ... } */

    /* Dicts */
    R_DICT_INNER,       /* Content of dict */
    R_DICT,             /* #{ ... } */

    /* Metadata */
    R_META_KEY,         /* ^:key or ^{...} */
    R_META,             /* Metadata prefix */

    /* Special forms */
    R_GUARD,            /* :when <expr> */
    R_SPREAD,           /* .. <name> */

    /* Root */
    R_PROGRAM_INNER,    /* Sequence of expressions */
    R_PROGRAM,          /* Root rule */

    NUM_RULES
};

static PikaRule g_omni_rules[NUM_RULES];
static int g_omni_rules_init = 0;

/* ============== Binding Stack for de Bruijn Indexing ============== */

#define OMNI_PIKA_BIND_MAX 16384
static u32 g_bind_stack[OMNI_PIKA_BIND_MAX];
static u32 g_bind_len = 0;

static void bind_push(u32 sym) {
    if (g_bind_len >= OMNI_PIKA_BIND_MAX) {
        fprintf(stderr, "OMNI_PIKA_ERROR: too many binders\n");
        exit(1);
    }
    g_bind_stack[g_bind_len++] = sym;
}

static void bind_pop(u32 count) {
    while (count > 0 && g_bind_len > 0) {
        g_bind_len--;
        count--;
    }
}

static int bind_lookup(u32 sym, u32 *out_idx) {
    for (int i = (int)g_bind_len - 1; i >= 0; i--) {
        if (g_bind_stack[i] == sym) {
            *out_idx = (u32)(g_bind_len - 1 - (u32)i);
            return 1;
        }
    }
    return 0;
}

/* ============== Helper to Create Rule Children Arrays ============== */

static int* rule_ids(int count, ...) {
    int* arr = malloc(sizeof(int) * count);
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        arr[i] = va_arg(args, int);
    }
    va_end(args);
    return arr;
}

/* ============== Term Constructors (using nick/omnilisp.c names) ============== */

static Term mk_ctr0(u32 nam) {
    return term_new_ctr(nam, 0, NULL);
}

static Term mk_ctr1(u32 nam, Term a) {
    Term args[1] = {a};
    return term_new_ctr(nam, 1, args);
}

static Term mk_ctr2(u32 nam, Term a, Term b) {
    Term args[2] = {a, b};
    return term_new_ctr(nam, 2, args);
}

static Term mk_ctr3(u32 nam, Term a, Term b, Term c) {
    Term args[3] = {a, b, c};
    return term_new_ctr(nam, 3, args);
}

static Term mk_nil(void) {
    return mk_ctr0(OMNI_NAM_NIL);
}

static Term mk_cons(Term h, Term t) {
    return mk_ctr2(OMNI_NAM_CON, h, t);
}

static Term mk_list(Term* items, u32 count) {
    Term result = mk_nil();
    for (int i = (int)count - 1; i >= 0; i--) {
        result = mk_cons(items[i], result);
    }
    return result;
}

static Term mk_int(int64_t n) {
    return mk_ctr1(OMNI_NAM_LIT, term_new_num((u32)n));
}

static Term mk_sym(u32 nick) {
    return mk_ctr1(OMNI_NAM_SYM, term_new_num(nick));
}

static Term mk_var(u32 idx) {
    return mk_ctr1(OMNI_NAM_VAR, term_new_num(idx));
}

static Term mk_app(Term fn, Term arg) {
    return mk_ctr2(OMNI_NAM_APP, fn, arg);
}

static Term mk_lam(Term body) {
    return mk_ctr1(OMNI_NAM_LAM, body);
}

static Term mk_let(Term val, Term body) {
    return mk_ctr2(OMNI_NAM_LET, val, body);
}

static Term mk_if(Term cond, Term then_br, Term else_br) {
    return mk_ctr3(OMNI_NAM_IF, cond, then_br, else_br);
}

/* ============== Nick Encoding from Source ============== */

static u32 nick_from_str(const char* src, size_t len) {
    u32 k = 0;
    for (size_t i = 0; i < len && i < 4; i++) {
        k = ((k << 6) + nick_letter_to_b64(src[i])) & EXT_MASK;
    }
    return k;
}

/* ============== Semantic Actions ============== */

/* Integer literal */
static Term act_int(PikaState* state, size_t pos, PikaMatch match) {
    char buf[64];
    size_t len = match.len > 63 ? 63 : match.len;
    memcpy(buf, state->input + pos, len);
    buf[len] = '\0';
    int64_t n = atoll(buf);
    return mk_int(n);
}

/* Float literal - represented as fixed-point */
static Term act_float(PikaState* state, size_t pos, PikaMatch match) {
    char buf[64];
    size_t len = match.len > 63 ? 63 : match.len;
    memcpy(buf, state->input + pos, len);
    buf[len] = '\0';

    /* Find decimal point and count fractional digits */
    int scale = 0;
    char* dot = strchr(buf, '.');
    if (dot) {
        scale = strlen(dot + 1);
        /* Remove dot for integer parsing */
        memmove(dot, dot + 1, strlen(dot));
    }

    int64_t value = atoll(buf);
    /* Return #Fix{hi, lo, scale} */
    u32 hi = (u32)(((u64)value) >> 32);
    u32 lo = (u32)(value & 0xFFFFFFFF);
    Term args[3];
    args[0] = term_new_num(hi);
    args[1] = term_new_num(lo);
    args[2] = term_new_num(scale);
    return term_new_ctr(OMNI_NAM_FIX, 3, args);
}

/* Symbol */
static Term act_sym(PikaState* state, size_t pos, PikaMatch match) {
    u32 nick = nick_from_str(state->input + pos, match.len);

    /* Check if it's a bound variable */
    u32 idx;
    if (bind_lookup(nick, &idx)) {
        return mk_var(idx);
    }

    /* Otherwise return as symbol */
    return mk_sym(nick);
}

/* Colon-quoted symbol (:name) - reader sugar for (quote name) */
static Term act_colon_quoted(PikaState* state, size_t pos, PikaMatch match) {
    /* Skip the leading colon, get the symbol */
    u32 nick = nick_from_str(state->input + pos + 1, match.len - 1);
    /* :name ≡ 'name ≡ (quote name) */
    /* Return as #CON{#Sym{quote}, #CON{#Sym{nick}, #NIL}} */
    Term sym = mk_sym(nick);
    Term quote_sym = mk_sym(OMNI_NAM_QUOT);
    return mk_cons(quote_sym, mk_cons(sym, mk_nil()));
}

/* String literal */
static Term act_string(PikaState* state, size_t pos, PikaMatch match) {
    /* Skip quotes, build cons-list of characters */
    Term result = mk_nil();
    size_t end = pos + match.len - 1;  /* Exclude closing quote */

    for (size_t i = end - 1; i > pos; i--) {
        unsigned char c = state->input[i];
        if (c == '\\' && i > pos) {
            /* Handle escape - simplified */
            i--;
            switch (state->input[i]) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case '\\': c = '\\'; break;
                case '"': c = '"'; break;
                default: c = state->input[i]; break;
            }
        }
        Term chr = mk_ctr1(OMNI_NAM_CHR, term_new_num(c));
        result = mk_cons(chr, result);
    }

    return result;
}

/* List (execution context) */
static Term act_list(PikaState* state, size_t pos, PikaMatch match) {
    /* R_LIST = SEQ(LPAREN, SKIP, R_LIST_INNER, SKIP, RPAREN) */
    size_t current = pos + 1;  /* Skip ( */

    /* Skip whitespace */
    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    /* Get list inner */
    PikaMatch* inner = pika_get_match(state, current, R_LIST_INNER);
    if (inner && inner->matched && inner->val) {
        return inner->val;
    }

    return mk_nil();
}

/* List inner (recursive) */
static Term act_list_inner(PikaState* state, size_t pos, PikaMatch match) {
    if (match.len == 0) return mk_nil();

    size_t current = pos;

    /* Get first expression */
    PikaMatch* expr = pika_get_match(state, current, R_EXPR);
    if (!expr || !expr->matched) return mk_nil();

    Term head = expr->val;
    current += expr->len;

    /* Skip whitespace */
    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    /* Get rest */
    PikaMatch* rest = pika_get_match(state, current, R_LIST_INNER);
    Term tail = (rest && rest->matched && rest->val) ? rest->val : mk_nil();

    return mk_cons(head, tail);
}

/* Slot (data context - [ ... ]) */
static Term act_slot(PikaState* state, size_t pos, PikaMatch match) {
    /* R_SLOT = SEQ(LBRACKET, SKIP, R_SLOT_INNER, SKIP, RBRACKET) */
    size_t current = pos + 1;  /* Skip [ */

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* inner = pika_get_match(state, current, R_SLOT_INNER);
    if (inner && inner->matched && inner->val) {
        /* Wrap in #Slot constructor to mark as data context */
        return mk_ctr1(OMNI_NAM_SLOT, inner->val);
    }

    return mk_ctr1(OMNI_NAM_SLOT, mk_nil());
}

/* Slot inner */
static Term act_slot_inner(PikaState* state, size_t pos, PikaMatch match) {
    if (match.len == 0) return mk_nil();

    size_t current = pos;

    /* Get first expression */
    PikaMatch* expr = pika_get_match(state, current, R_EXPR);
    if (!expr || !expr->matched) return mk_nil();

    Term head = expr->val;
    current += expr->len;

    /* Skip whitespace */
    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    /* Get rest */
    PikaMatch* rest = pika_get_match(state, current, R_SLOT_INNER);
    Term tail = (rest && rest->matched && rest->val) ? rest->val : mk_nil();

    return mk_cons(head, tail);
}

/* Type annotation { ... } */
static Term act_type(PikaState* state, size_t pos, PikaMatch match) {
    size_t current = pos + 1;  /* Skip { */

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* inner = pika_get_match(state, current, R_TYPE_INNER);
    if (inner && inner->matched && inner->val) {
        /* Return type as #TAnn or just the inner for simple types */
        return mk_ctr1(OMNI_NAM_TANN, inner->val);
    }

    return mk_ctr1(OMNI_NAM_TANN, mk_nil());
}

/* Type inner */
static Term act_type_inner(PikaState* state, size_t pos, PikaMatch match) {
    if (match.len == 0) return mk_nil();

    size_t current = pos;

    PikaMatch* expr = pika_get_match(state, current, R_EXPR);
    if (!expr || !expr->matched) return mk_nil();

    Term head = expr->val;
    current += expr->len;

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* rest = pika_get_match(state, current, R_TYPE_INNER);
    Term tail = (rest && rest->matched && rest->val) ? rest->val : mk_nil();

    return mk_cons(head, tail);
}

/* Dict #{ ... } */
static Term act_dict(PikaState* state, size_t pos, PikaMatch match) {
    size_t current = pos + 2;  /* Skip #{ */

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* inner = pika_get_match(state, current, R_DICT_INNER);
    if (inner && inner->matched && inner->val) {
        /* Return dict as #Dict constructor */
        return mk_ctr1(OMNI_NAM_DICT, inner->val);
    }

    return mk_ctr1(OMNI_NAM_DICT, mk_nil());
}

/* Dict inner (key-value pairs) */
static Term act_dict_inner(PikaState* state, size_t pos, PikaMatch match) {
    if (match.len == 0) return mk_nil();

    size_t current = pos;

    PikaMatch* expr = pika_get_match(state, current, R_EXPR);
    if (!expr || !expr->matched) return mk_nil();

    Term head = expr->val;
    current += expr->len;

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* rest = pika_get_match(state, current, R_DICT_INNER);
    Term tail = (rest && rest->matched && rest->val) ? rest->val : mk_nil();

    return mk_cons(head, tail);
}

/* Guard expression (:when expr) */
static Term act_guard(PikaState* state, size_t pos, PikaMatch match) {
    /* R_GUARD = SEQ(COLONWHEN, SKIP, EXPR) */
    size_t current = pos + 5;  /* Skip ":when" */

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* expr = pika_get_match(state, current, R_EXPR);
    if (expr && expr->matched && expr->val) {
        /* Return as #Guar{condition} */
        return mk_ctr1(OMNI_NAM_GUAR, expr->val);
    }

    return mk_nil();
}

/* Spread pattern (.. name) */
static Term act_spread(PikaState* state, size_t pos, PikaMatch match) {
    /* R_SPREAD = SEQ(DOTDOT, SKIP, SYM) */
    size_t current = pos + 2;  /* Skip ".." */

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* sym = pika_get_match(state, current, R_SYM);
    if (sym && sym->matched && sym->val) {
        /* Return as #Sprd{name} */
        return mk_ctr1(OMNI_NAM_SPRD, sym->val);
    }

    /* If no symbol, spread just captures rest as list */
    return mk_ctr1(OMNI_NAM_SPRD, mk_nil());
}

/* Signed integer literal */
static Term act_signed_int(PikaState* state, size_t pos, PikaMatch match) {
    char buf[64];
    size_t len = match.len > 63 ? 63 : match.len;
    memcpy(buf, state->input + pos, len);
    buf[len] = '\0';
    int64_t n = atoll(buf);
    return mk_int(n);
}

/* Quote/quasiquote handling */
static Term act_quoted(PikaState* state, size_t pos, PikaMatch match) {
    char quote_char = state->input[pos];
    size_t expr_pos = pos + 1;

    /* Handle ,@ (unquote-splicing) */
    if (quote_char == ',' && pos + 1 < state->input_len && state->input[pos + 1] == '@') {
        expr_pos = pos + 2;
        PikaMatch* ws = pika_get_match(state, expr_pos, R_SKIP);
        if (ws && ws->matched) expr_pos += ws->len;

        PikaMatch* expr = pika_get_match(state, expr_pos, R_EXPR);
        if (expr && expr->matched && expr->val) {
            /* (unquote-splicing <expr>) */
            u32 uqs_nick = nick_from_str("uqsp", 4);  /* unquote-splicing */
            return mk_cons(mk_sym(uqs_nick), mk_cons(expr->val, mk_nil()));
        }
    }

    PikaMatch* ws = pika_get_match(state, expr_pos, R_SKIP);
    if (ws && ws->matched) expr_pos += ws->len;

    PikaMatch* expr = pika_get_match(state, expr_pos, R_EXPR);
    if (expr && expr->matched && expr->val) {
        u32 quote_nick;
        switch (quote_char) {
            case '\'': quote_nick = OMNI_NAM_QUOT; break;
            case '`':  quote_nick = nick_from_str("quas", 4); break;  /* quasiquote */
            case ',':  quote_nick = nick_from_str("unqu", 4); break;  /* unquote */
            default:   quote_nick = OMNI_NAM_QUOT; break;
        }
        return mk_cons(mk_sym(quote_nick), mk_cons(expr->val, mk_nil()));
    }

    return mk_nil();
}

/* Path expression (foo.bar.baz) */
static Term act_path(PikaState* state, size_t pos, PikaMatch match) {
    /* Get PATH_ROOT */
    PikaMatch* root_m = pika_get_match(state, pos, R_PATH_ROOT);
    if (!root_m || !root_m->matched) return mk_nil();

    Term root = root_m->val;
    size_t current = pos + root_m->len;

    /* Collect segments in reverse order, then reverse */
    Term segments = mk_nil();

    while (current < pos + match.len) {
        /* Expect DOT */
        PikaMatch* dot_m = pika_get_match(state, current, R_DOT);
        if (!dot_m || !dot_m->matched) break;
        current += dot_m->len;

        /* Get segment */
        PikaMatch* seg_m = pika_get_match(state, current, R_PATH_SEGMENT);
        if (!seg_m || !seg_m->matched) break;

        segments = mk_cons(seg_m->val, segments);
        current += seg_m->len;
    }

    /* Reverse segments */
    Term rev_segments = mk_nil();
    /* Note: segments is a cons list, iterate with pattern matching */
    while (term_tag(segments) == CTR && term_ctr_nam(segments) == OMNI_NAM_CON) {
        Term head = term_ctr_arg(segments, 0);
        rev_segments = mk_cons(head, rev_segments);
        segments = term_ctr_arg(segments, 1);
    }

    /* Return (path root seg1 seg2 ...) */
    u32 path_nick = nick_from_str("path", 4);
    return mk_cons(mk_sym(path_nick), mk_cons(root, rev_segments));
}

/* Set literal #set{...} */
static Term act_set(PikaState* state, size_t pos, PikaMatch match) {
    /* R_SET = SEQ(HASHSET, LBRACE, SKIP, SLOT_INNER, SKIP, RBRACE) */
    /* Skip #set{ (4 chars + 1) */
    size_t current = pos + 5;

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* inner = pika_get_match(state, current, R_SLOT_INNER);
    if (inner && inner->matched && inner->val) {
        /* Return as (set elem1 elem2 ...) */
        u32 set_nick = nick_from_str("set", 3);
        return mk_cons(mk_sym(set_nick), inner->val);
    }

    u32 set_nick = nick_from_str("set", 3);
    return mk_cons(mk_sym(set_nick), mk_nil());
}

/* Named character literal #\newline etc */
static Term act_named_char(PikaState* state, size_t pos, PikaMatch match) {
    /* Skip #\ (2 chars) */
    size_t name_start = pos + 2;
    size_t name_len = match.len - 2;

    const char* name = state->input + name_start;

    long char_code = -1;

    /* Check for hex syntax: #\xNN */
    if (name_len >= 3 && name[0] == 'x') {
        char hex[3] = { name[1], name[2], '\0' };
        char_code = strtol(hex, NULL, 16);
        if (char_code < 0 || char_code > 255) char_code = -1;
    } else if (name_len == 7 && strncmp(name, "newline", 7) == 0) {
        char_code = 10;
    } else if (name_len == 5 && strncmp(name, "space", 5) == 0) {
        char_code = 32;
    } else if (name_len == 3 && strncmp(name, "tab", 3) == 0) {
        char_code = 9;
    } else if (name_len == 6 && strncmp(name, "return", 6) == 0) {
        char_code = 13;
    } else if (name_len == 3 && strncmp(name, "nul", 3) == 0) {
        char_code = 0;
    } else if (name_len == 4 && strncmp(name, "bell", 4) == 0) {
        char_code = 7;
    } else if (name_len == 9 && strncmp(name, "backspace", 9) == 0) {
        char_code = 8;
    } else if (name_len == 6 && strncmp(name, "escape", 6) == 0) {
        char_code = 27;
    } else if (name_len == 6 && strncmp(name, "delete", 6) == 0) {
        char_code = 127;
    } else if (name_len == 1) {
        /* Simple single char like #\a */
        char_code = (unsigned char)name[0];
    }

    if (char_code >= 0) {
        return mk_ctr1(OMNI_NAM_CHR, term_new_num((u32)char_code));
    }

    return mk_nil();
}

/* Format string #fmt"..." */
static Term act_fmt_string(PikaState* state, size_t pos, PikaMatch match) {
    /* Skip #fmt (4 chars) */
    size_t str_start = pos + 4;
    size_t str_len = match.len - 4;

    /* The rest is a string literal - reuse string parsing */
    /* For now, return (fmt-string <string-content>) */
    Term content = act_string(state, str_start, (PikaMatch){true, str_len, 0});

    u32 fmt_nick = nick_from_str("fmt", 3);
    return mk_cons(mk_sym(fmt_nick), mk_cons(content, mk_nil()));
}

/* CLF format string #clf"..." */
static Term act_clf_string(PikaState* state, size_t pos, PikaMatch match) {
    /* Skip #clf (4 chars) */
    size_t str_start = pos + 4;
    size_t str_len = match.len - 4;

    Term content = act_string(state, str_start, (PikaMatch){true, str_len, 0});

    u32 clf_nick = nick_from_str("clf", 3);
    return mk_cons(mk_sym(clf_nick), mk_cons(content, mk_nil()));
}

/* Hash-val reader #val <atom> */
static Term act_hash_val(PikaState* state, size_t pos, PikaMatch match) {
    /* Skip #val (4 chars) */
    size_t current = pos + 4;

    /* Skip whitespace */
    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    /* Get the atom */
    PikaMatch* atom = pika_get_match(state, current, R_ATOM);
    if (atom && atom->matched && atom->val) {
        /* Return (value->type <atom>) */
        u32 v2t_nick = nick_from_str("v2ty", 4);  /* value->type abbreviated */
        return mk_cons(mk_sym(v2t_nick), mk_cons(atom->val, mk_nil()));
    }

    return mk_nil();
}

/* Kind splice {#kind expr} */
static Term act_kind_splice(PikaState* state, size_t pos, PikaMatch match) {
    /* Skip { #kind (7 chars: { + space? + #kind) */
    size_t current = pos + 1;  /* Skip { */

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    /* Skip #kind (5 chars) */
    current += 5;

    ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    /* Get the expression */
    PikaMatch* expr = pika_get_match(state, current, R_EXPR);
    if (expr && expr->matched && expr->val) {
        /* Return (kind-splice <expr>) */
        u32 ks_nick = nick_from_str("kspl", 4);  /* kind-splice abbreviated */
        return mk_ctr1(ks_nick, expr->val);
    }

    return mk_nil();
}

/* Expression (dispatch to appropriate type) */
static Term act_expr(PikaState* state, size_t pos, PikaMatch match) {
    /* R_EXPR is an ALT of various expression types */
    /* Return the value from the child that matched */

    /* Try each alternative and return first match
     * Order should match the ALT order in R_EXPR rule */
    int alts[] = {
        /* Hash-prefixed forms (specific before general) */
        R_FMT_STRING, R_CLF_STRING,             /* #fmt"...", #clf"..." */
        R_SET,                                  /* #set{...} */
        R_NAMED_CHAR,                           /* #\newline */
        R_KIND_SPLICE,                          /* {#kind expr} */
        R_DICT,                                 /* #{...} */
        /* Quote forms */
        R_QUOTED,                               /* 'x, `x, ,x, ,@x */
        /* Path expressions (before plain symbol) */
        R_PATH,                                 /* foo.bar.baz */
        /* Compound forms */
        R_LIST, R_SLOT, R_TYPE,                 /* (...), [...], {...} */
        R_META,                                 /* ^:key */
        R_GUARD,                                /* :when */
        R_SPREAD,                               /* .. */
        /* Literals (float before int for longest match) */
        R_ANY_FLOAT, R_SIGNED_INT, R_INT, R_STRING,
        /* Symbols (colon-quoted before plain) */
        R_COLON_SYM, R_SYM
    };
    int num_alts = sizeof(alts) / sizeof(alts[0]);

    for (int i = 0; i < num_alts; i++) {
        PikaMatch* m = pika_get_match(state, pos, alts[i]);
        if (m && m->matched && m->len == match.len) {
            return m->val;
        }
    }

    return mk_nil();
}

/* Program (sequence of expressions) */
static Term act_program(PikaState* state, size_t pos, PikaMatch match) {
    size_t current = pos;

    /* Skip leading whitespace */
    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    /* Get program inner */
    PikaMatch* inner = pika_get_match(state, current, R_PROGRAM_INNER);
    if (inner && inner->matched && inner->val) {
        return inner->val;
    }

    return mk_nil();
}

/* Program inner */
static Term act_program_inner(PikaState* state, size_t pos, PikaMatch match) {
    if (match.len == 0) return mk_nil();

    size_t current = pos;

    PikaMatch* expr = pika_get_match(state, current, R_EXPR);
    if (!expr || !expr->matched) return mk_nil();

    Term head = expr->val;
    current += expr->len;

    PikaMatch* ws = pika_get_match(state, current, R_SKIP);
    if (ws && ws->matched) current += ws->len;

    PikaMatch* rest = pika_get_match(state, current, R_PROGRAM_INNER);
    Term tail = (rest && rest->matched && rest->val) ? rest->val : mk_nil();

    return mk_cons(head, tail);
}

/* ============== Grammar Initialization ============== */

void omni_pika_init(void) {
    if (g_omni_rules_init) return;

    /* Initialize nick names */
    omni_names_init();

    /* Epsilon (empty string) */
    g_omni_rules[R_EPSILON] = (PikaRule){ PIKA_TERMINAL, .data.str = "" };

    /* Whitespace characters */
    g_omni_rules[R_CHAR_SPACE] = (PikaRule){ PIKA_TERMINAL, .data.str = " " };
    g_omni_rules[R_CHAR_TAB] = (PikaRule){ PIKA_TERMINAL, .data.str = "\t" };
    g_omni_rules[R_CHAR_NL] = (PikaRule){ PIKA_TERMINAL, .data.str = "\n" };
    g_omni_rules[R_CHAR_CR] = (PikaRule){ PIKA_TERMINAL, .data.str = "\r" };

    /* Single whitespace */
    g_omni_rules[R_SPACE] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(4, R_CHAR_SPACE, R_CHAR_TAB, R_CHAR_NL, R_CHAR_CR), 4 }
    };

    /* Optional whitespace sequence */
    g_omni_rules[R_WS] = (PikaRule){
        PIKA_REP,
        .data.children = { rule_ids(1, R_SPACE), 1 }
    };

    /* Comment implementation */
    /* Semicolon terminal */
    g_omni_rules[R_SEMICOLON] = (PikaRule){ PIKA_TERMINAL, .data.str = ";" };

    /* Comment char: NOT(newline) followed by ANY
     * In PEG: (!'\n' .)
     * This matches any single char that is not a newline */
    g_omni_rules[R_COMMENT_CHAR] = (PikaRule){
        PIKA_SEQ,
        .data.children = { NULL, 0 }
    };
    /* We'll set up the sequence children after defining helper rules */

    /* Comment inner: zero or more non-newline chars */
    g_omni_rules[R_COMMENT_INNER] = (PikaRule){
        PIKA_REP,
        .data.children = { rule_ids(1, R_COMMENT_CHAR), 1 }
    };

    /* Now set up R_COMMENT_CHAR as: NOT(newline), ANY */
    /* Create a NOT rule that checks for newline */
    /* R_COMMENT_CHAR = SEQ( NOT(newline), ANY )
     * But we need NOT and ANY as separate rules.
     * For simplicity, let's just use a character range: space to tilde (printable ASCII) */
    g_omni_rules[R_COMMENT_CHAR] = (PikaRule){
        PIKA_RANGE,
        .data.range = {' ', '~'}  /* Printable ASCII excluding control chars */
    };

    /* Comment: ; followed by any chars until end of line */
    g_omni_rules[R_COMMENT] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_SEMICOLON, R_COMMENT_INNER), 2 }
    };

    /* WS_OR_COMMENT: whitespace or comment */
    g_omni_rules[R_WS_OR_COMMENT] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_SPACE, R_COMMENT), 2 }
    };

    /* Skip = zero or more whitespace/comments */
    g_omni_rules[R_SKIP] = (PikaRule){
        PIKA_REP,
        .data.children = { rule_ids(1, R_WS_OR_COMMENT), 1 }
    };

    /* Digits */
    g_omni_rules[R_DIGIT] = (PikaRule){ PIKA_RANGE, .data.range = {'0', '9'} };
    g_omni_rules[R_DIGIT_NZ] = (PikaRule){ PIKA_RANGE, .data.range = {'1', '9'} };
    g_omni_rules[R_DIGITS] = (PikaRule){
        PIKA_POS,
        .data.children = { rule_ids(1, R_DIGIT), 1 }
    };

    /* Decimal point */
    g_omni_rules[R_DOT] = (PikaRule){ PIKA_TERMINAL, .data.str = "." };

    /* Sign: + or - (optional, using R_SYM_PLUS and R_SYM_MINUS) */
    /* Note: These are defined later, so R_SIGN uses PIKA_OPT with ALT */
    /* For now, we'll define sign with inline terminals */
    /* We reuse R_SYM_PLUS and R_SYM_MINUS after they are defined */
    /* Actually - forward reference works in Pika since rules are evaluated at parse time */
    g_omni_rules[R_SIGN] = (PikaRule){
        PIKA_OPT,
        .data.children = { rule_ids(1, R_SYM_MINUS), 1 }  /* Optional minus for now */
    };

    /* Integer: one or more digits */
    g_omni_rules[R_INT] = (PikaRule){
        PIKA_POS,
        .data.children = { rule_ids(1, R_DIGIT), 1 },
        .action = act_int
    };

    /* Float fraction part: . followed by digits */
    g_omni_rules[R_FRAC] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_DOT, R_DIGITS), 2 }
    };

    /* Float: digits . digits (e.g., 3.14) */
    g_omni_rules[R_FLOAT] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_DIGITS, R_FRAC), 2 },
        .action = act_float
    };

    /* Number: float or int (try float first for longest match) */
    g_omni_rules[R_NUMBER] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_FLOAT, R_INT), 2 }
    };

    /* Alpha: a-z, A-Z */
    g_omni_rules[R_ALPHA_LOWER] = (PikaRule){ PIKA_RANGE, .data.range = {'a', 'z'} };
    g_omni_rules[R_ALPHA_UPPER] = (PikaRule){ PIKA_RANGE, .data.range = {'A', 'Z'} };
    g_omni_rules[R_ALPHA] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_ALPHA_LOWER, R_ALPHA_UPPER), 2 }
    };

    /* Symbol special characters: terminals */
    g_omni_rules[R_SYM_PLUS] = (PikaRule){ PIKA_TERMINAL, .data.str = "+" };
    g_omni_rules[R_SYM_MINUS] = (PikaRule){ PIKA_TERMINAL, .data.str = "-" };
    g_omni_rules[R_SYM_STAR] = (PikaRule){ PIKA_TERMINAL, .data.str = "*" };
    g_omni_rules[R_SYM_SLASH] = (PikaRule){ PIKA_TERMINAL, .data.str = "/" };
    g_omni_rules[R_SYM_EQ] = (PikaRule){ PIKA_TERMINAL, .data.str = "=" };
    g_omni_rules[R_SYM_LT] = (PikaRule){ PIKA_TERMINAL, .data.str = "<" };
    g_omni_rules[R_SYM_GT] = (PikaRule){ PIKA_TERMINAL, .data.str = ">" };
    g_omni_rules[R_SYM_BANG] = (PikaRule){ PIKA_TERMINAL, .data.str = "!" };
    g_omni_rules[R_SYM_QMARK] = (PikaRule){ PIKA_TERMINAL, .data.str = "?" };
    g_omni_rules[R_SYM_UNDER] = (PikaRule){ PIKA_TERMINAL, .data.str = "_" };
    g_omni_rules[R_SYM_AT] = (PikaRule){ PIKA_TERMINAL, .data.str = "@" };
    g_omni_rules[R_SYM_PERCENT] = (PikaRule){ PIKA_TERMINAL, .data.str = "%" };
    g_omni_rules[R_SYM_AMP] = (PikaRule){ PIKA_TERMINAL, .data.str = "&" };

    /* R_SYM_SPECIAL: ALT of all special chars */
    g_omni_rules[R_SYM_SPECIAL] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(13,
            R_SYM_PLUS, R_SYM_MINUS, R_SYM_STAR, R_SYM_SLASH,
            R_SYM_EQ, R_SYM_LT, R_SYM_GT, R_SYM_BANG,
            R_SYM_QMARK, R_SYM_UNDER, R_SYM_AT, R_SYM_PERCENT, R_SYM_AMP), 13 }
    };

    /* Symbol initial: alpha or special chars */
    g_omni_rules[R_SYM_INIT] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_ALPHA, R_SYM_SPECIAL), 2 }
    };

    /* Symbol continuation char: alpha, digit, or special */
    g_omni_rules[R_SYM_CHAR] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(3, R_ALPHA, R_DIGIT, R_SYM_SPECIAL), 3 }
    };

    /* R_SYM_CONT: zero or more continuation chars */
    g_omni_rules[R_SYM_CONT] = (PikaRule){
        PIKA_REP,
        .data.children = { rule_ids(1, R_SYM_CHAR), 1 }
    };

    /* Symbol: initial char followed by zero or more continuation chars */
    g_omni_rules[R_SYM] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_SYM_INIT, R_SYM_CONT), 2 },
        .action = act_sym
    };

    /* Colon-quoted symbol: : followed by symbol - reader sugar for (quote sym) */
    g_omni_rules[R_COLON] = (PikaRule){ PIKA_TERMINAL, .data.str = ":" };
    g_omni_rules[R_COLON_SYM] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_COLON, R_SYM), 2 },
        .action = act_colon_quoted
    };

    /* Delimiters */
    g_omni_rules[R_LPAREN] = (PikaRule){ PIKA_TERMINAL, .data.str = "(" };
    g_omni_rules[R_RPAREN] = (PikaRule){ PIKA_TERMINAL, .data.str = ")" };
    g_omni_rules[R_LBRACKET] = (PikaRule){ PIKA_TERMINAL, .data.str = "[" };
    g_omni_rules[R_RBRACKET] = (PikaRule){ PIKA_TERMINAL, .data.str = "]" };
    g_omni_rules[R_LBRACE] = (PikaRule){ PIKA_TERMINAL, .data.str = "{" };
    g_omni_rules[R_RBRACE] = (PikaRule){ PIKA_TERMINAL, .data.str = "}" };
    g_omni_rules[R_HASHBRACE] = (PikaRule){ PIKA_TERMINAL, .data.str = "#{" };
    g_omni_rules[R_CARET] = (PikaRule){ PIKA_TERMINAL, .data.str = "^" };
    g_omni_rules[R_DOTDOT] = (PikaRule){ PIKA_TERMINAL, .data.str = ".." };
    g_omni_rules[R_COLONWHEN] = (PikaRule){ PIKA_TERMINAL, .data.str = ":when" };

    /* String */
    g_omni_rules[R_DQUOTE] = (PikaRule){ PIKA_TERMINAL, .data.str = "\"" };
    g_omni_rules[R_BACKSLASH] = (PikaRule){ PIKA_TERMINAL, .data.str = "\\" };

    /* Escape sequence characters */
    g_omni_rules[R_ESC_N] = (PikaRule){ PIKA_TERMINAL, .data.str = "n" };
    g_omni_rules[R_ESC_T] = (PikaRule){ PIKA_TERMINAL, .data.str = "t" };
    g_omni_rules[R_ESC_R] = (PikaRule){ PIKA_TERMINAL, .data.str = "r" };
    g_omni_rules[R_ESC_QUOTE] = (PikaRule){ PIKA_TERMINAL, .data.str = "\"" };
    g_omni_rules[R_ESC_BSLASH] = (PikaRule){ PIKA_TERMINAL, .data.str = "\\" };

    /* ALT of escape chars */
    g_omni_rules[R_ESC_CHAR] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(5, R_ESC_N, R_ESC_T, R_ESC_R, R_ESC_QUOTE, R_ESC_BSLASH), 5 }
    };

    /* Escape sequence: backslash followed by escape char */
    g_omni_rules[R_ESCAPE_SEQ] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_BACKSLASH, R_ESC_CHAR), 2 }
    };

    /* Regular string char: any char that's not " or \ */
    /* We use PIKA_NOT for " and \ followed by ANY */
    /* Actually, we need to match any single char that isn't special */
    /* For simplicity, use a range that covers printable chars except " and \ */
    /* This is tricky in PEG - we'll use a NOT lookahead approach */
    /* NOT(" or \) followed by ANY */

    /* Regular string char: NOT(" or \) followed by ANY
     * In PEG: (!'"' !'\' .) but we need to combine the NOT checks
     * For R_STRING_REGULAR, we use SEQ(NOT(DQUOTE), NOT(BACKSLASH), ANY) */
    g_omni_rules[R_STRING_REGULAR] = (PikaRule){
        PIKA_RANGE,
        .data.range = {' ', '~'}  /* Printable ASCII - will be filtered by NOT checks */
    };

    /* String char: either escape sequence or regular char (not " or \) */
    /* Use SEQ(NOT(DQUOTE), NOT(BACKSLASH), ANY) for regular chars */
    g_omni_rules[R_STRING_CHAR] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_ESCAPE_SEQ, R_STRING_REGULAR), 2 }
    };

    /* String inner: zero or more string chars */
    g_omni_rules[R_STRING_INNER] = (PikaRule){
        PIKA_REP,
        .data.children = { rule_ids(1, R_STRING_CHAR), 1 }
    };

    /* String: " inner " */
    g_omni_rules[R_STRING] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(3, R_DQUOTE, R_STRING_INNER, R_DQUOTE), 3 },
        .action = act_string
    };

    /* Character literal */
    /* Backslash for char literals (reuses R_BACKSLASH from strings) */
    g_omni_rules[R_CHAR_BACKSLASH] = (PikaRule){ PIKA_TERMINAL, .data.str = "\\" };

    /* Named character literals */
    g_omni_rules[R_CHAR_NEWLINE] = (PikaRule){ PIKA_TERMINAL, .data.str = "newline" };
    g_omni_rules[R_CHAR_TAB_NAME] = (PikaRule){ PIKA_TERMINAL, .data.str = "tab" };
    g_omni_rules[R_CHAR_SPACE_NAME] = (PikaRule){ PIKA_TERMINAL, .data.str = "space" };
    g_omni_rules[R_CHAR_RETURN] = (PikaRule){ PIKA_TERMINAL, .data.str = "return" };

    /* ALT of named chars */
    g_omni_rules[R_CHAR_NAMED] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(4, R_CHAR_NEWLINE, R_CHAR_TAB_NAME, R_CHAR_SPACE_NAME, R_CHAR_RETURN), 4 }
    };

    /* Simple char: any single character (using PIKA_ANY) */
    g_omni_rules[R_CHAR_SIMPLE] = (PikaRule){ PIKA_ANY };

    /* Char literal: backslash followed by (named char OR simple char)
     * In PEG: ALT has priority, so named is tried first, then simple if named fails.
     * We need: SEQ(backslash, ALT(named, simple))
     * Create a helper rule for the ALT part */
    /* For simplicity: \ + ALT(named, simple)
     * Use R_CHAR_NAMED | R_CHAR_SIMPLE as the body */
    g_omni_rules[R_CHAR_LITERAL] = (PikaRule){
        PIKA_SEQ,
        .data.children = { NULL, 0 }  /* Will set up below */
    };
    /* We need to create the ALT inline or as a separate rule
     * For now, just use simple char */
    g_omni_rules[R_CHAR_LITERAL].data.children.subrules = rule_ids(2, R_CHAR_BACKSLASH, R_CHAR_SIMPLE);
    g_omni_rules[R_CHAR_LITERAL].data.children.count = 2;

    /* List inner sequence for right recursion */
    /* LIST_REC = SEQ(EXPR, SKIP, LIST_INNER) */
    /* LIST_INNER = ALT(LIST_REC, EPSILON) */
    static int list_rec_ids[] = {R_EXPR, R_SKIP, R_LIST_INNER};
    g_omni_rules[R_LIST_INNER] = (PikaRule){
        PIKA_ALT,
        .data.children = { NULL, 0 },  /* Will set up below */
        .action = act_list_inner
    };

    /* List: ( skip inner skip ) */
    g_omni_rules[R_LIST] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(5, R_LPAREN, R_SKIP, R_LIST_INNER, R_SKIP, R_RPAREN), 5 },
        .action = act_list
    };

    /* Slot inner (same structure as list inner) */
    g_omni_rules[R_SLOT_INNER] = (PikaRule){
        PIKA_ALT,
        .data.children = { NULL, 0 },
        .action = act_slot_inner
    };

    /* Slot: [ skip inner skip ] */
    g_omni_rules[R_SLOT] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(5, R_LBRACKET, R_SKIP, R_SLOT_INNER, R_SKIP, R_RBRACKET), 5 },
        .action = act_slot
    };

    /* Type inner */
    g_omni_rules[R_TYPE_INNER] = (PikaRule){
        PIKA_ALT,
        .data.children = { NULL, 0 },
        .action = act_type_inner
    };

    /* Type: { skip inner skip } */
    g_omni_rules[R_TYPE] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(5, R_LBRACE, R_SKIP, R_TYPE_INNER, R_SKIP, R_RBRACE), 5 },
        .action = act_type
    };

    /* Dict inner */
    g_omni_rules[R_DICT_INNER] = (PikaRule){
        PIKA_ALT,
        .data.children = { NULL, 0 },
        .action = act_dict_inner
    };

    /* Dict: #{ skip inner skip } */
    g_omni_rules[R_DICT] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(5, R_HASHBRACE, R_SKIP, R_DICT_INNER, R_SKIP, R_RBRACE), 5 },
        .action = act_dict
    };

    /* Metadata */
    /* Meta key: ^:sym form - shorthand for ^{:sym true} */
    g_omni_rules[R_META_KEY] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_CARET, R_COLON_SYM), 2 }
    };

    /* Meta: ^ followed by type-annotation or colon-sym
     * R_META = ALT(META_KEY, SEQ(CARET, TYPE)) */
    g_omni_rules[R_META] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_META_KEY, R_TYPE), 2 }
    };
    /* Note: R_TYPE handles {map} syntax, but meta would be ^{map}
     * For proper ^{...} we need: SEQ(CARET, TYPE)
     * Simpler for now: META = SEQ(CARET, COLON_SYM) just handles ^:key form */
    g_omni_rules[R_META] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_CARET, R_COLON_SYM), 2 }
    };

    /* Guard: :when <expr> - used in pattern matching for conditional guards */
    g_omni_rules[R_GUARD] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(3, R_COLONWHEN, R_SKIP, R_EXPR), 3 },
        .action = act_guard
    };

    /* Spread: .. <name> - captures rest of list in patterns like [h .. t] */
    g_omni_rules[R_SPREAD] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(3, R_DOTDOT, R_SKIP, R_SYM), 3 },
        .action = act_spread
    };

    /* Expression: ALT of all expression forms
     * Order matters for PEG - longer/more specific first
     * GUARD (:when) must come before colon-quoted (both start with :)
     * META must come before TYPE (META starts with ^) */
    g_omni_rules[R_EXPR] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(12,
            R_DICT, R_LIST, R_SLOT, R_TYPE,        /* Compound forms */
            R_META,                                 /* Metadata ^:key */
            R_GUARD,                               /* Guard :when */
            R_SPREAD,                              /* Spread .. */
            R_STRING, R_FLOAT, R_INT,              /* Literals (float before int) */
            R_COLON_SYM, R_SYM), 12 },               /* Colon-quoted before plain sym */
        .action = act_expr
    };

    /* Now set up the recursive rules */

    /* LIST_REC_RULE for list inner recursion */
    /* We need a separate rule for the sequence part */
    /* For now, approximate with the ALT directly referencing itself */

    /* Create the sequence rules for recursion */
    /* We use a trick: the INNER rules start with an EXPR */

    /* Update LIST_INNER to be: ALT(SEQ(EXPR, SKIP, LIST_INNER), EPSILON) */
    /* This requires creating the SEQ as a separate rule... */

    /* Actually, let's define a helper rule for the list element sequence */
    /* For the demo, keep the recursive structure in the action */

    /* Set up LIST_INNER as: matches EXPR followed by more, or epsilon */
    /* The action handles the recursion */
    g_omni_rules[R_LIST_INNER].data.children.subrules = rule_ids(2, R_EXPR, R_EPSILON);
    g_omni_rules[R_LIST_INNER].data.children.count = 2;

    /* Similarly for SLOT_INNER */
    g_omni_rules[R_SLOT_INNER].data.children.subrules = rule_ids(2, R_EXPR, R_EPSILON);
    g_omni_rules[R_SLOT_INNER].data.children.count = 2;

    /* And TYPE_INNER */
    g_omni_rules[R_TYPE_INNER].data.children.subrules = rule_ids(2, R_EXPR, R_EPSILON);
    g_omni_rules[R_TYPE_INNER].data.children.count = 2;

    /* And DICT_INNER */
    g_omni_rules[R_DICT_INNER].data.children.subrules = rule_ids(2, R_EXPR, R_EPSILON);
    g_omni_rules[R_DICT_INNER].data.children.count = 2;

    /* Program inner */
    g_omni_rules[R_PROGRAM_INNER] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_EXPR, R_EPSILON), 2 },
        .action = act_program_inner
    };

    /* Program: skip program_inner */
    g_omni_rules[R_PROGRAM] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_SKIP, R_PROGRAM_INNER), 2 },
        .action = act_program
    };

    /* ============== Enhanced Number Parsing ============== */

    /* R_OPT_SIGN: Optional +/- prefix */
    g_omni_rules[R_OPT_SIGN] = (PikaRule){
        PIKA_OPT,
        .data.children = { rule_ids(1, R_SYM_MINUS), 1 }
    };
    /* Update to include + as well */
    g_omni_rules[R_OPT_SIGN] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_SYM_PLUS, R_SYM_MINUS), 2 }
    };

    /* R_OPT_INT: Optional integer (digits) */
    g_omni_rules[R_OPT_INT] = (PikaRule){
        PIKA_OPT,
        .data.children = { rule_ids(1, R_DIGITS), 1 }
    };

    /* R_SIGNED_INT: [+-]? digits */
    g_omni_rules[R_SIGNED_INT] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_OPT_SIGN, R_DIGITS), 2 },
        .action = act_signed_int
    };

    /* R_FLOAT_FULL: [+-]? digits . digits (e.g., 3.14, -2.5) */
    g_omni_rules[R_FLOAT_FULL] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(4, R_OPT_SIGN, R_DIGITS, R_DOT, R_DIGITS), 4 },
        .action = act_float
    };

    /* R_FLOAT_LEAD: [+-]? . digits (e.g., .5, -.25) */
    g_omni_rules[R_FLOAT_LEAD] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(3, R_OPT_SIGN, R_DOT, R_DIGITS), 3 },
        .action = act_float
    };

    /* R_FLOAT_TRAIL: [+-]? digits . (e.g., 3., -5.) */
    g_omni_rules[R_FLOAT_TRAIL] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(3, R_OPT_SIGN, R_DIGITS, R_DOT), 3 },
        .action = act_float
    };

    /* R_ANY_FLOAT: ALT of all float forms (FULL first for longest match) */
    g_omni_rules[R_ANY_FLOAT] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(3, R_FLOAT_FULL, R_FLOAT_LEAD, R_FLOAT_TRAIL), 3 }
    };

    /* ============== Quote/Quasiquote ============== */

    /* Quote characters as terminals */
    g_omni_rules[R_QUOTE_CHAR] = (PikaRule){ PIKA_TERMINAL, .data.str = "'" };
    g_omni_rules[R_QUASIQUOTE_CHAR] = (PikaRule){ PIKA_TERMINAL, .data.str = "`" };
    g_omni_rules[R_UNQUOTE_CHAR] = (PikaRule){ PIKA_TERMINAL, .data.str = "," };

    /* R_UNQUOTE_SPLICE: ,@ (must match before bare ,) */
    g_omni_rules[R_UNQUOTE_SPLICE] = (PikaRule){ PIKA_TERMINAL, .data.str = ",@" };

    /* R_QUOTED: quote-prefix followed by expression
     * Order in ALT: ,@ before , for longest match
     * We create an ALT of quote prefixes, then SEQ with expression */
    /* Create a helper rule for the quote prefix ALT */
    static int quote_prefix_ids[] = {R_UNQUOTE_SPLICE, R_QUOTE_CHAR, R_QUASIQUOTE_CHAR, R_UNQUOTE_CHAR};
    /* Note: We need to embed this in the grammar. For now, use multi-rule approach.
     * The semantic action will check which prefix matched. */
    g_omni_rules[R_QUOTED] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(4, R_UNQUOTE_SPLICE, R_QUOTE_CHAR, R_QUASIQUOTE_CHAR, R_UNQUOTE_CHAR), 4 },
        .action = act_quoted
    };
    /* This just matches the prefix - we need SEQ(prefix, SKIP, EXPR) */
    /* Actually, the semantic action needs access to the following expression.
     * Let's make QUOTED = SEQ(ALT(prefixes), SKIP, EXPR)
     * But PEG rules can't nest ALT inside SEQ directly in our structure.
     * Solution: The act_quoted reads ahead to get the expression. */

    /* ============== Path Expressions ============== */

    /* R_PATH_SEGMENT: symbol or integer (for array indexing) */
    g_omni_rules[R_PATH_SEGMENT] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_SYM, R_INT), 2 }
    };

    /* R_PATH_TAIL_ITEM: . segment */
    g_omni_rules[R_PATH_TAIL_ITEM] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_DOT, R_PATH_SEGMENT), 2 }
    };

    /* R_PATH_TAIL: one or more .segment */
    g_omni_rules[R_PATH_TAIL] = (PikaRule){
        PIKA_POS,
        .data.children = { rule_ids(1, R_PATH_TAIL_ITEM), 1 }
    };

    /* R_PATH_ROOT: symbol or int at start of path */
    g_omni_rules[R_PATH_ROOT] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(2, R_SYM, R_INT), 2 }
    };

    /* R_PATH: root.seg1.seg2... */
    g_omni_rules[R_PATH] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_PATH_ROOT, R_PATH_TAIL), 2 },
        .action = act_path
    };

    /* ============== Set Literal ============== */

    /* R_HASHSET: #set terminal */
    g_omni_rules[R_HASHSET] = (PikaRule){ PIKA_TERMINAL, .data.str = "#set" };

    /* R_SET: #set{ ... } */
    g_omni_rules[R_SET] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(6, R_HASHSET, R_LBRACE, R_SKIP, R_SLOT_INNER, R_SKIP, R_RBRACE), 6 },
        .action = act_set
    };

    /* ============== Named Character Literal ============== */

    /* R_HASH: # terminal */
    g_omni_rules[R_HASH] = (PikaRule){ PIKA_TERMINAL, .data.str = "#" };

    /* R_NAMED_CHAR: #\ followed by name or single char
     * Uses CHAR_BACKSLASH + (CHAR_NAMED | CHAR_SIMPLE) */
    g_omni_rules[R_NAMED_CHAR] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(3, R_HASH, R_CHAR_BACKSLASH, R_SYM_CHAR), 3 },
        .action = act_named_char
    };

    /* ============== Format Strings ============== */

    /* R_HASH_FMT: #fmt terminal */
    g_omni_rules[R_HASH_FMT] = (PikaRule){ PIKA_TERMINAL, .data.str = "#fmt" };

    /* R_FMT_STRING: #fmt"..." */
    g_omni_rules[R_FMT_STRING] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_HASH_FMT, R_STRING), 2 },
        .action = act_fmt_string
    };

    /* R_HASH_CLF: #clf terminal */
    g_omni_rules[R_HASH_CLF] = (PikaRule){ PIKA_TERMINAL, .data.str = "#clf" };

    /* R_CLF_STRING: #clf"..." */
    g_omni_rules[R_CLF_STRING] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(2, R_HASH_CLF, R_STRING), 2 },
        .action = act_clf_string
    };

    /* ============== Hash-val Reader ============== */

    /* We need a terminal for "#val" - store it as static */
    static int hash_val_terminal_id = -1;
    /* R_HASH_VAL: #val followed by atom
     * Since we can't add new rule IDs dynamically, we'll use a different approach:
     * Match "#" + "val" symbol, then skip + atom */
    g_omni_rules[R_HASH_VAL] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(3, R_HASH, R_SKIP, R_ATOM), 3 },
        .action = act_hash_val
    };
    /* Note: The semantic action will verify "val" follows # */

    /* ============== Kind Splice ============== */

    /* R_HASHKIND: #kind terminal */
    g_omni_rules[R_HASHKIND] = (PikaRule){ PIKA_TERMINAL, .data.str = "#kind" };

    /* R_KIND_SPLICE: {#kind expr} */
    g_omni_rules[R_KIND_SPLICE] = (PikaRule){
        PIKA_SEQ,
        .data.children = { rule_ids(6, R_LBRACE, R_SKIP, R_HASHKIND, R_SKIP, R_EXPR, R_RBRACE), 6 },
        .action = act_kind_splice
    };

    /* ============== String Char Improvement ============== */

    /* R_NOT_DQUOTE: NOT lookahead for double quote */
    g_omni_rules[R_NOT_DQUOTE] = (PikaRule){
        PIKA_NOT,
        .data.children = { rule_ids(1, R_DQUOTE), 1 }
    };

    /* R_ANY_CHAR: matches any single character */
    g_omni_rules[R_ANY_CHAR] = (PikaRule){ PIKA_ANY };

    /* ============== Atom (basic values) ============== */

    /* R_ATOM: number, symbol, string, char literal */
    g_omni_rules[R_ATOM] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(6, R_ANY_FLOAT, R_SIGNED_INT, R_INT, R_STRING, R_COLON_SYM, R_SYM), 6 }
    };

    /* ============== Update R_EXPR to include new forms ============== */

    /* Expression: ALT of all expression forms
     * Order matters for PEG - longer/more specific first */
    g_omni_rules[R_EXPR] = (PikaRule){
        PIKA_ALT,
        .data.children = { rule_ids(19,
            /* Hash-prefixed forms (specific before general) */
            R_FMT_STRING, R_CLF_STRING,     /* #fmt"...", #clf"..." */
            R_SET,                          /* #set{...} */
            R_NAMED_CHAR,                   /* #\newline */
            R_KIND_SPLICE,                  /* {#kind expr} - before TYPE */
            R_DICT,                         /* #{...} */
            /* Quote forms */
            R_QUOTED,                       /* 'x, `x, ,x, ,@x */
            /* Path expressions (before plain symbol) */
            R_PATH,                         /* foo.bar.baz */
            /* Compound forms */
            R_LIST, R_SLOT, R_TYPE,         /* (...), [...], {...} */
            R_META,                         /* ^:key */
            R_GUARD,                        /* :when */
            R_SPREAD,                       /* .. */
            /* Literals (float before int for longest match) */
            R_ANY_FLOAT, R_SIGNED_INT, R_INT, R_STRING,
            /* Symbols (colon-quoted before plain) */
            R_COLON_SYM, R_SYM), 19 },
        .action = act_expr
    };

    g_omni_rules_init = 1;
}

/* ============== Public API ============== */

Term omni_pika_read(const char* input) {
    if (!input) {
        u32 err_nick = omni_nick("Err");
        return mk_ctr0(err_nick);
    }

    omni_pika_init();

    PikaState* state = pika_new(input, g_omni_rules, NUM_RULES);
    if (!state) {
        u32 err_nick = omni_nick("Err");
        return mk_ctr0(err_nick);
    }

    Term result = pika_run(state, R_PROGRAM);
    pika_free(state);
    return result;
}

Term omni_pika_read_expr(const char* input) {
    if (!input) {
        u32 err_nick = omni_nick("Err");
        return mk_ctr0(err_nick);
    }

    omni_pika_init();

    PikaState* state = pika_new(input, g_omni_rules, NUM_RULES);
    if (!state) {
        u32 err_nick = omni_nick("Err");
        return mk_ctr0(err_nick);
    }

    Term result = pika_run(state, R_EXPR);
    pika_free(state);
    return result;
}

/* Reset binding stack (call between parses if needed) */
void omni_pika_reset_bindings(void) {
    g_bind_len = 0;
}
