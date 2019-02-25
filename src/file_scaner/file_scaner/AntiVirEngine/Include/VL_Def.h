/****************************************************
 *
 *  病毒库中引用的类型定义，在此登记
 *
 *  整理：何公道
 *  日期：2003.12.9
 *
 ****************************************************/

#ifndef VL_DEF_H
#define VL_DEF_H

#ifndef LOBYTE
#define LOBYTE(w)   ((BYTE)(w))
#endif
#ifndef HIBYTE
#define HIBYTE(w)   ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif

#ifndef LOWORD
#define LOWORD(l)   ((WORD)((DWORD)(l) & 0xffff))
#define HIWORD(l)   ((WORD)((DWORD)(l) >> 16))
#endif


//#include <netinet/in.h>		/* for network / host byte order conversions */
#define HTONS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define NTOHS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))

#define HTONL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
	((((unsigned long)(n) & 0xFF00)) << 8) | \
	((((unsigned long)(n) & 0xFF0000)) >> 8) | \
	((((unsigned long)(n) & 0xFF000000)) >> 24))

#define NTOHL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
	((((unsigned long)(n) & 0xFF00)) << 8) | \
	((((unsigned long)(n) & 0xFF0000)) >> 8) | \
	((((unsigned long)(n) & 0xFF000000)) >> 24))

#define SWAP16(a)        ((unsigned short)((((unsigned int)(a) >> 8) & 0xFF) | (((unsigned int)(a) << 8) & 0xFF00)))

#define SWAP32(a)        ((((unsigned int)(a) >> 24) & 0x000000FF) | (((unsigned int)(a) >> 8) & 0x0000FF00)|\
	(((unsigned int)(a) << 8) & 0x00FF0000) | (((unsigned int)(a) << 24) & 0xFF000000))

#define DWORD_LO(x) (*(WORD*)&x)
#define DWORD_HI(x) (*(WORD*)((BYTE*)&x+2))

#define PLOWORD(l)   (*(WORD*)&(l))
#define PHIWORD(l)   (*((WORD*)&(l)+1))
#define PLOBYTE(w)   (*(BYTE*)&(w))
#define PHIBYTE(w)   (*((BYTE*)&(w)+1))

#define ROL_B(B,n) ((BYTE)(((BYTE)(B)<<((n)&7)) | ((BYTE)(B)>>(8-((n)&7)))))
#define ROR_B(B,n) ((BYTE)(((BYTE)(B)>>((n)&7)) | ((BYTE)(B)<<(8-((n)&7)))))
#define ROL_W(W,n) ((WORD)(((WORD)(W)<<((n)&0xf)) | ((WORD)(W)>>(0x10-((n)&0xf)))))
#define ROR_W(W,n) ((WORD)(((WORD)(W)>>((n)&0xf)) | ((WORD)(W)<<(0x10-((n)&0xf)))))
#define ROL_D(D,n) ((DWORD)(((DWORD)(D)<<((n)&0x1f)) | ((DWORD)(D)>>(0x20-((n)&0x1f)))))
#define ROR_D(D,n) ((DWORD)(((DWORD)(D)>>((n)&0x1f)) | ((DWORD)(D)<<(0x20-((n)&0x1f)))))

//在16位程序操作中，引用IP32_Entry不方便，故用宏定义来完成。
#define IP32_Entry_lo (*(WORD*)&IP32_Entry)
#define IP32_Entry_lo1 (*(WORD*)&AVE->IP32_Entry)
#define IP32_Entry_hi (*(WORD*)((BYTE*)&IP32_Entry + 2))
#define IP32_Entry_hi1 (*(WORD*)((BYTE*)&AVE->IP32_Entry + 2))

// 将多字节字符合并成WORD和DWORD，这样做可以保证字节序不会变化
#define MK2CC(c1,c2)		((WORD)(BYTE)(c1) | ((WORD)(BYTE)(c2)<<8) )
#define MK4CC(c1,c2,c3,c4)	((DWORD)(BYTE)(c1) | ((DWORD)(BYTE)(c2)<<8) | ((DWORD)(BYTE)(c3)<<16) | ((DWORD)(BYTE)(c4)<<24))

#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD)(a) & 0xff)) | ((WORD)((BYTE)((DWORD)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD)(b) & 0xffff))) << 16))
#endif

#define Header_Size	(sizeof(B_Header))
#define Entry_Size	(sizeof(B_Entry))
#define Next_Size	(sizeof(B_Next))
#define Tail_Size	(sizeof(B_Tail))
#define Misc_Size	(sizeof(B_Misc))
#define Temp_Size	(sizeof(B_Temp))
#define Full_Size   Header_Size+Entry_Size+Next_Size+Tail_Size+Misc_Size+Temp_Size

#ifdef WINCE

	#if defined(BIG_ENDIAN)
		#define GetWord(p) ((WORD)(*((BYTE*)(p) + 0) << 8) + *((BYTE*)(p) + 1))
		#define GetDWord(p) ((DWORD)(*((BYTE*)(p) + 0) << 24) + (DWORD)(*((BYTE*)(p) + 1) << 16) + (DWORD)(*((BYTE*)(p) + 2) <<8 ) + *((BYTE*)(p) + 3))
		#define PutWord(p, w) {*((BYTE*)(p) + 1) = (w) & 0xff; *((BYTE*)(p) + 0) = ((w) >> 8) & 0xff;}
		#define PutDWord(p, d) {*((BYTE*)(p) + 3) = (d) & 0xff; *((BYTE*)(p) + 2) = ((d) >> 8) & 0xff; *((BYTE*)(p) + 1) = ((d) >> 16) & 0xff; *((BYTE*)(p) + 0) = ((d) >> 24) & 0xff;}
	#else
		#define GetWord(p) ((WORD)(*((BYTE*)(p) + 1) << 8) + *((BYTE*)(p) + 0))
		#define GetDWord(p) ((DWORD)(*((BYTE*)(p) + 3) << 24) + (DWORD)(*((BYTE*)(p) + 2) << 16) + (DWORD)(*((BYTE*)(p) + 1) <<8 ) + *((BYTE*)(p) + 0))
		#define PutWord(p, w) {*((BYTE*)(p) + 0) = (w) & 0xff; *((BYTE*)(p) + 1) = ((w) >> 8) & 0xff;}
		#define PutDWord(p, d) {*((BYTE*)(p) + 0) = (d) & 0xff; *((BYTE*)(p) + 1) = ((d) >> 8) & 0xff; *((BYTE*)(p) + 2) = ((d) >> 16) & 0xff; *((BYTE*)(p) + 3) = ((d) >> 24) & 0xff;}
	#endif

#else

	#define GetWord(x)		(*(WORD*)(BYTE*)(x))
	#define GetDWord(x)		(*(DWORD*)(BYTE*)(x))
	#define PutWord(x, y)	(*(WORD*)(BYTE*)(x) = y)
	#define PutDWord(x, y)  (*(DWORD*)(BYTE*)(x) = y)

#endif

#ifdef WINCE
	#define wHeader(x)	(((WORD)B_Header[(x)+1] << 8) | B_Header[(x)])
	#define wEntry(x)	(((WORD)B_Entry[(x)+1] << 8) | B_Entry[(x)])
	#define wNext(x)	(((WORD)B_Next[(x)+1] << 8) | B_Next[(x)])
	#define wTail(x)	(((WORD)B_Tail[(x)+1] << 8) | B_Tail[(x)])
	#define wMisc(x)	(((WORD)B_Misc[(x)+1] << 8) | B_Misc[(x)])
	#define wTemp(x)	(((WORD)B_Temp[(x)+1] << 8) | B_Temp[(x)])

	#define dwHeader(x)	(((DWORD)B_Header[(x)+3] << 24) | ((DWORD)B_Header[(x)+2] << 16) | ((DWORD)B_Header[(x)+1] << 8) | B_Header[(x)])
	#define dwEntry(x)	(((DWORD)B_Entry[(x)+3] << 24) | ((DWORD)B_Entry[(x)+2] << 16) | ((DWORD)B_Entry[(x)+1] << 8) | B_Entry[(x)])
	#define dwNext(x)	(((DWORD)B_Next[(x)+3] << 24) | ((DWORD)B_Next[(x)+2] << 16) | ((DWORD)B_Next[(x)+1] << 8) | B_Next[(x)])
	#define dwTail(x)	(((DWORD)B_Tail[(x)+3] << 24) | ((DWORD)B_Tail[(x)+2] << 16) | ((DWORD)B_Tail[(x)+1] << 8) | B_Tail[(x)])
	#define dwMisc(x)	(((DWORD)B_Misc[(x)+3] << 24) | ((DWORD)B_Misc[(x)+2] << 16) | ((DWORD)B_Misc[(x)+1] << 8) | B_Misc[(x)])
	#define dwTemp(x)	(((DWORD)B_Temp[(x)+3] << 24) | ((DWORD)B_Temp[(x)+2] << 16) | ((DWORD)B_Temp[(x)+1] << 8) | B_Temp[(x)])

	#define wHeader1(x)	(((WORD)AVE->B_Header[(x)+1] << 8) | AVE->B_Header[(x)])
	#define wEntry1(x)	(((WORD)AVE->B_Entry[(x)+1] << 8) | AVE->B_Entry[(x)])
	#define wNext1(x)	(((WORD)AVE->B_Next[(x)+1] << 8) | AVE->B_Next[(x)])
	#define wTail1(x)	(((WORD)AVE->B_Tail[(x)+1] << 8) | AVE->B_Tail[(x)])
	#define wMisc1(x)	(((WORD)AVE->B_Misc[(x)+1] << 8) | AVE->B_Misc[(x)])
	#define wTemp1(x)	(((WORD)AVE->B_Temp[(x)+1] << 8) | AVE->B_Temp[(x)])

	#define dwHeader1(x) (((DWORD)AVE->B_Header[(x)+3] << 24) | ((DWORD)AVE->B_Header[(x)+2] << 16) | ((DWORD)AVE->B_Header[(x)+1] << 8) | AVE->B_Header[(x)])
	#define dwEntry1(x)	(((DWORD)AVE->B_Entry[(x)+3] << 24) | ((DWORD)AVE->B_Entry[(x)+2] << 16) | ((DWORD)AVE->B_Entry[(x)+1] << 8) | AVE->B_Entry[(x)])
	#define dwNext1(x)	(((DWORD)AVE->B_Next[(x)+3] << 24) | ((DWORD)AVE->B_Next[(x)+2] << 16) | ((DWORD)AVE->B_Next[(x)+1] << 8) | AVE->B_Next[(x)])
	#define dwTail1(x)	(((DWORD)AVE->B_Tail[(x)+3] << 24) | ((DWORD)AVE->B_Tail[(x)+2] << 16) | ((DWORD)AVE->B_Tail[(x)+1] << 8) | AVE->B_Tail[(x)])
	#define dwMisc1(x)	(((DWORD)AVE->B_Misc[(x)+3] << 24) | ((DWORD)AVE->B_Misc[(x)+2] << 16) | ((DWORD)AVE->B_Misc[(x)+1] << 8) | AVE->B_Misc[(x)])
	#define dwTemp1(x)	(((DWORD)AVE->B_Temp[(x)+3] << 24) | ((DWORD)AVE->B_Temp[(x)+2] << 16) | ((DWORD)AVE->B_Temp[(x)+1] << 8) | AVE->B_Temp[(x)])
#else
	#define wHeader(x)	(*(WORD*)(B_Header+(x)))
	#define wEntry(x)	(*(WORD*)(B_Entry+(x)))
	#define wNext(x)	(*(WORD*)(B_Next+(x)))
	#define wTail(x)	(*(WORD*)(B_Tail+(x)))
	#define wMisc(x)	(*(WORD*)(B_Misc+(x)))
	#define wTemp(x)	(*(WORD*)(B_Temp+(x)))

	#define dwHeader(x)	(*(DWORD*)(B_Header+(x)))
	#define dwEntry(x)	(*(DWORD*)(B_Entry+(x)))
	#define dwNext(x)	(*(DWORD*)(B_Next+(x)))
	#define dwTail(x)	(*(DWORD*)(B_Tail+(x)))
	#define dwMisc(x)	(*(DWORD*)(B_Misc+(x)))
	#define dwTemp(x)	(*(DWORD*)(B_Temp+(x)))

	#define wHeader1(x)	(*(WORD*)(AVE->B_Header+(x)))
	#define wEntry1(x)	(*(WORD*)(AVE->B_Entry+(x)))
	#define wNext1(x)	(*(WORD*)(AVE->B_Next+(x)))
	#define wTail1(x)	(*(WORD*)(AVE->B_Tail+(x)))
	#define wMisc1(x)	(*(WORD*)(AVE->B_Misc+(x)))
	#define wTemp1(x)	(*(WORD*)(AVE->B_Temp+(x)))

	#define dwHeader1(x)(*(DWORD*)(AVE->B_Header+(x)))
	#define dwEntry1(x)	(*(DWORD*)(AVE->B_Entry+(x)))
	#define dwNext1(x)  (*(DWORD*)(AVE->B_Next+(x)))
	#define dwTail1(x)	(*(DWORD*)(AVE->B_Tail+(x)))
	#define dwMisc1(x)	(*(DWORD*)(AVE->B_Misc+(x)))
	#define dwTemp1(x)	(*(DWORD*)(AVE->B_Temp+(x)))
#endif

// 0－KVRT     1－生成decode和jmp函数
#ifdef _DEBUG
	#define DEBUG_CODE		0
#else
	#define DEBUG_CODE		0
#endif


#ifdef MIPS
#ifndef __GNUC__
extern "C" 
{
	void __asm(char*,...);
};

#pragma intrinsic(__asm)

#endif
#endif

#endif //!VL_DEF_H
