/*
 *  The interface of function.
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

#ifndef _CSTL_FUNCTION_H_
#define _CSTL_FUNCTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/

/** exported global variable declaration section **/

/** exported function prototype section **/
/* arithmetic function */
/* plus */
extern void fun_plus_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_plus_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* minus */
extern void fun_minus_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_minus_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* multiplies */
extern void fun_multiplies_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_multiplies_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* divides */
extern void fun_divides_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_divides_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* modulus */
extern void fun_modulus_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_modulus_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_modulus_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_modulus_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_modulus_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_modulus_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_modulus_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_modulus_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* negation */
extern void fun_negate_char(cSTL* pSTL,
    const void* cpv_input, void* pv_output);
extern void fun_negate_short(cSTL* pSTL,
    const void* cpv_input, void* pv_output);
extern void fun_negate_int(cSTL* pSTL,
    const void* cpv_input, void* pv_output);
extern void fun_negate_long(cSTL* pSTL,
    const void* cpv_input, void* pv_output);
extern void fun_negate_float(cSTL* pSTL,
    const void* cpv_input, void* pv_output);
extern void fun_negate_double(cSTL* pSTL,
    const void* cpv_input, void* pv_output);
extern void fun_negate_long_double(cSTL* pSTL,
    const void* cpv_input, void* pv_output);

/* comparisons */
/* the output parameter must be bool_t */
/* equality */
extern void fun_equal_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_cstr(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);

extern void fun_equal_vector(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_deque(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_list(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_slist(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_queue(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_stack(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_string(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_pair(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_hash_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_hash_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_hash_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_equal_hash_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* inequality */
extern void fun_not_equal_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_cstr(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);

extern void fun_not_equal_vector(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_deque(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_list(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_slist(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_queue(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_stack(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_string(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_pair(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_hash_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_hash_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_hash_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_not_equal_hash_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* greater then */
extern void fun_greater_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_cstr(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);

extern void fun_greater_vector(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_deque(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_list(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_slist(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_queue(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_stack(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_string(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_pair(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_hash_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_hash_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_hash_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_hash_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* greater then or equal*/
extern void fun_greater_equal_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_cstr(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);

extern void fun_greater_equal_vector(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_deque(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_list(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_slist(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_queue(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_stack(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_string(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_pair(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_hash_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_hash_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_hash_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_greater_equal_hash_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* less then */
extern void fun_less_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_cstr(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);

extern void fun_less_vector(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_deque(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_list(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_slist(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_queue(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_stack(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_string(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_pair(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_hash_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_hash_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_hash_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_hash_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* less then or equal*/
extern void fun_less_equal_char(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_uchar(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_short(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_ushort(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_int(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_uint(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_long(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_ulong(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_float(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_long_double(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_cstr(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);

extern void fun_less_equal_vector(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_deque(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_list(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_slist(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_queue(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_stack(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_string(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_pair(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_hash_set(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_hash_map(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_hash_multiset(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
extern void fun_less_equal_hash_multimap(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* locical */
/* logical and */
extern void fun_logical_and_bool(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* logical or */
extern void fun_logical_or_bool(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* pv_output);
/* logical not */
extern void fun_logical_not_bool(cSTL* pSTL,
    const void* cpv_input, void* pv_output);

/* random number */
extern void fun_random_number(cSTL* pSTL,
    const void* cpv_input, void* pv_output);

/* note: there is no implementation of identity select and project function. */
/* note: there is no implementation of function adapters */

/* default unary and binary function */
extern void fun_default_unary(cSTL* pSTL,
    const void* cpv_input, void* pv_output);
extern void fun_default_binary(cSTL* pSTL,
    const void* cpv_first, const void* cpv_second, void* output);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_FUNCTION_H_ */
/** eof **/

