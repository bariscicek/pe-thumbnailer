/* 
 * File:   thumbnailer.h
 * Author: draco
 *
 * Created on November 14, 2008, 6:01 AM
 */

#ifndef _THUMBNAILER_H
#define	_THUMBNAILER_H

#ifdef	__cplusplus
extern "C" {
#endif

/* 
 * COFF header structure
 */
struct _coff_header_struct
{
    gint16  machine;
    gint16  numberofsections;
    gint32  timedatestamp;
    gint32  pointertosymboltable;
    gint32  numberofsymbols;
    gint16  sizeofoptionalheader;
    gint16  characteristics;
};

/*
 * Image characteristics
 */

#define IMAGE_FILE_RELOCS_STRIPPED          0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE         0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED       0x0004
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED      0x0008
#define IMAGE_FILE_AGGRESSIVE_WS_TRIM       0x0010
#define IMAGE_FILE_LARGE_ADDRESS_AWARE      0x0020
#define IMAGE_FILE_16BIT_MACHINE            0x0040
#define IMAGE_FILE_32BIT_MACHINE            0x0080
#define IMAGE_FILE_BYTES_REVERSED_LO        0x0100
#define IMAGE_FILE_DEBUG_STRIPPED           0x0200
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP  0x0400
#define IMAGE_FILE_SYSTEM                   0x1000
#define IMAGE_FILE_DLL                      0x2000
#define IMAGE_FILE_UP_SYSTEM_ONLY           0x4000
#define IMAGE_FILE_BYTES_REVERSED_HI        0x8000


/*
 *  Optional header standard fields
 */

struct _optional_header_std_struct
{
    guint16 magic;
    guchar  majorlinkerversion;
    guchar  minorlinkerversion;
    gint32  sizeofcode;
    gint32  sizeofinitializeddata;
    gint32  sizeofuninitializedata;
    gint32  addressofentrypoint;
    gint32  baseofcose;
    gint32  baseofdata;
};

struct _optional_header_nt_struct
{
    gint32  imagebase;
    gint32  sectionalignment;
    gint32  filealignment;
    gint16  majoroperatingsystemversion;
    gint16  minoroperatingsystemversion;
    gint16  majorimageversion;
    gint16  minorimageversion;
    gint16  majorsubsystemversion;
    gint16  minorsubsystemversion;
    gint32  reserved1;
    gint32  sizeofimage;
    gint32  sizeofheaders;
    gint32  checksum;
    gint16  subsystem;
    gint16  dllcharacteristics;
    gint32  sizeofstackreserve;
    gint32  sizeofstackcommit;
    gint32  sizeofheapreserve;
    gint32  sizeofheapcommit;
    gint32  loaderflags;
    gint32  numberofrvaandsizes;
    
};

/*
 *  Image Data Directory Struc
 */

struct _IMAGE_DATA_DIRECTORY
{
    gint32  rva;
    gint32  size;
};

/*
 * Section table
 */
struct _section_table_struct
{
    gint64  name;
    gint32  virtualsize;
    gint32  virtualaddress;
    gint32  sizeofrawdata;
    gint32  pointertorawdata;
    gint32  pointertorelocations;
    gint32  pointertolinenumbers;
    gint16  numberofrelocations;
    gint16  numberoflinenumbers;
    gint32  characteristics;
};

/*
 *  Resource directory table struct and resource directory entries struct
 */
struct _resource_directory_table_struct
{
    gint32  characteristics;
    gint32  timeanddatestamp;
    gint16  majorversion;
    gint16  minorversion;
    gint16  numberofnameentries;
    gint16  numberofidentries;
};

struct _resource_directory_entries_struct
{
    union
    {
        gint32  offsetofname;
        struct
        {
            unsigned    offsetofname:31;
            unsigned    isname:1;
        } name;
        gint32  id;
    } nameoridrva;
    union
    {
        gint32  offsetofdata;
        struct
        {
            unsigned offsetofdirectory:31;
            unsigned isdirectory:1;
        } directory;
    } data;
};

/*
 *  Resource directory string struct and resource data entry struct
 */

struct _resource_directory_string_struct
{
    gint16  lenght;
    gchar   *unicodestring;
};

struct _resource_data_entry_struct
{
    gint32  datarva;
    gint32  size;
    gint32  codepage;
    gint32  reserved1;
};


/*
 *  Taken from wine/include/wingdi.h
 */


typedef struct {
    guchar   rgbBlue;
    guchar   rgbGreen;
    guchar   rgbRed;
    guchar   rgbReserved;
} RGBQUAD;


typedef struct {
    gint32  biSize;
    gint64  biWidth;
    gint64  biHeight;
    gint16  biPlanes;
    gint16  biBitCount;
    gint32  biCompression;
    gint32  biSizeImage;
    gint64  biXPelsPerMeter;
    gint64  biYPelsPerMeter;
    gint32  biClrUsed;
    gint32  biClrImportant;
} BITMAPINFOHEADER;

typedef struct {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;

typedef struct {
		guchar	bWidth;
		guchar	bHeight;
		guchar	bColorCount;
		guchar	bReserved;
		gint16	wPlanes;
		gint16	wBitCount;
		gint32	dwBytesInRes;
		gint32	dwImageOffset;
} ICONDIRENTRY;

typedef struct {
		gint16	idReserved;
		gint16	idType;
		gint16	idCount;
		ICONDIRENTRY		idEntries[1];
} ICONDIR;




#ifdef	__cplusplus
}
#endif

#endif	/* _THUMBNAILER_H */

