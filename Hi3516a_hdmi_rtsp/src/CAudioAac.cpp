
#include <string.h>
#include "CAudioAac.h"


#define CONFIG_DEBUG
#undef CONFIG_DEBUG

#ifdef CONFIG_DEBUG
#define DBG(x...)           printf(x)
#else
#define DBG(x...)
#endif


typedef unsigned char   uint8_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;


static char szTemp[TEMP_BUFFER_SIZE]={0};
static BYTE szPcmBuff[PCM_BUFFER_SIZE] = {0};//1024
//////////////////////////////////////////////////////////////////////////
static short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF,
			    0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

static int search(int val, short	*table, int	size)
{
	int	i;

	for (i = 0; i < size; i++) {
		if (val <= *table++)
			return (i);
	}
	return (size);
}

/*
* alaw2linear() - Convert an A-law value to 16-bit linear PCM
*
*/
static int alaw2linear(unsigned char a_val)
{
	int	t;
	int	seg;

	a_val ^= 0x55;

	t = (a_val & QUANT_MASK) << 4;
	seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
	switch (seg) 
	{
	case 0:
		t += 8;
		break;
	case 1:
		t += 0x108;
		break;
	default:
		t += 0x108;
		t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
}


/*
* ulaw2linear() - Convert a u-law value to 16-bit linear PCM
*
* First, a biased linear code is derived from the code word. An unbiased
* output can then be obtained by subtracting 33 from the biased code.
*
* Note that this function expects to be passed the complement of the
* original code word. This is in keeping with ISDN conventions.
*/
static int ulaw2linear(unsigned char u_val)
{
	int	t;

	/* Complement to obtain normal u-law value. */
	u_val = ~u_val;

	/*
	* Extract and bias the quantization bits. Then
	* shift up by the segment number and subtract out the bias.
	*/
	t = ((u_val & QUANT_MASK) << 3) + BIAS;
	t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

	return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}


/*
 * linear2alaw() - Convert a 16-bit linear PCM value to 8-bit A-law
 *
 */
static unsigned char linear2alaw(int pcm_val)	/* 2's complement (16-bit range) */
{
	int		mask;
	int		seg;
	unsigned char	aval;

	if (pcm_val >= 0) {
		mask = 0xD5;		/* sign (7th) bit = 1 */
	} else {
		mask = 0x55;		/* sign bit = 0 */
		pcm_val = -pcm_val - 8;
	}

	/* Convert the scaled magnitude to segment number. */
	seg = search(pcm_val, seg_end, 8);

	/* Combine the sign, segment, and quantization bits. */

	if (seg >= 8)		/* out of range, return maximum value. */
		return (0x7F ^ mask);
	else {
		aval = seg << SEG_SHIFT;
		if (seg < 2)
			aval |= (pcm_val >> 4) & QUANT_MASK;
		else
			aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
		return (aval ^ mask);
	}
}


/*
 * linear2ulaw() - Convert a linear PCM value to u-law
 *
 */
static unsigned char linear2ulaw(int pcm_val)	/* 2's complement (16-bit range) */
{
	int		mask;
	int		seg;
	unsigned char	uval;

	/* Get the sign and the magnitude of the value. */
	if (pcm_val < 0) {
		pcm_val = BIAS - pcm_val;
		mask = 0x7F;
	} else {
		pcm_val += BIAS;
		mask = 0xFF;
	}

	/* Convert the scaled magnitude to segment number. */
	seg = search(pcm_val, seg_end, 8);

	/*
	 * Combine the sign, segment, quantization bits;
	 * and complement the code word.
	 */
	if (seg >= 8)		/* out of range, return maximum value. */
		return (0x7F ^ mask);
	else {
		uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);
		return (uval ^ mask);
	}

}

int g711a_decode(short amp[], const unsigned char g711a_data[], int g711a_bytes)
{
	int i;
	int samples;
	unsigned char code;
	int sl;

	for (samples = i = 0;;)
	{
		if (i >= g711a_bytes)
			break;
		code = g711a_data[i++];

		sl = alaw2linear(code);

		amp[samples++] = (short) sl;
	}
	return samples*2;
}

int g711u_decode(short amp[], const unsigned char g711u_data[], int g711u_bytes)
{
	int i;
	int samples;
	unsigned char code;
	int sl;

	for (samples = i = 0;;)
	{
		if (i >= g711u_bytes)
			break;
		code = g711u_data[i++];

		sl = ulaw2linear(code);

		amp[samples++] = (short) sl;
	}
	return samples*2;
}

int g711a_encode(unsigned char g711_data[], const short amp[], int len)
{
    int i;

    for (i = 0;  i < len;  i++)
	{
        g711_data[i] = linear2alaw(amp[i]);
    }

    return len;
}

int g711u_encode(unsigned char g711_data[], const short amp[], int len)
{
    int i;

    for (i = 0;  i < len;  i++)
	{
        g711_data[i] = linear2ulaw(amp[i]);
    }

    return len;
}
//////////////////////////////////////////////////////////////////////////
#if 1
CAudioAac::CAudioAac(void)
{
	m_nTempPos = 0;
	m_pTempBuffer = NULL;
	m_pOutAACBuffer = NULL;
	m_pbPCMBuffer = NULL;

	b_init_flag = 0;

#if USE_FAAC
	m_hEncoder = NULL;
#elif USE_LABFAAC
	h_aacenc = NULL;
#elif USE_FDKAAC
	handle = NULL;
#endif
}

CAudioAac::~CAudioAac(void)
{
/*
	if ( m_pTempBuffer)
		delete [] m_pTempBuffer;

	if ( m_pOutAACBuffer)
		delete [] m_pOutAACBuffer;

	if ( m_pbPCMBuffer)
		delete [] m_pbPCMBuffer;
*/
	Finish();
}


bool CAudioAac::init( int nSampleRate, int nChannal, int bitsPerSample, int bps)
{
	m_nTempPos = 0;

	m_nSampleRate = nSampleRate;
	m_nAudioChannal = nChannal;
	m_nBitsPerSample = bitsPerSample;

#if USE_FAAC
	// init faac
	m_hEncoder = faacEncOpen( nSampleRate, nChannal, &m_nInputSamples, &m_nMaxOutputBytes);

#elif USE_LABFAAC
    h_aacenc = fa_aacenc_init(nSampleRate, 16000, nChannal,
                   FA_AACENC_MPEG_VER_DEF , FA_AACENC_OBJ_TYPE_DEF, 0,
                   20, 3, 0);
    m_nInputSamples = 2048;
#elif USE_FDKAAC
	int bitrate;
	int ch;
	int format, sample_rate, channels, bits_per_sample;
	int input_size;
	uint8_t* input_buf;
	int16_t* convert_buf;
	int aot = 2;		// MPEG-4 AAC Low Complexity, aacenc_lib.h" 1228 lines
	int afterburner = 0; //1;
	int eld_sbr = 0;
	int vbr = 0;
	CHANNEL_MODE mode;
	AACENC_InfoStruct info = { 0 };

	printf("sampleRate:%d, chn:%d, bps:%d\n", nSampleRate, nChannal, bps);
	DBG("%s, %d\n", __FUNCTION__, __LINE__);

	format = 1;

	channels = 1;
	bitrate = bps;   // for > frequecy 32K, bps must > 32000
	sample_rate =  nSampleRate; // 8000;
 	bits_per_sample = 16;

	mode = MODE_1;   // 1 chn
	if (aacEncOpen(&handle, 0x01, channels) != AACENC_OK) {    //0x01: AAC module.
		fprintf(stderr, "Unable to open encoder\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_AOT, aot) != AACENC_OK) {
		fprintf(stderr, "Unable to set the AOT\n");
		return 1;
	}
	if (aot == 39 && eld_sbr) {
		if (aacEncoder_SetParam(handle, AACENC_SBR_MODE, 1) != AACENC_OK) {
			fprintf(stderr, "Unable to set SBR mode for ELD\n");
			return 1;
		}
	}
	if (aacEncoder_SetParam(handle, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
		fprintf(stderr, "Unable to set the AOT\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
		fprintf(stderr, "Unable to set the channel mode\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
		fprintf(stderr, "Unable to set the wav channel order\n");
		return 1;
	}
	if (vbr) {
		if (aacEncoder_SetParam(handle, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
			fprintf(stderr, "Unable to set the VBR bitrate mode\n");
			return 1;
		}
	} else {
		if (aacEncoder_SetParam(handle, AACENC_BITRATE, bitrate) != AACENC_OK) {
			fprintf(stderr, "Unable to set the bitrate\n");
			return 1;
		}
	}
	if (aacEncoder_SetParam(handle, AACENC_TRANSMUX, 0) != AACENC_OK) {
		fprintf(stderr, "Unable to set the ADTS transmux\n");
		return 1;
	}
	if (aacEncoder_SetParam(handle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
		fprintf(stderr, "Unable to set the afterburner mode\n");
		return 1;
	}
	if (aacEncEncode(handle, NULL, NULL, NULL, NULL) != AACENC_OK) {
		fprintf(stderr, "Unable to initialize the encoder\n");
		return 1;
	}
	if (aacEncInfo(handle, &info) != AACENC_OK) {
		fprintf(stderr, "Unable to get the encoder info\n");
		return 1;
	}

	printf("aac encoder config: ");   
	int confSize;
	for(confSize = 0; confSize < info.confSize; confSize++)
		printf("0x%02x ", info.confBuf[confSize]);
	
	aac_config = info.confBuf[1] << 8 | info.confBuf[0]; 
	printf(", aac_config:0x%04x\n", aac_config);

    m_nInputSamples = 1024;
    m_nMaxOutputBytes = 2048;
#endif
	m_nMaxInputBytes = m_nInputSamples * bitsPerSample/8;	
	m_pbPCMBuffer = new BYTE [m_nMaxInputBytes];
	m_pOutAACBuffer = new BYTE [m_nMaxOutputBytes];
	m_pTempBuffer = new BYTE [TEMP_BUFFER_SIZE];
	memset(m_pTempBuffer, 0 , TEMP_BUFFER_SIZE);

	DBG("m_nMaxInputBytes:%d\n", m_nMaxInputBytes);	
#if USE_FAAC
	// Get current encoding configuration
	faacEncConfigurationPtr pConfiguration = faacEncGetCurrentConfiguration(m_hEncoder);
	if( !pConfiguration )
	{
		printf("GetCurrentConfiguration error!\n");
		return false;
	}

	//设置版本,录制MP4文件时要用MPEG4
	pConfiguration->version = MPEG4 ;
	pConfiguration->aacObjectType = MAIN; //LOW; //LC编码

	//输入数据类型
	pConfiguration->inputFormat = FAAC_INPUT_16BIT;
	
	// outputFormat (0 = Raw; 1 = ADTS)
	// 录制MP4文件时，要用raw流。检验编码是否正确时可设置为 adts传输流，
	pConfiguration->outputFormat= 0;//1;    // 设置为0，没有声音出来

	//瞬时噪声定形(temporal noise shaping，TNS)滤波器
	pConfiguration->shortctl = SHORTCTL_NORMAL;

	pConfiguration->useTns=true;
	//pConfiguration->useLfe=false;
	pConfiguration->quantqual=100;
	pConfiguration->bandWidth=0;
	pConfiguration->bitRate= 0;

	//  Set encoding configuration
	faacEncSetConfiguration( m_hEncoder, pConfiguration);

	unsigned char* faacDecoderInfo = NULL;    
	unsigned long  faacDecoderInfoSize = 0;    
	if( faacEncGetDecoderSpecificInfo( m_hEncoder, &faacDecoderInfo, &faacDecoderInfoSize )) 
	{        
		free(faacDecoderInfo);
		return false ;   
	}

	printf("faac encoder config: ");   
	int confSize;
	for(confSize = 0; confSize < faacDecoderInfoSize; confSize++)
		printf("0x%02x ", faacDecoderInfo[confSize]);
	
	//faacDecoderInfo[0] = 0x15, faacDecoderInfo[1] = 0x88;  // 8K
	//faacDecoderInfo[0] = 0x12, faacDecoderInfo[1] = 0x08;	// 44.1K
	//memcpy(&aac_config, faacDecoderInfo, confSize);
	aac_config =  (faacDecoderInfo[1] << 8) | faacDecoderInfo[0];
	printf(", aac_config:0x%04x\n", aac_config);
printf("-------------------------\n");
	free(faacDecoderInfo);

#elif USE_LABFAAC
    
#endif

/*
	// 2.1 create mp4 file
	m_Mp4File = MP4Create( pMp4File,  0);
	if ( m_Mp4File == MP4_INVALID_FILE_HANDLE)
	{
		printf("open file fialed.\n");
		return false;
	}

	// 2.2 add audio track
	m_AudioTrackId  = MP4AddAudioTrack( m_Mp4File, nSampleRate, m_nInputSamples , MP4_MPEG4_AUDIO_TYPE );
	if (m_AudioTrackId == MP4_INVALID_TRACK_ID)
	{
		MP4Close(m_Mp4File);
		//free(faacDecoderInfo);
		return false;
	}

	// 2.3 set audio level  LC
	MP4SetAudioProfileLevel(m_Mp4File, 0x2 ); //  
*/
#if 0
	// 2.4 get decoder info
	unsigned char* faacDecoderInfo = NULL;    
	unsigned long  faacDecoderInfoSize = 0;    
	if( faacEncGetDecoderSpecificInfo( m_hEncoder, &faacDecoderInfo, &faacDecoderInfoSize )) 
	{        
		//MP4Close(m_Mp4File);
		free(faacDecoderInfo);
		return false ;   
	} 

	// 2.5 set encoder info [16bit-8000hz-1channal->{ 0x15, 0x88 } ]
	bool bOk = TRUE;
	//bOk = MP4SetTrackESConfiguration(m_Mp4File, m_AudioTrackId, faacDecoderInfo, faacDecoderInfoSize );
	if( !bOk )
	{
		free(faacDecoderInfo);
		//MP4Close(m_Mp4File);
		return false;   
	}
	free( faacDecoderInfo );
#endif

	DBG("aac init\n");

	b_init_flag = 1;

	return true;
}

void CAudioAac::Finish()
{
	if ( m_pTempBuffer)
	{
		delete [] m_pTempBuffer;
		m_pTempBuffer = NULL;
	}	
	
	if ( m_pOutAACBuffer)
	{
		delete [] m_pOutAACBuffer;
		m_pOutAACBuffer = NULL;
	}
	
	if ( m_pbPCMBuffer)
	{
		delete [] m_pbPCMBuffer;
		m_pbPCMBuffer = NULL;
	}

	m_pTempBuffer = NULL;
	m_pbPCMBuffer = NULL;
	m_pOutAACBuffer = NULL;
#if USE_FAAC
	if(m_hEncoder)
	{
		faacEncClose( m_hEncoder);
		m_hEncoder = NULL;
	}
#elif USE_LABFAAC
    /*free the encode handle*/
    if(h_aacenc)
	{
		fa_aacenc_uninit(h_aacenc);
		h_aacenc = NULL;
	}
#elif USE_FDKAAC
	if(handle)
	{
		aacEncClose(&handle);
		handle = NULL;
	}
#endif

	b_init_flag = 0;
}

// 目前输入的是原始的PCM数据
//void CAudioAac::StartWriteMp4(unsigned char * pBufferG711,int nG711Len)
int CAudioAac::G7112Aac(unsigned char * pBufferG711,int nG711Len)
{
	aacSize = -1; // 每次必须要清 0
	/*
	if ( G711_BUFFER_SIZE < nG711Len )
	{
		return -1;
	}
	*/

	//BYTE szPcmBuff[PCM_BUFFER_SIZE] = {0};//1024
	//////////////////////////////////////////////////////////////////////////
	int nPcmLen; 
#if 0
    // 输入的是G.711数据
	nPcmLen = g711a_decode( (short*)szPcmBuff, pBufferG711, nG711Len );
#else
	// 输入的是原始数据
	nPcmLen = nG711Len;
	memcpy(szPcmBuff, pBufferG711, nG711Len);
#endif

	DBG("input, nPcmLen : %d, m_nTempPos : %d\n", nPcmLen, m_nTempPos);
	//////////////////////////////////////////////////////////////////////////
	if((m_nTempPos + nPcmLen) > TEMP_BUFFER_SIZE) 
	{
		printf("exceed max buffersize, m_nTempPos + nPcmLen: %d> %d\n", m_nTempPos + nPcmLen, TEMP_BUFFER_SIZE);
		exit(1);
	}

	memcpy(m_pTempBuffer + m_nTempPos, szPcmBuff, nPcmLen) ;
	m_nTempPos += nPcmLen;

    DBG("after add, m_nTempPos:%d\n", m_nTempPos);

	if ( m_nTempPos < m_nMaxInputBytes )
	{
		return -1;
	}

	//DBG("%s, %d\n", __FUNCTION__, __LINE__);

	memcpy(m_pbPCMBuffer, m_pTempBuffer, m_nMaxInputBytes ) ;
	
	//char szTemp[TEMP_BUFFER_SIZE]={0};
	int nLeft = m_nTempPos - m_nMaxInputBytes;
	memcpy( szTemp, (m_pTempBuffer + m_nMaxInputBytes), nLeft );
	memset(m_pTempBuffer, 0, TEMP_BUFFER_SIZE );
	memcpy( m_pTempBuffer, szTemp, nLeft );
	m_nTempPos -= m_nMaxInputBytes ;

    DBG("after sub, m_nTempPos:%d, m_nInputSamples:%d\n", m_nTempPos, m_nInputSamples);

#if USE_FAAC
	aacSize = faacEncEncode(m_hEncoder, (int*)m_pbPCMBuffer, m_nInputSamples, m_pOutAACBuffer, m_nMaxOutputBytes );
#elif USE_LABFAAC
    fa_aacenc_encode(h_aacenc, (unsigned char *)m_pbPCMBuffer, m_nInputSamples, m_pOutAACBuffer, &aacSize);
#elif USE_FDKAAC
		AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
		AACENC_InArgs in_args = { 0 };
		AACENC_OutArgs out_args = { 0 };
		int in_identifier = IN_AUDIO_DATA;
		int in_size, in_elem_size;
		int out_identifier = OUT_BITSTREAM_DATA;
		int out_size, out_elem_size;
		int read, i;
		void *in_ptr, *out_ptr;
		//uint8_t outbuf[20480];
		AACENC_ERROR err;
		{
			read = m_nMaxInputBytes;

			in_ptr = m_pbPCMBuffer;
			in_size = read;
			in_elem_size = 2;

			in_args.numInSamples = read/2;
			in_buf.numBufs = 1;
			in_buf.bufs = &in_ptr;
			in_buf.bufferIdentifiers = &in_identifier;
			in_buf.bufSizes = &in_size;
			in_buf.bufElSizes = &in_elem_size;
		}

		out_ptr = m_pOutAACBuffer;
		out_size = m_nMaxOutputBytes;
		out_elem_size = 1;
		out_buf.numBufs = 1;
		out_buf.bufs = &out_ptr;
		out_buf.bufferIdentifiers = &out_identifier;
		out_buf.bufSizes = &out_size;
		out_buf.bufElSizes = &out_elem_size;

		if ((err = aacEncEncode(handle, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
			if (err == AACENC_ENCODE_EOF)
				return -1;
			fprintf(stderr, "Encoding failed\n");
			return -1;
		}
		
		DBG("aac encode size :%d\n", out_args.numOutBytes);

		if (out_args.numOutBytes == 0)
			return -1;

		aacSize = out_args.numOutBytes;

#endif
	if ( aacSize <= 0 )
	{
		return -1;
	}

	//MP4WriteSample( m_Mp4File, m_AudioTrackId, (BYTE*)&m_pOutAACBuffer[7], aacSize-7 , 1024, 0, true);
	
	//
	return aacSize;
}

int CAudioAac::Aac2G711(unsigned char * pBufferAac, int aacLen, unsigned char * pBufferG711,int * nG711Len)
{
	return 0;
}

#endif
