// Microsoft version of 'inline'
#define inline __inline


//#define FIXED_POINT

/* Default to floating point */
#ifndef FIXED_POINT
#  define FLOATING_POINT
#endif


// In Visual Studio, _M_IX86_FP=1 means /arch:SSE was used, likewise
// _M_IX86_FP=2 means /arch:SSE2 was used.
// Also, enable both _USE_SSE and _USE_SSE2 if we're compiling for x86-64
#ifndef FIXED_POINT

#if _M_IX86_FP >= 1 || defined(_M_X64)
#define _USE_SSE
#endif

#if _M_IX86_FP >= 2 || defined(_M_X64)
#define _USE_SSE2
#endif

#endif	// !FIXED_POINT


// Visual Studio support alloca(), but it always align variables to 16-bit
// boundary, while SSE need 128-bit alignment. So we disable alloca() when
// SSE is enabled.
#if defined(_WIN32)
#ifndef _USE_SSE
#  define USE_ALLOCA
#endif
#else
#define	VAR_ARRAYS
#endif


/* We don't support visibility on Win32 */
#define EXPORT
