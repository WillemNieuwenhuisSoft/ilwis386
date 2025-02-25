/* source/headers/geos/platform.h.  Generated from platform.h.in by configure.  */
#ifndef GEOS_PLATFORM_H
#define GEOS_PLATFORM_H

/* Set to 1 if you have `int64_t' type */
#define HAVE_INT64_T_64 /**/

/* Set to 1 if `long int' is 64 bits */
/* #undef HAVE_LONG_INT_64 */

/* Set to 1 if `long long int' is 64 bits */
/* #undef HAVE_LONG_LONG_INT_64 */

/* Set to 1 if you have ieeefp.h */
/* #undef HAVE_IEEEFP_H */

/* Has finite */
#define HAVE_FINITE 1

/* Has isfinite */
#define HAVE_ISFINITE 1

/* Has isnan */
/* #undef HAVE_ISNAN */

#ifdef HAVE_IEEEFP_H
extern "C"
{
#include <ieeefp.h>
}
#endif

#ifdef HAVE_INT64_T_64
extern "C"
{
#include <inttypes.h>
}
#endif

#if defined(__GNUC__) && defined(_WIN32)
/* For MingW the appropriate definitions are included in
 math.h and float.h but the definitions in 
 math.h are only included if __STRICT_ANSI__
 is not defined.  Since GEOS is compiled with -ansi that
 means those definitions are not available. */
#include <float.h>
#endif

#include <cmath> // for finite()/isfinite() and isnan()
#include <limits> // for std::numeric_limits



//Defines NaN for intel platforms
#define DoubleNotANumber std::numeric_limits<double>::quiet_NaN()

//Don't forget to define infinities
#define DoubleInfinity std::numeric_limits<double>::infinity()
#define DoubleNegInfinity -std::numeric_limits<double>::infinity()

#define DoubleMax 1e308

inline bool
isFinite(double d)
{
#if defined(HAVE_FINITE) && !defined(HAVE_ISFINITE) && !defined(__MINGW32__)
    return (finite(d));
#else
    // Put using namespace std; here if you have to
    // put it anywhere.
    using namespace std;
	return d != std::numeric_limits<double>::infinity();
#endif
}
#define FINITE(x) ( isFinite(x) ) 

#ifdef HAVE_ISNAN
#define ISNAN(x) ( isnan(x) )
#else
// Hack for OS/X <cmath> incorrectly re-defining isnan() into
// oblivion. It does leave a version in std.
#define ISNAN(x) ( _isnan(x) )
#endif

#ifdef HAVE_INT64_T_64
  typedef int64_t int64;
#else
# ifdef HAVE_LONG_LONG_INT_64
   typedef long long int int64;
# else
   typedef long int int64;
#  ifndef HAVE_LONG_INT_64
#   define INT64_IS_REALLY32 1
#   warning "Could not find 64bit integer definition!"
#  endif
# endif
#endif

#define _export __declspec(dllexport)

inline int getMachineByteOrder() {
	static int endian_check = 1; // don't modify !!
	return *((char *)&endian_check); // 0 == big_endian | xdr,
					 // 1 == little_endian | ndr
}

#endif
