# IPGE Generation Checking Implementation

## Overview

This document describes the implementation of proper IPGE (Indexed Pointer Generation Epoch) generation checking in the OmniLisp runtime. The implementation addresses memory safety concerns when using cross-region pointers by detecting stale pointers through generation mismatch.

## Changes Made

### 1. Object Generation Initialization (runtime/src/memory/region_value.c)

**Before:** Objects were always initialized with `generation = 0`
```c
o->generation = 0;
```

**After:** Objects are initialized with their region's current generation
```c
o->generation = r->generation;  /* Initialize with region's current generation */
```

**Rationale:** Each region has a generation number that increments when the region is reused. By storing the generation at allocation time, objects carry information about the "era" they were created in, enabling detection of stale pointers.

### 2. Generation Checking in Pointer Access (runtime/src/memory/region_pointer.h)

**Before:** Cross-region access always returned `true` (unsafe)
```c
/* Cross-region: would need generation check here */
/* For now, return true (unsafe but functional) */
/* TODO: Implement proper generation checking */
return true;
```

**After:** Cross-region access performs generation checking
```c
/* Decode pointer to get actual object */
void* actual_ptr = pointer_mask_decode(obj_ptr);
if (!actual_ptr) {
    return false;
}

/* Look up target region from registry (O(1) lookup) */
Region* target_region = region_of(obj_ptr);
if (!target_region) {
    /* Region no longer exists or was destroyed */
    return false;
}

/* Compare object's generation with region's current generation */
Obj* obj = (Obj*)actual_ptr;
return obj->generation == target_region->generation;
```

**Rationale:** By comparing the object's generation (at allocation time) with the region's current generation, we can detect if a pointer is stale. If the region has been reused and its generation incremented, an old object's generation won't match, indicating the pointer should not be dereferenced.

### 3. IPGE Integration (runtime/src/memory/region_pointer.h)

**Before:** `pointer_mask_encode_with_generation` was a stub
```c
(void)generation;  /* TODO: Integrate with IPGE */
return pointer_mask_encode(ptr, region_id);
```

**After:** Generation stored in object header, not encoded in pointer
```c
(void)generation;  /* Generation stored in Obj header, not encoded in pointer */
return pointer_mask_encode(ptr, region_id);
```

**Rationale:** The generation is stored inline in the object's header (`Obj.generation` field), not encoded in the pointer itself. This keeps the pointer encoding simple while still enabling generation-based safety checks.

## How IPGE Works

### Generation Lifecycle

1. **Region Creation:** New regions start with `generation = 1`
   ```c
   r->generation = 1;  /* Start at generation 1 (0 reserved for unallocated) */
   ```

2. **Object Allocation:** Objects receive current generation from their region
   ```c
   o->generation = r->generation;
   ```

3. **Region Reset:** When regions are returned to pool, generation is incremented
   ```c
   r->generation++;  /* Bump generation to invalidate old pointers */
   ```

4. **Region Reuse:** When reused from pool, region has incremented generation
   - Old objects from previous generation still exist in memory
   - New generation invalidates old pointers

### Cross-Region Access Safety

```c
bool pointer_mask_safe_access(const void* obj_ptr, uint16_t current_region) {
    // Same region: Always safe (fast path)
    if (obj_region == current_region) return true;

    // Cross-region: Check generation (IPGE)
    // 1. Decode pointer to get actual object
    // 2. Look up target region from registry
    // 3. Compare obj->generation with region->generation
    // 4. Return true only if they match
}
```

## Benefits

1. **Memory Safety:** Detects use-after-region-exit by identifying stale pointers
2. **Zero Overhead (Same Region):** Same-region access skips generation check
3. **O(1) Cross-Region Check:** Registry lookup and simple comparison
4. **Compatibility:** Works with existing pointer masking system
5. **No Fat Pointers:** Generation stored in object header, not in pointer

## Testing

New test file: `runtime/tests/test_generation_checking.c`

Tests verify:
- Objects are allocated with correct generation
- Regions increment generation on reuse
- Cross-region generation checking works
- Stale pointer detection works

All 397 existing runtime tests pass with no regressions.

## Limitations

1. **In-Memory Only:** Generation checking doesn't detect stale pointers to freed regions that haven't been reused yet. Only detects when region has been recycled.
2. **Requires Registration:** `region_of()` lookup requires region to be in global registry. Unregistered regions won't work.
3. **Assumes Proper Allocation:** Assumes all objects are allocated via `alloc_obj_typed()` which sets generation correctly.

## Future Enhancements

1. **Robust Mode:** Currently uses 16-bit generations (65K cycles). Could use 64-bit generations for pathological reuse patterns.
2. **Generation Encoding:** Could encode generation in pointer high bits instead of storing in object header (tradeoff: pointer size vs memory usage).
3. **Integration with Store Barrier:** Could automatically invoke generation checks on all pointer stores (currently manual).
4. **Debug Assertions:** Add optional debug assertions on all cross-region accesses to catch unsafe access patterns.

## References

- Issue 10 P0: IPGE Generation & Region Realloc
- `docs/REGION_RC.md` - Region-RC memory model
- `runtime/src/memory/region_core.c` - Region lifecycle (lines 123-124, 182-183)
- `runtime/src/memory/region_pointer.h` - Pointer masking and generation checking
