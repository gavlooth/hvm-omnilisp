/*
 * Pika Parser - Packrat PEG Parser for HVM-OmniLisp
 *
 * Based on the Pika parsing algorithm.
 * Adapted for HVM4 term generation.
 *
 * Original: OmniLisp/csrc/parser/pika.h
 */

#ifndef PIKA_PARSER_H
#define PIKA_PARSER_H

// hvm4.c is already included by main.c before this file
// #include "../../../hvm4/clang/hvm4.c"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct PikaState;
struct PikaMatch;

/*
 * Match result
 * - matched: whether the rule matched at this position
 * - len: number of characters consumed
 * - val: cached HVM4 Term (result of semantic action)
 */
typedef struct PikaMatch {
    bool matched;
    size_t len;
    Term val;  /* Cached HVM4 term */
} PikaMatch;

/* Semantic action callback - returns HVM4 Term */
typedef Term (*PikaActionFn)(struct PikaState* state, size_t pos, PikaMatch match);

/* Output mode for parser */
typedef enum {
    PIKA_OUTPUT_AST,      /* Default: Return processed AST nodes (via semantic actions) */
    PIKA_OUTPUT_STRING    /* Return raw matched text as string term */
} PikaOutputMode;

/* Rule types (PEG operators) */
typedef enum {
    PIKA_TERMINAL,  /* Literal string */
    PIKA_RANGE,     /* Character range [a-z] */
    PIKA_ANY,       /* Any character (.) */
    PIKA_SEQ,       /* Sequence (A B C) */
    PIKA_ALT,       /* Prioritized Choice (A / B / C) */
    PIKA_REP,       /* Zero-or-more (A*) */
    PIKA_POS,       /* One-or-more (A+) */
    PIKA_OPT,       /* Optional (A?) */
    PIKA_NOT,       /* Negative lookahead (!A) */
    PIKA_AND,       /* Positive lookahead (&A) */
    PIKA_REF        /* Reference to another rule (by ID) */
} PikaRuleType;

/*
 * Rule definition
 * Each rule has a type and associated data depending on the type.
 */
typedef struct PikaRule {
    PikaRuleType type;
    union {
        const char* str;                           /* PIKA_TERMINAL: literal string */
        struct { char min; char max; } range;      /* PIKA_RANGE: character range */
        struct { int* subrules; int count; } children;  /* PIKA_SEQ/ALT/REP/POS/OPT/NOT/AND */
        struct { int subrule; } ref;               /* PIKA_REF: reference to another rule */
    } data;
    const char* name;       /* Optional name for debugging */
    PikaActionFn action;    /* Semantic action (may be NULL) */
} PikaRule;

/*
 * Parser state
 * Contains input, rules, and memoization table.
 */
typedef struct PikaState {
    const char* input;      /* Input string to parse */
    size_t input_len;       /* Length of input */

    int num_rules;          /* Number of rules in grammar */
    PikaRule* rules;        /* Array of rule definitions */

    PikaOutputMode output_mode;  /* AST or STRING mode */

    /* Memoization table: [input_len + 1][num_rules] */
    PikaMatch* table;
} PikaState;

/* ============== Public API ============== */

/* Create a new parser state */
PikaState* pika_new(const char* input, PikaRule* rules, int num_rules);

/* Free parser state */
void pika_free(PikaState* state);

/* Set the output mode for the parser */
void pika_set_output_mode(PikaState* state, PikaOutputMode mode);

/* Run the parser and return the result of the root rule at position 0 */
Term pika_run(PikaState* state, int root_rule_id);

/* Get match at position for rule (useful for semantic actions) */
PikaMatch* pika_get_match(PikaState* state, size_t pos, int rule_id);

/*
 * Convenience function: Run pattern matching in one call.
 * Creates PikaState, runs parser, returns result, frees state.
 *
 * Parameters:
 *   - input: Input string to parse
 *   - rules: Array of PikaRule definitions
 *   - num_rules: Number of rules in the array
 *   - root_rule: Index of the rule to use as root
 *
 * Returns:
 *   - HVM4 Term representing the parse result
 *   - Error term if parsing failed
 */
Term pika_match(const char* input, PikaRule* rules, int num_rules, int root_rule);

/*
 * Compile a pattern string for later use.
 * Creates a parser state and returns it (may be cached).
 *
 * Parameters:
 *   - pattern: Pattern string to compile
 *   - rules: Array of PikaRule definitions
 *   - num_rules: Number of rules in the array
 *
 * Returns:
 *   - PikaState* ready for matching
 *   - NULL if allocation failed
 */
PikaState* pika_compile_pattern(const char* pattern, PikaRule* rules, int num_rules);

/* ============== Pattern Cache API ============== */

typedef struct {
    size_t entry_count;  /* Number of cached patterns */
    size_t bucket_count; /* Number of buckets in hash table */
} PatternCacheStats;

/* Clear the pattern cache */
void pika_pattern_cache_clear(void);

/* Get pattern cache statistics */
void pika_pattern_cache_stats(PatternCacheStats* stats);

/* ============== Helper Macros for Rule Definition ============== */

/* Terminal (literal string) */
#define PIKA_T(str) (PikaRule){ PIKA_TERMINAL, .data.str = (str) }

/* Character range */
#define PIKA_R(min, max) (PikaRule){ PIKA_RANGE, .data.range = {(min), (max)} }

/* Any character */
#define PIKA_DOT (PikaRule){ PIKA_ANY }

/* Reference to another rule */
#define PIKA_REF_(id) (PikaRule){ PIKA_REF, .data.ref.subrule = (id) }

#ifdef __cplusplus
}
#endif

#endif /* PIKA_PARSER_H */
