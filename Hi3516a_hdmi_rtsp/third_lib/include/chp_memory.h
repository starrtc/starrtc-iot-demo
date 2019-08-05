/**====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

FILE: chp_memory.h

The memory manage unit. deal with the malloc and free for memory.

Copyright (c) 2005 by CHIPNUTS Incorporated. All Rights Reserved.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/**===========================================================================

REVISIONS:
Version  		Name       	Date			Description
1.0  			Tanent	  	04/14/2006  	Initial Version
1.1			Baggio		05/30/2007	To avoid data abort exception,add address aligned by 4 bytes 
									to chp_create_mempool(), add data length aligned by 4 bytes to 
									chp_mempool_malloc().
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

#ifndef _CHP_MEMORY_HEAD
#define _CHP_MEMORY_HEAD

#include "chp_comdef.h"

// The Block max number
#define CHP_MBLOCK_NUM			256
// The Structure define the all information of the memory pool
typedef struct _CHP_MEMPOOL_INFO_T
{
	CHP_U8 *p_addr;				// The start addr of the memory
	CHP_U32 size;						// The size of the memory
	CHP_U8 *p_free;					// The start addr of the first free block
	CHP_U32 malloced[CHP_MBLOCK_NUM];		// The malloced block's start addr record
} CHP_MEMPOOL_INFO_T;

typedef enum
{
	CHP_MP_SUCCESS,
	CHP_MP_CANNOT_MALLOC,
	CHP_MP_CANNOT_RELEASE,
	CHP_MP_SIZE_TOO_LARGE,
	CHP_MP_LINK_ERROR,
	CHP_MP_SIZE_NOT_SUPPORTED
}CHP_MEMPOOL_RTN_E;


// The length of the block's head is 4 bytes
// the lower three bytes describle the length of the block
#define CHP_MBLOCK_SIZE_SIG		0x00ffffff
// the higher one byte describle some information of the block, 
// the higher first bit: 1, describle the block is free; 0, describle the block is malloc-ed
// the rest bits is reserved
#define CHP_MBLOCK_HEAD_LEN		4
#define CHP_MBLOCK_FREE_SIG		0x80000000
#define CHP_MBLOCK_RESERVE1		0x40000000
#define CHP_MBLOCK_RESERVE2		0x20000000
#define CHP_MBLOCK_RESERVE3		0x10000000
#define CHP_MBLOCK_RESERVE4		0x08000000
#define CHP_MBLOCK_RESERVE5		0x04000000
#define CHP_MBLOCK_RESERVE6		0x02000000
#define CHP_MBLOCK_RESERVE7		0x01000000

// The length of the block's tail is 4 bytes
// if the block is free that the 4 bytes is the next free block's start addr
// else if the block is malloc-ed that it is the next malloced or free block's start addr
#define CHP_MBLOCK_TAIL_LEN		4

// The external define
extern CHP_MEMPOOL_RTN_E 	chp_create_mempool(CHP_U8 *p_buf, CHP_U32 size_t);
extern void 	*chp_mempool_malloc(CHP_U32 size_t);
extern void	chp_mempool_release(void *p_buffer);


#endif /*_CHP_MEMORY_HEAD*/
