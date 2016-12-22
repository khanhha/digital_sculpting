#ifndef VMESH_VMUTIL_DEFINE_H
#define VMESH_VMUTIL_DEFINE_H

#include <assert.h>

# define VM_BEGIN_NAMESPACE namespace VM{
# define VM_END_NAMESPACE }

#define LIMIT_MEM_POOL



/* varargs macros (keep first so others can use) */
/* --- internal helpers --- */
#define _VA_NARGS_GLUE(x, y) x y
#define _VA_NARGS_RETURN_COUNT(\
	_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, \
	_17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, \
	_33_, _34_, _35_, _36_, _37_, _38_, _39_, _40_, _41_, _42_, _43_, _44_, _45_, _46_, _47_, _48_, \
	_49_, _50_, _51_, _52_, _53_, _54_, _55_, _56_, _57_, _58_, _59_, _60_, _61_, _62_, _63_, _64_, \
	count, ...) count
#define _VA_NARGS_EXPAND(args) _VA_NARGS_RETURN_COUNT args
/* 64 args max */
#define _VA_NARGS_COUNT(...) _VA_NARGS_EXPAND((__VA_ARGS__, \
	64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, \
	48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, \
	32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
	16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2, 1, 0))
#define _VA_NARGS_OVERLOAD_MACRO2(name, count) name##count
#define _VA_NARGS_OVERLOAD_MACRO1(name, count) _VA_NARGS_OVERLOAD_MACRO2(name, count)
#define _VA_NARGS_OVERLOAD_MACRO(name,  count) _VA_NARGS_OVERLOAD_MACRO1(name, count)
/* --- expose for re-use --- */
#define VA_NARGS_CALL_OVERLOAD(name, ...) \
	_VA_NARGS_GLUE(_VA_NARGS_OVERLOAD_MACRO(name, _VA_NARGS_COUNT(__VA_ARGS__)), (__VA_ARGS__))

/* BMESH_ASSERT */
#ifdef WITH_ASSERT_ABORT
#  define _BMESH_DUMMY_ABORT abort
#else
#  define _BMESH_DUMMY_ABORT() (void)0
#endif

/* this is meant to be higher level then BLI_assert(),
* its enabled even when in Release mode*/
#define BMESH_ASSERT(a)                                                       \
	(void)((!(a)) ?  (                                                        \
		(                                                                     \
		fprintf(stderr,                                                       \
		        "BMESH_ASSERT failed: %s, %s(), %d at \'%s\'\n",              \
		        __FILE__, __FUNCTION__, __LINE__, STRINGIFY(a)),                  \
		_BMESH_DUMMY_ABORT(),                                                 \
		NULL)) : NULL)


/* ELEM#(v, ...): is the first arg equal any others? */
/* internal helpers*/
#define _VA_ELEM2(v, a) \
       ((v) == (a))
#define _VA_ELEM3(v, a, b) \
       (_VA_ELEM2(v, a) || ((v) == (b)))
#define _VA_ELEM4(v, a, b, c) \
       (_VA_ELEM3(v, a, b) || ((v) == (c)))
#define _VA_ELEM5(v, a, b, c, d) \
       (_VA_ELEM4(v, a, b, c) || ((v) == (d)))
#define _VA_ELEM6(v, a, b, c, d, e) \
       (_VA_ELEM5(v, a, b, c, d) || ((v) == (e)))
#define _VA_ELEM7(v, a, b, c, d, e, f) \
       (_VA_ELEM6(v, a, b, c, d, e) || ((v) == (f)))
#define _VA_ELEM8(v, a, b, c, d, e, f, g) \
       (_VA_ELEM7(v, a, b, c, d, e, f) || ((v) == (g)))
#define _VA_ELEM9(v, a, b, c, d, e, f, g, h) \
       (_VA_ELEM8(v, a, b, c, d, e, f, g) || ((v) == (h)))
#define _VA_ELEM10(v, a, b, c, d, e, f, g, h, i) \
       (_VA_ELEM9(v, a, b, c, d, e, f, g, h) || ((v) == (i)))
#define _VA_ELEM11(v, a, b, c, d, e, f, g, h, i, j) \
       (_VA_ELEM10(v, a, b, c, d, e, f, g, h, i) || ((v) == (j)))
#define _VA_ELEM12(v, a, b, c, d, e, f, g, h, i, j, k) \
       (_VA_ELEM11(v, a, b, c, d, e, f, g, h, i, j) || ((v) == (k)))
#define _VA_ELEM13(v, a, b, c, d, e, f, g, h, i, j, k, l) \
       (_VA_ELEM12(v, a, b, c, d, e, f, g, h, i, j, k) || ((v) == (l)))
#define _VA_ELEM14(v, a, b, c, d, e, f, g, h, i, j, k, l, m) \
       (_VA_ELEM13(v, a, b, c, d, e, f, g, h, i, j, k, l) || ((v) == (m)))
#define _VA_ELEM15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) \
       (_VA_ELEM14(v, a, b, c, d, e, f, g, h, i, j, k, l, m) || ((v) == (n)))
#define _VA_ELEM16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
       (_VA_ELEM15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) || ((v) == (o)))
#define _VA_ELEM17(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
       (_VA_ELEM16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) || ((v) == (p)))

/* reusable ELEM macro */
#define ELEM(...) VA_NARGS_CALL_OVERLOAD(_VA_ELEM, __VA_ARGS__)

/* no-op for expressions we don't want to instansiate, but must remian valid */
#define EXPR_NOP(expr) (void)(0 ? ((void)(expr), 1) : 0)



/*little macro so inline keyword works*/
#if defined(_MSC_VER)
#  define BLI_INLINE static __forceinline
#else
#  if (defined(__APPLE__) && defined(__ppc__))
/* static inline __attribute__ here breaks osx ppc gcc42 build */
#    define BLI_INLINE static __attribute__((always_inline)) __attribute__((__unused__))
#  else
#    define BLI_INLINE static inline __attribute__((always_inline)) __attribute__((__unused__))
#  endif
#endif

#if defined(_MSC_VER)
#	define BLI_MEMBER_INLINE __forceinline
#else
/*TODO*/
#endif

#if defined(__GNUC__) || defined(__clang__)
#if defined(__cplusplus) && (__cplusplus > 199711L)
#define BLI_array_alloca(arr, realsize) \
	(decltype(arr))alloca(sizeof(*arr) * (realsize))
#else
#define BLI_array_alloca(arr, realsize) \
	(typeof(arr))alloca(sizeof(*arr) * (realsize))
#endif
#else
#define BLI_array_alloca(arr, realsize) \
	alloca(sizeof(*arr) * (realsize))
#endif


#endif