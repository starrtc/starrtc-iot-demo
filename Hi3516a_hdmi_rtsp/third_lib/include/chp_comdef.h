/*=================================================================
  Copyright (c) 2003 by CHIPNUTS Incorporated. All Rights Reserved.
  FileName: chp_comdef.h
  
  Author: Jerry       Date: 05/31/2006

  Description: header file for common definition.
  Version:     V1.0
  
  History:     
  <author>  <time>       <version>        <desc>    
  Jerry        05/31/2007     1.0        Initial Version  
  baggio	   2007-10-31	      1.1  	     modify  the architecture
==================================================================*/
#ifndef _CHP_COMDEF_H
#define _CHP_COMDEF_H

#ifndef NULL
#define NULL 		0
#endif

#ifndef	CHP_U8
	#define CHP_U8		unsigned char
#endif

#ifndef	CHP_U16
#define CHP_U16		unsigned short
#endif

#ifndef	CHP_U32
#define CHP_U32		unsigned int
#endif

#ifndef	CHP_U64
#define CHP_U64		unsigned long long 
#endif

#ifndef	CHP_8
#define CHP_8		signed char
#endif

#ifndef	CHP_16
#define CHP_16		signed short
#endif

#ifndef	CHP_32
#define CHP_32		signed long
#endif

#ifndef	CHP_FILE
#define CHP_FILE		int
#endif

#ifndef	CHP_RTN_T
#define CHP_RTN_T	unsigned int
#endif


#ifndef	UINT8
#define	UINT8					unsigned char
#endif

#ifndef	UINT16
#define	UINT16				unsigned short
#endif

#ifndef	UINT32
#define	UINT32				unsigned int
#endif

#ifndef	WCHAR
#define	WCHAR				unsigned short int
#endif

#ifndef	UNICODE
#define	UNICODE			unsigned short int
#endif

#ifndef	UINT64
#define	UINT64				unsigned long long
#endif

#ifndef	INT64
#define	INT64				long long int
#endif

#ifndef	INT32
#define	INT32				long int
#endif

#ifndef	INT16
#define	INT16				short int
#endif

#ifndef	INT8
#define	INT8				char
#endif

#ifndef	CHAR
#define	CHAR				char
#endif

/*
#ifndef	bool
#define	bool					unsigned short int
#endif
*/

#ifndef	STATUS_T
#define	STATUS_T			unsigned short int
#endif




#ifndef	UWORD8
#define	UWORD8		unsigned char
#endif

#ifndef	UWORD16
#define	UWORD16	unsigned short 
#endif

#ifndef	UWORD32
#define	UWORD32	unsigned int 
#endif

#ifndef	UWORD64
#define	UWORD64	unsigned long long
#endif

#ifndef	WORD8
#define	WORD8		signed char
#endif

#ifndef	WORD16
#define	WORD16	 	short 
#endif

#ifndef	WORD32
#define	WORD32	 	int 
#endif

#ifndef	WORD64
#define	WORD64		long long
#endif

#ifndef	Word8
#define	Word8		WORD8
#endif

#ifndef	UWord8
#define	UWord8		UWORD8
#endif

#ifndef	Word16
#define	Word16		WORD16
#endif

#ifndef	Word32
#define	Word32		WORD32
#endif

#ifndef	UWord16
#define	UWord16	UWORD16
#endif

#ifndef	UWord32
#define	UWord32	UWORD32
#endif

#ifndef	inpb
#define inpb(port)        (*((volatile CHP_U8 *) (port)))
#endif

#ifndef	inpw
#define inpw(port)       (*((volatile CHP_U32 *) (port)))
#endif

#ifndef	inph 
#define inph(port)       (*((volatile CHP_U16 *) (port)))
#endif

#ifndef	outpb 
#define outpb(port, val)  (*((volatile CHP_U8 *) (port)) = ((CHP_U8) (val)))
#endif

#ifndef	outpw
#define outpw(port, val) (*((volatile CHP_U32 *) (port)) = ((CHP_U32) (val)))
#endif

#ifndef	outph
#define outph(port, val) (*((volatile CHP_U16 *) (port)) = ((CHP_U16) (val)))
#endif

#ifndef	MA_INB
#define MA_INB( io )  (CHP_U8) inpb( io )
#endif

#ifndef	MA_INW
#define MA_INW( io )  (CHP_U32) inpw( io )
#endif

#ifndef	MA_INH 
#define MA_INH( io )  (CHP_U16) inph( io )
#endif

#ifndef	MA_INBM
#define MA_INBM( io, mask ) ( inpb( io ) & (mask) )
#endif

#ifndef	MA_INWM
#define MA_INWM( io, mask ) ( inpw( io ) & (mask) )
#endif

#ifndef	MA_INHM
#define MA_INHM( io, mask ) ( inph( io ) & (mask) )
#endif

#ifndef	MA_OUTB
#define MA_OUTB( io, val )  (void) outpb( io, (int) val)
#endif

#ifndef	MA_OUTW
#define MA_OUTW( io, val )  (void) outpw( io, (int) val)
#endif

#ifndef	MA_OUTH
#define MA_OUTH( io, val )  (void) outph( io, (int) val)
#endif


#ifndef	MA_OUTBM
#define MA_OUTBM( io, mask, val ) {CHP_U8 temp; (temp) =(((MA_INW(io) & (CHP_U8)(~(mask))) | ((CHP_U8)((val) & (mask))))); ((void) outpb( io, (CHP_U8)(temp)));}
#endif

#ifndef	MA_OUTWM 
#define MA_OUTWM( io, mask, val) {CHP_U32 temp; (temp) =(((MA_INW(io) & (CHP_U32)(~(mask))) |((CHP_U32)((val) & (mask))))); (void) outpw( io, (CHP_U32)(temp)); }
#endif  

#ifndef	MA_OUTHM
#define MA_OUTHM( io, mask, val){CHP_U32 temp; (temp) =(((MA_INW(io) & (CHP_U16)(~(mask))) |((CHP_U16)((val) & (mask)))));(void) outph( io, (CHP_U16)(temp)); }
#endif

enum 
{ 
	CHP_RTN_SUCCESS,
};



typedef void *(*CHP_MALLOC_FUNC)(CHP_U32);
typedef void (*CHP_FREE_FUNC)(void *); 
typedef void *(*CHP_MEMSET)(void *, CHP_32 c, CHP_U32);
typedef void *(*CHP_MEMCPY)(void *, const void *, CHP_U32);


typedef struct
{
	CHP_MALLOC_FUNC 	chp_malloc;
	CHP_FREE_FUNC 		chp_free;
	CHP_MEMSET			chp_memset;
	CHP_MEMCPY			chp_memcpy;
}CHP_MEM_FUNC_T;


/*switch little endian long long to local endian long long, x is a char pointer*/
#define PLE64TOCPU(X)   ((CHP_U64)(*(X+7))<<56 | (CHP_U64)(*(X+6))<<48 \
				|(CHP_U64)(*(X+5))<<40 | (CHP_U64)(*(X+4))<<32 \
				|(CHP_U64)(*(X+3))<<24 | (CHP_U64)(*(X+2))<<16 \
		              | (CHP_U64)(*(X+1))<<8 | *(X))


/*switch little endian long to local endian long, x is a char pointer*/
#define PLE32TOCPU(X)   ((CHP_U32)(*(X+3))<<24 | (CHP_U32)(*(X+2))<<16 \
		             | (CHP_U32)(*(X+1))<<8 | *(X))
		             
/*switch little endian short to local endian short, x is a char pointer*/
#define PLE16TOCPU(X)   ((CHP_U16)(*(X+1))<<8 | *(X))

/*switch big endian long to local endian long, x is a char pointer*/
#define PBE32TOCPU(X)   ((CHP_U32)(*(X))<<24 | (CHP_U32)(*(X+1))<<16 \
		             | (CHP_U32)(*(X+2))<<8 | *(X+3))

/*switch big endian short to local endian short, x is a char pointer*/
#define PBE16TOCPU(X)   ((CHP_U16)(*(X))<<8 | *(X+1))

#define BYTE_SWAP_32(X)	{CHP_U8	*p_byte = (CHP_U8*)&(X);\
							 (X) = (p_byte[0] << 24) | (p_byte[1] << 16) | (p_byte[2] << 8) | (p_byte[3]);}

#define BYTE_SWAP_16(X)	{CHP_U8	*p_byte = (CHP_U8*)&(X);\
							 (X) = (p_byte[0] << 8) | (p_byte[1]);}


#endif
