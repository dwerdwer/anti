#ifndef _CSTL_H_
#define _CSTL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cstl_def.h"
#include "cstl_alloc.h"
#include "cstl_types.h"
#include "../../kv0000/cSTL/cstl_types_parse.h"
#include "../../Include/pub.h"


#define EOF     (-1)


#define NULL   0
#define exit(a)   
#define assert(exp) 

#define islower(x)	((x)>='a' && (x)<='z')
#define isupper(x)	((x)>='A' && (x)<='Z')
#define tolower(x)	(isupper(x)?(x)-'A'+'a':(x))
#define toupper(x)	(islower(x)?(x)-'a'+'A':(x))
#define isalpha(x)	(tolower(x)>='a' && tolower(x)<='z')
#define isdigit(x)	((x)>='0' && (x)<='9')
#define isdigitx(x) (isdigit(x) || (tolower(x)>='a' && tolower(x)<='f'))
#define hexvalue(x)	(isdigit(x)?(x)-'0':tolower(x)-'a')
#define isspace(x)  ((x)==' ' || (x)=='\f' || (x)=='\n' || (x)=='\r' || (x)== '\t' || (x)== '\v')

#define abs(x)         fabs(x)
#define strncpy(x,y,z) ((CAntiVirEngine*)0)->_str_ncpy(x,y,z)
#define strchr(a,b)    ((CAntiVirEngine*)0)->_str_chr(a,b) 
#define strrchr(a,b)   ((CAntiVirEngine*)0)->_str_rchr(a,b) 
#define memset(x,y,z)  ((CAntiVirEngine*)0)->_mem_set(x,y,z)
#define memcpy(d,s,l)  ((CAntiVirEngine*)0)->_mem_cpy(d,s,l)
#define memcmp(d,s,l)  ((CAntiVirEngine*)0)->_mem_cmp(d,s,l)
#define memmove(d,s,l) ((CAntiVirEngine*)0)->_mem_move(d,s,l)
#define strncpy(x,y,z) ((CAntiVirEngine*)0)->_str_ncpy(x,y,z)
#define strncat(x,y,z) ((CAntiVirEngine*)0)->_str_ncat(x,y,z)
#define strlen(p)      ((CAntiVirEngine*)0)->_str_len(p)
#define strncmp(x,y,z) ((CAntiVirEngine*)0)->_str_ncmp(x,y,z)




class cSTL
{

	typedef void (*_gpfun_malloc_handler)();

public:
	CAntiVirEngine*  AVE;
	_typeanalysis_t _gt_typeanalysis;
	_typeregister_t _gt_typeregister;
	_gpfun_malloc_handler _pfun_malloc_handler;

public:
	cSTL();
	~cSTL();
public:
	int   Init(CAntiVirEngine* AVE);
	void * cSTL_Alloc(int length){return AVE->New(length);}
	void   cSTL_Free(void *addr){AVE->Delete(addr);}
	
};

#ifdef __cplusplus
}
#endif

#endif