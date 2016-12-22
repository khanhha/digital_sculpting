#ifndef BASELIB_UTIL_MACRO_H
#define BASELIB_UTIL_MACRO_H

/* hints for branch prediction, only use in code that runs a _lot_ where */
#ifdef __GNUC__
#  define LIKELY(x)       __builtin_expect(!!(x), 1)
#  define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#  define LIKELY(x)       (x)
#  define UNLIKELY(x)     (x)
#endif


/* Macro to convert a value to string in the preprocessor
* STRINGIFY_ARG: gives the argument as a string
* STRINGIFY_APPEND: appends any argument 'b' onto the string argument 'a',
*   used by STRINGIFY because some preprocessors warn about zero arguments
* STRINGIFY: gives the argument's value as a string */
#define STRINGIFY_ARG(x) "" #x
#define STRINGIFY_APPEND(a, b) "" a #b
#define STRINGIFY(x) STRINGIFY_APPEND("", x)

#ifdef NDEBUG
#define BLI_assert(a)(void)0
#else
#define BLI_assert(a) assert(a)
#endif

#if 0
#ifndef _DEBUG
#define NDEBUG
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#define POINTER_OFFSET(v, ofs) \
	((typeof(v))((char *)(v) + (ofs)))
#else
#define POINTER_OFFSET(v, ofs) \
	((void *)((char *)(v) + (ofs)))
#endif

/* hint to make sure function result is actually used */
#ifdef __GNUC__
#  define ATTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#  define ATTR_WARN_UNUSED_RESULT
#endif


/* hint to mark function arguments expected to be non-null
* if no arguments are given to the macro, all of pointer
* arguments would be expected to be non-null
*/
#ifdef __GNUC__
#  define ATTR_NONNULL(args ...) __attribute__((nonnull(args)))
#else
#  define ATTR_NONNULL(...)
#endif


#define STREQ(a, b) (strcmp(a, b) == 0)
#define STRCASEEQ(a, b) (strcasecmp(a, b) == 0)
#define STREQLEN(a, b, n) (strncmp(a, b, n) == 0)
#define STRCASEEQLEN(a, b, n) (strncasecmp(a, b, n) == 0)

#define STRPREFIX(a, b) (strncmp((a), (b), strlen(b)) == 0)

/* add platform/compiler checks here if it is not supported */
/* all platforms support forcing inline so this is always enabled */
#define BLI_MATH_DO_INLINE 1

#if BLI_MATH_DO_INLINE
#  ifdef _MSC_VER
#    define MINLINE static __forceinline
#    define MALWAYS_INLINE MINLINE
#  else
#    define MINLINE static inline
#    if (defined(__APPLE__) && defined(__ppc__))
/* static inline __attribute__ here breaks osx ppc gcc42 build */
#      define MALWAYS_INLINE static __attribute__((always_inline)) __attribute__((unused))
#    else
#      define MALWAYS_INLINE static inline __attribute__((always_inline)) __attribute__((unused))
#    endif
#  endif
#else
#  define MINLINE
#  define MALWAYS_INLINE
#endif

#endif