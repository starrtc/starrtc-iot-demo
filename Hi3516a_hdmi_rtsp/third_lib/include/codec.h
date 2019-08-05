#ifndef _CODEC_H_
#define _CODEC_H_

#include "chp_comdef.h"

typedef enum
{
	CHP_DRI_CODEC_MP3 = 0x100,
	CHP_DRI_CODEC_AAC_ADTS,
	CHP_DRI_CODEC_AAC_RAW,
	CHP_DRI_CODEC_ADPCM,
	CHP_DRI_CODEC_AMRNB,
	CHP_DRI_CODEC_AMRWB,
	CHP_DRI_CODEC_WMA,
	CHP_DRI_CODEC_MIDI0,
	CHP_DRI_CODEC_OGG,
	CHP_DRI_CODEC_SBC,
	CHP_DRI_CODEC_FLAC,
	CHP_DRI_CODEC_APE,	
	CHP_DRI_CODEC_RA,//just suppot LBR

	CHP_DRI_CODEC_H263 = 0x200,
	CHP_DRI_CODEC_MPEG4,
	CHP_DRI_CODEC_H264,
	CHP_DRI_CODEC_VC1,
	CHP_DRI_CODEC_MPEG2,
	CHP_DRI_CODEC_AVS,
	CHP_DRI_CODEC_RV
}CHP_CODEC_E;

enum
{
	CHP_RTN_AUD_SUCCESS = 0,
	CHP_RTN_AUD_MEM_MALLOC_FAIL,
	//decode
	CHP_RTN_AUD_DEC_INIT_FAIL = 0x200,
	CHP_RTN_AUD_GET_INFO_FAIL,
	CHP_RTN_AUD_DEC_FAIL,
	CHP_RTN_AUD_DEC_CLOSE_FAIL,
	CHP_RTN_AUD_OP_TIMEOUT,
	CHP_RTN_AUD_NEED_MORE_DATA,
	//encode
	CHP_RTN_AUD_ENC_INIT_FAIL = 0x2000,
	CHP_RTN_AUD_ENC_FAIL,
	CHP_RTN_AUD_ENC_TIMEOUT,
	CHP_RTN_AUD_ENC_CLOSE_FAIL,
	CHP_RTN_AUD_ENC_NEED_MORE_DATA
};


typedef struct
{
	CHP_U32 bit_rate;			//比特率
	CHP_U32 sample_rate;		//采样率
	CHP_U32 channel_mode;		//mono, 0 ; stereo, 1
	void*	para_info;
}CHP_AUD_DEC_INFO_T;

typedef struct
{
	void *p_in_buf;				//输入原始数据的缓冲区地址
	void *p_out_buf;			//输出PCM数据的缓冲区地址
	CHP_U32 in_buf_len;			//有效输入原始数据的大小
	CHP_U32 out_buf_len;		//输出PCM数据缓冲区的大小
	CHP_U32 frame_cnt;			//需要解码的帧数
	CHP_U32 used_size;			//解码实际用掉的原始数据大小
	CHP_U32 dec_frame_cnt;		//实际解码的帧数
	CHP_U32 dec_pcm_size;		//实际解码的PCM数据大小
	void*	p_para;			//some codecs need the current bitstream format,so add this item here
					//the format of bitstream defined by codec
}CHP_AUD_DEC_DATA_T;


typedef struct
{
	CHP_U32 audio_type;			//AAC,mp3...
	CHP_U32 bit_rate;			//12800…
	CHP_U32 sample_rate;		//8K, 32K, 44.1K…
	CHP_U32 sample_size;		//16bit, 18bit, 24bit…
	CHP_U32 channel_mode;		//mono, 0 ; stereo, 1
}CHP_AUD_ENC_INFO_T;

typedef struct
{
	void *p_in_buf;				//RAW PCM数据存储的地址
	void *p_out_buf;			//编码后的音频数据存放地址
	CHP_U32 in_buf_len;			//RAW PCM缓存的大小
	CHP_U32 out_buf_len;		//压缩缓存区的大小
	CHP_U32 frame_cnt;			//需要编码的帧数
	CHP_U32 used_size;			//实际使用的RAW PCM存储缓冲区大小
	CHP_U32 enc_frame_cnt;		//实际编码的帧数
	CHP_U32 enc_data_len; 		//压缩后音频数据的长度
}CHP_AUD_ENC_DATA_T;

#endif
