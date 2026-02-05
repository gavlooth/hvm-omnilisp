// OmniLisp DateTime Operations
// Time and date manipulation via FFI
//
// DateTime is represented as #Dt{year, month, day, hour, min, sec, nsec}
// All numeric values are stored as NUM terms

// Required includes for time functions
#include <sys/time.h>
#include <time.h>
#include <string.h>

// strptime is a POSIX function, declare it manually if not available
extern char *strptime(const char *s, const char *format, struct tm *tm);

// FFI reduce function - dispatches nested FFI terms recursively
// Defined in thread_pool.c
fn Term omni_ffi_reduce(Term t);

// =============================================================================
// Helper Functions
// =============================================================================

// Extract u32 value from a term that may be NUM or #Cst{n}
fn u32 omni_term_to_u32(Term t) {
  t = wnf(t);
  // Raw NUM term (HVM4 native)
  if (term_tag(t) == NUM) {
    return term_val(t);
  }
  // #Cst{n} - OmniLisp wrapped constant
  if (term_tag(t) == C01 && term_ext(t) == OMNI_NAM_CST) {
    Term inner = wnf(HEAP[term_val(t)]);
    return term_val(inner);
  }
  // Fallback: treat as NUM (may give wrong value for other types)
  return term_val(t);
}

// =============================================================================
// DateTime Creation and Access
// =============================================================================

// Get current datetime
// Returns #Dt{year, month, day, hour, min, sec, nsec}
fn Term omni_dt_now(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  struct tm *tm = localtime(&tv.tv_sec);

  Term args[7] = {
    term_new_num(tm->tm_year + 1900),  // year
    term_new_num(tm->tm_mon + 1),       // month (1-12)
    term_new_num(tm->tm_mday),          // day (1-31)
    term_new_num(tm->tm_hour),          // hour (0-23)
    term_new_num(tm->tm_min),           // minute (0-59)
    term_new_num(tm->tm_sec),           // second (0-59)
    term_new_num(tv.tv_usec * 1000)     // nanoseconds
  };

  return term_new_ctr(OMNI_NAM_DT, 7, args);
}

// Extract year from datetime
fn Term omni_dt_year(Term dt) {
  dt = wnf(dt);
  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(dt);
  return wnf(HEAP[loc]);  // First field is year
}

// Extract month from datetime
fn Term omni_dt_month(Term dt) {
  dt = wnf(dt);
  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(dt);
  return wnf(HEAP[loc + 1]);  // Second field is month
}

// Extract day from datetime
fn Term omni_dt_day(Term dt) {
  dt = wnf(dt);
  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(dt);
  return wnf(HEAP[loc + 2]);  // Third field is day
}

// Extract hour from datetime
fn Term omni_dt_hour(Term dt) {
  dt = wnf(dt);
  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(dt);
  return wnf(HEAP[loc + 3]);  // Fourth field is hour
}

// Extract minute from datetime
fn Term omni_dt_minute(Term dt) {
  dt = wnf(dt);
  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(dt);
  return wnf(HEAP[loc + 4]);  // Fifth field is minute
}

// Extract second from datetime
fn Term omni_dt_second(Term dt) {
  dt = wnf(dt);
  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(dt);
  return wnf(HEAP[loc + 5]);  // Sixth field is second
}

// Convert datetime to Unix timestamp (seconds since epoch)
fn Term omni_dt_to_timestamp(Term dt) {
  dt = wnf(dt);
  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(dt);
  Term year = wnf(HEAP[loc]);
  Term month = wnf(HEAP[loc + 1]);
  Term day = wnf(HEAP[loc + 2]);
  Term hour = wnf(HEAP[loc + 3]);
  Term min = wnf(HEAP[loc + 4]);
  Term sec = wnf(HEAP[loc + 5]);

  struct tm tm = {0};
  tm.tm_year = term_val(year) - 1900;
  tm.tm_mon = term_val(month) - 1;
  tm.tm_mday = term_val(day);
  tm.tm_hour = term_val(hour);
  tm.tm_min = term_val(min);
  tm.tm_sec = term_val(sec);
  tm.tm_isdst = -1;  // Let mktime determine DST

  time_t ts = mktime(&tm);
  return term_new_num((u32)ts);
}

// Convert Unix timestamp to datetime
fn Term omni_dt_from_timestamp(Term ts) {
  time_t timestamp = (time_t)omni_term_to_u32(ts);
  struct tm *tm = localtime(&timestamp);

  Term args[7] = {
    term_new_num(tm->tm_year + 1900),
    term_new_num(tm->tm_mon + 1),
    term_new_num(tm->tm_mday),
    term_new_num(tm->tm_hour),
    term_new_num(tm->tm_min),
    term_new_num(tm->tm_sec),
    term_new_num(0)  // nanoseconds
  };

  return term_new_ctr(OMNI_NAM_DT, 7, args);
}

// =============================================================================
// DateTime Arithmetic
// =============================================================================

// Add duration to datetime
// Duration is in seconds (can be negative for subtraction)
fn Term omni_dt_add(Term dt, Term duration) {
  dt = wnf(dt);
  duration = wnf(duration);

  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  // Convert to timestamp, add duration, convert back
  Term ts = omni_dt_to_timestamp(dt);
  int secs = (int)omni_term_to_u32(duration);
  time_t new_ts = (time_t)term_val(ts) + secs;

  struct tm *tm = localtime(&new_ts);

  // Preserve nanoseconds from original
  u32 loc = term_val(dt);
  Term nsec = wnf(HEAP[loc + 6]);

  Term args[7] = {
    term_new_num(tm->tm_year + 1900),
    term_new_num(tm->tm_mon + 1),
    term_new_num(tm->tm_mday),
    term_new_num(tm->tm_hour),
    term_new_num(tm->tm_min),
    term_new_num(tm->tm_sec),
    nsec
  };

  return term_new_ctr(OMNI_NAM_DT, 7, args);
}

// Subtract duration from datetime
fn Term omni_dt_sub(Term dt, Term duration) {
  // Negate the duration and call add
  int secs = (int)omni_term_to_u32(duration);
  Term neg_dur = term_new_num((u32)(-secs));
  return omni_dt_add(dt, neg_dur);
}

// Get difference between two datetimes in seconds
fn Term omni_dt_diff(Term dt1, Term dt2) {
  Term ts1 = omni_dt_to_timestamp(dt1);
  Term ts2 = omni_dt_to_timestamp(dt2);

  int diff = (int)term_val(ts1) - (int)term_val(ts2);
  return term_new_num((u32)diff);
}

// =============================================================================
// DateTime Formatting
// =============================================================================

// Format datetime as ISO 8601 string
// Returns char list like "2025-01-15T10:30:45"
fn Term omni_dt_format(Term dt, Term fmt) {
  dt = wnf(dt);

  if (term_tag(dt) < C00 || term_ext(dt) != OMNI_NAM_DT) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  u32 loc = term_val(dt);
  Term year = wnf(HEAP[loc]);
  Term month = wnf(HEAP[loc + 1]);
  Term day = wnf(HEAP[loc + 2]);
  Term hour = wnf(HEAP[loc + 3]);
  Term min = wnf(HEAP[loc + 4]);
  Term sec = wnf(HEAP[loc + 5]);

  // Convert format string
  char *fmt_str = omni_list_to_cstr(fmt);
  if (!fmt_str) {
    // Default to ISO 8601
    fmt_str = strdup("%Y-%m-%dT%H:%M:%S");
  }

  struct tm tm = {0};
  tm.tm_year = term_val(year) - 1900;
  tm.tm_mon = term_val(month) - 1;
  tm.tm_mday = term_val(day);
  tm.tm_hour = term_val(hour);
  tm.tm_min = term_val(min);
  tm.tm_sec = term_val(sec);

  char buf[256];
  strftime(buf, sizeof(buf), fmt_str, &tm);
  free(fmt_str);

  return omni_cstr_to_list(buf);
}

// Parse datetime from string
// Returns datetime or error
fn Term omni_dt_parse(Term str, Term fmt) {
  char *str_c = omni_list_to_cstr(str);
  char *fmt_c = omni_list_to_cstr(fmt);

  if (!str_c || !fmt_c) {
    if (str_c) free(str_c);
    if (fmt_c) free(fmt_c);
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  struct tm tm = {0};
  char *result = strptime(str_c, fmt_c, &tm);
  free(str_c);
  free(fmt_c);

  if (!result) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }

  Term args[7] = {
    term_new_num(tm.tm_year + 1900),
    term_new_num(tm.tm_mon + 1),
    term_new_num(tm.tm_mday),
    term_new_num(tm.tm_hour),
    term_new_num(tm.tm_min),
    term_new_num(tm.tm_sec),
    term_new_num(0)
  };

  return term_new_ctr(OMNI_NAM_DT, 7, args);
}

// =============================================================================
// FFI Wrapper Functions
// =============================================================================

// Wrapper for datetime-now (no args)
fn Term omni_ffi_dt_now(Term args) {
  (void)args;
  return omni_dt_now();
}

// Wrapper for datetime-year (single dt arg)
fn Term omni_ffi_dt_year(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  // Use omni_ffi_reduce to dispatch nested FFI terms (e.g., from datetime-now)
  Term dt = omni_ffi_reduce(HEAP[loc]);
  return omni_dt_year(dt);
}

// Wrapper for datetime-month
fn Term omni_ffi_dt_month(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  return omni_dt_month(dt);
}

// Wrapper for datetime-day
fn Term omni_ffi_dt_day(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  return omni_dt_day(dt);
}

// Wrapper for datetime-hour
fn Term omni_ffi_dt_hour(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  return omni_dt_hour(dt);
}

// Wrapper for datetime-minute
fn Term omni_ffi_dt_minute(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  return omni_dt_minute(dt);
}

// Wrapper for datetime-second
fn Term omni_ffi_dt_second(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  return omni_dt_second(dt);
}

// Wrapper for datetime->timestamp
fn Term omni_ffi_dt_to_timestamp(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  return omni_dt_to_timestamp(dt);
}

// Wrapper for timestamp->datetime
fn Term omni_ffi_dt_from_timestamp(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term ts = wnf(HEAP[loc]);
  return omni_dt_from_timestamp(ts);
}

// Wrapper for datetime-add
fn Term omni_ffi_dt_add(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 tail_loc = term_val(tail);
  Term duration = wnf(HEAP[tail_loc]);

  return omni_dt_add(dt, duration);
}

// Wrapper for datetime-sub
fn Term omni_ffi_dt_sub(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 tail_loc = term_val(tail);
  Term duration = wnf(HEAP[tail_loc]);

  return omni_dt_sub(dt, duration);
}

// Wrapper for datetime-diff
fn Term omni_ffi_dt_diff(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt1 = omni_ffi_reduce(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 tail_loc = term_val(tail);
  Term dt2 = omni_ffi_reduce(HEAP[tail_loc]);

  return omni_dt_diff(dt1, dt2);
}

// Wrapper for datetime-format
fn Term omni_ffi_dt_format(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term dt = omni_ffi_reduce(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 tail_loc = term_val(tail);
  Term fmt = wnf(HEAP[tail_loc]);

  return omni_dt_format(dt, fmt);
}

// Wrapper for datetime-parse
fn Term omni_ffi_dt_parse(Term args) {
  if (term_tag(args) != C02 || term_ext(args) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 loc = term_val(args);
  Term str = wnf(HEAP[loc]);
  Term tail = wnf(HEAP[loc + 1]);

  if (term_tag(tail) != C02 || term_ext(tail) != NAM_CON) {
    Term err_args[1] = {term_new_num(EINVAL)};
    return term_new_ctr(OMNI_NAM_ERR, 1, err_args);
  }
  u32 tail_loc = term_val(tail);
  Term fmt = wnf(HEAP[tail_loc]);

  return omni_dt_parse(str, fmt);
}

// =============================================================================
// DateTime Dispatch
// =============================================================================

// Dispatch for datetime operations
fn Term omni_ffi_dt_dispatch(u32 name_nick, Term args) {
  if (name_nick == OMNI_NAM_DTNW) {
    return omni_ffi_dt_now(args);
  }
  if (name_nick == OMNI_NAM_DTYR) {
    return omni_ffi_dt_year(args);
  }
  if (name_nick == OMNI_NAM_DTMO) {
    return omni_ffi_dt_month(args);
  }
  if (name_nick == OMNI_NAM_DTDY) {
    return omni_ffi_dt_day(args);
  }
  if (name_nick == OMNI_NAM_DTHR) {
    return omni_ffi_dt_hour(args);
  }
  if (name_nick == OMNI_NAM_DTMI) {
    return omni_ffi_dt_minute(args);
  }
  if (name_nick == OMNI_NAM_DTSC) {
    return omni_ffi_dt_second(args);
  }
  if (name_nick == OMNI_NAM_DTTS) {
    return omni_ffi_dt_to_timestamp(args);
  }
  if (name_nick == OMNI_NAM_DTFT) {
    return omni_ffi_dt_from_timestamp(args);
  }
  if (name_nick == OMNI_NAM_DTAD) {
    return omni_ffi_dt_add(args);
  }
  if (name_nick == OMNI_NAM_DTSB) {
    return omni_ffi_dt_sub(args);
  }
  if (name_nick == OMNI_NAM_DTDF) {
    return omni_ffi_dt_diff(args);
  }
  if (name_nick == OMNI_NAM_DTFM) {
    return omni_ffi_dt_format(args);
  }
  if (name_nick == OMNI_NAM_DTPR) {
    return omni_ffi_dt_parse(args);
  }

  return 0;  // Not a datetime operation
}
