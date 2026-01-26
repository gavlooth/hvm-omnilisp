/*
 * Pika Parser Core Implementation for HVM-OmniLisp
 *
 * Based on the Pika parsing algorithm - a packrat PEG parser.
 * Uses right-to-left pass with memoization for O(n) parsing.
 *
 * Adapted to generate HVM4 terms instead of OmniValue*.
 */

#include "pika.h"
#include <stdlib.h>
#include <string.h>

/* ============== Pattern Cache ============== */

typedef struct PatternCacheEntry {
    char* pattern;
    size_t rules_hash;
    PikaState* compiled_state;
    struct PatternCacheEntry* next;
} PatternCacheEntry;

typedef struct PatternCache {
    PatternCacheEntry** buckets;
    size_t bucket_count;
    size_t entry_count;
} PatternCache;

static PatternCache* g_pattern_cache = NULL;

#define PATTERN_CACHE_INITIAL_BUCKETS 32

/* Simple hash function for strings */
static size_t hash_string(const char* str) {
    if (!str) return 0;
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/* Compute hash for rules array */
static size_t hash_rules(PikaRule* rules, int num_rules) {
    if (!rules || num_rules <= 0) return 0;
    size_t hash = 0;
    for (int i = 0; i < num_rules; i++) {
        hash = (hash << 5) + hash + (size_t)rules[i].type;
        switch (rules[i].type) {
            case PIKA_TERMINAL:
                if (rules[i].data.str) {
                    hash = (hash << 5) + hash + hash_string(rules[i].data.str);
                }
                break;
            case PIKA_RANGE:
                hash = (hash << 5) + hash + rules[i].data.range.min;
                hash = (hash << 5) + hash + rules[i].data.range.max;
                break;
            case PIKA_SEQ:
            case PIKA_ALT:
                hash = (hash << 5) + hash + rules[i].data.children.count;
                break;
            case PIKA_REF:
                hash = (hash << 5) + hash + rules[i].data.ref.subrule;
                break;
            default:
                break;
        }
    }
    return hash;
}

static void pattern_cache_init(void) {
    if (g_pattern_cache) return;
    g_pattern_cache = malloc(sizeof(PatternCache));
    if (!g_pattern_cache) return;
    g_pattern_cache->bucket_count = PATTERN_CACHE_INITIAL_BUCKETS;
    g_pattern_cache->entry_count = 0;
    g_pattern_cache->buckets = calloc(PATTERN_CACHE_INITIAL_BUCKETS, sizeof(PatternCacheEntry*));
    if (!g_pattern_cache->buckets) {
        free(g_pattern_cache);
        g_pattern_cache = NULL;
    }
}

static void pattern_cache_cleanup(void) {
    if (!g_pattern_cache) return;
    for (size_t i = 0; i < g_pattern_cache->bucket_count; i++) {
        PatternCacheEntry* entry = g_pattern_cache->buckets[i];
        while (entry) {
            PatternCacheEntry* next = entry->next;
            free(entry->pattern);
            if (entry->compiled_state) {
                pika_free(entry->compiled_state);
            }
            free(entry);
            entry = next;
        }
        g_pattern_cache->buckets[i] = NULL;
    }
    free(g_pattern_cache->buckets);
    free(g_pattern_cache);
    g_pattern_cache = NULL;
}

static PikaState* pattern_cache_get(const char* pattern, PikaRule* rules, int num_rules) {
    if (!g_pattern_cache || !pattern || !rules || num_rules <= 0) {
        return NULL;
    }
    size_t pattern_hash = hash_string(pattern);
    size_t rules_hash_val = hash_rules(rules, num_rules);
    size_t combined_hash = pattern_hash ^ rules_hash_val;
    size_t bucket = combined_hash % g_pattern_cache->bucket_count;

    PatternCacheEntry* entry = g_pattern_cache->buckets[bucket];
    while (entry) {
        if (entry->rules_hash == rules_hash_val &&
            strcmp(entry->pattern, pattern) == 0) {
            return entry->compiled_state;
        }
        entry = entry->next;
    }
    return NULL;
}

static void pattern_cache_put(const char* pattern, PikaRule* rules, int num_rules, PikaState* compiled) {
    if (!g_pattern_cache || !pattern || !rules || num_rules <= 0 || !compiled) {
        return;
    }
    size_t pattern_hash = hash_string(pattern);
    size_t rules_hash_val = hash_rules(rules, num_rules);
    size_t combined_hash = pattern_hash ^ rules_hash_val;
    size_t bucket = combined_hash % g_pattern_cache->bucket_count;

    PatternCacheEntry* entry = malloc(sizeof(PatternCacheEntry));
    if (!entry) return;

    entry->pattern = strdup(pattern);
    if (!entry->pattern) {
        free(entry);
        return;
    }
    entry->rules_hash = rules_hash_val;
    entry->compiled_state = compiled;
    entry->next = g_pattern_cache->buckets[bucket];
    g_pattern_cache->buckets[bucket] = entry;
    g_pattern_cache->entry_count++;
}

void pika_pattern_cache_clear(void) {
    pattern_cache_cleanup();
}

void pika_pattern_cache_stats(PatternCacheStats* stats) {
    if (!stats) return;
    if (!g_pattern_cache) {
        stats->entry_count = 0;
        stats->bucket_count = 0;
        return;
    }
    stats->entry_count = g_pattern_cache->entry_count;
    stats->bucket_count = g_pattern_cache->bucket_count;
}

/* ============== Core Parser ============== */

PikaState* pika_new(const char* input, PikaRule* rules, int num_rules) {
    PikaState* state = malloc(sizeof(PikaState));
    if (!state) return NULL;

    state->input = input;
    state->input_len = strlen(input);
    state->num_rules = num_rules;
    state->rules = rules;
    state->output_mode = PIKA_OUTPUT_AST;

    size_t table_size = (state->input_len + 1) * num_rules;
    state->table = calloc(table_size, sizeof(PikaMatch));
    if (!state->table) {
        free(state);
        return NULL;
    }

    return state;
}

void pika_free(PikaState* state) {
    if (!state) return;
    if (state->table) free(state->table);
    free(state);
}

void pika_set_output_mode(PikaState* state, PikaOutputMode mode) {
    if (!state) return;
    state->output_mode = mode;
}

PikaMatch* pika_get_match(PikaState* state, size_t pos, int rule_id) {
    if (pos > state->input_len || rule_id < 0 || rule_id >= state->num_rules) return NULL;
    return &state->table[pos * state->num_rules + rule_id];
}

static inline PikaMatch* get_match(PikaState* state, size_t pos, int rule_id) {
    return pika_get_match(state, pos, rule_id);
}

static PikaMatch evaluate_rule(PikaState* state, size_t pos, int rule_id) {
    PikaRule* rule = &state->rules[rule_id];
    PikaMatch m = {false, 0, 0};

    switch (rule->type) {
        case PIKA_TERMINAL: {
            if (!rule->data.str) break;
            size_t len = strlen(rule->data.str);
            if (pos + len <= state->input_len &&
                strncmp(state->input + pos, rule->data.str, len) == 0) {
                m.matched = true;
                m.len = len;
            }
            break;
        }

        case PIKA_RANGE: {
            if (pos < state->input_len) {
                char c = state->input[pos];
                if (c >= rule->data.range.min && c <= rule->data.range.max) {
                    m.matched = true;
                    m.len = 1;
                }
            }
            break;
        }

        case PIKA_ANY: {
            if (pos < state->input_len) {
                m.matched = true;
                m.len = 1;
            }
            break;
        }

        case PIKA_SEQ: {
            size_t current_pos = pos;
            bool all_matched = true;
            for (int i = 0; i < rule->data.children.count; i++) {
                PikaMatch* sub = get_match(state, current_pos, rule->data.children.subrules[i]);
                if (!sub || !sub->matched) {
                    all_matched = false;
                    break;
                }
                current_pos += sub->len;
            }
            if (all_matched) {
                m.matched = true;
                m.len = current_pos - pos;
            }
            break;
        }

        case PIKA_ALT: {
            /* PEG Prioritized Choice: First one that matches wins */
            for (int i = 0; i < rule->data.children.count; i++) {
                PikaMatch* sub = get_match(state, pos, rule->data.children.subrules[i]);
                if (sub && sub->matched) {
                    m = *sub;
                    break;
                }
            }
            break;
        }

        case PIKA_REP: {
            /* Zero or more: A* */
            PikaMatch* first = get_match(state, pos, rule->data.children.subrules[0]);
            if (first && first->matched && first->len > 0) {
                PikaMatch* rest = get_match(state, pos + first->len, rule_id);
                if (rest && rest->matched) {
                    m.matched = true;
                    m.len = first->len + rest->len;
                } else {
                    m = *first;
                }
            } else {
                /* Match empty */
                m.matched = true;
                m.len = 0;
            }
            break;
        }

        case PIKA_POS: {
            /* One or more: A+ */
            PikaMatch* first = get_match(state, pos, rule->data.children.subrules[0]);
            if (first && first->matched) {
                m.matched = true;
                m.len = first->len;
                if (pos + first->len <= state->input_len) {
                    PikaMatch* more = get_match(state, pos + first->len, rule_id);
                    if (more && more->matched) {
                        m.len += more->len;
                    }
                }
            }
            break;
        }

        case PIKA_OPT: {
            PikaMatch* sub = get_match(state, pos, rule->data.children.subrules[0]);
            if (sub && sub->matched) {
                m = *sub;
            } else {
                m.matched = true;
                m.len = 0;
            }
            break;
        }

        case PIKA_NOT: {
            PikaMatch* sub = get_match(state, pos, rule->data.children.subrules[0]);
            if (!sub || !sub->matched) {
                m.matched = true;
                m.len = 0;
            }
            break;
        }

        case PIKA_AND: {
            PikaMatch* sub = get_match(state, pos, rule->data.children.subrules[0]);
            if (sub && sub->matched) {
                m.matched = true;
                m.len = 0;
            }
            break;
        }

        case PIKA_REF: {
            PikaMatch* sub = get_match(state, pos, rule->data.ref.subrule);
            if (sub) m = *sub;
            break;
        }
    }

    return m;
}

Term pika_run(PikaState* state, int root_rule_id) {
    /* Right-to-Left Pass with fixpoint iteration */
    for (ptrdiff_t pos = (ptrdiff_t)state->input_len; pos >= 0; pos--) {
        bool changed = true;
        int fixpoint_limit = state->num_rules * 2;
        int iters = 0;

        while (changed && iters < fixpoint_limit) {
            changed = false;
            iters++;

            for (int r = 0; r < state->num_rules; r++) {
                PikaMatch result = evaluate_rule(state, (size_t)pos, r);
                PikaMatch* existing = get_match(state, (size_t)pos, r);

                /*
                 * Value propagation for non-action rules.
                 * Wrapper rules without actions need their child values.
                 */
                bool val_changed = false;
                if (state->output_mode == PIKA_OUTPUT_AST &&
                    result.matched &&
                    state->rules[r].action == NULL &&
                    result.val != existing->val) {
                    val_changed = true;
                }

                if (result.matched != existing->matched || result.len != existing->len || val_changed) {
                    *existing = result;
                    /* Only run semantic actions in AST mode */
                    if (result.matched && state->rules[r].action && state->output_mode == PIKA_OUTPUT_AST) {
                        existing->val = state->rules[r].action(state, (size_t)pos, result);
                    }
                    changed = true;
                }
            }
        }
    }

    /*
     * Semantic stabilization pass (AST mode only)
     * Re-run all semantic actions after structural convergence.
     */
    if (state->output_mode == PIKA_OUTPUT_AST) {
        for (ptrdiff_t pos = (ptrdiff_t)state->input_len; pos >= 0; pos--) {
            for (int r = 0; r < state->num_rules; r++) {
                PikaMatch* m = get_match(state, (size_t)pos, r);
                if (!m || !m->matched) continue;
                if (!state->rules[r].action) continue;
                m->val = state->rules[r].action(state, (size_t)pos, *m);
            }
        }
    }

    PikaMatch* root = get_match(state, 0, root_rule_id);
    if (root && root->matched) {
        /* In STRING mode, return raw matched text as string */
        if (state->output_mode == PIKA_OUTPUT_STRING) {
            /* Build a cons-list of characters */
            Term result = term_new_ctr(NAM_NIL, 0, NULL);
            for (size_t i = root->len; i > 0; i--) {
                unsigned char c = state->input[i - 1];
                Term chr = term_new_ctr(NAM_CHR, 1, (Term[]){term_new_num(c)});
                result = term_new_ctr(NAM_CON, 2, (Term[]){chr, result});
            }
            return result;
        }

        /* In AST mode (default), return processed AST node */
        if (root->val) return root->val;

        /* Fallback: return matched text as symbol if no action */
        char* s = malloc(root->len + 1);
        memcpy(s, state->input, root->len);
        s[root->len] = '\0';
        /* Create symbol from text - nick encode first 4 chars */
        u32 k = 0;
        for (u32 i = 0; s[i] != '\0' && i < 4; i++) {
            k = ((k << 6) + nick_letter_to_b64(s[i])) & EXT_MASK;
        }
        free(s);
        return term_new_ctr(NAM_SYM, 1, (Term[]){term_new_num(k)});
    }

    /* Parse failed - return error */
    u32 err_nick = ((u32)'E' << 18) | ((u32)'r' << 12) | ((u32)'r' << 6);
    return term_new_ctr(err_nick, 0, NULL);
}

Term pika_match(const char* input, PikaRule* rules, int num_rules, int root_rule) {
    if (!input) {
        u32 err_nick = ((u32)'E' << 18) | ((u32)'r' << 12) | ((u32)'r' << 6);
        return term_new_ctr(err_nick, 0, NULL);
    }
    if (!rules || num_rules <= 0) {
        u32 err_nick = ((u32)'E' << 18) | ((u32)'r' << 12) | ((u32)'r' << 6);
        return term_new_ctr(err_nick, 0, NULL);
    }
    if (root_rule < 0 || root_rule >= num_rules) {
        u32 err_nick = ((u32)'E' << 18) | ((u32)'r' << 12) | ((u32)'r' << 6);
        return term_new_ctr(err_nick, 0, NULL);
    }

    PikaState* state = pika_new(input, rules, num_rules);
    if (!state) {
        u32 err_nick = ((u32)'E' << 18) | ((u32)'r' << 12) | ((u32)'r' << 6);
        return term_new_ctr(err_nick, 0, NULL);
    }

    Term result = pika_run(state, root_rule);
    pika_free(state);
    return result;
}

PikaState* pika_compile_pattern(const char* pattern, PikaRule* rules, int num_rules) {
    if (!pattern) return NULL;
    if (!rules || num_rules <= 0) return NULL;

    pattern_cache_init();

    PikaState* cached = pattern_cache_get(pattern, rules, num_rules);
    if (cached) {
        return cached;
    }

    PikaState* state = pika_new(pattern, rules, num_rules);
    if (!state) return NULL;

    pattern_cache_put(pattern, rules, num_rules, state);
    return state;
}
