/*
 *  The common define.
 *  Copyright (C)  2008 - 2012  Wangbo
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should hpSTL received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Author e-mail: activesys.wb@gmail.com
 *                 activesys@sina.com.cn
 */

#ifndef _CSTL_DEF_H_
#define _CSTL_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif


/** include section **/
#ifdef HpSTL_CONFIG_H
#   include <config.h>
#endif

#ifdef __GNUC__
typedef __SIZE_TYPE__ size_t;
#endif

#ifdef  __cplusplus
#define _ADDRESSOF(v)   ( &reinterpret_cast<const char &>(v) )
#else
#define _ADDRESSOF(v)   ( &(v) )
#endif

typedef char *  va_list;
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#define DBL_DIG         15                      /* # of decimal digits of precision */
#define DBL_EPSILON     2.2204460492503131e-016 /* smallest such that 1.0+DBL_EPSILON != 1.0 */
#define DBL_MANT_DIG    53                      /* # of bits in mantissa */
#define DBL_MAX         1.7976931348623158e+308 /* max value */
#define DBL_MAX_10_EXP  308                     /* max decimal exponent */
#define DBL_MAX_EXP     1024                    /* max binary exponent */
#define DBL_MIN         2.2250738585072014e-308 /* min positive value */
#define DBL_MIN_10_EXP  (-307)                  /* min decimal exponent */
#define DBL_MIN_EXP     (-1021)                 /* min binary exponent */
#define _DBL_RADIX      2                       /* exponent radix */
#define _DBL_ROUNDS     1                       /* addition rounding: near */

#define FLT_DIG         6                       /* # of decimal digits of precision */
#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */
#define FLT_GUARD       0
#define FLT_MANT_DIG    24                      /* # of bits in mantissa */
#define FLT_MAX         3.402823466e+38F        /* max value */
#define FLT_MAX_10_EXP  38                      /* max decimal exponent */
#define FLT_MAX_EXP     128                     /* max binary exponent */
#define FLT_MIN         1.175494351e-38F        /* min positive value */
#define FLT_MIN_10_EXP  (-37)                   /* min decimal exponent */
#define FLT_MIN_EXP     (-125)                  /* min binary exponent */
#define FLT_NORMALIZE   0
#define FLT_RADIX       2                       /* exponent radix */
#define FLT_ROUNDS      1                       /* addition rounding: near */

#define LDBL_DIG        DBL_DIG                 /* # of decimal digits of precision */
#define LDBL_EPSILON    DBL_EPSILON             /* smallest such that 1.0+LDBL_EPSILON != 1.0 */
#define LDBL_MANT_DIG   DBL_MANT_DIG            /* # of bits in mantissa */
#define LDBL_MAX        DBL_MAX                 /* max value */
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP          /* max decimal exponent */
#define LDBL_MAX_EXP    DBL_MAX_EXP             /* max binary exponent */
#define LDBL_MIN        DBL_MIN                 /* min positive value */
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP          /* min decimal exponent */
#define LDBL_MIN_EXP    DBL_MIN_EXP             /* min binary exponent */
#define _LDBL_RADIX     DBL_RADIX               /* exponent radix */
#define _LDBL_ROUNDS    DBL_ROUNDS              /* addition rounding: near */


/** constant declaration and macro section **/
#ifdef _CSTL_UNIT_TESTING
    extern void  mock_assert(const int result, const char* const expression, const char* const file, const int line);
    /*
    extern void* _test_malloc(const size_t size, const char* file, const int line);
    extern void* _test_calloc(const size_t num, const size_t size, const char* file, const int line);
    extern void* _test_free(void* const prt, const char* file, const int line);
    */

#   undef assert
#   define assert(expression) mock_assert((int)(expression), #expression, __FILE__, __LINE__)
/*
#   define malloc(size)       _test_malloc((size), __FILE__, __LINE__)
#   define calloc(num, size)  _test_calloc((num), (size), __FILE__, __LINE__)
#   define free(ptr)          _test_free((ptr), __FILE__, __LINE__)
*/
#	ifdef NDEBUG
#		undef NDEBUG
#	endif
#endif /* _CSTL_UNIT_TESTING */

#ifdef _MSC_VER
#   define va_copy(dest, src) ((dest) = (src))
#endif

/**
 * libcstl version macro.
 */
#define CSTL_VERSION             20100 /* libcstl version 2.1.0 */
#define CSTL_MAJOR_VERSION       2
#define CSTL_MINOR_VERSION       1
#define CSTL_REVISION_VERSION    0

/**
 * for bool_t type
 */
#define FALSE   0               /* declaration false bool type */
#define false   0
#define TRUE    1               /* declaration true bool type */
#define true    1

/** data type declaration and struct, union, enum section **/
typedef unsigned char            _byte_t;
typedef size_t                   bool_t;         /* declaration for bool type */

/** exported global variable declaration section **/

/** exported function prototype section **/

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_DEF_H_ */
/** eof **/

