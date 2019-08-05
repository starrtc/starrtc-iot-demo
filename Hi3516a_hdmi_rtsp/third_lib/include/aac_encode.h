#ifndef _AAC_ENCODE_H_
#define _AAC_ENCODE_H_

#include "codec.h"

typedef struct _AAC_ENC_OP_INFO_
{
	void *p_aacEncInfo;			//store mp3 software decoder internal Info
	CHP_MEM_FUNC_T p_mem_func;			//store Jubit-8 heap/stack managed function
}AACEncOpInfo_T;

CHP_RTN_T aac_encoder_init(CHP_MEM_FUNC_T *p_mem_func,CHP_AUD_ENC_INFO_T *p_enc_info,CHP_U32 *bl_handle);
CHP_RTN_T aac_encode(CHP_U32 bl_handle,CHP_AUD_ENC_DATA_T *p_enc_data);
CHP_RTN_T aac_encoder_close(CHP_U32 bl_handle);

#endif
