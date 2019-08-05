#ifndef _MP4_MUTEX_H_
#define _MP4_MUTEX_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define USE_FAAC        0
#define USE_LABFAAC     0
#define USE_FDKAAC      1

#if USE_FAAC
#include "faac.h"
#elif USE_LABFAAC
#include "fa_aacapi.h"
#elif USE_FDKAAC
#include "fdk-aac/aacdecoder_lib.h"
#include "fdk-aac/aacenc_lib.h"
#include "fdk-aac/FDK_audio.h"
#include "fdk-aac/genericStds.h"
#include "fdk-aac/machine_type.h"
#endif

//#define G711_BUFFER_SIZE	(1200)
#define PCM_BUFFER_SIZE		(2400)

#define TEMP_BUFFER_SIZE	(1024 * 4)
#define ADTS_HEADER_LENGTH	(7)



typedef unsigned long   ULONG;
typedef unsigned int    UINT;
typedef unsigned char   BYTE;
typedef char            _TCHAR;

/*
* u-law, A-law and linear PCM conversions.
*/
#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS		(8)		/* Number of A-law segments. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */
#define	BIAS		(0x84)		/* Bias for linear code. */


int g711a_decode(short amp[], const unsigned char g711a_data[], int g711a_bytes);

int g711u_decode(short amp[], const unsigned char g711u_data[], int g711u_bytes);

int g711a_encode(unsigned char g711_data[], const short amp[], int len);

int g711u_encode(unsigned char g711_data[], const short amp[], int len);

//--------------------------------------------------------
//-- Name :		CAudioAac
//-- Describle: MP4音频文件接口
//--------------------------------------------------------

#if 1
class CAudioAac
{
public:
	CAudioAac(void);
	virtual ~CAudioAac(void);
	
public:
	
	//--------------------------------------------------------
	//-- Name     : init
	//-- Describle: 初始化 
	//-- param[in]: nSampleRate		音频采样率 8000
	//-- param[in]: nChannal		通道数  1
	//-- param[in]: bitsPerSample	位深 16
	//-- param[in]: pMp4File		要创建的mp4文件绝对路径
	//--------------------------------------------------------
	bool init( int nSampleRate, int nChannal,int bitsPerSample, int bps);
	
	
	//--------------------------------------------------------
	//-- Name     : 
	//-- Describle: G711 to AAC
	//-- param[in]: pBufferG711		输入G711缓冲区
	//-- param[in]: nG711Len		缓冲区长度
	//--------------------------------------------------------
	int G7112Aac(unsigned char * pBufferG711,int nG711Len);
	
	//--------------------------------------------------------
	//-- Name     : 
	//-- Describle: AAC to G711
	//-- param[in]: pBufferG711		输入G711缓冲区
	//-- param[in]: nG711Len		缓冲区长度
	//--------------------------------------------------------
	int Aac2G711(unsigned char * pBufferAac, int aacLen, unsigned char * pBufferG711,int * nG711Len);
	//--------------------------------------------------------
	//-- Name     : Finish
	//-- Describle: 操作结束 资源释放
	//--------------------------------------------------------
	void Finish();
	
	
public:
	int b_init_flag;

#if USE_FAAC	
	faacEncHandle m_hEncoder;	
#elif USE_LABFAAC
    uintptr_t h_aacenc;
#elif USE_FDKAAC
    HANDLE_AACENCODER handle;
#endif

	int m_nAudioChannal;		// 声道数
	int m_nSampleRate;			// 采样率
	int m_nBitsPerSample;		// 单样本位数(eg:16) 单个采样音频信息位数 
	
	ULONG m_nInputSamples ;		// 输入样本数
	ULONG m_nMaxOutputBytes;	// 输出所需最大空间
	ULONG m_nMaxInputBytes;		// 输入所需最大空间
	
	//MP4FileHandle m_Mp4File;
	//MP4TrackId m_AudioTrackId; //MP4TrackId
	int aacSize;
	unsigned short aac_config;
	BYTE* m_pbPCMBuffer ;
    BYTE* m_pOutAACBuffer;
private:
	BYTE* m_pTempBuffer;
	int m_nTempPos;
};
#endif

#endif
