#include <string>

inline static std::string wasm_rt_content =
    "/*\n"
    " * Copyright 2018 WebAssembly Community Group participants\n"
    " *\n"
    " * Licensed under the Apache License, Version 2.0 (the \"License\");\n"
    " * you may not use this file except in compliance with the License.\n"
    " * You may obtain a copy of the License at\n"
    " *\n"
    " *     http://www.apache.org/licenses/LICENSE-2.0\n"
    " *\n"
    " * Unless required by applicable law or agreed to in writing, software\n"
    " * distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    " * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or "
    "implied.\n"
    " * See the License for the specific language governing permissions and\n"
    " * limitations under the License.\n"
    " */\n"
    "\n"
    "#ifndef WASM_RT_H_\n"
    "#define WASM_RT_H_\n"
    "\n"
    "#include <setjmp.h>\n"
    "#include <stdbool.h>\n"
    "#include <stdint.h>\n"
    "#include <stdlib.h>\n"
    "#include <string.h>\n"
    "\n"
    "#ifdef __cplusplus\n"
    "extern \"C\" {\n"
    "#endif\n"
    "\n"
    "#ifndef __has_builtin\n"
    "#define __has_builtin(x) 0  // Compatibility with non-clang compilers.\n"
    "#endif\n"
    "\n"
    "#if __has_builtin(__builtin_expect)\n"
    "#define UNLIKELY(x) __builtin_expect(!!(x), 0)\n"
    "#define LIKELY(x) __builtin_expect(!!(x), 1)\n"
    "#else\n"
    "#define UNLIKELY(x) (x)\n"
    "#define LIKELY(x) (x)\n"
    "#endif\n"
    "\n"
    "#if __has_builtin(__builtin_memcpy)\n"
    "#define wasm_rt_memcpy __builtin_memcpy\n"
    "#else\n"
    "#define wasm_rt_memcpy memcpy\n"
    "#endif\n"
    "\n"
    "#if __has_builtin(__builtin_unreachable)\n"
    "#define wasm_rt_unreachable __builtin_unreachable\n"
    "#else\n"
    "#define wasm_rt_unreachable abort\n"
    "#endif\n"
    "\n"
    "#ifdef _MSC_VER\n"
    "#define WASM_RT_THREAD_LOCAL __declspec(thread)\n"
    "#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)\n"
    "#define WASM_RT_THREAD_LOCAL _Thread_local\n"
    "#else\n"
    "#define WASM_RT_THREAD_LOCAL\n"
    "#endif\n"
    "\n"
    "#define WASM_RT_PAGE_SIZE 65536\n"
    "\n"
    "/**\n"
    " * Backward compatibility: Convert the previously exposed\n"
    " * WASM_RT_MEMCHECK_SIGNAL_HANDLER macro to the ALLOCATION and CHECK "
    "macros that\n"
    " * are now used.\n"
    " */\n"
    "#if defined(WASM_RT_MEMCHECK_SIGNAL_HANDLER)\n"
    "\n"
    "#if WASM_RT_MEMCHECK_SIGNAL_HANDLER\n"
    "#define WASM_RT_USE_MMAP 1\n"
    "#define WASM_RT_MEMCHECK_GUARD_PAGES 1\n"
    "#else\n"
    "#define WASM_RT_USE_MMAP 0\n"
    "#define WASM_RT_MEMCHECK_BOUNDS_CHECK 1\n"
    "#endif\n"
    "\n"
    "#warning \\\n"
    "    \"WASM_RT_MEMCHECK_SIGNAL_HANDLER has been deprecated in favor of "
    "WASM_RT_USE_MMAP and WASM_RT_MEMORY_CHECK_* macros\"\n"
    "#endif\n"
    "\n"
    "/**\n"
    " * Specify if we use OR mmap/mprotect (+ Windows equivalents) OR "
    "malloc/realloc\n"
    " * for the Wasm memory allocation and growth. mmap/mprotect guarantees "
    "memory\n"
    " * will grow without being moved, while malloc ensures the virtual memory "
    "is\n"
    " * consumed only as needed, but may relocate the memory to handle memory\n"
    " * fragmentation.\n"
    " *\n"
    " * This defaults to malloc on 32-bit platforms or if memory64 support is "
    "needed.\n"
    " * It defaults to mmap on 64-bit platforms assuming memory64 support is "
    "not\n"
    " * needed (so we can use the guard based range checks below).\n"
    " */\n"
    "#ifndef WASM_RT_USE_MMAP\n"
    "#if UINTPTR_MAX > 0xffffffff && !SUPPORT_MEMORY64\n"
    "#define WASM_RT_USE_MMAP 1\n"
    "#else\n"
    "#define WASM_RT_USE_MMAP 0\n"
    "#endif\n"
    "#endif\n"
    "\n"
    "/**\n"
    " * Set the range checking strategy for Wasm memories.\n"
    " *\n"
    " * GUARD_PAGES:  memory accesses rely on unmapped pages/guard pages to "
    "trap\n"
    " * out-of-bound accesses.\n"
    " *\n"
    " * BOUNDS_CHECK: memory accesses are checked with explicit bounds "
    "checks.\n"
    " *\n"
    " * This defaults to GUARD_PAGES as this is the fasest option, iff the\n"
    " * requirements of GUARD_PAGES --- 64-bit platforms, MMAP allocation "
    "strategy,\n"
    " * no 64-bit memories --- are met. This falls back to BOUNDS otherwise.\n"
    " */\n"
    "\n"
    "// Check if Guard checks are supported\n"
    "#if UINTPTR_MAX > 0xffffffff && WASM_RT_USE_MMAP && !SUPPORT_MEMORY64\n"
    "#define WASM_RT_GUARD_PAGES_SUPPORTED 1\n"
    "#else\n"
    "#define WASM_RT_GUARD_PAGES_SUPPORTED 0\n"
    "#endif\n"
    "\n"
    "// Specify defaults for memory checks if unspecified\n"
    "#if !defined(WASM_RT_MEMCHECK_GUARD_PAGES) && \\\n"
    "    !defined(WASM_RT_MEMCHECK_BOUNDS_CHECK)\n"
    "#if WASM_RT_GUARD_PAGES_SUPPORTED\n"
    "#define WASM_RT_MEMCHECK_GUARD_PAGES 1\n"
    "#else\n"
    "#define WASM_RT_MEMCHECK_BOUNDS_CHECK 1\n"
    "#endif\n"
    "#endif\n"
    "\n"
    "// Ensure the macros are defined\n"
    "#ifndef WASM_RT_MEMCHECK_GUARD_PAGES\n"
    "#define WASM_RT_MEMCHECK_GUARD_PAGES 0\n"
    "#endif\n"
    "#ifndef WASM_RT_MEMCHECK_BOUNDS_CHECK\n"
    "#define WASM_RT_MEMCHECK_BOUNDS_CHECK 0\n"
    "#endif\n"
    "\n"
    "// Sanity check the use of guard pages\n"
    "#if WASM_RT_MEMCHECK_GUARD_PAGES && !WASM_RT_GUARD_PAGES_SUPPORTED\n"
    "#error \\\n"
    "    \"WASM_RT_MEMCHECK_GUARD_PAGES not supported on this "
    "platform/configuration\"\n"
    "#endif\n"
    "\n"
    "#if WASM_RT_MEMCHECK_GUARD_PAGES && WASM_RT_MEMCHECK_BOUNDS_CHECK\n"
    "#error \\\n"
    "    \"Cannot use both WASM_RT_MEMCHECK_GUARD_PAGES and "
    "WASM_RT_MEMCHECK_BOUNDS_CHECK\"\n"
    "\n"
    "#elif !WASM_RT_MEMCHECK_GUARD_PAGES && !WASM_RT_MEMCHECK_BOUNDS_CHECK\n"
    "#error \\\n"
    "    \"Must choose at least one from WASM_RT_MEMCHECK_GUARD_PAGES and "
    "WASM_RT_MEMCHECK_BOUNDS_CHECK\"\n"
    "#endif\n"
    "\n"
    "/**\n"
    " * Some configurations above require the Wasm runtime to install a "
    "signal\n"
    " * handler. However, this can be explicitly disallowed by the host using\n"
    " * WASM_RT_SKIP_SIGNAL_RECOVERY. In this case, when the wasm code "
    "encounters an\n"
    " * OOB access, it may either trap or abort.\n"
    " */\n"
    "#ifndef WASM_RT_SKIP_SIGNAL_RECOVERY\n"
    "#define WASM_RT_SKIP_SIGNAL_RECOVERY 0\n"
    "#endif\n"
    "\n"
    "#if WASM_RT_MEMCHECK_GUARD_PAGES && !WASM_RT_SKIP_SIGNAL_RECOVERY\n"
    "#define WASM_RT_INSTALL_SIGNAL_HANDLER 1\n"
    "#else\n"
    "#define WASM_RT_INSTALL_SIGNAL_HANDLER 0\n"
    "#endif\n"
    "\n"
    "#ifndef WASM_RT_USE_STACK_DEPTH_COUNT\n"
    "/* The signal handler on POSIX can detect call stack overflows. On "
    "windows, or\n"
    " * platforms without a signal handler, we use stack depth counting. */\n"
    "#if WASM_RT_INSTALL_SIGNAL_HANDLER && !defined(_WIN32)\n"
    "#define WASM_RT_USE_STACK_DEPTH_COUNT 0\n"
    "#else\n"
    "#define WASM_RT_USE_STACK_DEPTH_COUNT 1\n"
    "#endif\n"
    "#endif\n"
    "\n"
    "#if WASM_RT_USE_STACK_DEPTH_COUNT\n"
    "/**\n"
    " * When the signal handler cannot be used to detect stack overflows, "
    "stack depth\n"
    " * is limited explicitly. The maximum stack depth before trapping can be\n"
    " * configured by defining this symbol before including wasm-rt when "
    "building the\n"
    " * generated c files, for example:\n"
    " *\n"
    " * ```\n"
    " *   cc -c -DWASM_RT_MAX_CALL_STACK_DEPTH=100 my_module.c -o my_module.o\n"
    " * ```\n"
    " */\n"
    "#ifndef WASM_RT_MAX_CALL_STACK_DEPTH\n"
    "#define WASM_RT_MAX_CALL_STACK_DEPTH 500\n"
    "#endif\n"
    "\n"
    "/** Current call stack depth. */\n"
    "extern WASM_RT_THREAD_LOCAL uint32_t wasm_rt_call_stack_depth;\n"
    "\n"
    "#endif\n"
    "\n"
    "#if defined(_MSC_VER)\n"
    "#define WASM_RT_NO_RETURN __declspec(noreturn)\n"
    "#else\n"
    "#define WASM_RT_NO_RETURN __attribute__((noreturn))\n"
    "#endif\n"
    "\n"
    "#if defined(__APPLE__) && WASM_RT_INSTALL_SIGNAL_HANDLER\n"
    "#define WASM_RT_MERGED_OOB_AND_EXHAUSTION_TRAPS 1\n"
    "#else\n"
    "#define WASM_RT_MERGED_OOB_AND_EXHAUSTION_TRAPS 0\n"
    "#endif\n"
    "\n"
    "/** Reason a trap occurred. Provide this to `wasm_rt_trap`. */\n"
    "typedef enum {\n"
    "  WASM_RT_TRAP_NONE, /** No error. */\n"
    "  WASM_RT_TRAP_OOB,  /** Out-of-bounds access in linear memory or a "
    "table. */\n"
    "  WASM_RT_TRAP_INT_OVERFLOW, /** Integer overflow on divide or "
    "truncation. */\n"
    "  WASM_RT_TRAP_DIV_BY_ZERO,  /** Integer divide by zero. */\n"
    "  WASM_RT_TRAP_INVALID_CONVERSION, /** Conversion from NaN to integer. "
    "*/\n"
    "  WASM_RT_TRAP_UNREACHABLE,        /** Unreachable instruction executed. "
    "*/\n"
    "  WASM_RT_TRAP_CALL_INDIRECT,      /** Invalid call_indirect, for any "
    "reason. */\n"
    "  WASM_RT_TRAP_UNCAUGHT_EXCEPTION, /* Exception thrown and not caught */\n"
    "#if WASM_RT_MERGED_OOB_AND_EXHAUSTION_TRAPS\n"
    "  WASM_RT_TRAP_EXHAUSTION = WASM_RT_TRAP_OOB,\n"
    "#else\n"
    "  WASM_RT_TRAP_EXHAUSTION, /** Call stack exhausted. */\n"
    "#endif\n"
    "} wasm_rt_trap_t;\n"
    "\n"
    "/** Value types. Used to define function signatures. */\n"
    "typedef enum {\n"
    "  WASM_RT_I32,\n"
    "  WASM_RT_I64,\n"
    "  WASM_RT_F32,\n"
    "  WASM_RT_F64,\n"
    "  WASM_RT_V128,\n"
    "  WASM_RT_FUNCREF,\n"
    "  WASM_RT_EXTERNREF,\n"
    "} wasm_rt_type_t;\n"
    "\n"
    "/**\n"
    " * A generic function pointer type, both for Wasm functions (`code`)\n"
    " * and host functions (`hostcode`). All function pointers are stored\n"
    " * in this canonical form, but must be cast to their proper signature\n"
    " * to call.\n"
    " */\n"
    "typedef void (*wasm_rt_function_ptr_t)(void);\n"
    "\n"
    "/**\n"
    " * The type of a function (an arbitrary number of param and result "
    "types).\n"
    " * This is represented as an opaque 256-bit ID.\n"
    " */\n"
    "typedef const char* wasm_rt_func_type_t;\n"
    "\n"
    "/** A function instance (the runtime representation of a function).\n"
    " * These can be stored in tables of type funcref, or used as values. */\n"
    "typedef struct {\n"
    "  /** The function's type. */\n"
    "  wasm_rt_func_type_t func_type;\n"
    "  /** The function. The embedder must know the actual C signature of the\n"
    "   * function and cast to it before calling. */\n"
    "  wasm_rt_function_ptr_t func;\n"
    "  /** A function instance is a closure of the function over an instance\n"
    "   * of the originating module. The module_instance element will be "
    "passed into\n"
    "   * the function at runtime. */\n"
    "  void* module_instance;\n"
    "} wasm_rt_funcref_t;\n"
    "\n"
    "/** Default (null) value of a funcref */\n"
    "static const wasm_rt_funcref_t wasm_rt_funcref_null_value = {NULL, NULL, "
    "NULL};\n"
    "\n"
    "/** The type of an external reference (opaque to WebAssembly). */\n"
    "typedef unsigned char __attribute__( ( vector_size( 32 ) ) ) u8x32;\n"
    "typedef u8x32 wasm_rt_externref_t;\n"
    "\n"
    "/** Default (null) value of an externref. Strict accessible blob of size "
    "zero. */\n"
    "static const wasm_rt_externref_t wasm_rt_externref_null_value = { 0 };\n"
    "\n"
    "/** A Memory object. */\n"
    "typedef struct {\n"
    "  wasm_rt_externref_t ref; // Pointer to blob currently attached to "
    "memory\n"
    "  bool read_only;\n"
    "\n"
    "  /** The linear memory data, with a byte length of `size`. */\n"
    "  uint8_t* data;\n"
    "  /** The current and maximum page count for this Memory object. If there "
    "is no\n"
    "   * maximum, `max_pages` is 0xffffffffu (i.e. UINT32_MAX). */\n"
    "  uint64_t pages, max_pages;\n"
    "  /** The current size of the linear memory, in bytes. */\n"
    "  uint64_t size;\n"
    "  /** Is this memory indexed by u64 (as opposed to default u32) */\n"
    "  bool is64;\n"
    "} wasm_rt_memory_t;\n"
    "\n"
    "/** A Table of type funcref. */\n"
    "typedef struct {\n"
    "  wasm_rt_externref_t ref;\n"
    "  bool read_only;\n"
    "\n"
    "  /** The table element data, with an element count of `size`. */\n"
    "  wasm_rt_funcref_t* data;\n"
    "  /** The maximum element count of this Table object. If there is no "
    "maximum,\n"
    "   * `max_size` is 0xffffffffu (i.e. UINT32_MAX). */\n"
    "  uint32_t max_size;\n"
    "  /** The current element count of the table. */\n"
    "  uint32_t size;\n"
    "} wasm_rt_funcref_table_t;\n"
    "\n"
    "/** A Table of type externref. */\n"
    "typedef struct {\n"
    "  wasm_rt_externref_t ref; // Pointer to tree currently attached to "
    "table\n"
    "  bool read_only;\n"
    "\n"
    "  /** The table element data, with an element count of `size`. */\n"
    "  wasm_rt_externref_t* data;\n"
    "  /** The maximum element count of this Table object. If there is no "
    "maximum,\n"
    "   * `max_size` is 0xffffffffu (i.e. UINT32_MAX). */\n"
    "  uint32_t max_size;\n"
    "  /** The current element count of the table. */\n"
    "  uint32_t size;\n"
    "} wasm_rt_externref_table_t;\n"
    "\n"
    "/** Initialize the runtime. */\n"
    "void wasm_rt_init(void);\n"
    "\n"
    "/** Is the runtime initialized? */\n"
    "bool wasm_rt_is_initialized(void);\n"
    "\n"
    "/** Free the runtime's state. */\n"
    "void wasm_rt_free(void);\n"
    "\n"
    "/**\n"
    " * Stop execution immediately and jump back to the call to "
    "`wasm_rt_impl_try`.\n"
    " * The result of `wasm_rt_impl_try` will be the provided trap reason.\n"
    " *\n"
    " * This is typically called by the generated code, and not the embedder.\n"
    " */\n"
    "WASM_RT_NO_RETURN void wasm_rt_trap(wasm_rt_trap_t);\n"
    "\n"
    "/**\n"
    " * Return a human readable error string based on a trap type.\n"
    " */\n"
    "const char* wasm_rt_strerror(wasm_rt_trap_t trap);\n"
    "\n"
    "/**\n"
    " * A tag is represented as an arbitrary pointer.\n"
    " */\n"
    "typedef const void* wasm_rt_tag_t;\n"
    "\n"
    "/**\n"
    " * Set the active exception to given tag, size, and contents.\n"
    " */\n"
    "void wasm_rt_load_exception(const wasm_rt_tag_t tag,\n"
    "                            uint32_t size,\n"
    "                            const void* values);\n"
    "\n"
    "/**\n"
    " * Throw the active exception.\n"
    " */\n"
    "WASM_RT_NO_RETURN void wasm_rt_throw(void);\n"
    "\n"
    "/**\n"
    " * A hardened jmp_buf that allows us to checks if it is initialized "
    "before use\n"
    " */\n"
    "typedef struct {\n"
    "  /* Is the jmp buf intialized? */\n"
    "  bool initialized;\n"
    "  /* jmp_buf contents */\n"
    "  jmp_buf buffer;\n"
    "} wasm_rt_jmp_buf;\n"
    "\n"
    "/**\n"
    " * The type of an unwind target if an exception is thrown and caught.\n"
    " */\n"
    "#define WASM_RT_UNWIND_TARGET wasm_rt_jmp_buf\n"
    "\n"
    "/**\n"
    " * Get the current unwind target if an exception is thrown.\n"
    " */\n"
    "WASM_RT_UNWIND_TARGET* wasm_rt_get_unwind_target(void);\n"
    "\n"
    "/**\n"
    " * Set the unwind target if an exception is thrown.\n"
    " */\n"
    "void wasm_rt_set_unwind_target(WASM_RT_UNWIND_TARGET* target);\n"
    "\n"
    "/**\n"
    " * Tag of the active exception.\n"
    " */\n"
    "wasm_rt_tag_t wasm_rt_exception_tag(void);\n"
    "\n"
    "/**\n"
    " * Size of the active exception.\n"
    " */\n"
    "uint32_t wasm_rt_exception_size(void);\n"
    "\n"
    "/**\n"
    " * Contents of the active exception.\n"
    " */\n"
    "void* wasm_rt_exception(void);\n"
    "\n"
    "#if WASM_RT_INSTALL_SIGNAL_HANDLER && !defined(_WIN32)\n"
    "#define WASM_RT_SETJMP_SETBUF(buf) sigsetjmp(buf, 1)\n"
    "#else\n"
    "#define WASM_RT_SETJMP_SETBUF(buf) setjmp(buf)\n"
    "#endif\n"
    "\n"
    "#define WASM_RT_SETJMP(buf) \\\n"
    "  ((buf).initialized = true, WASM_RT_SETJMP_SETBUF((buf).buffer))\n"
    "\n"
    "#define wasm_rt_try(target) WASM_RT_SETJMP(target)\n"
    "\n"
    "/**\n"
    " * Initialize a Memory object with an initial page size of "
    "`initial_pages` and\n"
    " * a maximum page size of `max_pages`, indexed with an i32 or i64.\n"
    " *\n"
    " *  ```\n"
    " *    wasm_rt_memory_t my_memory;\n"
    " *    // 1 initial page (65536 bytes), and a maximum of 2 pages,\n"
    " *    // indexed with an i32\n"
    " *    wasm_rt_allocate_memory(&my_memory, 1, 2, false);\n"
    " *  ```\n"
    " */\n"
    "void wasm_rt_allocate_memory(wasm_rt_memory_t*,\n"
    "                             uint64_t initial_pages,\n"
    "                             uint64_t max_pages,\n"
    "                             bool is64);\n"
    "\n"
    "extern void wasm_rt_allocate_memory_sw_checked(wasm_rt_memory_t*,\n"
    "                                               uint64_t initial_pages,\n"
    "                                               uint64_t max_pages,\n"
    "                                               bool is64);\n"
    "\n"
    "/**\n"
    " * Grow a Memory object by `pages`, and return the previous page count. "
    "If\n"
    " * this new page count is greater than the maximum page count, the grow "
    "fails\n"
    " * and 0xffffffffu (UINT32_MAX) is returned instead.\n"
    " *\n"
    " *  ```\n"
    " *    wasm_rt_memory_t my_memory;\n"
    " *    ...\n"
    " *    // Grow memory by 10 pages.\n"
    " *    uint32_t old_page_size = wasm_rt_grow_memory(&my_memory, 10);\n"
    " *    if (old_page_size == UINT32_MAX) {\n"
    " *      // Failed to grow memory.\n"
    " *    }\n"
    " *  ```\n"
    " */\n"
    "uint64_t wasm_rt_grow_memory(wasm_rt_memory_t*, uint64_t pages);\n"
    "\n"
    "extern uint64_t wasm_rt_grow_memory_sw_checked(wasm_rt_memory_t*,\n"
    "                                               uint64_t pages);\n"
    "\n"
    "/**\n"
    " * Free a Memory object.\n"
    " */\n"
    "void wasm_rt_free_memory(wasm_rt_memory_t*);\n"
    "\n"
    "extern void wasm_rt_free_memory_sw_checked(wasm_rt_memory_t*);\n"
    "\n"
    "/**\n"
    " * Initialize a funcref Table object with an element count of `elements` "
    "and a\n"
    " * maximum size of `max_elements`.\n"
    " *\n"
    " *  ```\n"
    " *    wasm_rt_funcref_table_t my_table;\n"
    " *    // 5 elements and a maximum of 10 elements.\n"
    " *    wasm_rt_allocate_funcref_table(&my_table, 5, 10);\n"
    " *  ```\n"
    " */\n"
    "void wasm_rt_allocate_funcref_table(wasm_rt_funcref_table_t*,\n"
    "                                    uint32_t elements,\n"
    "                                    uint32_t max_elements);\n"
    "\n"
    "/**\n"
    " * Free a funcref Table object.\n"
    " */\n"
    "void wasm_rt_free_funcref_table(wasm_rt_funcref_table_t*);\n"
    "\n"
    "/**\n"
    " * Initialize an externref Table object with an element count\n"
    " * of `elements` and a maximum size of `max_elements`.\n"
    " * Usage as per wasm_rt_allocate_funcref_table.\n"
    " */\n"
    "void wasm_rt_allocate_externref_table(wasm_rt_externref_table_t*,\n"
    "                                      uint32_t elements,\n"
    "                                      uint32_t max_elements);\n"
    "\n"
    "/**\n"
    " * Free an externref Table object.\n"
    " */\n"
    "void wasm_rt_free_externref_table(wasm_rt_externref_table_t*);\n"
    "\n"
    "/**\n"
    " * Grow a Table object by `delta` elements (giving the new elements the "
    "value\n"
    " * `init`), and return the previous element count. If this new element "
    "count is\n"
    " * greater than the maximum element count, the grow fails and "
    "0xffffffffu\n"
    " * (UINT32_MAX) is returned instead.\n"
    " */\n"
    "uint32_t wasm_rt_grow_funcref_table(wasm_rt_funcref_table_t*,\n"
    "                                    uint32_t delta,\n"
    "                                    wasm_rt_funcref_t init);\n"
    "uint32_t wasm_rt_grow_externref_table(wasm_rt_externref_table_t*,\n"
    "                                      uint32_t delta,\n"
    "                                      wasm_rt_externref_t init);\n"
    "\n"
    "#ifdef __cplusplus\n"
    "}\n"
    "#endif\n"
    "\n"
    "#endif /* WASM_RT_H_ */\n";