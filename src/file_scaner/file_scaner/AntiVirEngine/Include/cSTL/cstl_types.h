/*
 *  This header file declaration some type definition, micors for cstl.
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

#ifndef _CSTL_TYPES_H_
#define _CSTL_TYPES_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "cSTL.h"
/** include section **/


/** constant declaration and macro section **/
/* c built-in type */
#define _CHAR_TYPE                   "char"
#define _SIGNED_CHAR_TYPE            "signed char"

#define _UNSIGNED_CHAR_TYPE          "unsigned char"

#define _SIGNED_SHORT_INT_TYPE       "signed short int"
#define _SIGNED_SHORT_TYPE           "signed short"
#define _SHORT_INT_TYPE              "short int"
#define _SHORT_TYPE                  "short"

#define _UNSIGNED_SHORT_INT_TYPE     "unsigned short int"
#define _UNSIGNED_SHORT_TYPE         "unsigned short"

#define _INT_TYPE                    "int"
#define _SIGNED_TYPE                 "signed"
#define _SIGNED_INT_TYPE             "signed int"

#define _UNSIGNED_INT_TYPE           "unsigned int"
#define _UNSIGNED_TYPE               "unsigned"

#define _LONG_TYPE                   "long"
#define _LONG_INT_TYPE               "long int"
#define _SIGNED_LONG_TYPE            "signed long"
#define _SIGNED_LONG_INT_TYPE        "signed long int"

#define _UNSIGNED_LONG_TYPE          "unsigned long"
#define _UNSIGNED_LONG_INT_TYPE      "unsigned long int"

#define _FLOAT_TYPE                  "float"
#define _DOUBLE_TYPE                 "double"
#define _LONG_DOUBLE_TYPE            "long double"

#define _C_STRING_TYPE               "char*"

#define _BOOL_TYPE                   "bool_t"
/* cstl type */
#define _VECTOR_TYPE                 "vector_t"
#define _LIST_TYPE                   "list_t"
#define _SLIST_TYPE                  "slist_t"
#define _DEQUE_TYPE                  "deque_t"
#define _STACK_TYPE                  "stack_t"
#define _QUEUE_TYPE                  "queue_t"
#define _PRIORITY_QUEUE_TYPE         "priority_queue_t"
#define _SET_TYPE                    "set_t"
#define _MAP_TYPE                    "map_t"
#define _MULTISET_TYPE               "multiset_t"
#define _MULTIMAP_TYPE               "multimap_t"
#define _HASH_SET_TYPE               "hash_set_t"
#define _HASH_MAP_TYPE               "hash_map_t"
#define _HASH_MULTISET_TYPE          "hash_multiset_t"
#define _HASH_MULTIMAP_TYPE          "hash_multimap_t"
#define _STRING_TYPE                 "string_t"
#define _PAIR_TYPE                   "pair_t"
/* cstl iterator type */
#define _ITERATOR_TYPE               "iterator_t"
#define _VECTOR_ITERATOR_TYPE        "vector_iterator_t"
#define _LIST_ITERATOR_TYPE          "list_iterator_t"
#define _SLIST_ITERATOR_TYPE         "slist_iterator_t"
#define _DEQUE_ITERATOR_TYPE         "deque_iterator_t"
#define _SET_ITERATOR_TYPE           "set_iterator_t"
#define _MAP_ITERATOR_TYPE           "map_iterator_t"
#define _MULTISET_ITERATOR_TYPE      "multiset_iterator_t"
#define _MULTIMAP_ITERATOR_TYPE      "multimap_iterator_t"
#define _HASH_SET_ITERATOR_TYPE      "hash_set_iterator_t"
#define _HASH_MAP_ITERATOR_TYPE      "hash_map_iterator_t"
#define _HASH_MULTISET_ITERATOR_TYPE "hash_multiset_iterator_t"
#define _HASH_MULTIMAP_ITERATOR_TYPE "hash_multimap_iterator_t"
#define _STRING_ITERATOR_TYPE        "string_iterator_t"
#define _INPUT_ITERATOR_TYPE         "input_iterator_t"
#define _OUTPUT_ITERATOR_TYPE        "output_iterator_t"
#define _FORWARD_ITERATOR_TYPE       "forward_iterator_t"
#define _BIDIRECTIONAL_ITERATOR_TYPE "bidirectional_iterator_t"
#define _RANDOM_ACCESS_ITERATOR_TYPE "random_access_iterator_t"

/* structure type */
#define _STRUCT_TYPE                 "struct"
#define _UNION_TYPE                  "union"
#define _ENUM_TYPE                   "enum"


/* cstl container type */
#define _CSTL_LEFT_BRACKET           '<'
#define _CSTL_RIGHT_BRACKET          '>'
#define _CSTL_COMMA                  ','
/* set */
#define _SET_IDENTIFY                "set"
#define _SET_LEFT_BRACKET            "<"
#define _SET_RIGHT_BRACKET           ">"
#define _SET_ITERATOR_NAME           "set_iterator_t"
/* multiset */
#define _MULTISET_IDENTIFY           "multiset"
#define _MULTISET_LEFT_BRACKET       "<"
#define _MULTISET_RIGHT_BRACKET      ">"
#define _MULTISET_ITERATOR_NAME      "multiset_iterator_t"
/* map */
#define _MAP_IDENTIFY                "map"
#define _MAP_LEFT_BRACKET            "<"
#define _MAP_RIGHT_BRACKET           ">"
#define _MAP_COMMA                   ","
#define _MAP_ITERATOR_NAME           "map_iterator_t"
/* multimap */
#define _MULTIMAP_IDENTIFY           "multimap"
#define _MULTIMAP_LEFT_BRACKET       "<"
#define _MULTIMAP_RIGHT_BRACKET      ">"
#define _MULTIMAP_COMMA              ","
#define _MULTIMAP_ITERATOR_NAME      "multimap_iterator_t"
/* hash_set */
#define _HASH_SET_IDENTIFY           "hash_set"
#define _HASH_SET_LEFT_BRACKET       "<"
#define _HASH_SET_RIGHT_BRACKET      ">"
#define _HASH_SET_ITERATOR_NAME      "hash_set_iterator_t"
/* hash_multiset */
#define _HASH_MULTISET_IDENTIFY      "hash_multiset"
#define _HASH_MULTISET_LEFT_BRACKET  "<"
#define _HASH_MULTISET_RIGHT_BRACKET ">"
#define _HASH_MULTISET_ITERATOR_NAME "hash_multiset_iterator_t"
/* hash_map */
#define _HASH_MAP_IDENTIFY           "hash_map"
#define _HASH_MAP_LEFT_BRACKET       "<"
#define _HASH_MAP_RIGHT_BRACKET      ">"
#define _HASH_MAP_COMMA              ","
#define _HASH_MAP_ITERATOR_NAME      "hash_map_iterator_t"
/* hash_multimap */
#define _HASH_MULTIMAP_IDENTIFY      "hash_multimap"
#define _HASH_MULTIMAP_LEFT_BRACKET  "<"
#define _HASH_MULTIMAP_RIGHT_BRACKET ">"
#define _HASH_MULTIMAP_COMMA         ","
#define _HASH_MULTIMAP_ITERATOR_NAME "hash_multimap_iterator_t"
/* basic_string */
#define _BASIC_STRING_IDENTIFY       "basic_string"
#define _BASIC_STRING_LEFT_BRACKET   "<"
#define _BASIC_STRING_RIGHT_BRACKET  ">"
#define _BASIC_STRING_ITERATOR_NAME  "basic_string_iterator_t"

/** data type declaration and struct, union, enum section **/

/*
 * Type definition of unary function and binary function.
 * Note: The last parameter is for output
 */
typedef void (*unary_function_t)(cSTL* pSTL,const void*, void*);
typedef void (*binary_function_t)(cSTL* pSTL,const void*, const void*, void*);

/* 
 * Type register hash table.
 */
/* type structure for all container. */
#define _TYPE_NAME_SIZE              255
/* type style */
typedef enum _tagtypestley
{
    _TYPE_INVALID, _TYPE_C_BUILTIN, _TYPE_USER_DEFINE, _TYPE_CSTL_BUILTIN
}_typestyle_t;

typedef struct _tagtype
{
    size_t               _t_typesize;                        /* type size */
    char                 _s_typename[_TYPE_NAME_SIZE + 1];   /* type name */
    _typestyle_t         _t_style;                           /* type style */
    binary_function_t    _t_typecopy;                        /* type copy function */
    binary_function_t    _t_typeless;                        /* type less function */
    unary_function_t     _t_typeinit;                        /* type initialize function */
    unary_function_t     _t_typedestroy;                     /* type destroy function */
}_type_t;

/* type register node */
typedef struct _tagtypenode
{
    char                 _s_typename[_TYPE_NAME_SIZE + 1];   /* type name */
    struct _tagtypenode* _pt_next;                           /* next node */
    _type_t*             _pt_type;                           /* the registered type */
}_typenode_t;

/* type register table */
#define _TYPE_REGISTER_BUCKET_COUNT  997   /* register hash table bucket count */
typedef struct _tagtyperegister
{
    bool_t               _t_isinit; /* is initializate for built in types */
    _typenode_t*         _apt_bucket[_TYPE_REGISTER_BUCKET_COUNT]; /* hash table */
    _alloc_t             _t_allocator;
}_typeregister_t;

typedef struct _tagtypeinfo
{
    char                 _s_typename[_TYPE_NAME_SIZE + 1];
    _type_t*             _pt_type;
    _typestyle_t         _t_style;
}_typeinfo_t;

/** exported global variable declaration section **/

/** exported function prototype section **/
#define type_register(pSTL,type, type_init, type_copy, type_less, type_destroy)\
    _type_register((pSTL),sizeof(type), #type, (type_init), (type_copy), (type_less), (type_destroy))
#define type_unregister(pSTL,type)\
    _type_unregister((pSTL),sizeof(type), #type)
#define type_duplicate(type1, type2)\
    _type_duplicate(sizeof(type1), #type1, sizeof(type2), #type2)

extern bool_t _type_register(cSTL* pSTL,
    size_t t_typesize, const char* s_typename,
    unary_function_t t_typeinit, binary_function_t t_typecopy,
    binary_function_t t_typeless, unary_function_t t_typedestroy);
/*extern void _type_unregister(size_t t_typesize, const char* s_typename);*/
extern bool_t _type_duplicate(cSTL* pSTL,
    size_t t_typesize1, const char* s_typename1,
    size_t t_typesize2, const char* s_typename2);
extern void _type_get_type(cSTL* pSTL,_typeinfo_t* pt_typeinfo, const char* s_typename);
extern void _type_get_type_pair(cSTL* pSTL,_typeinfo_t* pt_typeinfofirst, _typeinfo_t* pt_typeinfosecond, const char* s_typename);
extern bool_t _type_is_same(cSTL* pSTL,const char* s_typename1, const char* s_typename2);
extern bool_t _type_is_same_ex(cSTL* pSTL,const _typeinfo_t* pt_first, const _typeinfo_t* pt_second);
extern void _type_get_varg_value(cSTL* pSTL,_typeinfo_t* pt_typeinfo, va_list val_elemlist, void* pv_output);
extern void _type_get_elem_typename(const char* s_typename, char* s_elemtypename);

extern void _type_debug(void);

/* default initialize, copy, less, and destroy function */
/* node: the pv_output is used for the size of default type, 
 * so invoke the copy, less, and destroy functions must be put the 
 * type size into the function through pv_output, even if is not used.
 */
extern void _type_init_default(cSTL* pSTL,const void* cpv_input, void* pv_output);
extern void _type_copy_default(cSTL* pSTL,const void* cpv_first, const void* cpv_second, void* pv_output);
extern void _type_less_default(cSTL* pSTL,const void* cpv_first, const void* cpv_second, void* pv_output);
extern void _type_destroy_default(cSTL* pSTL,const void* cpv_input, void* pv_output);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_TYPES_H_ */
/** eof **/

