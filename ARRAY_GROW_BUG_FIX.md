# Array Growth Bug Fix

## Date: 2026-01-17

## Issue Description
`array_push()` did not grow arrays when they reached capacity. When pushing more elements than the initial capacity, the array would silently ignore the extra elements instead of growing to accommodate them.

## Bug Location
`runtime/src/runtime.c:745-748`

## Root Cause
The `array_push()` function had a TODO comment and empty else branch for handling full arrays:
```c
} else {
    /* Array is full, need reallocation (TODO: implement proper grow with region_realloc) */
    /* For now, skip or fail silently */
}
```

## Fix Applied

### 1. Implemented `array_grow()` function
```c
static void array_grow(Obj* arr) {
    if (!arr || !IS_BOXED(arr) || arr->tag != TAG_ARRAY) return;
    Array* a = (Array*)arr->ptr;

    Region* r = omni_obj_region(arr);
    if (!r) {
        _ensure_global_region();
        r = _global_region;
    }

    int new_capacity = a->capacity * 2;
    if (new_capacity < 8) new_capacity = 8;

    Obj** new_data = (Obj**)region_alloc(r, new_capacity * sizeof(Obj*));
    if (!new_data) return;

    for (int i = 0; i < a->len; i++) {
        new_data[i] = a->data[i];
    }

    a->data = new_data;
    a->capacity = new_capacity;
}
```

### 2. Updated `array_push()` to call `array_grow()`
Changed from:
```c
if (a->len < a->capacity) {
    // ... store value
} else {
    /* TODO: implement proper grow */
}
```

To:
```c
if (a->len >= a->capacity) {
    array_grow(arr);
}

if (a->len < a->capacity) {
    // ... store value
}
```

## Design Rationale

1. **CTRR Memory Model**: Uses `region_alloc()` for new data buffer. Old buffer is automatically reclaimed when region exits (no explicit free needed).

2. **Growth Strategy**: Doubles capacity (minimum 8 elements) to achieve amortized O(1) push performance.

3. **Region Safety**: Gets the array's owning region via `omni_obj_region(arr)` to ensure new data is allocated in the same region.

4. **Error Handling**: Silently fails on allocation failure (consistent with existing CTRR pattern where region_alloc returns NULL on OOM).

## Testing

### Test Case
```c
Obj* arr = mk_array_region(r, 4);
for (int i = 0; i < 10; i++) {
    array_push(arr, mk_int_region(r, i));
}
assert(array_length(arr) == 10);
```

### Results
- Before fix: Array stayed at length 4 (elements 5-10 silently dropped)
- After fix: Array grows to capacity 8 → 16, correctly storing all 10 elements

### Test Suite
All 392 runtime tests pass after fix.

## Related TODO Item
This completes **Issue 17 P0: Array Growth with Region Realloc** (previously marked `[DONE] (Review Needed)` but not actually implemented).

## Files Modified
- `runtime/src/runtime.c` - Added `array_grow()` function and updated `array_push()`

## Impact
- Fixes critical bug where arrays silently lost data on overflow
- Enables proper dynamic array usage in OmniLisp programs
- Aligns implementation with documented behavior in Issue 17
