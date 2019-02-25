/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef Json__h
#define Json__h

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Json Types: */
#define Json_False 0
#define Json_True 1
#define Json_NULL 2
#define Json_Number 3
#define Json_String 4
#define Json_Array 5
#define Json_Object 6
	
#define Json_IsReference 256
#define Json_StringIsConst 512

/* The Json structure: */
typedef struct Json {
	struct Json *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct Json *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==Json_String */
	int valueint;				/* The item's number, if type==Json_Number */
	double valuedouble;			/* The item's number, if type==Json_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} Json;

typedef struct Json_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} Json_Hooks;

/* Supply malloc, realloc and free functions to Json */
extern void Json_InitHooks(Json_Hooks* hooks);


/* Supply a block of JSON, and this returns a Json object you can interrogate. Call Json_Delete when finished. */
extern Json *Json_Parse(const char *value);
/* Render a Json entity to text for transfer/storage. Free the char* when finished. */
extern char  *Json_Print(Json *item);
/* Render a Json entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *Json_PrintUnformatted(Json *item);
/* Render a Json entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
extern char *Json_PrintBuffered(Json *item,int prebuffer,int fmt);
/* Delete a Json entity and all subentities. */
extern void   Json_Delete(Json *c);

/* Returns the number of items in an array (or object). */
extern int	  Json_GetArraySize(Json *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern Json *Json_GetArrayItem(Json *array,int item);
/* Get item "string" from object. Case insensitive. */
extern Json *Json_GetObjectItem(Json *object,const char *string);
extern int Json_HasObjectItem(Json *object,const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when Json_Parse() returns 0. 0 when Json_Parse() succeeds. */
extern const char *Json_GetErrorPtr(void);
	
/* These calls create a Json item of the appropriate type. */
extern Json *Json_CreateNull(void);
extern Json *Json_CreateTrue(void);
extern Json *Json_CreateFalse(void);
extern Json *Json_CreateBool(int b);
extern Json *Json_CreateNumber(double num);
extern Json *Json_CreateString(const char *string);
extern Json *Json_CreateArray(void);
extern Json *Json_CreateObject(void);

/* These utilities create an Array of count items. */
extern Json *Json_CreateIntArray(const int *numbers,int count);
extern Json *Json_CreateFloatArray(const float *numbers,int count);
extern Json *Json_CreateDoubleArray(const double *numbers,int count);
extern Json *Json_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void Json_AddItemToArray(Json *array, Json *item);
extern void	Json_AddItemToObject(Json *object,const char *string,Json *item);
extern void	Json_AddItemToObjectCS(Json *object,const char *string,Json *item);	/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the Json object */
/* Append reference to item to the specified array/object. Use this when you want to add an existing Json to a new Json, but don't want to corrupt your existing Json. */
extern void Json_AddItemReferenceToArray(Json *array, Json *item);
extern void	Json_AddItemReferenceToObject(Json *object,const char *string,Json *item);

/* Remove/Detatch items from Arrays/Objects. */
extern Json *Json_DetachItemFromArray(Json *array,int which);
extern void   Json_DeleteItemFromArray(Json *array,int which);
extern Json *Json_DetachItemFromObject(Json *object,const char *string);
extern void   Json_DeleteItemFromObject(Json *object,const char *string);
	
/* Update array items. */
extern void Json_InsertItemInArray(Json *array,int which,Json *newitem);	/* Shifts pre-existing items to the right. */
extern void Json_ReplaceItemInArray(Json *array,int which,Json *newitem);
extern void Json_ReplaceItemInObject(Json *object,const char *string,Json *newitem);

/* Duplicate a Json item */
extern Json *Json_Duplicate(Json *item,int recurse);
/* Duplicate will create a new, identical Json item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */

/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
extern Json *Json_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated);

extern void Json_Minify(char *json);

/* Macros for creating things quickly. */
#define Json_AddNullToObject(object,name)		Json_AddItemToObject(object, name, Json_CreateNull())
#define Json_AddTrueToObject(object,name)		Json_AddItemToObject(object, name, Json_CreateTrue())
#define Json_AddFalseToObject(object,name)		Json_AddItemToObject(object, name, Json_CreateFalse())
#define Json_AddBoolToObject(object,name,b)	Json_AddItemToObject(object, name, Json_CreateBool(b))
#define Json_AddNumberToObject(object,name,n)	Json_AddItemToObject(object, name, Json_CreateNumber(n))
#define Json_AddStringToObject(object,name,s)	Json_AddItemToObject(object, name, Json_CreateString(s))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define Json_SetIntValue(object,val)			((object)?(object)->valueint=(object)->valuedouble=(val):(val))
#define Json_SetNumberValue(object,val)		((object)?(object)->valueint=(object)->valuedouble=(val):(val))

/* Macro for iterating over an array */
#define Json_ArrayForEach(pos, head)			for(pos = (head)->child; pos != NULL; pos = pos->next)

#ifdef __cplusplus
}
#endif

#endif
