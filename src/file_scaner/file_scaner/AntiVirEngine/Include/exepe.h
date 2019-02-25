#include "VL_Type.h"

#ifndef EXE_PE_H
#define EXE_PE_H

#pragma pack (1)

#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_OS2_SIGNATURE                 0x454E      // NE
#define IMAGE_OS2_SIGNATURE_LE              0x454C      // LE
#define IMAGE_VXD_SIGNATURE                 0x454C      // LE
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00
#define IMAGE_FILE_EXECUTABLE_IMAGE         0x2

typedef struct _IMAGE_DOS_HEADER_X {      // DOS .EXE header
	WORD   e_magic;                     // Magic number
	WORD   e_cblp;                      // Bytes on last page of file
	WORD   e_cp;                        // Pages in file
	WORD   e_crlc;                      // Relocations
	WORD   e_cparhdr;                   // Size of header in paragraphs
	WORD   e_minalloc;                  // Minimum extra paragraphs needed
	WORD   e_maxalloc;                  // Maximum extra paragraphs needed
	WORD   e_ss;                        // Initial (relative) SS value
	WORD   e_sp;                        // Initial SP value
	WORD   e_csum;                      // Checksum
	WORD   e_ip;                        // Initial IP value
	WORD   e_cs;                        // Initial (relative) CS value
	WORD   e_lfarlc;                    // File address of relocation table
	WORD   e_ovno;                      // Overlay number
	WORD   e_res[4];                    // Reserved words
	WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
	WORD   e_oeminfo;                   // OEM information; e_oemid specific
	WORD   e_res2[10];                  // Reserved words
	LONG   e_lfanew;                    // File address of new exe header
} IMAGE_DOS_HEADER_X, *PIMAGE_DOS_HEADER_X;

typedef struct _IMAGE_OS2_HEADER_X {      // OS/2 .EXE header
	WORD   ne_magic;                    // Magic number
	CHAR   ne_ver;                      // Version number
	CHAR   ne_rev;                      // Revision number
	WORD   ne_enttab;                   // Offset of Entry Table
	WORD   ne_cbenttab;                 // Number of bytes in Entry Table
	LONG   ne_crc;                      // Checksum of whole file
	WORD   ne_flags;                    // Flag word
	WORD   ne_autodata;                 // Automatic data segment number
	WORD   ne_heap;                     // Initial heap allocation
	WORD   ne_stack;                    // Initial stack allocation
	LONG   ne_csip;                     // Initial CS:IP setting
	LONG   ne_sssp;                     // Initial SS:SP setting
	WORD   ne_cseg;                     // Count of file segments
	WORD   ne_cmod;                     // Entries in Module Reference Table
	WORD   ne_cbnrestab;                // Size of non-resident name table
	WORD   ne_segtab;                   // Offset of Segment Table
	WORD   ne_rsrctab;                  // Offset of Resource Table
	WORD   ne_restab;                   // Offset of resident name table
	WORD   ne_modtab;                   // Offset of Module Reference Table
	WORD   ne_imptab;                   // Offset of Imported Names Table
	LONG   ne_nrestab;                  // Offset of Non-resident Names Table
	WORD   ne_cmovent;                  // Count of movable entries
	WORD   ne_align;                    // Segment alignment shift count
	WORD   ne_cres;                     // Count of resource segments
	BYTE   ne_exetyp;                   // Target Operating system
	BYTE   ne_flagsothers;              // Other .EXE flags
	WORD   ne_pretthunks;               // offset to return thunks
	WORD   ne_psegrefbytes;             // offset to segment ref. bytes
	WORD   ne_swaparea;                 // Minimum code swap area size
	WORD   ne_expver;                   // Expected Windows version number
} IMAGE_OS2_HEADER_X, *PIMAGE_OS2_HEADER_X;

typedef struct _IMAGE_VXD_HEADER_X {    // Windows VXD header
	WORD   e32_magic;                   // +00 Magic number
	BYTE   e32_border;                  // +02 The byte ordering for the VXD
	BYTE   e32_worder;                  // +03 The word ordering for the VXD
	DWORD  e32_level;                   // +04 The EXE format level for now = 0
	WORD   e32_cpu;                     // +08 The CPU type
	WORD   e32_os;                      // +0A The OS type
	DWORD  e32_ver;                     // +0C Module version
	DWORD  e32_mflags;                  // +10 Module flags
	DWORD  e32_mpages;                  // +14 Module # pages
	DWORD  e32_startobj;                // +18 Object # for instruction pointer
	DWORD  e32_eip;                     // +1C Extended instruction pointer
	DWORD  e32_stackobj;                // +20 Object # for stack pointer
	DWORD  e32_esp;                     // +24 Extended stack pointer
	DWORD  e32_pagesize;                // +28 VXD page size
	DWORD  e32_lastpagesize;            // +2C Last page size in VXD
	DWORD  e32_fixupsize;               // +30 Fixup section size
	DWORD  e32_fixupsum;                // +34 Fixup section checksum
	DWORD  e32_ldrsize;                 // +38 Loader section size
	DWORD  e32_ldrsum;                  // +3C Loader section checksum
	DWORD  e32_objtab;                  // +40 Object table offset
	DWORD  e32_objcnt;                  // +44 Number of objects in module
	DWORD  e32_objmap;                  // +48 Object page map offset
	DWORD  e32_itermap;                 // +4C Object iterated data map offset
	DWORD  e32_rsrctab;                 // +50 Offset of Resource Table
	DWORD  e32_rsrccnt;                 // +54 Number of resource entries
	DWORD  e32_restab;                  // +58 Offset of resident name table
	DWORD  e32_enttab;                  // +5C Offset of Entry Table
	DWORD  e32_dirtab;                  // +60 Offset of Module Directive Table
	DWORD  e32_dircnt;                  // +64 Number of module directives
	DWORD  e32_fpagetab;                // +68 Offset of Fixup Page Table
	DWORD  e32_frectab;                 // +6C Offset of Fixup Record Table
	DWORD  e32_impmod;                  // +70 Offset of Import Module Name Table
	DWORD  e32_impmodcnt;               // +74 Number of entries in Import Module Name Table
	DWORD  e32_impproc;                 // +78 Offset of Import Procedure Name Table
	DWORD  e32_pagesum;                 // +7C Offset of Per-Page Checksum Table
	DWORD  e32_datapage;                // +80 Offset of Enumerated Data Pages
	DWORD  e32_preload;                 // +84 Number of preload pages
	DWORD  e32_nrestab;                 // +88 Offset of Non-resident Names Table
	DWORD  e32_cbnrestab;               // +8C Size of Non-resident Name Table
	DWORD  e32_nressum;                 // +90 Non-resident Name Table Checksum
	DWORD  e32_autodata;                // +94 Object # for automatic data object
	DWORD  e32_debuginfo;               // +98 Offset of the debugging information
	DWORD  e32_debuglen;                // +9C The length of the debugging info. in bytes
	DWORD  e32_instpreload;             // +A0 Number of instance pages in preload section of VXD file
	DWORD  e32_instdemand;              // Number of instance pages in demand load section of VXD file
	DWORD  e32_heapsize;                // Size of heap - for 16-bit apps
	BYTE   e32_res3[12];                // Reserved words
	DWORD  e32_winresoff;
	DWORD  e32_winreslen;
	WORD   e32_devid;                   // Device ID for VxD
	WORD   e32_ddkver;                  // DDK version for VxD
} IMAGE_VXD_HEADER_X, *PIMAGE_VXD_HEADER_X;


////////////////////////////////////////////////////////////////////////////
//	PE file struct
////////////////////////////////////////////////////////////////////////////

#define IMAGE_SCN_CNT_CODE					 0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA		 0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA	 0x00000080
#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

#define IMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.
#define IMAGE_FILE_SYSTEM                    0x1000  // System File.
#define IMAGE_FILE_DLL                       0x2000  // File is a DLL.

#define IMAGE_FILE_MACHINE_I386              0x014c  // Intel 386.
#define IMAGE_FILE_MACHINE_ARM               0x01c0  // ARM Little-Endian
#define IMAGE_FILE_MACHINE_AMD64             0x8664  // AMD64 (K8)

//
//	COFF image format
//
typedef struct _IMAGE_FILE_HEADER_X {
	WORD    Machine;
	WORD    NumberOfSections;
	DWORD   TimeDateStamp;
	DWORD   PointerToSymbolTable;
	DWORD   NumberOfSymbols;
	WORD    SizeOfOptionalHeader;
	WORD    Characteristics;
} IMAGE_FILE_HEADER_X, *PIMAGE_FILE_HEADER_X;

#define IMAGE_SIZEOF_FILE_HEADER             20

//
// Directory format.
//
typedef struct _IMAGE_DATA_DIRECTORY_X {
	DWORD   VirtualAddress;
	DWORD   Size;
} IMAGE_DATA_DIRECTORY_X, *PIMAGE_DATA_DIRECTORY_X;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

//
// Optional header format.
//
typedef struct _IMAGE_OPTIONAL_HEADER_X {
	//
	// Standard fields.
	//
	WORD    Magic;
	BYTE    MajorLinkerVersion;
	BYTE    MinorLinkerVersion;
	DWORD   SizeOfCode;
	DWORD   SizeOfInitializedData;
	DWORD   SizeOfUninitializedData;
	DWORD   AddressOfEntryPoint;
	DWORD   BaseOfCode;
	DWORD   BaseOfData;

	//
	// NT additional fields.
	//

	DWORD   ImageBase;
	DWORD   SectionAlignment;
	DWORD   FileAlignment;
	WORD    MajorOperatingSystemVersion;
	WORD    MinorOperatingSystemVersion;
	WORD    MajorImageVersion;
	WORD    MinorImageVersion;
	WORD    MajorSubsystemVersion;
	WORD    MinorSubsystemVersion;
	DWORD   Win32VersionValue;
	DWORD   SizeOfImage;
	DWORD   SizeOfHeaders;
	DWORD   CheckSum;
	WORD    Subsystem;
	WORD    DllCharacteristics;
	DWORD   SizeOfStackReserve;
	DWORD   SizeOfStackCommit;
	DWORD   SizeOfHeapReserve;
	DWORD   SizeOfHeapCommit;
	DWORD   LoaderFlags;
	DWORD   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY_X DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32_X, *PIMAGE_OPTIONAL_HEADER32_X;

//
//	Section
//

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER_X {
	BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		DWORD   PhysicalAddress;
		DWORD   VirtualSize;
	}Misc;
	DWORD   VirtualAddress;
	DWORD   SizeOfRawData;
	DWORD   PointerToRawData;
	DWORD   PointerToRelocations;
	DWORD   PointerToLinenumbers;
	WORD    NumberOfRelocations;
	WORD    NumberOfLinenumbers;
	DWORD   Characteristics;
} IMAGE_SECTION_HEADER_X, *PIMAGE_SECTION_HEADER_X;

typedef struct _IMAGE_IMPORT_DESCRIPTOR_X {
	union {
		DWORD   Characteristics;            // 0 for terminating null import descriptor
		DWORD   OriginalFirstThunk;         // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
	};
	DWORD   TimeDateStamp;                  // 0 if not bound,
	// -1 if bound, and real date\time stamp
	//     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
	// O.W. date/time stamp of DLL bound to (Old BIND)

	DWORD   ForwarderChain;                 // -1 if no forwarders
	DWORD   Name;
	DWORD   FirstThunk;                     // RVA to IAT (if bound this IAT has actual addresses)
} IMAGE_IMPORT_DESCRIPTOR_X;

#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY_X {
	union {
		struct {
			DWORD NameOffset:31;
			DWORD NameIsString:1;
		};
		DWORD   Name;
		WORD    Id;
	};
	union {
		DWORD   OffsetToData;
		struct {
			DWORD   OffsetToDirectory:31;
			DWORD   DataIsDirectory:1;
		};
	};
} IMAGE_RESOURCE_DIRECTORY_ENTRY_X, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_X;

typedef struct _IMAGE_RESOURCE_DIRECTORY_X {
	DWORD   Characteristics;
	DWORD   TimeDateStamp;
	WORD    MajorVersion;
	WORD    MinorVersion;
	WORD    NumberOfNamedEntries;
	WORD    NumberOfIdEntries;
	//  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
} IMAGE_RESOURCE_DIRECTORY_X, *PIMAGE_RESOURCE_DIRECTORY_X;

typedef struct _IMAGE_RESOURCE_DATA_ENTRY_X {
	DWORD   OffsetToData;
	DWORD   Size;
	DWORD   CodePage;
	DWORD   Reserved;
} IMAGE_RESOURCE_DATA_ENTRY_X, *PIMAGE_RESOURCE_DATA_ENTRY_X;

typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING_X {
	WORD    Length;
	char    NameString[ 1 ];
} IMAGE_RESOURCE_DIRECTORY_STRING_X, *PIMAGE_RESOURCE_DIRECTORY_STRING_X;


typedef struct _IMAGE_OPTIONAL_HEADER64_X {
	WORD        Magic;
	BYTE        MajorLinkerVersion;
	BYTE        MinorLinkerVersion;
	DWORD       SizeOfCode;
	DWORD       SizeOfInitializedData;
	DWORD       SizeOfUninitializedData;
	DWORD       AddressOfEntryPoint;
	DWORD       BaseOfCode;
	ULONGLONG   ImageBase;		// OPTIONAL_HEADER64 在此删去了 OPTIONAL_HEADER32中的BaseOfData字段
								// 保证了后续大部分数据兼容 OPTIONAL_HEADER32 
	DWORD       SectionAlignment;
	DWORD       FileAlignment;
	WORD        MajorOperatingSystemVersion;
	WORD        MinorOperatingSystemVersion;
	WORD        MajorImageVersion;
	WORD        MinorImageVersion;
	WORD        MajorSubsystemVersion;
	WORD        MinorSubsystemVersion;
	DWORD       Win32VersionValue;
	DWORD       SizeOfImage;
	DWORD       SizeOfHeaders;
	DWORD       CheckSum;
	WORD        Subsystem;
	WORD        DllCharacteristics;
	ULONGLONG   SizeOfStackReserve;
	ULONGLONG   SizeOfStackCommit;
	ULONGLONG   SizeOfHeapReserve;
	ULONGLONG   SizeOfHeapCommit;
	DWORD       LoaderFlags;
	DWORD       NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY_X DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64_X, *PIMAGE_OPTIONAL_HEADER64_X;


typedef struct _PEHeader0
{
	DWORD signature;
	IMAGE_FILE_HEADER_X _head;
	IMAGE_OPTIONAL_HEADER32_X opt_head;
	IMAGE_SECTION_HEADER_X pSection_Header[10];
} PEHeader0, PPEHeader0;

typedef struct _PEHeader64
{
	DWORD signature;
	IMAGE_FILE_HEADER_X _head;
	IMAGE_OPTIONAL_HEADER64_X opt_head;
	IMAGE_SECTION_HEADER_X pSection_Header[10];
} PEHeader64, PPEHeader64;


#define IMAGE_SIZEOF_ROM_OPTIONAL_HEADER      56
#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define IMAGE_SIZEOF_NT_OPTIONAL32_HEADER    224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER    240

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

#if 0 //def _WIN64
typedef IMAGE_OPTIONAL_HEADER64             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER64            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER     IMAGE_SIZEOF_NT_OPTIONAL64_HEADER
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR64_MAGIC
#else
typedef IMAGE_OPTIONAL_HEADER32_X           IMAGE_OPTIONAL_HEADER_X;
typedef PIMAGE_OPTIONAL_HEADER32_X          PIMAGE_OPTIONAL_HEADER_X;
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER     IMAGE_SIZEOF_NT_OPTIONAL32_HEADER
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR32_MAGIC
#endif

typedef struct _IMAGE_NT_HEADERS64_X {
	DWORD Signature;
	IMAGE_FILE_HEADER_X FileHeader;
	IMAGE_OPTIONAL_HEADER64_X OptionalHeader;
} IMAGE_NT_HEADERS64_X, *PIMAGE_NT_HEADERS64_X;

typedef struct _IMAGE_NT_HEADERS_X {
	DWORD Signature;
	IMAGE_FILE_HEADER_X FileHeader;
	IMAGE_OPTIONAL_HEADER32_X OptionalHeader;
} IMAGE_NT_HEADERS32_X, *PIMAGE_NT_HEADERS32_X;

#if 0 //def _WIN64
typedef IMAGE_NT_HEADERS64                  IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS64                 PIMAGE_NT_HEADERS;
#else
typedef IMAGE_NT_HEADERS32_X                IMAGE_NT_HEADERS_X;
typedef PIMAGE_NT_HEADERS32_X               PIMAGE_NT_HEADERS_X;
#endif

#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_OS2_SIGNATURE                 0x454E      // NE
#define IMAGE_OS2_SIGNATURE_LE              0x454C      // LE
#define IMAGE_VXD_SIGNATURE                 0x454C      // LE
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00


// LE  Vxd文件节表结构
// -----------------------------------------------------------------------
//Virtual Size     0000104D, Relocation Base  00000000, Object Flags 00002045
//Page Table Index 00000001, Page Table Count 00000002, Object Name  ....  01
typedef struct _LE_IMAGE_SECTION_HEADER_X {
	DWORD   VirtualAddress;
	DWORD   RelocationBase;
	DWORD   ObjectFlags;
	DWORD   PageTableIndex;
	DWORD   PageTableCount;
	DWORD   ObjectName;
} LE_IMAGE_SECTION_HEADER_X, *PLE_IMAGE_SECTION_HEADER_X;

typedef struct _LE_OBJMAP
{
	WORD	wIndex;		// 0x1e8
	BYTE	cNum;		// 0x1ea
	DWORD	dwTemp;		// 0x1eb
	BYTE	cAtrtib;	// 0x1ef
}LE_OBJMAP, *POBJMAP;


typedef struct _IMAGE_EXPORT_DIRECTORY_X {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Name;
    DWORD   Base;
    DWORD   NumberOfFunctions;
    DWORD   NumberOfNames;
    DWORD   AddressOfFunctions;     // RVA from base of image
    DWORD   AddressOfNames;         // RVA from base of image
    DWORD   AddressOfNameOrdinals;  // RVA from base of image
} IMAGE_EXPORT_DIRECTORY_X, *PIMAGE_EXPORT_DIRECTORY_X;

typedef struct _IMAGE_IMPORT_BY_NAME_X {
WORD Hint;
BYTE Name[1];
} IMAGE_IMPORT_BY_NAME_X, *PIMAGE_IMPORT_BY_NAME_X;

#pragma pack ()


//VB 程序一些结构定义


//Thread Flags
#define  ApartmentModel   0x1
#define  RequireLicense   0x2
#define  Unattended       0x4
#define  SingleThreaded   0x8
#define  Retained         0x10

#pragma pack (1)

typedef struct _VBEntry{
   BYTE    codePush;
   DWORD   MagicOffset;
   BYTE    codeCall;
   DWORD   ThunRTmain;
}VBEntry;

enum ControlType
{ 
	PictureBox = 0,
	Label,
	TextBox,
	Frame,
	CommandButton,
	CheckBox,
	OptionButton,
	ComboBox,
	ListBox,
	HScrollBar,
	VScrollBar,
	Timer,
	Form = 13,
	DriveListBox = 16,
	DirListBox,
	FileListBox,
	Menu,
	MDIForm,
	Shape = 22,
	Line,
	Image,
	Data = 37,
	OLE,
	UserControl = 40,
	PropertyPage,
	UserDocument,
	OLE1 = 255,
};

typedef struct _VBControlInfo
{
	DWORD           dwControlType;  //Type of control
	WORD            wEventcount;    //Number of Event Handlers supported
	WORD            wEventsOffset;  //Offset in to Memory struct to copy Events
	BYTE*           lpGuid;         //Pointer to GUID of this Control
	DWORD           dwIndex;        //Index ID of this Control
	DWORD           dwNull;         //Unused
	DWORD           dwNull2;        //Unused
	BYTE*           lpEventTable;   //Pointer to Event Handler Table
	BYTE*           lpIdeData;      //Valid in IDE only
	BYTE*           lpszName;       //Name of this Control
	DWORD           dwIndexCopy;    //Secondary Index ID of this Control

}VBControlInfo;

//if VBObjectPublicDescriptor.ObjectType & 0x80 then the Objectinfo  follow by the Optional Object 
typedef struct _VBOptionalObject
{
	DWORD                dwObjectGuids;        //How many GUIDs to Register
	BYTE*                lpObjectGuid;
	DWORD                dwNull;
	BYTE*                lpuuidObjectTypes;     //Pointer to Array of Object Interface GUIDs
	DWORD                dwObjectTypeGuids;     //How many GUIDs in the Array above
	VBControlInfo*       lpControls2;
	DWORD                dwNull2;
	BYTE*                lpObjectGuid2;       //Pointer to Array of Object GUIDs
	DWORD                dwControlCount;      //Number of Controls in array below
	VBControlInfo*       lpControls;          //Pointer to Controls Array
	WORD                 wEventCount;         //Number of Events in Event Array
	WORD                 wPCodeCount;         //Number of P-Codes used by this Object
	WORD                 bWInitializeEvent;   //Offset to Initialize Event from Event Table
	WORD                 bWTerminateEvent;   //Offset to Terminate Event in Event Table
	BYTE*                lpEvents;          //Pointer to Events Array
	BYTE*                lpBasicClassObject;
	DWORD                dwNull3;
	BYTE*                lpIdeData;
}VBOptionalObject;

struct _VBObjectInfo;

typedef struct _VBObjectPrivateDescriptor
{
	DWORD               lpHeapLink;          //alaways 0
	_VBObjectInfo*       lpObjectInfo;        //Pointer to the Object Info for this Object
	DWORD               dwReserved;          //Always set to -1
	DWORD               dwIdeData[3];        //Not valid after compilation
	BYTE*               lpObjectList;        //Points to the Parent Structure Array
	DWORD               dwIdeData2;          //Not valid after compilation
	DWORD               lpObjectList2[3];    //Points to the Parent Structure Array
	DWORD               dwIdeData3[3];       //Not valid after compilation
	DWORD               dwObjectType;        //Type of the Object described
	DWORD               dwIdentifier;        //Template Version of Structure
}VBObjectPrivateDescriptor;

struct _VBObjectTable;
struct _VBObjectInfo;

typedef struct _VBObjectPublicDescriptor
{
	_VBObjectInfo*      pObjectInfo;          //指向一个VBObjectInfo类型
	DWORD               dwConst1;              //没有用的填充东西
	DWORD               dwPublicBytes;        //VA 指向公用变量表大小
	DWORD               dwStaticBytes;          //VA 指向静态变量表地址
	DWORD               dwModulePublic;        //VA 指向公用变量表
	DWORD               dwModuleStatic;        //VA 指向静态变量表
	DWORD               dwObjectName;          //VA 字符串,这个OBJECT的名字
	DWORD               dwMethodCount;         //events, funcs, subs(时间/函数/过程)数目
	DWORD               dwProcNamesArray;     //VA 一般都是0
	DWORD               dwStaticVar;          //OFFSET  从aModuleStatic指向的静态变量表偏移
	DWORD               ObjectType;           //比较重要显示了这个OBJECT的实行,具体见下表
	DWORD               Null3;                //没有用的填充东西
}VBObjectPublicDescriptor;

struct _VBObjectTable;

typedef struct _VBObjectInfo
{
	WORD               wRefCount;       //Always 1 
	WORD               wObjectIndex;    //Index of this Object
	_VBObjectTable*    lpObjectTable;   //Pointer to the Object Table
	BYTE*              lpIdeData;      //Zero after compilation
	VBObjectPrivateDescriptor*      lpPrivateObject; //Pointer to Private Object Descriptor
	DWORD              dwReserved;    //Always -1
	DWORD              dwNull;       //Unused
	VBObjectPublicDescriptor*        lpObject;      //Back-Pointer to Public Object Descriptor
	BYTE*              lpProjectData;  //Pointer to in-memory Project Object
	WORD               wMethodCount;   //Number of Methods
	WORD               wMethodCount2;  //Zeroed
	BYTE*              lpMethods;     //Pointer to Array of Methods
	WORD               wConstants;    //Number of Constants in Constant Pool
	WORD               wMaxConstants; //Constants to allocate in Constant Pool
	BYTE*              lpIdeData2;    //Valid in IDE only
	BYTE*              lpIdeData3;   //Valid in IDE only
	BYTE*              lpConstants;  //Pointer to Constants Pool
}VBObjectInfo;



/*
'VBPublicObjectDescriptor.ObjectType 属性...//重要的属性表部分
'#########################################################
'form:              0000 0001 1000 0000 1000 0011 --> 18083
'                   0000 0001 1000 0000 1010 0011 --> 180A3
'                   0000 0001 1000 0000 1100 0011 --> 180C3
'module:            0000 0001 1000 0000 0000 0001 --> 18001
'                   0000 0001 1000 0000 0010 0001 --> 18021
'class:             0001 0001 1000 0000 0000 0011 --> 118003
'                   0001 0011 1000 0000 0000 0011 --> 138003
'                   0000 0001 1000 0000 0010 0011 --> 18023
'                   0000 0001 1000 1000 0000 0011 --> 18803
'                   0001 0001 1000 1000 0000 0011 --> 118803
'usercontrol:       0001 1101 1010 0000 0000 0011 --> 1DA003
'                   0001 1101 1010 0000 0010 0011 --> 1DA023
'                   0001 1101 1010 1000 0000 0011 --> 1DA803
'propertypage:      0001 0101 1000 0000 0000 0011 --> 158003
'                      | ||     |  |    | |    |
'[moog]                | ||     |  |    | |    |
'HasPublicInterface ---+ ||     |  |    | |    |  （有公用的接口）
'HasPublicEvents --------+|     |  |    | |    |  （有公用的事件）
'IsCreatable/Visible? ----+     |  |    | |    |  （是否可以创建，可见）
'Same as "HasPublicEvents" -----+  |    | |    |  
'[aLfa]                         |  |    | |    |
'usercontrol (1) ---------------+  |    | |    |  （用户控制）
'ocx/dll (1) ----------------------+    | |    |  （OCX/DLL）
'form (1) ------------------------------+ |    |  （是不是FORM是就是1）
'vb5 (1) ---------------------------------+    |  （是不是VB5是就是1）
'HasOptInfo (1) -------------------------------+  （有没有额外的信息信息由就是1,决定是不是指向OptionalObjectInfo_t类似与PEHEAD里的Optional信息一样）
*/

typedef struct _VBMethodInfo
{
   VBObjectInfo*       pObjectInfo;
   WORD                Unknow;
   WORD                Flag;
   WORD                CodeOffset;
   WORD                Unknow2;
}VBMethodInfo;

//下面这个结构的成员好像会挪动，无所谓了，可能用不到
typedef struct _VBEventInfo
{
	DWORD               EventEntry0;
	DWORD               dwNull;
	VBControlInfo*      lpControl;
	VBObjectInfo*       lpObject;
	DWORD               SINK_QueryInterface_Entry;
	DWORD               SINK_AddRef_Entry;
	DWORD               SINK_Release_Entry;
	DWORD               EventArray[0x1F];
}VBEventInfo;


typedef struct _VBProjectInfo2
{ 
	DWORD      dwHeapLink;       //always 0
	BYTE*      lpObjectTable;    //Back-Pointer to the Object Table
	DWORD      dwReserved;       //Always set to -1
	DWORD      dwUnused;         //Not written or read in any case
	BYTE*      lpObjectList;     //Pointer to Object Private Descriptor Pointers
	DWORD      dwUnused2;        //Not written or read in any case
	BYTE*      szProjectDescription; //Pointer to Project Description
	BYTE*      szProjectHelpFile;   //Pointer to Project Help File
	DWORD      dwReserved2;         //Always set to -1
	DWORD      dwHelpContextId;     //Help Context ID set in Project Settings

}VBProjectInfo2;


typedef struct _VBObjectTable
{
      DWORD                   dwHeapLink;           //always 0
	  BYTE*                   lpExecProj;         //Pointer to VB Project Exec COM Object
	  VBProjectInfo2*         lpProjectInfo2;     //Secondary Project Information
	  DWORD                   dwReserved;          //Always set to -1 after compiling
	  DWORD                   dwNull;              //Not used
	  BYTE*                   lpProjectObject;     //Pointer to in-memory Project Data
	  BYTE                    uuidObjectTable[0x10]; //GUID of the Object Table
	  WORD                    wCompileType;      //Internal flag used during compilation
	  WORD                    wObjectsCount;     //Total objects present in Project
	  WORD                    wCompiledObjects;  //Equal to above after compiling
	  WORD                    wObjectsInUse;    //Usually equal to above after compile
	  VBObjectInfo*           lpObjectArray;    //Pointer to VBObject Object 
	  DWORD                   dwIdeFlag;         //Flag/Pointer used in IDE only
	  DWORD                   dwIdeData;         //Flag/Pointer used in IDE only
	  DWORD                   dwIdeData2;        //Flag/Pointer used in IDE only
	  BYTE*                   lpProjectName;    //Pointer to Project Name
	  DWORD                   dwlLcID1;          //LCID of Project
	  DWORD                   dwlLcID2;          //Alternate LCID of Project
	  DWORD                   dwlpIdeData3;     //Flag/Pointer used in IDE only
	  DWORD                   dwIdentifier;     //Template Version of Structure
}VBObjectTable;

typedef struct _VBExternalLibrary
{
	DWORD pLibraryName;      // VA 指向  NTS
	DWORD pLibraryFunction;  // VA 指向  NTS

}VBExternalLibrary;

typedef struct _VBExternalTable
{
	DWORD              Flag;              // 标志
	VBExternalLibrary* pExternalLibrary;  // VA 指向ExternalLibrary结构的地址指针

}VBExternalTable;


typedef struct _VBProjectInfo{
    DWORD                dwVersio;            //VB compatible version
	VBObjectTable*       lpObjectTable;      //Pointer to ObjectTable
	DWORD                lNull1;
	BYTE*                lpStartOfCode;     //Pointer to the start of some Assembly listing
	BYTE*                lpEndOfCode;       //Pointer to the end of some Assembly listing
	DWORD                dwDataBufferSize;  //Size of Data Buffer
	BYTE*                lpThreadSpace;
	BYTE*                lpVBAExceptionhandler; //Pointer to VBA Exception Handler
	DWORD                dwNativeCode;       //Pointer to the start of RAW Native Code,if aNativeCode equal,program compilered with p-code
	BYTE                 IncludeID[528];   //VB Include Internal Project Identifier
	VBExternalTable*     lpExternalTable;    //Pointer to API Imports Table
	DWORD                dwExternalCount;   //Number of API's imported

}VBProjectInfo;

typedef struct _VBGUITable
{
	DWORD     dwStructSize;       // 这个结构的总大小
	BYTE      uuidObjectGUI[16];  // Object GUI的UUID
	DWORD     Unknown1;          
	DWORD     Unknown2;         
	DWORD     Unknown3;         
	DWORD     Unknown4;          
	DWORD     dwObjectID;        // 当前工程的组件ID
	DWORD     Unknown5;         
	DWORD     dwOLEMisc;         // OLEMisc标志
	BYTE      uuidObject[16];    // 组件的UUID
	DWORD     Unknown6; 
	DWORD     Unknown7;          
	BYTE*     pFormPointer;     // VA 指向GUI Object Info结构的地址指针
	DWORD     Unknown8;         
}VBGUITable;

typedef struct _VBCOMRegData
{
	DWORD     dwRegInfo;                    //Offset 指向COM Interfaces Info结构（COM接口信息）
	DWORD     dwNTSProjectName;            //Offset 指向Project/Typelib Name（工程名）
	DWORD     dwNTSHelpDirectory;          //Offset 指向Help Directory（帮助文件目录）
	DWORD     dwNTSProjectDescription;      //Offset 指向Project Description（工程描述）
	BYTE      uuidProjectClsId[15];        //Project/Typelib的CLSID
	DWORD     dwlTlbLcid;                    //Type Library的LCID
	WORD      wPadding1;                    //没有用的内存对齐空间1
	WORD      wTlbVerMajor;                //Typelib 主版本
	WORD      wTlbVerMinor;                //Typelib 次版本
	WORD      wPadding2;                    //没有用的内存对齐空间2
	DWORD     dwPadding3;                  //没有用的内存对齐空间3
}VBCOMRegData;

typedef struct _VBHeader{
    BYTE   szMagic[4];              //"VB5!" signature
    WORD   wRuntimeBuild;           //Runtime flag
    BYTE   szLangDll[14];           //Language DLL
	BYTE   szBackupLanguageDLL[14]; //Backup language DLL
    WORD   wRuntimeRevision;       //Version of the runtime DLL
    DWORD  dwLanguageID;          //Application language
	DWORD  dwBackupLanguageID;   //Used with backup language DLL
	BYTE*  lpSubMain;            //Procedure to start
	VBProjectInfo*  lpProjectData; //Pointer to ProjectInfo
	DWORD  fMdlIntCtls;
	DWORD  fMdlIntCtls2;
	DWORD  dwThreadFlags;       //Thread flags
	DWORD  dwThreadCount;       //Number of threads
	WORD   wFormCount;          //Number of forms in this application
	WORD   wExternalComponentCount; //Number of external OCX components
	DWORD  dwThunkCount;
	VBGUITable* lpGuiTable;         //Pointer to GUITable
	BYTE*  lpExternalTable;        //Pointer to ExternalComponentTable
	VBCOMRegData*  lpComRegisterData;      //Pointer to ComRegisterData
	DWORD  dwProjectExenameOffset;  //Pointer to the string containing EXE filename     //以下3个结构的偏移都是相对于本结构的偏移
	DWORD  dwProjectTitleOffset;      //Pointer to the string containing project's title
	DWORD  dwHelpFileOffset;     //Pointer to the string containing name of the Help file
	DWORD  dwProjectNameOffset;        //Pointer to the string containing project's name
}VBHeader;



#pragma pack()

#endif
