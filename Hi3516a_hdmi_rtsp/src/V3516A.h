

#include  "sample_comm.h"
#include  "v_ini.h"

#include "XHLiveManager.h"
extern XHLiveManager* g_pXHLiveManager;
extern bool g_bInit;
//#include "StartLiveSrc.h"
//extern StartLiveSrc gStartLiveSrc;
bool gloableSettingStarrtc();
HI_S32 StartVideoEnc();
HI_S32 StopVideoEnc();

HI_S32 SAMPLE_AUDIO_AiAenc(HI_VOID);
void UDP_THREAD(void);
void init_VideoParam(void);
HI_S32 Load_Param(HI_VOID);
HI_U32 HTX_CreatTrdCheckViMode(void);
HI_U32 SetFrameRateAndBitRate(HI_U32 nFrameRate, HI_U32 nBitRate);
extern ini_reader  reader;

