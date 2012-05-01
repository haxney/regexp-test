
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef REGEX_STANDALONE
# define	REGEX_STANDALONE
#endif

#ifdef REGEX_WCHAR
#	include <wctype.h>
#	include <wchar.h>
	typedef wchar_t chr;
	typedef chr Tcl_UniChar;
#else
#	include <ctype.h>
	typedef unsigned char chr;
	typedef wchar_t Tcl_UniChar;
#endif

/*
 * In The standalone version we are more concerned with performance,
 * so an automatic var is our best choice.
 */
#define AllocVars(vPtr)					\
		struct vars regex_autovar;		\
		register struct vars *vPtr = &regex_autovar;

#define MALLOC(n)	calloc(1,n)
#define FREE(p)		free(VS(p))
#define REALLOC(p,n)	realloc(VS(p),n)
#define ckalloc(n)	calloc(1,n)
#define ckrealloc(p,n)	realloc(p,n)
#define ckfree(p)	free(p)

#ifdef REGEX_WCHAR
#	define Tcl_UniCharToLower(c)		towlower(c)
#	define Tcl_UniCharToUpper(c)		towupper(c)
#	define Tcl_UniCharToTitle(c)		towupper(c)
#	define Tcl_UniCharIsAlpha(c)		iswalpha(c)
#	define Tcl_UniCharIsAlnum(c)		iswalnum(c)
#	define Tcl_UniCharIsDigit(c)		iswdigit(c)
#	define Tcl_UniCharIsSpace(c)		iswspace(c)
#else
#	define Tcl_DStringInit(ds)
#	define Tcl_UniCharToUtfDString(s,l,ds)	(s)
#	define Tcl_DStringFree(ds)
#	define Tcl_UniCharToLower(c)		tolower(c)
#	define Tcl_UniCharToUpper(c)		toupper(c)
#	define Tcl_UniCharToTitle(c)		toupper(c)
#	define Tcl_UniCharIsAlpha(c)		isalpha(c)
#	define Tcl_UniCharIsAlnum(c)		isalnum(c)
#	define Tcl_UniCharIsDigit(c)		isdigit(c)
#	define Tcl_UniCharIsSpace(c)		isspace(c)
#endif


/*
 * The maximum number of bytes that are necessary to represent a single
 * Unicode character in UTF-8. The valid values should be 3 or 6 (or perhaps 1
 * if we want to support a non-unicode enabled core). If 3, then Tcl_UniChar
 * must be 2-bytes in size (UCS-2) (the default). If 6, then Tcl_UniChar must
 * be 4-bytes in size (UCS-4). At this time UCS-2 mode is the default and
 * recommended mode. UCS-4 is experimental and not recommended. It works for
 * the core, but most extensions expect UCS-2.
 */

#ifndef TCL_UTF_MAX
#define TCL_UTF_MAX		3
#endif


/*
 * The structure defined below is used to hold dynamic strings. The only
 * fields that clients should use are string and length, accessible via the
 * macros Tcl_DStringValue and Tcl_DStringLength.
 */

#define TCL_DSTRING_STATIC_SIZE 200
typedef struct Tcl_DString {
    char *string;		/* Points to beginning of string: either
				 * staticSpace below or a malloced array. */
    int length;			/* Number of non-NULL characters in the
				 * string. */
    int spaceAvl;		/* Total number of bytes available for the
				 * string and its terminating NULL char. */
    char staticSpace[TCL_DSTRING_STATIC_SIZE];
				/* Space to use in common case where string is
				 * small. */
} Tcl_DString;

#define Tcl_DStringLength(dsPtr) ((dsPtr)->length)
#define Tcl_DStringValue(dsPtr) ((dsPtr)->string)


/*
 * The macro below is used to modify a "char" value (e.g. by casting it to an
 * unsigned character) so that it can be used safely with macros such as
 * isspace.
 */

#define UCHAR(c) ((unsigned char) (c))


/*
 * Used to tag functions that are only to be visible within the module being
 * built and not outside it (where this is supported by the linker).
 */

#ifndef MODULE_SCOPE
#   ifdef __cplusplus
#	define MODULE_SCOPE extern "C"
#   else
#	define MODULE_SCOPE extern
#   endif
#endif


/*
 * Macros used to declare a function to be exported by a DLL. Used by Windows,
 * maps to no-op declarations on non-Windows systems. The default build on
 * windows is for a DLL, which causes the DLLIMPORT and DLLEXPORT macros to be
 * nonempty. To build a static library, the macro STATIC_BUILD should be
 * defined.
 *
 * Note: when building static but linking dynamically to MSVCRT we must still
 *       correctly decorate the C library imported function.  Use CRTIMPORT
 *       for this purpose.  _DLL is defined by the compiler when linking to
 *       MSVCRT.
 */

#if (defined(__WIN32__) && (defined(_MSC_VER) || (__BORLANDC__ >= 0x0550) || defined(__LCC__) || defined(__WATCOMC__) || (defined(__GNUC__) && defined(__declspec))))
#   define HAVE_DECLSPEC 1
#   ifdef STATIC_BUILD
#       define DLLIMPORT
#       define DLLEXPORT
#       ifdef _DLL
#           define CRTIMPORT __declspec(dllimport)
#       else
#           define CRTIMPORT
#       endif
#   else
#       define DLLIMPORT __declspec(dllimport)
#       define DLLEXPORT __declspec(dllexport)
#       define CRTIMPORT __declspec(dllimport)
#   endif
#else
#   define DLLIMPORT
#   if defined(__GNUC__) && __GNUC__ > 3
#       define DLLEXPORT __attribute__ ((visibility("default")))
#   else
#       define DLLEXPORT
#   endif
#   define CRTIMPORT
#endif

/*
 * These macros are used to control whether functions are being declared for
 * import or export. If a function is being declared while it is being built
 * to be included in a shared library, then it should have the DLLEXPORT
 * storage class. If is being declared for use by a module that is going to
 * link against the shared library, then it should have the DLLIMPORT storage
 * class. If the symbol is beind declared for a static build or for use from a
 * stub library, then the storage class should be empty.
 *
 * The convention is that a macro called BUILD_xxxx, where xxxx is the name of
 * a library we are building, is set on the compile line for sources that are
 * to be placed in the library. When this macro is set, the storage class will
 * be set to DLLEXPORT. At the end of the header file, the storage class will
 * be reset to DLLIMPORT.
 */

#undef TCL_STORAGE_CLASS
#ifdef BUILD_tcl
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_TCL_STUBS
#      define TCL_STORAGE_CLASS
#   else
#      define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif

/*
 * Definitions that allow this header file to be used either with or without
 * ANSI C features like function prototypes.
 */

#undef _ANSI_ARGS_
#undef CONST
#ifndef INLINE
#   define INLINE
#endif

#ifndef NO_CONST
#   define CONST const
#else
#   define CONST
#endif

#ifndef NO_PROTOTYPES
#   define _ANSI_ARGS_(x)	x
#else
#   define _ANSI_ARGS_(x)	()
#endif

#ifdef USE_NON_CONST
#   ifdef USE_COMPAT_CONST
#      error define at most one of USE_NON_CONST and USE_COMPAT_CONST
#   endif
#   define CONST84
#   define CONST84_RETURN
#else
#   ifdef USE_COMPAT_CONST
#      define CONST84
#      define CONST84_RETURN CONST
#   else
#      define CONST84 CONST
#      define CONST84_RETURN CONST
#   endif
#endif

#ifndef CONST86
#      define CONST86 CONST
#endif

/*
 * Make sure EXTERN isn't defined elsewhere
 */

#ifdef EXTERN
#   undef EXTERN
#endif /* EXTERN */

#ifdef __cplusplus
#   define EXTERN extern "C" TCL_STORAGE_CLASS
#else
#   define EXTERN extern TCL_STORAGE_CLASS
#endif


#ifdef REGEX_WCHAR
EXTERN void		Tcl_DStringFree (Tcl_DString * dsPtr);
EXTERN void		Tcl_DStringInit (Tcl_DString * dsPtr);
EXTERN char *		Tcl_UniCharToUtfDString (CONST Tcl_UniChar * uniStr, 
				int uniLength, Tcl_DString * dsPtr);
EXTERN void		Tcl_DStringSetLength (Tcl_DString * dsPtr, 
				int length);
#endif		/* REGEX_WCHAR	*/
