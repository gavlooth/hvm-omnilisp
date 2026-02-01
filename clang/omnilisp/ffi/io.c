// OmniLisp IO Operations
// File I/O, environment variables, and string conversion utilities
//
// This file provides FFI-backed IO operations for OmniLisp:
// - String conversion between HVM4 char lists and C strings
// - File read/write operations
// - Directory operations
// - Environment variable access

// hvm4.c is already included by main.c before this file
// #include "../../../hvm4/clang/hvm4.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

// =============================================================================
// String Conversion Utilities
// =============================================================================

// Get length of an HVM4 char list (CON/NIL)
fn u32 omni_list_length(Term list) {
  u32 len = 0;
  Term cur = wnf(list);  // Evaluate the list first

  // Walk the list, counting elements
  // #CON{h, t} has tag C02 and ext NAM_CON
  while (term_tag(cur) == C02 && term_ext(cur) == NAM_CON) {
    len++;
    u32 loc = term_val(cur);
    cur = wnf(HEAP[loc + 1]);  // Get tail and evaluate it
    // Limit to prevent infinite loops
    if (len > 1000000) break;
  }

  return len;
}

// Convert HVM4 char list to C string
// Returns malloc'd string (caller must free)
// Returns NULL on error
fn char* omni_list_to_cstr(Term list) {
  // Evaluate the list first
  list = wnf(list);

  // First pass: count length
  u32 len = omni_list_length(list);

  if (len == 0) {
    // Empty list -> empty string
    char *s = (char*)malloc(1);
    if (s) s[0] = '\0';
    return s;
  }

  // Allocate buffer
  char *buf = (char*)malloc(len + 1);
  if (!buf) return NULL;

  // Second pass: extract characters (list is already wnf'd)
  Term cur = list;
  u32 i = 0;

  while (term_tag(cur) == C02 && term_ext(cur) == NAM_CON && i < len) {
    u32 loc = term_val(cur);
    Term head = wnf(HEAP[loc]);  // WNF the head to evaluate thunks
    Term tail = wnf(HEAP[loc + 1]);  // WNF the tail too

    // Head should be #CHR{code} - C01 with ext NAM_CHR
    if (term_tag(head) == C01 && term_ext(head) == NAM_CHR) {
      u32 chr_loc = term_val(head);
      Term chr_val = wnf(HEAP[chr_loc]);  // WNF the character code
      buf[i++] = (char)term_val(chr_val);
    } else if (term_tag(head) == NUM) {
      // Direct number (ASCII code)
      buf[i++] = (char)term_val(head);
    } else {
      // Unknown format - skip
      buf[i++] = ' ';
    }

    cur = tail;
  }

  buf[i] = '\0';
  return buf;
}

// Convert C string to HVM4 char list
// Returns #NIL for empty string, #CON{#CHR{c}, ...} otherwise
fn Term omni_cstr_to_list(const char *str) {
  if (!str || str[0] == '\0') {
    return term_new_ctr(NAM_NIL, 0, NULL);
  }

  // Build list from end to beginning (cons cells)
  size_t len = strlen(str);
  Term result = term_new_ctr(NAM_NIL, 0, NULL);

  for (size_t i = len; i > 0; i--) {
    // Create #CHR{code}
    Term chr_args[1] = {term_new_num((u32)(unsigned char)str[i-1])};
    Term chr = term_new_ctr(NAM_CHR, 1, chr_args);

    // Create #CON{chr, result}
    Term con_args[2] = {chr, result};
    result = term_new_ctr(NAM_CON, 2, con_args);
  }

  return result;
}

// =============================================================================
// File Operations
// =============================================================================

// Read entire file contents as char list
// Returns char list on success, #Err{errno} on failure
fn Term omni_io_read_file(Term path_list) {
  char *path = omni_list_to_cstr(path_list);
  if (!path) {
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  FILE *f = fopen(path, "rb");
  if (!f) {
    int err = errno;
    free(path);
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  // Get file size
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (size < 0 || size > 100000000) {  // 100MB limit
    fclose(f);
    free(path);
    Term args[1] = {term_new_num(EFBIG)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  // Read content
  char *content = (char*)malloc(size + 1);
  if (!content) {
    fclose(f);
    free(path);
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  size_t read_size = fread(content, 1, size, f);
  content[read_size] = '\0';
  fclose(f);
  free(path);

  // Convert to char list
  Term result = omni_cstr_to_list(content);
  free(content);

  return result;
}

// Write string content to file
// Returns #True on success, #Err{errno} on failure
fn Term omni_io_write_file(Term path_list, Term content_list) {
  char *path = omni_list_to_cstr(path_list);
  if (!path) {
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  char *content = omni_list_to_cstr(content_list);
  if (!content) {
    free(path);
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  FILE *f = fopen(path, "wb");
  if (!f) {
    int err = errno;
    free(path);
    free(content);
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  size_t len = strlen(content);
  size_t written = fwrite(content, 1, len, f);
  fclose(f);
  free(path);
  free(content);

  if (written != len) {
    Term args[1] = {term_new_num(EIO)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  return term_new_ctr(OMNI_NAM_TRUE, 0, NULL);
}

// Append string content to file
// Returns #True on success, #Err{errno} on failure
fn Term omni_io_append_file(Term path_list, Term content_list) {
  char *path = omni_list_to_cstr(path_list);
  if (!path) {
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  char *content = omni_list_to_cstr(content_list);
  if (!content) {
    free(path);
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  FILE *f = fopen(path, "ab");
  if (!f) {
    int err = errno;
    free(path);
    free(content);
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  size_t len = strlen(content);
  size_t written = fwrite(content, 1, len, f);
  fclose(f);
  free(path);
  free(content);

  if (written != len) {
    Term args[1] = {term_new_num(EIO)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  return term_new_ctr(OMNI_NAM_TRUE, 0, NULL);
}

// Check if file exists
// Returns #True or #Fals
fn Term omni_io_file_exists(Term path_list) {
  char *path = omni_list_to_cstr(path_list);
  if (!path) {
    return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
  }

  struct stat st;
  int exists = (stat(path, &st) == 0);
  free(path);

  return term_new_ctr(exists ? OMNI_NAM_TRUE : OMNI_NAM_FALS, 0, NULL);
}

// Check if path is a directory
// Returns #True or #Fals
fn Term omni_io_is_dir(Term path_list) {
  char *path = omni_list_to_cstr(path_list);
  if (!path) {
    return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
  }

  struct stat st;
  int is_dir = (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
  free(path);

  return term_new_ctr(is_dir ? OMNI_NAM_TRUE : OMNI_NAM_FALS, 0, NULL);
}

// Create directory
// Returns #True on success, #Err{errno} on failure
fn Term omni_io_mkdir(Term path_list) {
  char *path = omni_list_to_cstr(path_list);
  if (!path) {
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  int result = mkdir(path, 0755);
  int err = errno;
  free(path);

  if (result != 0) {
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  return term_new_ctr(OMNI_NAM_TRUE, 0, NULL);
}

// List directory contents
// Returns list of filename strings on success, #Err{errno} on failure
fn Term omni_io_list_dir(Term path_list) {
  char *path = omni_list_to_cstr(path_list);
  if (!path) {
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  DIR *dir = opendir(path);
  if (!dir) {
    int err = errno;
    free(path);
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }
  free(path);

  // Build list of entries
  Term result = term_new_ctr(NAM_NIL, 0, NULL);
  struct dirent *entry;

  // Collect entries in a temporary buffer to reverse order
  char *entries[10000];
  int count = 0;

  while ((entry = readdir(dir)) != NULL && count < 10000) {
    // Skip . and ..
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    entries[count++] = strdup(entry->d_name);
  }
  closedir(dir);

  // Build list from end to beginning (for proper order)
  for (int i = count - 1; i >= 0; i--) {
    Term name = omni_cstr_to_list(entries[i]);
    free(entries[i]);
    Term con_args[2] = {name, result};
    result = term_new_ctr(NAM_CON, 2, con_args);
  }

  return result;
}

// Delete file
// Returns #True on success, #Err{errno} on failure
fn Term omni_io_delete_file(Term path_list) {
  char *path = omni_list_to_cstr(path_list);
  if (!path) {
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  int result = unlink(path);
  int err = errno;
  free(path);

  if (result != 0) {
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  return term_new_ctr(OMNI_NAM_TRUE, 0, NULL);
}

// Rename file
// Returns #True on success, #Err{errno} on failure
fn Term omni_io_rename_file(Term from_list, Term to_list) {
  char *from = omni_list_to_cstr(from_list);
  if (!from) {
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  char *to = omni_list_to_cstr(to_list);
  if (!to) {
    free(from);
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  int result = rename(from, to);
  int err = errno;
  free(from);
  free(to);

  if (result != 0) {
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  return term_new_ctr(OMNI_NAM_TRUE, 0, NULL);
}

// Copy file
// Returns #True on success, #Err{errno} on failure
fn Term omni_io_copy_file(Term from_list, Term to_list) {
  char *from = omni_list_to_cstr(from_list);
  if (!from) {
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  char *to = omni_list_to_cstr(to_list);
  if (!to) {
    free(from);
    Term args[1] = {term_new_num(ENOMEM)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  FILE *src = fopen(from, "rb");
  if (!src) {
    int err = errno;
    free(from);
    free(to);
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  FILE *dst = fopen(to, "wb");
  if (!dst) {
    int err = errno;
    fclose(src);
    free(from);
    free(to);
    Term args[1] = {term_new_num(err)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  // Copy in chunks
  char buf[8192];
  size_t n;
  int success = 1;

  while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
    if (fwrite(buf, 1, n, dst) != n) {
      success = 0;
      break;
    }
  }

  fclose(src);
  fclose(dst);
  free(from);
  free(to);

  if (!success) {
    Term args[1] = {term_new_num(EIO)};
    return term_new_ctr(OMNI_NAM_ERR, 1, args);
  }

  return term_new_ctr(OMNI_NAM_TRUE, 0, NULL);
}

// =============================================================================
// Environment Variables
// =============================================================================

// Get environment variable
// Returns char list on success, #Noth on failure
fn Term omni_io_getenv(Term name_list) {
  char *name = omni_list_to_cstr(name_list);
  if (!name) {
    return term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
  }

  char *value = getenv(name);
  free(name);

  if (!value) {
    return term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
  }

  return omni_cstr_to_list(value);
}

// Set environment variable
// Returns #True on success, #Fals on failure
fn Term omni_io_setenv(Term name_list, Term value_list) {
  char *name = omni_list_to_cstr(name_list);
  if (!name) {
    return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
  }

  char *value = omni_list_to_cstr(value_list);
  if (!value) {
    free(name);
    return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
  }

  int result = setenv(name, value, 1);
  free(name);
  free(value);

  return term_new_ctr(result == 0 ? OMNI_NAM_TRUE : OMNI_NAM_FALS, 0, NULL);
}

// =============================================================================
// FFI Wrapper Functions
// =============================================================================

// These wrapper functions are called by the FFI dispatch system
// They extract arguments from the args list and call the actual implementation

// Wrapper for read-file: takes single path argument
fn Term omni_ffi_io_read_file(Term args) {
  // Args is a cons list with one element: the path
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(args);
  Term path = wnf(HEAP[loc]);

  return omni_io_read_file(path);
}

// Wrapper for write-file: takes path and content arguments
fn Term omni_ffi_io_write_file(Term args) {
  // Args is a cons list with two elements: path, content
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(args);
  Term path = wnf(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 tail_loc = term_val(tail);
  Term content = wnf(HEAP[tail_loc]);

  return omni_io_write_file(path, content);
}

// Wrapper for append-file: takes path and content arguments
fn Term omni_ffi_io_append_file(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(args);
  Term path = wnf(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 tail_loc = term_val(tail);
  Term content = wnf(HEAP[tail_loc]);

  return omni_io_append_file(path, content);
}

// Wrapper for file-exists: takes single path argument
fn Term omni_ffi_io_file_exists(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
  }

  u32 loc = term_val(args);
  Term path = wnf(HEAP[loc]);

  return omni_io_file_exists(path);
}

// Wrapper for is-dir: takes single path argument
fn Term omni_ffi_io_is_dir(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
  }

  u32 loc = term_val(args);
  Term path = wnf(HEAP[loc]);

  return omni_io_is_dir(path);
}

// Wrapper for mkdir: takes single path argument
fn Term omni_ffi_io_mkdir(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(args);
  Term path = wnf(HEAP[loc]);

  return omni_io_mkdir(path);
}

// Wrapper for list-dir: takes single path argument
fn Term omni_ffi_io_list_dir(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(args);
  Term path = wnf(HEAP[loc]);

  return omni_io_list_dir(path);
}

// Wrapper for delete-file: takes single path argument
fn Term omni_ffi_io_delete_file(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(args);
  Term path = wnf(HEAP[loc]);

  return omni_io_delete_file(path);
}

// Wrapper for rename-file: takes from and to arguments
fn Term omni_ffi_io_rename_file(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(args);
  Term from = wnf(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 tail_loc = term_val(tail);
  Term to = wnf(HEAP[tail_loc]);

  return omni_io_rename_file(from, to);
}

// Wrapper for copy-file: takes from and to arguments
fn Term omni_ffi_io_copy_file(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(args);
  Term from = wnf(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 tail_loc = term_val(tail);
  Term to = wnf(HEAP[tail_loc]);

  return omni_io_copy_file(from, to);
}

// Wrapper for getenv: takes single name argument
fn Term omni_ffi_io_getenv(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    return term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
  }

  u32 loc = term_val(args);
  Term name = wnf(HEAP[loc]);

  return omni_io_getenv(name);
}

// Wrapper for setenv: takes name and value arguments
fn Term omni_ffi_io_setenv(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
  }

  u32 loc = term_val(args);
  Term name = wnf(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    return term_new_ctr(OMNI_NAM_FALS, 0, NULL);
  }

  u32 tail_loc = term_val(tail);
  Term value = wnf(HEAP[tail_loc]);

  return omni_io_setenv(name, value);
}

// =============================================================================
// BOOK Lookup for Forward References
// =============================================================================

// Wrapper for book_get: takes table_id, returns term from BOOK
// Used by #FRef handler in runtime.hvm4 for lazy function lookup
fn Term omni_ffi_io_book_get(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    return term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
  }

  u32 loc = term_val(args);
  Term id_term = wnf(HEAP[loc]);

  // Extract the table_id from the argument
  u32 table_id;
  if (term_tag(id_term) == NUM) {
    table_id = term_val(id_term);
  } else if (term_tag(id_term) == C01 && term_ext(id_term) == OMNI_NAM_CST) {
    // #Cst{n} - extract the number
    Term inner = wnf(HEAP[term_val(id_term)]);
    table_id = term_val(inner);
  } else {
    return term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
  }

  // Check bounds
  if (table_id >= BOOK_CAP) {
    return term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
  }

  // Look up BOOK entry
  u32 heap_loc = BOOK[table_id];
  if (heap_loc == 0) {
    return term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
  }

  // Return the term stored at BOOK[table_id]
  // This is the function's AST (typically a #Lam or #LamR)
  Term result = HEAP[heap_loc];
  return result;
}

// =============================================================================
// FFI IO Registration
// =============================================================================

// Nick codes for IO operations (computed at runtime in omni_names_init)
// These should match the values in nick/omnilisp.c

fn void omni_ffi_register_io(void) {
  // Register IO functions with their nick-encoded names
  // These use OMNI_FFI_VARIADIC since they need special argument handling
  // The dispatch happens via direct function table lookup

  // We'll register these as special handlers in the FFI dispatch
  // See omni_ffi_io_dispatch below
}

// =============================================================================
// IO Dispatch
// =============================================================================

// Special dispatch for IO operations
// Called from omni_ffi_dispatch when encountering IO-related FFI names
fn Term omni_ffi_io_dispatch(u32 name_nick, Term args) {
  // Check which IO operation this is
  if (name_nick == OMNI_NAM_RDFL) {
    return omni_ffi_io_read_file(args);
  }
  if (name_nick == OMNI_NAM_WRFL) {
    return omni_ffi_io_write_file(args);
  }
  if (name_nick == OMNI_NAM_APFL) {
    return omni_ffi_io_append_file(args);
  }
  if (name_nick == OMNI_NAM_EXST) {
    return omni_ffi_io_file_exists(args);
  }
  if (name_nick == OMNI_NAM_ISDR) {
    return omni_ffi_io_is_dir(args);
  }
  if (name_nick == OMNI_NAM_MKDR) {
    return omni_ffi_io_mkdir(args);
  }
  if (name_nick == OMNI_NAM_LSDR) {
    return omni_ffi_io_list_dir(args);
  }
  if (name_nick == OMNI_NAM_DLFL) {
    return omni_ffi_io_delete_file(args);
  }
  if (name_nick == OMNI_NAM_RNFL) {
    return omni_ffi_io_rename_file(args);
  }
  if (name_nick == OMNI_NAM_CPFL) {
    return omni_ffi_io_copy_file(args);
  }
  if (name_nick == OMNI_NAM_GTEV) {
    return omni_ffi_io_getenv(args);
  }
  if (name_nick == OMNI_NAM_STEV) {
    return omni_ffi_io_setenv(args);
  }
  if (name_nick == OMNI_NAM_BKGT) {
    return omni_ffi_io_book_get(args);
  }

  // Debug FFI: DbgT (debug term) - inspect first arg (silent unless omni_ffi_debug)
  // nick("DbgT") = 1165396 (computed)
  if (name_nick == 1165396) {  // DbgT
    // Get first arg
    if (term_tag(args) == C02 && term_ext(args) == NAM_CON) {
      u32 aloc = term_val(args);
      Term t = wnf(HEAP[aloc]);
      // Return the term unchanged
      return t;
    }
    return term_new_num(0);
  }

  // Not an IO operation
  return 0;  // Return 0 to indicate not handled
}
