// OmniLisp JSON Operations
// JSON parsing and stringification via FFI
//
// JSON values are represented as:
// - JSON object -> #Dct{...} (dict)
// - JSON array  -> #CON list
// - JSON string -> #CON char list (wrapped in #Str if needed)
// - JSON number -> #Cst{n}
// - JSON true   -> #True
// - JSON false  -> #Fals
// - JSON null   -> #Noth

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Forward declarations
fn Term omni_json_parse_value(const char **p);
fn void omni_json_stringify_value(Term val, char **buf, size_t *len, size_t *cap);

// =============================================================================
// JSON Parsing Helpers
// =============================================================================

// Skip whitespace
fn void omni_json_skip_ws(const char **p) {
  while (**p && isspace((unsigned char)**p)) (*p)++;
}

// JSON parse error codes
#define OMNI_JSON_ERR_STRING   1
#define OMNI_JSON_ERR_NUMBER   2
#define OMNI_JSON_ERR_ARRAY    3
#define OMNI_JSON_ERR_OBJECT   4
#define OMNI_JSON_ERR_COLON    5
#define OMNI_JSON_ERR_EOF      6
#define OMNI_JSON_ERR_SYNTAX   7
#define OMNI_JSON_ERR_MEMORY   8

// Parse a JSON string, returning OmniLisp string (char list)
fn Term omni_json_parse_string(const char **p) {
  if (**p != '"') {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_STRING)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  (*p)++;  // Skip opening quote

  // Find end of string (handling escapes)
  const char *start = *p;
  size_t len = 0;
  while (**p && **p != '"') {
    if (**p == '\\' && (*p)[1]) {
      (*p) += 2;  // Skip escape sequence
      len++;
    } else {
      (*p)++;
      len++;
    }
  }

  if (**p != '"') {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_STRING)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  // Build char list from string content
  char *buf = (char*)malloc(len + 1);
  if (!buf) {
    Term err_args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  const char *q = start;
  size_t i = 0;
  while (q < *p) {
    if (*q == '\\' && q[1]) {
      q++;
      switch (*q) {
        case 'n': buf[i++] = '\n'; break;
        case 't': buf[i++] = '\t'; break;
        case 'r': buf[i++] = '\r'; break;
        case '\\': buf[i++] = '\\'; break;
        case '"': buf[i++] = '"'; break;
        case '/': buf[i++] = '/'; break;
        default: buf[i++] = *q; break;
      }
      q++;
    } else {
      buf[i++] = *q++;
    }
  }
  buf[i] = '\0';

  (*p)++;  // Skip closing quote

  Term result = omni_cstr_to_list(buf);
  free(buf);
  return result;
}

// Parse a JSON number
fn Term omni_json_parse_number(const char **p) {
  char *end;
  double d = strtod(*p, &end);
  if (end == *p) {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_NUMBER)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  *p = end;

  // Convert to integer if it's a whole number, otherwise use as-is
  // HVM4 only supports integers, so truncate
  int n = (int)d;
  Term num_arg[1] = {term_new_num((u32)n)};
  return term_new_ctr(OMNI_NAM_CST, 1, num_arg);
}

// Parse a JSON array - builds list in reverse then reverses
fn Term omni_json_parse_array(const char **p) {
  if (**p != '[') {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_ARRAY)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  (*p)++;  // Skip [

  omni_json_skip_ws(p);

  if (**p == ']') {
    (*p)++;
    return term_new_ctr(NAM_NIL, 0, NULL);  // Empty list
  }

  // Collect all elements into a temporary array (max 1024 elements)
  Term elements[1024];
  int count = 0;

  // Parse first element
  elements[count] = omni_json_parse_value(p);
  if (term_tag(elements[count]) >= C00 && term_ext(elements[count]) == OMNI_NAM_ERR) {
    return elements[count];  // Propagate error
  }
  count++;

  omni_json_skip_ws(p);

  // Parse remaining elements
  while (**p == ',' && count < 1024) {
    (*p)++;  // Skip comma
    omni_json_skip_ws(p);

    elements[count] = omni_json_parse_value(p);
    if (term_tag(elements[count]) >= C00 && term_ext(elements[count]) == OMNI_NAM_ERR) {
      return elements[count];  // Propagate error
    }
    count++;

    omni_json_skip_ws(p);
  }

  // Close array
  if (**p != ']') {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_ARRAY)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  (*p)++;

  // Build list from back to front (so it ends up in correct order)
  Term result = term_new_ctr(NAM_NIL, 0, NULL);
  for (int i = count - 1; i >= 0; i--) {
    Term cons_args[2] = {elements[i], result};
    result = term_new_ctr(NAM_CON, 2, cons_args);
  }

  return result;
}

// Parse a JSON object - collects all key-value pairs
fn Term omni_json_parse_object(const char **p) {
  if (**p != '{') {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_OBJECT)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  (*p)++;  // Skip {

  omni_json_skip_ws(p);

  if (**p == '}') {
    (*p)++;
    // Return empty dict: #Dict{#NIL}
    Term nil = term_new_ctr(OMNI_NAM_NIL, 0, NULL);
    Term dict_args[1] = {nil};
    return term_new_ctr(OMNI_NAM_DICT, 1, dict_args);
  }

  // Collect key-value pairs (max 256 pairs)
  Term keys[256];
  Term vals[256];
  int count = 0;

  // Parse first key-value pair
  keys[count] = omni_json_parse_string(p);
  if (term_tag(keys[count]) >= C00 && term_ext(keys[count]) == OMNI_NAM_ERR) {
    return keys[count];
  }

  omni_json_skip_ws(p);
  if (**p != ':') {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_COLON)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  (*p)++;

  omni_json_skip_ws(p);
  vals[count] = omni_json_parse_value(p);
  if (term_tag(vals[count]) >= C00 && term_ext(vals[count]) == OMNI_NAM_ERR) {
    return vals[count];
  }
  count++;

  omni_json_skip_ws(p);

  // Parse remaining key-value pairs
  while (**p == ',' && count < 256) {
    (*p)++;  // Skip comma
    omni_json_skip_ws(p);

    keys[count] = omni_json_parse_string(p);
    if (term_tag(keys[count]) >= C00 && term_ext(keys[count]) == OMNI_NAM_ERR) {
      return keys[count];
    }

    omni_json_skip_ws(p);
    if (**p != ':') {
      Term err_args[1] = {term_new_num(OMNI_JSON_ERR_COLON)};
      return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
    }
    (*p)++;

    omni_json_skip_ws(p);
    vals[count] = omni_json_parse_value(p);
    if (term_tag(vals[count]) >= C00 && term_ext(vals[count]) == OMNI_NAM_ERR) {
      return vals[count];
    }
    count++;

    omni_json_skip_ws(p);
  }

  // Close object
  if (**p != '}') {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_OBJECT)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  (*p)++;

  // Build entries list from back to front
  // Each entry is a pair: #CON{key, #CON{val, #NIL}}
  Term nil = term_new_ctr(OMNI_NAM_NIL, 0, NULL);
  Term entries = nil;

  for (int i = count - 1; i >= 0; i--) {
    // Build pair: (key val)
    Term val_cons_args[2] = {vals[i], nil};
    Term val_cons = term_new_ctr(OMNI_NAM_CON, 2, val_cons_args);
    Term pair_args[2] = {keys[i], val_cons};
    Term pair = term_new_ctr(OMNI_NAM_CON, 2, pair_args);

    // Prepend to entries
    Term entry_args[2] = {pair, entries};
    entries = term_new_ctr(OMNI_NAM_CON, 2, entry_args);
  }

  // Build the dict: #Dict{entries}
  Term dict_args[1] = {entries};
  return term_new_ctr(OMNI_NAM_DICT, 1, dict_args);
}

// Parse any JSON value
fn Term omni_json_parse_value(const char **p) {
  omni_json_skip_ws(p);

  if (!**p) {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_EOF)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  switch (**p) {
    case '"':
      return omni_json_parse_string(p);

    case '{':
      return omni_json_parse_object(p);

    case '[':
      return omni_json_parse_array(p);

    case 't':  // true
      if (strncmp(*p, "true", 4) == 0) {
        *p += 4;
        return term_new_ctr(OMNI_NAM_TRUE, 0, NULL);
      }
      break;

    case 'f':  // false
      if (strncmp(*p, "false", 5) == 0) {
        *p += 5;
        return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
      }
      break;

    case 'n':  // null
      if (strncmp(*p, "null", 4) == 0) {
        *p += 4;
        return term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
      }
      break;

    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return omni_json_parse_number(p);
  }

  Term err_args[1] = {term_new_num(OMNI_JSON_ERR_SYNTAX)};
  return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
}

// =============================================================================
// JSON Stringification Helpers
// =============================================================================

// Ensure buffer has capacity
fn void omni_json_ensure_cap(char **buf, size_t *len, size_t *cap, size_t need) {
  if (*len + need >= *cap) {
    *cap = (*cap + need) * 2;
    *buf = (char*)realloc(*buf, *cap);
  }
}

// Append string to buffer
fn void omni_json_append(char **buf, size_t *len, size_t *cap, const char *s) {
  size_t slen = strlen(s);
  omni_json_ensure_cap(buf, len, cap, slen);
  memcpy(*buf + *len, s, slen);
  *len += slen;
  (*buf)[*len] = '\0';
}

// Append char to buffer
fn void omni_json_append_char(char **buf, size_t *len, size_t *cap, char c) {
  omni_json_ensure_cap(buf, len, cap, 1);
  (*buf)[(*len)++] = c;
  (*buf)[*len] = '\0';
}

// Stringify a char list as JSON string
fn void omni_json_stringify_string(Term list, char **buf, size_t *len, size_t *cap) {
  char *s = omni_list_to_cstr(list);
  omni_json_append_char(buf, len, cap, '"');
  if (s) {
    for (const char *p = s; *p; p++) {
      switch (*p) {
        case '"': omni_json_append(buf, len, cap, "\\\""); break;
        case '\\': omni_json_append(buf, len, cap, "\\\\"); break;
        case '\n': omni_json_append(buf, len, cap, "\\n"); break;
        case '\r': omni_json_append(buf, len, cap, "\\r"); break;
        case '\t': omni_json_append(buf, len, cap, "\\t"); break;
        default: omni_json_append_char(buf, len, cap, *p); break;
      }
    }
    free(s);
  }
  omni_json_append_char(buf, len, cap, '"');
}

// Stringify any value
fn void omni_json_stringify_value(Term val, char **buf, size_t *len, size_t *cap) {
  val = wnf(val);

  // Check for NIL (empty list)
  if (term_tag(val) == C00 && term_ext(val) == NAM_NIL) {
    omni_json_append(buf, len, cap, "[]");
    return;
  }

  // Check for True
  if (term_tag(val) == C00 && term_ext(val) == OMNI_NAM_TRUE) {
    omni_json_append(buf, len, cap, "true");
    return;
  }

  // Check for False
  if (term_tag(val) == C00 && term_ext(val) == OMNI_NAM_FALS) {
    omni_json_append(buf, len, cap, "false");
    return;
  }

  // Check for Nothing (null)
  if (term_tag(val) == C00 && term_ext(val) == OMNI_NAM_NOTH) {
    omni_json_append(buf, len, cap, "null");
    return;
  }

  // Check for #Cst{n} (number)
  if (term_tag(val) == C01 && term_ext(val) == OMNI_NAM_CST) {
    Term inner = wnf(HEAP[term_val(val)]);
    char num_buf[32];
    snprintf(num_buf, sizeof(num_buf), "%d", (int)term_val(inner));
    omni_json_append(buf, len, cap, num_buf);
    return;
  }

  // Check for raw NUM
  if (term_tag(val) == NUM) {
    char num_buf[32];
    snprintf(num_buf, sizeof(num_buf), "%u", term_val(val));
    omni_json_append(buf, len, cap, num_buf);
    return;
  }

  // Check for CON (list/array)
  if (term_tag(val) == C02 && term_ext(val) == NAM_CON) {
    // Check if it's a char list (string)
    u32 loc = term_val(val);
    Term head = wnf(HEAP[loc]);
    if (term_tag(head) == C01 && term_ext(head) == OMNI_NAM_CHR) {
      // It's a string (char list)
      omni_json_stringify_string(val, buf, len, cap);
      return;
    }

    // It's an array
    omni_json_append_char(buf, len, cap, '[');
    int first = 1;
    Term cur = val;
    while (term_tag(cur) == C02 && term_ext(cur) == NAM_CON) {
      if (!first) omni_json_append_char(buf, len, cap, ',');
      first = 0;
      u32 cur_loc = term_val(cur);
      Term elem = wnf(HEAP[cur_loc]);
      omni_json_stringify_value(elem, buf, len, cap);
      cur = wnf(HEAP[cur_loc + 1]);
    }
    omni_json_append_char(buf, len, cap, ']');
    return;
  }

  // Check for #Dict{entries}
  if (term_tag(val) == C01 && term_ext(val) == OMNI_NAM_DICT) {
    Term entries = wnf(HEAP[term_val(val)]);

    omni_json_append_char(buf, len, cap, '{');
    int first = 1;
    Term cur = entries;

    // Iterate through entries list: #CON{#CON{key, #CON{val, #NIL}}, rest}
    while (term_tag(cur) == C02 && term_ext(cur) == NAM_CON) {
      if (!first) omni_json_append_char(buf, len, cap, ',');
      first = 0;
      u32 cur_loc = term_val(cur);
      Term pair = wnf(HEAP[cur_loc]);  // #CON{key, #CON{val, #NIL}}

      if (term_tag(pair) == C02 && term_ext(pair) == NAM_CON) {
        u32 pair_loc = term_val(pair);
        Term key = wnf(HEAP[pair_loc]);
        Term val_cons = wnf(HEAP[pair_loc + 1]);
        if (term_tag(val_cons) == C02 && term_ext(val_cons) == NAM_CON) {
          Term v = wnf(HEAP[term_val(val_cons)]);
          omni_json_stringify_string(key, buf, len, cap);
          omni_json_append_char(buf, len, cap, ':');
          omni_json_stringify_value(v, buf, len, cap);
        }
      }
      cur = wnf(HEAP[cur_loc + 1]);
    }
    omni_json_append_char(buf, len, cap, '}');
    return;
  }

  // Check for #Arr{len, data} (array type)
  if (term_tag(val) == C02 && term_ext(val) == OMNI_NAM_ARR) {
    u32 loc = term_val(val);
    // len is at HEAP[loc], data list is at HEAP[loc+1]
    Term data = wnf(HEAP[loc + 1]);

    omni_json_append_char(buf, len, cap, '[');
    int first = 1;
    Term cur = data;
    while (term_tag(cur) == C02 && term_ext(cur) == NAM_CON) {
      if (!first) omni_json_append_char(buf, len, cap, ',');
      first = 0;
      u32 cur_loc = term_val(cur);
      Term elem = wnf(HEAP[cur_loc]);
      omni_json_stringify_value(elem, buf, len, cap);
      cur = wnf(HEAP[cur_loc + 1]);
    }
    // Handle empty array case (data is NIL)
    omni_json_append_char(buf, len, cap, ']');
    return;
  }

  // Default: output as null
  omni_json_append(buf, len, cap, "null");
}

// =============================================================================
// Main JSON Functions
// =============================================================================

// Parse JSON string to OmniLisp value
fn Term omni_json_parse(Term str) {
  char *cstr = omni_list_to_cstr(str);
  if (!cstr) {
    Term err_args[1] = {term_new_num(OMNI_JSON_ERR_MEMORY)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  const char *p = cstr;
  Term result = omni_json_parse_value(&p);
  free(cstr);

  return result;
}

// Stringify OmniLisp value to JSON string
fn Term omni_json_stringify(Term val) {
  size_t cap = 256;
  size_t len = 0;
  char *buf = (char*)malloc(cap);
  if (!buf) {
    Term err_args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  buf[0] = '\0';

  omni_json_stringify_value(val, &buf, &len, &cap);

  Term result = omni_cstr_to_list(buf);
  free(buf);

  return result;
}

// =============================================================================
// FFI Wrapper Functions
// =============================================================================

fn Term omni_ffi_json_parse(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term str = wnf(HEAP[loc]);
  return omni_json_parse(str);
}

fn Term omni_ffi_json_stringify(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term val = wnf(HEAP[loc]);
  return omni_json_stringify(val);
}

// =============================================================================
// JSON Dispatch
// =============================================================================

fn Term omni_ffi_json_dispatch(u32 name_nick, Term args) {
  if (name_nick == OMNI_NAM_JPRS) {
    return omni_ffi_json_parse(args);
  }
  if (name_nick == OMNI_NAM_JSTR) {
    return omni_ffi_json_stringify(args);
  }

  return 0;  // Not a JSON operation
}
