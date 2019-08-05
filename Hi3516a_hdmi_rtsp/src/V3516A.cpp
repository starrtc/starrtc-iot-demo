
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include  "V3516A.h"
#include "hi_mipi.h"

#include "hi_common.h"
#include "users.h"
#include <vector>

#include "sample_comm.h"
#include "acodec.h"

#include "RtspCaster.h"
#include "g711.h"
//#include "autils1.h"

extern "C"
{
//#include "interface_starLiveVideo.h"
//#include "autils1.h"
//	#include "aac_encode.h"
};
#include "CAudioAac.h"

#define FILENAME  "param.ini"
#define DEV_FILE   "/dev/sil9135"

SAMPLE_VI_MODE_E InputMode[4]={SAMPLE_VI_MODE_BT1120_720P,SAMPLE_VI_MODE_BT1120_1080P,SAMPLE_VI_MODE_BT1120_1080I,SAMPLE_VI_MODE_1_D1};

int Vi_Mode,RcMode,SaveMode=0,StRotate = 0,Vi_Input = -1,AudioEncType,SDI_Input;
	
VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
static SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara;
static pthread_t gs_VencPid;
static int rl_frame ;
static int rl_rate ;
static int rl_fbl;
ini_reader  reader;

void my_aac_init(void);

#define AUDIO_FREQ AUDIO_SAMPLE_RATE_48000 //
// for aac audio encoder
CAudioAac	m_aac;

#include <string.h>
#include <time.h>

/// caster
HANDLE g_handle[2] = {NULL, NULL};

MFormat g_format[2];
int g_videoPktCount = 0;
int g_audioPktCount = 0;
int g_videoDuration = 0;
int g_audioDuration = 0;
int g_audioAllSize=0;
int g_audioPts=0;
HI_U64  g_audioPtsPre=0;
struct timespec g_videoTimespec;
struct timespec g_audioTimespec;
int64_t g_lastVideoPts = 0;
int64_t g_lastAudioPts = 0;
int64_t g_startVideoPts = 0;
int64_t g_startAudioPts = 0;

//StartLiveSrc gStartLiveSrc;
XHLiveManager* g_pXHLiveManager = NULL;
bool g_bInit = false;
/// 
typedef struct _CHECK_VI_MODE
{
	int fd;
	HI_BOOL bStart;
	pthread_t checkid;
	
}CHECK_VI_MODE;

//arm-hisiv300-linux-g++ -o readpara readpara.cpp  -L ./ -lcommfun-1.0
/////////////////////////////////////////////
#include <iostream>
using namespace std;
  

#define STORE_STREAM

//��i��ͷ���Ǵ�cgi��ȡ��ֵ
int i_enc_media_size ;
int i_enc_keyframe ;
int i_enc_ratemode ;
int i_enc_quant;
int i_enc_bitrate ;
int i_enc_framerate ;
int i_enc_bright;
int i_enc_contrast;
int i_enc_saturation;
int i_enc_hue;
int g_ld ;
int g_sd;
int g_bhd;
int g_dbd;

VI_DEV_ATTR_S DEV_ATTR_BT656D1_1MUX =
{
    /* interface mode */
    VI_MODE_BT656,
    /* multiplex mode */
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF0000,    0x0},
    /* progessive or interleaving */
    VI_SCAN_INTERLACED,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, only support yuv*/
    VI_INPUT_DATA_YVYU,     
    /* synchronization information */
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,    
    
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    },    
    /* ISP bypass */
    VI_PATH_BYPASS,
    /* input data type */
    VI_DATA_TYPE_YUV
};

/* BT1120 1080I���� */
VI_DEV_ATTR_S DEV_ATTR_BT1120_1080I_1MUX =
{
    /* interface mode */
    VI_MODE_BT1120_STANDARD,
    /* multiplex mode */
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0xFF0000},
    /* progessive or interleaving */
    VI_SCAN_INTERLACED,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, only support yuv*/
    VI_INPUT_DATA_UVUV,

    /* synchronization information */
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,
   
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    },    
    /* ISP bypass */
    VI_PATH_BYPASS,
     /* input data type */
    VI_DATA_TYPE_YUV,
    /* bReverse */
    HI_FALSE,    
     /* DEV CROP */
    {0, 0, 1920, 1080}
};

/* BT1120 1080p */
VI_DEV_ATTR_S DEV_ATTR_BT1120_1080P_BASE =
{
    /* interface mode */
    VI_MODE_BT1120_STANDARD,
    /* multiplex mode */
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0xFF0000},
    /* progessive or interleaving */
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, only support yuv*/
    VI_INPUT_DATA_UVUV,

    /* synchronization information */
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,
   
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    },    
    /* ISP bypass */
    VI_PATH_BYPASS,
     /* input data type */
    VI_DATA_TYPE_YUV,
    /* bReverse */
    HI_FALSE,    
     /* DEV CROP */
    {0, 0, 1920, 1080}
};

 
/* BT1120 720P */
VI_DEV_ATTR_S DEV_ATTR_BT1120_720P_BASE =
/* classical timing 3:7441 BT1120 720P@60fps*/
{
    /* interface mode */
    VI_MODE_BT1120_STANDARD,
    /* multiplex mode */
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0xFF0000},
    /* progessive or interleaving */
    VI_SCAN_PROGRESSIVE,
    //VI_SCAN_INTERLACED,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, only support yuv*/
    VI_INPUT_DATA_UVUV,

     /* synchronization information */
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,
    
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /* ISP bypass */
    VI_PATH_BYPASS,
    /* input data type */
    VI_DATA_TYPE_YUV,
    /* bReverse */
    HI_FALSE,    
     /* DEV CROP */
    {0, 0, 1280, 720}
};



VI_DEV_ATTR_S DEV_ATTR_MIPI_BASE =
{
    /* interface mode */
    VI_MODE_MIPI,
    /* multiplex mode */
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFFF00000,    0x0},
    /* progessive or interleaving */
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, only support yuv*/
    VI_INPUT_DATA_YUYV,

    /* synchronization information */
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,
   
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /* use interior ISP */
    VI_PATH_ISP,
    /* input data type */
    VI_DATA_TYPE_RGB,    
    /* bRever */
    HI_FALSE,    
    /* DEV CROP */
    {0, 0, 1920, 1080}
};


combo_dev_attr_t MIPI_BT1120_ATTR =
{
#if 1
    /* input mode */
    .input_mode = INPUT_MODE_BT1120,
    {
        
    }
#endif
};



/******************************************************************************
* function : get picture size(w*h), according Norm and enPicSize
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize)
{
	HI_U32 tmp;
    switch (enPicSize)
    {
        case PIC_QCIF:
            pstSize->u32Width  = 176;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?144:120;
            break;
        case PIC_CIF:
            pstSize->u32Width  = 352;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?288:240;
            break;
        case PIC_D1:
            pstSize->u32Width  = 720;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_960H:
            pstSize->u32Width  = 960;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;			
        case PIC_2CIF:
            pstSize->u32Width  = 360;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_QVGA:    /* 320 * 240 */
            pstSize->u32Width  = 320;
            pstSize->u32Height = 240;
            break;
        case PIC_VGA:     /* 640 * 480 */
            pstSize->u32Width  = 640;
            pstSize->u32Height = 480;
            break;
        case PIC_XGA:     /* 1024 * 768 */
            pstSize->u32Width  = 1024;
            pstSize->u32Height = 768;
            break;
        case PIC_SXGA:    /* 1400 * 1050 */
            pstSize->u32Width  = 1400;
            pstSize->u32Height = 1050;
            break;
        case PIC_UXGA:    /* 1600 * 1200 */
            pstSize->u32Width  = 1600;
            pstSize->u32Height = 1200;
            break;
        case PIC_QXGA:    /* 2048 * 1536 */
            pstSize->u32Width  = 2048;
            pstSize->u32Height = 1536;
            break;
        case PIC_WVGA:    /* 854 * 480 */
            pstSize->u32Width  = 854;
            pstSize->u32Height = 480;
            break;
        case PIC_WSXGA:   /* 1680 * 1050 */
            pstSize->u32Width = 1680;
            pstSize->u32Height = 1050;
            break;
        case PIC_WUXGA:   /* 1920 * 1200 */
            pstSize->u32Width  = 1920;
            pstSize->u32Height = 1200;
            break;
        case PIC_WQXGA:   /* 2560 * 1600 */
            pstSize->u32Width  = 2560;
            pstSize->u32Height = 1600;
            break;
        case PIC_HD720:   /* 1280 * 720 */
            pstSize->u32Width  = 1280;
            pstSize->u32Height = 720;
            break;
        case PIC_HD1080:  /* 1920 * 1080 */
            pstSize->u32Width  = 1920;
            pstSize->u32Height = 1080;
            break;
		case PIC_2304x1296:  /* 2304 * 1296 */
			pstSize->u32Width  = 2304;
            pstSize->u32Height = 1296;
			break;
		case PIC_2592x1520:  /* 2592 * 1520 */
			pstSize->u32Width  = 2592;
            pstSize->u32Height = 1520;
			break;
        case PIC_5M:      /* 2592 * 1944 */
            pstSize->u32Width  = 2592;
            pstSize->u32Height = 1944;
            break;
            
        default:
            return HI_FAILURE;
    }
	
	if(StRotate==1||StRotate==3)
	{
		tmp=pstSize->u32Width;
		pstSize->u32Width = pstSize->u32Height;
		pstSize->u32Height = tmp;
	}
    return HI_SUCCESS;
}


/******************************************************************************
* function : calculate VB Block size of picture.
******************************************************************************/
HI_U32 SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, PIXEL_FORMAT_E enPixFmt, HI_U32 u32AlignWidth)
{
    HI_S32 s32Ret = HI_FAILURE;
    SIZE_S stSize;
    HI_U32 u32VbSize;
    HI_U32 u32HeaderSize;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size[%d] failed!\n", enPicSize);
            return HI_FAILURE;
    }

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 != enPixFmt && PIXEL_FORMAT_YUV_SEMIPLANAR_420 != enPixFmt)
    {
        SAMPLE_PRT("pixel format[%d] input failed!\n", enPixFmt);
            return HI_FAILURE;
    }

    if (16!=u32AlignWidth && 32!=u32AlignWidth && 64!=u32AlignWidth)
    {
        SAMPLE_PRT("system align width[%d] input failed!\n",\
               u32AlignWidth);
            return HI_FAILURE;
    }
    //SAMPLE_PRT("w:%d, u32AlignWidth:%d\n", CEILING_2_POWER(stSize.u32Width,u32AlignWidth), u32AlignWidth);
    u32VbSize = (CEILING_2_POWER(stSize.u32Width, u32AlignWidth) * \
            CEILING_2_POWER(stSize.u32Height,u32AlignWidth) * \
           ((PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixFmt)?2:1.5));

    VB_PIC_HEADER_SIZE(stSize.u32Width, stSize.u32Height, enPixFmt, u32HeaderSize);
    u32VbSize += u32HeaderSize;

    return u32VbSize;
}

/******************************************************************************
* funciton : Get enSize by diffrent sensor
******************************************************************************/
HI_S32 SAMPLE_COMM_VI_GetSizeBySensor(PIC_SIZE_E *penSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_VI_MODE_E enMode = InputMode[Vi_Input];

    if (!penSize)
    {
        return HI_FAILURE;
    }
    
    switch (enMode)
    {
        case PANASONIC_MN34220_SUBLVDS_720P_120FPS:
        case PANASONIC_MN34220_MIPI_720P_120FPS:
            *penSize = PIC_HD720;
            break;
        case APTINA_MT9P006_DC_1080P_30FPS:
        case PANASONIC_MN34220_SUBLVDS_1080P_30FPS:
        case PANASONIC_MN34220_MIPI_1080P_30FPS:
        case OMNIVISION_OV4689_MIPI_1080P_30FPS:
        case APTINA_AR0330_MIPI_1080P_30FPS:
        case SONY_IMX178_LVDS_1080P_30FPS:
        case SONY_IMX185_MIPI_1080P_30FPS:            
            *penSize = PIC_HD1080;
            break;
		case APTINA_AR0330_MIPI_1536P_25FPS:
			*penSize = PIC_QXGA;
			break;
		case APTINA_AR0330_MIPI_1296P_25FPS:
			*penSize = PIC_2304x1296;
			break;
		case OMNIVISION_OV4689_MIPI_4M_30FPS:
			*penSize = PIC_2592x1520;
			break;
        case SONY_IMX178_LVDS_5M_30FPS:
        case OMNIVISION_OV5658_MIPI_5M_30FPS:
            *penSize = PIC_5M;
            break;

        default:
            break;
    }

    return s32Ret;
}

/******************************************************************************
* function : vb init & MPI system init
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_Init(VB_CONF_S *pstVbConf)
{
    MPP_SYS_CONF_S stSysConf = {0};
    HI_S32 s32Ret = HI_FAILURE;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    if (NULL == pstVbConf)
    {
        SAMPLE_PRT("input parameter is null, it is invaild!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_SetConf(pstVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = SAMPLE_SYS_ALIGN_WIDTH;
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_SetConf failed\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Init failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VI_StartMIPI_BT1120(SAMPLE_VI_MODE_E enViMode)
{
	HI_S32 fd;
	combo_dev_attr_t *pstcomboDevAttr;
	
	fd = open("/dev/hi_mipi", O_RDWR);
	if (fd < 0)
	{
	   printf("warning: open hi_mipi dev failed\n");
	   return -1;
	}

	if((enViMode == SAMPLE_VI_MODE_BT1120_1080I)
		||(enViMode == SAMPLE_VI_MODE_1_D1)
		||(enViMode == SAMPLE_VI_MODE_BT1120_720P)
		||(enViMode == SAMPLE_VI_MODE_BT1120_1080P))
	{
		pstcomboDevAttr = &MIPI_BT1120_ATTR;
	}
	else
	{

	}
	
	if (ioctl(fd, HI_MIPI_SET_DEV_ATTR, pstcomboDevAttr))
	{
		printf("set mipi attr failed\n");
		close(fd);
		return -1;
	}
	close(fd);
	return HI_SUCCESS;
}

/*****************************************************************************
* function : star vi dev (cfg vi_dev_attr; set_dev_cfg; enable dev)
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_StartDev(VI_DEV ViDev, SAMPLE_VI_MODE_E enViMode)
{
    HI_S32 s32Ret;
    HI_S32 s32IspDev = 0;
    ISP_WDR_MODE_S stWdrMode;
    VI_DEV_ATTR_S  stViDevAttr;
    
    memset(&stViDevAttr,0,sizeof(stViDevAttr));

    switch (enViMode)
    {
        case SAMPLE_VI_MODE_1_D1:
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_1MUX,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
				stViDevAttr.stDevRect.s32Y = 0;
				stViDevAttr.stDevRect.u32Width = 720;
				stViDevAttr.stDevRect.u32Height = 576;
            break;

        case SAMPLE_VI_MODE_BT1120_1080I:
            memcpy(&stViDevAttr,&DEV_ATTR_BT1120_1080I_1MUX,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width = 1920;
            stViDevAttr.stDevRect.u32Height = 1080;
            break;

        case SAMPLE_VI_MODE_BT1120_1080P:			
        	stViDevAttr.stDevRect.s32X = 0;
        	            stViDevAttr.stDevRect.s32Y = 0;
        	            stViDevAttr.stDevRect.u32Width = 1920;
        	            stViDevAttr.stDevRect.u32Height = 1080;
        	memcpy(&stViDevAttr,&DEV_ATTR_BT1120_1080P_BASE,sizeof(stViDevAttr));
			break;
			
	    case SAMPLE_VI_MODE_BT1120_720P:
    	    memcpy(&stViDevAttr,&DEV_ATTR_BT1120_720P_BASE,sizeof(stViDevAttr));        
    	    stViDevAttr.stDevRect.s32X = 0;
    	            	            stViDevAttr.stDevRect.s32Y = 0;
    	            	            stViDevAttr.stDevRect.u32Width = 1280;
    	            	            stViDevAttr.stDevRect.u32Height = 720;
    	    break;
			
		case SONY_IMX122_DC_1080P_30FPS:
		//	memcpy(&stViDevAttr,&DEV_ATTR_IMX122_DC_1080P,sizeof(stViDevAttr));
			break;		
       
        case APTINA_MT9P006_DC_1080P_30FPS:     
         //   memcpy(&stViDevAttr,&DEV_ATTR_MT9P006_DC_1080P,sizeof(stViDevAttr));
            break; 

        case PANASONIC_MN34220_SUBLVDS_1080P_30FPS:
        case SONY_IMX178_LVDS_1080P_30FPS:        
         //   memcpy(&stViDevAttr,&DEV_ATTR_LVDS_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 20;
            stViDevAttr.stDevRect.u32Width  = 1920;
            stViDevAttr.stDevRect.u32Height = 1080;
            break;

        case SONY_IMX185_MIPI_1080P_30FPS:
        case PANASONIC_MN34220_MIPI_1080P_30FPS:
            memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 8;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width  = 1920;
            stViDevAttr.stDevRect.u32Height = 1080;
            break;

        case SONY_IMX178_LVDS_5M_30FPS:
          //  memcpy(&stViDevAttr,&DEV_ATTR_LVDS_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 20;
            stViDevAttr.stDevRect.u32Width  = 2592;
            stViDevAttr.stDevRect.u32Height = 1944;
            break; 
			
        case PANASONIC_MN34220_MIPI_720P_120FPS:
			memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 8;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width  = 1280;
            stViDevAttr.stDevRect.u32Height = 720;
			break;

		case PANASONIC_MN34220_SUBLVDS_720P_120FPS:
		//	memcpy(&stViDevAttr,&DEV_ATTR_LVDS_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 20;
            stViDevAttr.stDevRect.u32Width  = 1280;
            stViDevAttr.stDevRect.u32Height = 720;
			break;
			
		case APTINA_AR0330_MIPI_1080P_30FPS:
		//	memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width  = 1920;
            stViDevAttr.stDevRect.u32Height = 1080;
			break;
			
		case APTINA_AR0330_MIPI_1296P_25FPS:
			memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width  = 2304;
            stViDevAttr.stDevRect.u32Height = 1296;
			break;
			
		case APTINA_AR0330_MIPI_1536P_25FPS:
			memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width  = 2048;
            stViDevAttr.stDevRect.u32Height = 1536;
			break;
			
		case OMNIVISION_OV4689_MIPI_1080P_30FPS:
			memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width  = 1920;
            stViDevAttr.stDevRect.u32Height = 1080;
			break;
			
		case OMNIVISION_OV4689_MIPI_4M_30FPS:
			memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width  = 2592;
            stViDevAttr.stDevRect.u32Height = 1520;
			break;
			
		case OMNIVISION_OV5658_MIPI_5M_30FPS:
			memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
            stViDevAttr.stDevRect.s32X = 0;
            stViDevAttr.stDevRect.s32Y = 0;
            stViDevAttr.stDevRect.u32Width  = 2592;
            stViDevAttr.stDevRect.u32Height = 1944;
			break;
			
        default:
            memcpy(&stViDevAttr,&DEV_ATTR_MIPI_BASE,sizeof(stViDevAttr));
    }

    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    if ((SAMPLE_VI_MODE_BT1120_1080P != enViMode)
		&&(SAMPLE_VI_MODE_BT1120_1080I != enViMode)
		&&(SAMPLE_VI_MODE_1_D1 != enViMode)
		&&(SAMPLE_VI_MODE_BT1120_720P != enViMode))
	{
#if 0
	    s32Ret = HI_MPI_ISP_GetWDRMode(s32IspDev, &stWdrMode);
	    if (s32Ret != HI_SUCCESS)
	    {
	        SAMPLE_PRT("HI_MPI_ISP_GetWDRMode failed with %#x!\n", s32Ret);
	        return HI_FAILURE;
	    }

	    if (stWdrMode.enWDRMode)  //wdr mode
	    {
	        VI_WDR_ATTR_S stWdrAttr;

	        stWdrAttr.enWDRMode = stWdrMode.enWDRMode;
	        stWdrAttr.bCompress = HI_FALSE;

	        s32Ret = HI_MPI_VI_SetWDRAttr(ViDev, &stWdrAttr);
	        if (s32Ret)
	        {
	            SAMPLE_PRT("HI_MPI_VI_SetWDRAttr failed with %#x!\n", s32Ret);
	            return HI_FAILURE;
	        }
	    }
#endif
	}
    
    s32Ret = HI_MPI_VI_EnableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
/*****************************************************************************
* function : star vi chn
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stChnAttr;
    ROTATE_E enRotate = ROTATE_NONE;
    SAMPLE_VI_CHN_SET_E enViChnSet = VI_CHN_SET_NORMAL;

    if(pstViConfig)
    {
        enViChnSet = pstViConfig->enViChnSet;
        enRotate = pstViConfig->enRotate;
    }

    /* step  5: config & start vicap dev */
    memcpy(&stChnAttr.stCapRect, pstCapRect, sizeof(RECT_S));
    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
    /* to show scale. this is a sample only, we want to show dist_size = D1 only */
    stChnAttr.stDestSize.u32Width = pstTarSize->u32Width;
    stChnAttr.stDestSize.u32Height = pstTarSize->u32Height;
    stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;   /* sp420 or sp422 */

    stChnAttr.bMirror = HI_FALSE;
    stChnAttr.bFlip = HI_FALSE;

    switch(enViChnSet)
    {
        case VI_CHN_SET_MIRROR:
            stChnAttr.bMirror = HI_TRUE;
            break;

        case VI_CHN_SET_FLIP:
            stChnAttr.bFlip = HI_TRUE;
            break;
            
        case VI_CHN_SET_FLIP_MIRROR:
            stChnAttr.bMirror = HI_TRUE;
            stChnAttr.bFlip = HI_TRUE;
            break;
            
        default:
            break;
    }

    stChnAttr.s32SrcFrameRate = -1;
    stChnAttr.s32DstFrameRate = -1;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;

    s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
	
    if(ROTATE_NONE != enRotate)
    {
        s32Ret = HI_MPI_VI_SetRotate(ViChn, enRotate);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VI_SetRotate failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
    s32Ret = HI_MPI_VI_EnableChn(ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
/*****************************************************************************
* function : star vi according to product type
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_StartBT656(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_U32 u32DevNum = 1;
    HI_U32 u32ChnNum = 1;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    SAMPLE_VI_MODE_E enViMode;

    if(!pstViConfig)
    {
        SAMPLE_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }
    enViMode = pstViConfig->enViMode;

	/******************************************
	 step 1: mipi configure
	******************************************/

	//if(Vi_Mode==1)
	{
		s32Ret = SAMPLE_COMM_VI_StartMIPI_BT1120(enViMode);   
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("%s: MIPI init failed!\n", __FUNCTION__);
			return HI_FAILURE;
		}	
	}
		
	for (i = 0; i < u32DevNum; i++)
	{
		ViDev = i;
		s32Ret = SAMPLE_COMM_VI_StartDev(ViDev, enViMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("%s: start vi dev[%d] failed!\n", __FUNCTION__, i);
			return HI_FAILURE;
		}
	}
    
    /******************************************************
    * Step 2: config & start vicap chn (max 1) 
    ******************************************************/
	for (i = 0; i < u32ChnNum; i++)
	{
		ViChn = i;

		stCapRect.s32X = 0;
		stCapRect.s32Y = 0;
		switch (enViMode)
		{
			case SAMPLE_VI_MODE_BT1120_720P:
				stCapRect.u32Width = 1280;
				stCapRect.u32Height = 720;
				break;
			case SAMPLE_VI_MODE_BT1120_1080I:
			case SAMPLE_VI_MODE_BT1120_1080P:
				stCapRect.u32Width  = 1920;
				stCapRect.u32Height = 1080;
				break;
			case SAMPLE_VI_MODE_1_D1:
				stCapRect.u32Width  = 720;
				stCapRect.u32Height = 576;
				break;
			default:
				stCapRect.u32Width  = 1920;
				stCapRect.u32Height = 1080;
				break;
		}

        stTargetSize.u32Width = stCapRect.u32Width;
        stTargetSize.u32Height = stCapRect.u32Height;

        s32Ret = SAMPLE_COMM_VI_StartChn(ViChn, &stCapRect, &stTargetSize, pstViConfig);
        if (HI_SUCCESS != s32Ret)
        {
          //SAMPLE_COMM_ISP_Stop();
            return HI_FAILURE;
        }
    }

    return s32Ret;
}

HI_S32 SAMPLE_COMM_VI_Mode2Param(SAMPLE_VI_MODE_E enViMode, SAMPLE_VI_PARAM_S *pstViParam)
{
	switch (enViMode)
	{
		default:
			pstViParam->s32ViDevCnt      = 1;
			pstViParam->s32ViDevInterval = 1;
			pstViParam->s32ViChnCnt      = 1;
			pstViParam->s32ViChnInterval = 1;
			break;
	}
    return HI_SUCCESS;
}



HI_BOOL IsSensorInput(SAMPLE_VI_MODE_E enViMode)
{
    HI_BOOL bRet = HI_TRUE;

    switch(enViMode)
    {
        case SAMPLE_VI_MODE_1_D1:
        case SAMPLE_VI_MODE_BT1120_1080I:
        case SAMPLE_VI_MODE_BT1120_1080P:
        case SAMPLE_VI_MODE_BT1120_720P:
            bRet = HI_FALSE;
            break;
        default:
            break;
    }

    return bRet;    
}

HI_S32 SAMPLE_COMM_VI_StartVi(SAMPLE_VI_CONFIG_S* pstViConfig)
{
	HI_S32 s32Ret = HI_SUCCESS;
	SAMPLE_VI_MODE_E enViMode;

	if(!pstViConfig)
	{
		SAMPLE_PRT("%s: null ptr\n", __FUNCTION__);
		return HI_FAILURE;
	}
	
	enViMode = pstViConfig->enViMode;
	if(!IsSensorInput(enViMode))
	{
		s32Ret = SAMPLE_COMM_VI_StartBT656(pstViConfig);  //��߽���bt1120�Ľӿ�
	}
	else
	{
		//  s32Ret = SAMPLE_COMM_VI_StartIspAndVi(pstViConfig);//����ǽ���
	}


    return s32Ret; 
}

/*****************************************************************************
* function : Vi chn bind vpss group
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_BindVpss(SAMPLE_VI_MODE_E enViMode)
{
	HI_S32 j, s32Ret;
	VPSS_GRP VpssGrp;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	SAMPLE_VI_PARAM_S stViParam;
	VI_CHN ViChn;

	s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
	if (HI_SUCCESS !=s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
		return HI_FAILURE;
	}

	VpssGrp = 0;
	for (j=0; j<stViParam.s32ViChnCnt; j++)
	{
		ViChn = j * stViParam.s32ViChnInterval;

		stSrcChn.enModId  = HI_ID_VIU;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = ViChn;

		stDestChn.enModId  = HI_ID_VPSS;
		stDestChn.s32DevId = VpssGrp;
		stDestChn.s32ChnId = 0;

		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}

		VpssGrp ++;
	}
	return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VPSS_StartGroup(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    HI_S32 s32Ret;
    VPSS_GRP_PARAM_S stVpssParam;
    
    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM)
    {
        printf("VpssGrp%d is out of rang. \n", VpssGrp);
        return HI_FAILURE;
    }

    if (HI_NULL == pstVpssGrpAttr)
    {
        printf("null ptr,line%d. \n", __LINE__);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    /*** set vpss param ***/
    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    
    s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VPSS_EnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, 
                                                  VPSS_CHN_ATTR_S *pstVpssChnAttr,
                                                  VPSS_CHN_MODE_S *pstVpssChnMode,
                                                  VPSS_EXT_CHN_ATTR_S *pstVpssExtChnAttr)
{
	HI_S32 s32Ret;

	if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM)
	{
		printf("VpssGrp%d is out of rang[0,%d]. \n", VpssGrp, VPSS_MAX_GRP_NUM);
		return HI_FAILURE;
	}

	if (VpssChn < 0 || VpssChn > VPSS_MAX_CHN_NUM)
	{
		printf("VpssChn%d is out of rang[0,%d]. \n", VpssChn, VPSS_MAX_CHN_NUM);
		return HI_FAILURE;
	}

	if (HI_NULL == pstVpssChnAttr && HI_NULL == pstVpssExtChnAttr)
	{
		printf("null ptr,line%d. \n", __LINE__);
		return HI_FAILURE;
	}

	if (VpssChn < VPSS_MAX_PHY_CHN_NUM)
	{
		s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, pstVpssChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}
	}
	else
	{
		s32Ret = HI_MPI_VPSS_SetExtChnAttr(VpssGrp, VpssChn, pstVpssExtChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("%s failed with %#x\n", __FUNCTION__, s32Ret);
			return HI_FAILURE;
		}
	}
    
	if (VpssChn < VPSS_MAX_PHY_CHN_NUM && HI_NULL != pstVpssChnMode)
	{
		s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, pstVpssChnMode);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("%s failed with %#x\n", __FUNCTION__, s32Ret);
			return HI_FAILURE;
		}
	}
    
	s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
		return HI_FAILURE;
	}
	
    return HI_SUCCESS;
}


/******************************************************************************
* funciton : Start venc stream mode (h264, mjpeg)
* note      : rate control parameter need adjust, according your case.
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_Start(VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode,HI_U32  u32Profile)
{
	HI_S32 s32Ret;
	VENC_CHN_ATTR_S stVencChnAttr;
	VENC_ATTR_H264_S stH264Attr;
	VENC_ATTR_H264_CBR_S    stH264Cbr;
	VENC_ATTR_H264_VBR_S    stH264Vbr;
	VENC_ATTR_H264_FIXQP_S  stH264FixQp;
	VENC_ATTR_H265_S        stH265Attr;
	VENC_ATTR_H265_CBR_S    stH265Cbr;
	VENC_ATTR_H265_VBR_S    stH265Vbr;
	VENC_ATTR_H265_FIXQP_S  stH265FixQp;
	VENC_ATTR_MJPEG_S stMjpegAttr;
	VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
	VENC_ATTR_JPEG_S stJpegAttr;
	SIZE_S stPicSize;
	HI_S32 tmp;

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enSize, &stPicSize);
	 if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Get picture size failed!\n");
		return HI_FAILURE;
	}

	if(Vi_Input == 0 && videoParam.VideoType > 3)
		videoParam.VideoType = 3;


	 if(videoParam.VideoType==0)  //CIF
	{
		stPicSize.u32Width = 352;
		stPicSize.u32Height = 288;
		enSize = PIC_CIF;
	}
	 else if(videoParam.VideoType==1) //VGA
	{
		stPicSize.u32Width = 640;
		stPicSize.u32Height = 480;
		enSize = PIC_VGA;
	}
	else if(videoParam.VideoType==2) //D1
	{
		stPicSize.u32Width=720;
		stPicSize.u32Height=576;
		enSize = PIC_D1;
	}
	else if(videoParam.VideoType==3)//720P
	{
		stPicSize.u32Width=1280;
		stPicSize.u32Height=720; 
		enSize = PIC_HD720;
	}
	else if(videoParam.VideoType==4)//1080P
	{
		stPicSize.u32Width=1920;
		stPicSize.u32Height=1080;
		enSize = PIC_HD1080;
	}

	if(StRotate == 1||StRotate == 3)
	{
		tmp = stPicSize.u32Width;
		stPicSize.u32Width = stPicSize.u32Height;
		stPicSize.u32Height = tmp;
	}

	/******************************************
	 step 1:  Create Venc Channel
	******************************************/
	stVencChnAttr.stVeAttr.enType = enType;

	videoParam.ViRrame=60;
	switch(enType)
	{
		case PT_H264:
		{
			stH264Attr.u32MaxPicWidth = stPicSize.u32Width;
			stH264Attr.u32MaxPicHeight = stPicSize.u32Height;
			stH264Attr.u32PicWidth = stPicSize.u32Width;/*the picture width*/
			stH264Attr.u32PicHeight = stPicSize.u32Height;/*the picture height*/
			stH264Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size*/
			stH264Attr.u32Profile  = u32Profile;/*0: baseline; 1:MP; 2:HP;  3:svc_t */
			stH264Attr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
			stH264Attr.u32BFrameNum = 0;/* 0: not support B frame; >=1: number of B frames */
			stH264Attr.u32RefNum = 1;/* 0: default; number of refrence frame*/
			memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

			if(SAMPLE_RC_CBR == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
				stH264Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
				
				if(Vi_Mode == 0)
				{
					stH264Cbr.u32SrcFrmRate = 25 ;/* input (vi) frame rate */
				}
				else 
				{
					stH264Cbr.u32SrcFrmRate = videoParam.ViRrame ;/* input (vi) frame rate */
				}

				if(rl_frame > stH264Cbr.u32SrcFrmRate )   
					rl_frame = stH264Cbr.u32SrcFrmRate;
				stH264Cbr.fr32DstFrmRate = rl_frame;//(VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* target frame rate */
				stH264Cbr.u32BitRate = rl_rate;
				printf("stH264Cbr.u32SrcFrmRate=%d\nstH264Cbr.fr32DstFrmRate=%d\n",stH264Cbr.u32SrcFrmRate,stH264Cbr.fr32DstFrmRate);
				#if 0
				switch (enSize)
                {
				  case PIC_QCIF:
					   stH264Cbr.u32BitRate = 256; /* average bit rate */
					   break;
				  case PIC_QVGA:    /* 320 * 240 */
				  case PIC_CIF:

					   stH264Cbr.u32BitRate = rl_rate;
					   break;

				  case PIC_D1:
				  case PIC_VGA:	   /* 640 * 480 */
					   stH264Cbr.u32BitRate = rl_rate;
					   break;
				  case PIC_HD720:   /* 1280 * 720 */
					   stH264Cbr.u32BitRate = rl_rate;
					   break;
				  case PIC_HD1080:  /* 1920 * 1080 */
					   stH264Cbr.u32BitRate = rl_rate;
					   break;
				  case PIC_5M:  /* 2592 * 1944 */
					   stH264Cbr.u32BitRate = rl_rate;
					   break;
				  default :
					   stH264Cbr.u32BitRate = rl_rate;
					   break;
                }
                #endif
				stH264Cbr.u32FluctuateLevel = 0; /* average bit rate */
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
			}
			else if (SAMPLE_RC_FIXQP == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
				stH264FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264FixQp.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264FixQp.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264FixQp.u32IQp = 20;
				stH264FixQp.u32PQp = 23;
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
			}
			else if (SAMPLE_RC_VBR == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
				stH264Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264Vbr.u32StatTime = 1;
				
				if(Vi_Mode == 0)
				{
					stH264Vbr.u32SrcFrmRate = 25 ;/* input (vi) frame rate */
				}
				else 
				{
					stH264Vbr.u32SrcFrmRate = videoParam.ViRrame ;/* input (vi) frame rate */
				}
				
				if(rl_frame > stH264Vbr.u32SrcFrmRate)
					rl_frame=stH264Vbr.u32SrcFrmRate;
				stH264Vbr.fr32DstFrmRate = rl_frame;// (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264Vbr.u32MinQp = 10;
				stH264Vbr.u32MaxQp = 40;
				printf("stH264Vbr.u32SrcFrmRate=%d\nstH264Vbr.fr32DstFrmRate=%d\n",stH264Vbr.u32SrcFrmRate,stH264Vbr.fr32DstFrmRate );
				#if 0
				switch (enSize)
				{
				  case PIC_QCIF:
					   stH264Vbr.u32MaxBitRate= 256*3; /* average bit rate */
					   break;
				  case PIC_QVGA:    /* 320 * 240 */
				  case PIC_CIF:
					   stH264Vbr.u32MaxBitRate = 512*3;
					   break;
				  case PIC_D1:
				  case PIC_VGA:	   /* 640 * 480 */
					   stH264Vbr.u32MaxBitRate = 1024*2;
					   break;
				  case PIC_HD720:   /* 1280 * 720 */
					   stH264Vbr.u32MaxBitRate = 1024*3;
					   break;
				  case PIC_HD1080:  /* 1920 * 1080 */
					   stH264Vbr.u32MaxBitRate = 1024*6;
					   break;
				  case PIC_5M:  /* 2592 * 1944 */
					   stH264Vbr.u32MaxBitRate = 1024*8;
					   break;
				  default :
					   stH264Vbr.u32MaxBitRate = 1024*4;
					   break;
				}
				#endif
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
			}
			else
			{
				return HI_FAILURE;
			}
		}
		break;

		case PT_MJPEG:
		{
			stMjpegAttr.u32MaxPicWidth = stPicSize.u32Width;
			stMjpegAttr.u32MaxPicHeight = stPicSize.u32Height;
			stMjpegAttr.u32PicWidth = stPicSize.u32Width;
			stMjpegAttr.u32PicHeight = stPicSize.u32Height;
			stMjpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
			stMjpegAttr.bByFrame = HI_TRUE;  /*get stream mode is field mode  or frame mode*/
			memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));

			if(SAMPLE_RC_FIXQP == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
				stMjpegeFixQp.u32Qfactor        = 90;
				stMjpegeFixQp.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stMjpegeFixQp.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				memcpy(&stVencChnAttr.stRcAttr.stAttrMjpegeFixQp, &stMjpegeFixQp,
					   sizeof(VENC_ATTR_MJPEG_FIXQP_S));
			}
			else if (SAMPLE_RC_CBR == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
				stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32StatTime       = 1;
				stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stVencChnAttr.stRcAttr.stAttrMjpegeCbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32FluctuateLevel = 0;
				switch (enSize)
				{
				  case PIC_QCIF:
					   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 384*3; /* average bit rate */
					   break;
				  case PIC_QVGA:    /* 320 * 240 */
				  case PIC_CIF:
					   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 768*3;
					   break;
				  case PIC_D1:
				  case PIC_VGA:	   /* 640 * 480 */
					   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*3*3;
					   break;
				  case PIC_HD720:   /* 1280 * 720 */
					   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*5*3;
					   break;
				  case PIC_HD1080:  /* 1920 * 1080 */
					   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*10*3;
					   break;
				  case PIC_5M:  /* 2592 * 1944 */
					   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*10*3;
					   break;
				  default :
					   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*10*3;
					   break;
				}
			}
			else if (SAMPLE_RC_VBR == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32StatTime = 1;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL == enNorm)?25:30;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.fr32DstFrmRate = 5;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MinQfactor = 50;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxQfactor = 95;
				switch (enSize)
				{
				  case PIC_QCIF:
					   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate= 256*3; /* average bit rate */
					   break;
				  case PIC_QVGA:    /* 320 * 240 */
				  case PIC_CIF:
					   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 512*3;
					   break;
				  case PIC_D1:
				  case PIC_VGA:	   /* 640 * 480 */
					   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*2*3;
					   break;
				  case PIC_HD720:   /* 1280 * 720 */
					   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*3*3;
					   break;
				  case PIC_HD1080:  /* 1920 * 1080 */
					   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*6*3;
					   break;
				  case PIC_5M:  /* 2592 * 1944 */
					   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*12*3;
					   break;
				  default :
					   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*4*3;
					   break;
				}
			}
			else
			{
				SAMPLE_PRT("cann't support other mode in this version!\n");

				return HI_FAILURE;
			}
		}
		break;

		case PT_JPEG:
			stJpegAttr.u32PicWidth  = stPicSize.u32Width;
			stJpegAttr.u32PicHeight = stPicSize.u32Height;
			stJpegAttr.u32MaxPicWidth  = stPicSize.u32Width;
			stJpegAttr.u32MaxPicHeight = stPicSize.u32Height;
			stJpegAttr.u32BufSize   = stPicSize.u32Width * stPicSize.u32Height * 2;
			stJpegAttr.bByFrame     = HI_TRUE;/*get stream mode is field mode  or frame mode*/
			stJpegAttr.bSupportDCF  = HI_FALSE;
			memcpy(&stVencChnAttr.stVeAttr.stAttrJpeg, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));
			break;

		case PT_H265:
		{
			stH265Attr.u32MaxPicWidth = stPicSize.u32Width;
			stH265Attr.u32MaxPicHeight = stPicSize.u32Height;
			stH265Attr.u32PicWidth = stPicSize.u32Width;/*the picture width*/
			stH265Attr.u32PicHeight = stPicSize.u32Height;/*the picture height*/
			stH265Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size*/
			if(u32Profile >=1)
				stH265Attr.u32Profile = 0;/*0:MP; */
			else
				stH265Attr.u32Profile  = u32Profile;/*0:MP*/
			stH265Attr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
			stH265Attr.u32BFrameNum = 0;/* 0: not support B frame; >=1: number of B frames */
			stH265Attr.u32RefNum = 1;/* 0: default; number of refrence frame*/
			memcpy(&stVencChnAttr.stVeAttr.stAttrH265e, &stH265Attr, sizeof(VENC_ATTR_H265_S));

			if(SAMPLE_RC_CBR == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
				stH265Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH265Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
				
				if(Vi_Mode == 0)
				{
					stH265Cbr.u32SrcFrmRate = 25 ;/* input (vi) frame rate */
				}
				else 
				{
					stH265Cbr.u32SrcFrmRate = videoParam.ViRrame ;/* input (vi) frame rate */
				}
				
				if(rl_frame > stH265Cbr.u32SrcFrmRate )   
					rl_frame = stH265Cbr.u32SrcFrmRate;
				
				stH265Cbr.fr32DstFrmRate = rl_frame;//(VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* target frame rate */
				stH265Cbr.u32BitRate = rl_rate;
#if 0
				switch (enSize)
				{
				  case PIC_QCIF:
					   stH265Cbr.u32BitRate = 256; /* average bit rate */
					   break;
				  case PIC_QVGA:    /* 320 * 240 */
				  case PIC_CIF:

					   stH265Cbr.u32BitRate = 512;
					   break;

				  case PIC_D1:
				  case PIC_VGA:	   /* 640 * 480 */
					   stH265Cbr.u32BitRate = rl_rate;
					   break;
				  case PIC_HD720:   /* 1280 * 720 */
					   stH265Cbr.u32BitRate = rl_rate;
					   break;
				  case PIC_HD1080:  /* 1920 * 1080 */
					   stH265Cbr.u32BitRate = rl_rate;
					   break;
				  case PIC_5M:  /* 2592 * 1944 */
					   stH265Cbr.u32BitRate =rl_rate;
					   break;
				  default :
					   stH265Cbr.u32BitRate = rl_rate;
					   break;
				}
#endif
				stH265Cbr.u32FluctuateLevel = 0; /* average bit rate */
				memcpy(&stVencChnAttr.stRcAttr.stAttrH265Cbr, &stH265Cbr, sizeof(VENC_ATTR_H265_CBR_S));
			}
			else if (SAMPLE_RC_FIXQP == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
				stH265FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH265FixQp.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH265FixQp.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH265FixQp.u32IQp = 20;
				stH265FixQp.u32PQp = 23;
				memcpy(&stVencChnAttr.stRcAttr.stAttrH265FixQp, &stH265FixQp,sizeof(VENC_ATTR_H265_FIXQP_S));
			}
			else if (SAMPLE_RC_VBR == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
				stH265Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH265Vbr.u32StatTime = 1;
		
				if(Vi_Mode == 0)
				{
					stH265Vbr.u32SrcFrmRate = 25 ;/* input (vi) frame rate */
				}
				else 
				{
					stH265Vbr.u32SrcFrmRate = videoParam.ViRrame ;/* input (vi) frame rate */
				}
				
				if(rl_frame > stH265Vbr.u32SrcFrmRate)
					rl_frame=stH265Vbr.u32SrcFrmRate;
				stH265Vbr.fr32DstFrmRate = rl_frame;
				
				stH265Vbr.u32MinQp = 10;
				stH265Vbr.u32MaxQp = 40;
				#if 1
				switch (enSize)
				{
				  case PIC_QCIF:
					   stH265Vbr.u32MaxBitRate= 256*3; /* average bit rate */
					   break;
				  case PIC_QVGA:    /* 320 * 240 */
				  case PIC_CIF:
					   stH265Vbr.u32MaxBitRate = 512*3;
					   break;
				  case PIC_D1:
				  case PIC_VGA:	   /* 640 * 480 */
					   stH265Vbr.u32MaxBitRate = 1024*2;
					   break;
				  case PIC_HD720:   /* 1280 * 720 */
					   stH265Vbr.u32MaxBitRate = 1024*3;
					   break;
				  case PIC_HD1080:  /* 1920 * 1080 */
					   stH265Vbr.u32MaxBitRate = 1024*6;
					   break;
				  case PIC_5M:  /* 2592 * 1944 */
					   stH265Vbr.u32MaxBitRate = 1024*8;
					   break;
				  default :
					   stH265Vbr.u32MaxBitRate = 1024*4;
					   break;
				}
				#endif
				memcpy(&stVencChnAttr.stRcAttr.stAttrH265Vbr, &stH265Vbr, sizeof(VENC_ATTR_H265_VBR_S));
			}
			else
			{
				return HI_FAILURE;
			}
		}
		break;
		default:
			return HI_ERR_VENC_NOT_SUPPORT;
	}

	s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
				VencChn, s32Ret);
		return s32Ret;
	}

	/******************************************
	 step 2:  Start Recv Venc Pictures
	******************************************/
	s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
		return HI_FAILURE;
	}
	
	//VENC_FRAME_RATE_S venc_frame ; //1080p 30=0  60p=1 720p  30=2 60=3

#if 0
	VI_CSC_ATTR_S attrs ;
	attrs.enViCscType=VI_CSC_TYPE_709 ;

	attrs.u32LumaVal=g_ld;
	attrs.u32ContrVal=g_dbd;
	attrs.u32HueVal=g_sd;
	attrs.u32SatuVal=g_bhd;
	HI_MPI_VI_SetCSCAttr(0,&attrs) ;
#endif

	return HI_SUCCESS;

}
/******************************************************************************
* function : venc bind vpss           
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_BindVpss(VENC_CHN VeChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

/******************************************************************************
* funciton : get file postfix according palyload_type.
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload, char *szFilePostfix)
{
    if (PT_H264 == enPayload)
    {
        strcpy(szFilePostfix, ".h264");
    }
    else if (PT_H265 == enPayload)
    {
        strcpy(szFilePostfix, ".h265");
    }
    else if (PT_JPEG == enPayload)
    {
        strcpy(szFilePostfix, ".jpg");
    }
    else if (PT_MJPEG == enPayload)
    {
        strcpy(szFilePostfix, ".mjp");
    }
    else if (PT_MP4VIDEO == enPayload)
    {
        strcpy(szFilePostfix, ".mp4");
    }
    else
    {
        SAMPLE_PRT("payload type err!\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : save H264 stream
******************************************************************************/
typedef unsigned char byte;
const byte cmd = 21;
const byte h264_video = 22;
const byte pcm_audio = 23;
const byte h265_video = 24;
const byte aac_audio = 25;
struct header{
	size_t size;
	byte type; // cmd, video, audio
	void * body()
	{
		return this + 1;
	}
} __attribute((packed));

struct video_header{
	unsigned long long ts;
	byte key;              //
	void * body()
	{
		return this + 1;
	}
} __attribute((packed));

struct audio_header{
	unsigned long long ts;
	byte flag;
	void * body()
	{
		return this + 1;
	}
} __attribute((packed));



HI_S32 SAMPLE_COMM_VENC_SaveH264(FILE* fpH264File, VENC_STREAM_S *pstStream)
{
#ifdef STORE_STREAM

    HI_S32 i,key_f=0;
    static int nIndex = 0;
    if(pstStream->u32PackCount > 1) key_f = 1;
	
    for (i = 0; i < pstStream->u32PackCount; i++)
    {
		if(SaveMode==1)
		{
			VENC_PACK_S& p = pstStream->pstPack[i];
			if(H264E_NALU_SPS == p.DataType.enH264EType)
			{
				if(g_bInit == false && g_pXHLiveManager->m_Param.videoParam.spsData == NULL)
				{
					printf("--------------H264E_NALU_SPS----------- %d, %x\n",pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset, (pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset)[0]);
					//gStartLiveSrcgStartLiveSrc.setSPSData(pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset, pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
					g_pXHLiveManager->m_Param.videoParam.setSPSData(pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset, pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
					printf("--------------H264E_NALU_SPS----------- end \n");
				}
				
			}
			else if(H264E_NALU_PPS == p.DataType.enH264EType)
			{
				if(g_bInit == false)
				{
					printf("--------------H264E_NALU_PPS----------- %d, %x\n",pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset, (pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset)[0]);
					//gStartLiveSrc.setPPSData(pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset, pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
					g_pXHLiveManager->m_Param.videoParam.setPPSData(pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset, pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
				}
				
			}
			else if(g_bInit)
			{
				g_pXHLiveManager->insertVideoRaw((unsigned char*)pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset, true);
				//gStartLiveSrc.insertVideo((unsigned char*)pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset, nIndex++);
			}
		//	interface_insertVideoNalu((byte_t*)pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset,STAR_VIDEO_ENC_CONFIG_SF_720W_1280H_FPS15_RATE1200_GOP2, nIndex++);
			//interface_debug();
			//printf("interface_insertVideoRaw ret=%d",ret);
			//static byte_t sperator[] = {1,2,3,4,5,6,7,8};
			//fwrite(sperator, 8, 1, fpH264File);

			fwrite(pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
			pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset, 1, fpH264File);
			
			fflush(fpH264File);
		}
	
  
		if (g_videoPktCount <= 0)
		{
			g_startVideoPts = pstStream->pstPack[i].u64PTS;
			g_lastVideoPts = pstStream->pstPack[i].u64PTS;
		}
		
		int64_t ptsDiff = pstStream->pstPack[i].u64PTS - g_lastVideoPts;
		if ((ptsDiff <= 0) || (ptsDiff > 1000000))
		{
			//ptsDiff = 0;
		}
		g_lastVideoPts = pstStream->pstPack[i].u64PTS;
		g_videoPktCount ++;
		
		MPacket pkt;
		memset(&pkt, 0, sizeof(pkt));
		pkt.type = MTYPE_VIDEO;
		pkt.data = pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
		pkt.size = pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
		pkt.duration = ptsDiff;
		pkt.pts = pstStream->pstPack[i].u64PTS;
		pkt.flags = key_f;
		
		/*	
		printf("#video pkt(%4d, %4d, %lld)\n",
					pkt.size, pkt.duration, pkt.pts
					);
		*/
	
		
		caster_chl_write_video(g_handle[0], &pkt);
		
    }
#else
	
	header h;
	h.type = h264_video;
	video_header vh;

	axis::shared_buffer sb;
	sb.resize(sizeof(h) + sizeof(vh));

	for (HI_S32 i = 0; i < pstStream->u32PackCount; i++)
	{
		byte * dat = pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
		size_t s=pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;

		vh.ts = pstStream->pstPack[i].u64PTS;

		int pck_type = pstStream->pstPack[i].DataType.enH264EType;
		
		if(i == 0 && pck_type == H264E_NALU_SPS)vh.key = 1;

		size_t old_size = sb.size();
		sb.resize(old_size + s);
		memcpy(sb.raw_data() + old_size, dat, s);
	
	}
    
	h.size = sb.size();
	memcpy(sb.raw_data(), &h, sizeof(h));
	memcpy(sb.raw_data() + sizeof(h), &vh, sizeof(vh));

	for(int i=0;i<udpuser.size();i++)
	{

		if(h.size>65000)
		{
			int mod= h.size/65000 ;
			int modyu=h.size%65000 ;
			for(int j=0;j<=mod;j++)
			{
				printf("mod=%d modyu=%d size=%d\n",mod,modyu,h.size);

				if(j==mod)   //
				{
					udpuser[i].sentclient((char *)sb.raw_data()+j*65000, modyu );
				
					printf("send1 m=%d\n",j);
				}
				else
				{
					udpuser[i].sentclient((char *)sb.raw_data()+j*65000, 65000 );
				
					printf("send2 m=%d\n",j);
				}
			  }

		}
		else  
		{

			udpuser[i].sentclient((char *)sb.raw_data(), h.size );
		}

	}
#endif
    return HI_SUCCESS;
}
#define MAXPACKET  
/******************************************************************************
* funciton : save H265 stream
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveH265(FILE* fpH265File, VENC_STREAM_S *pstStream)
{
    HI_S32 i,key_f=0;
#ifdef STORE_STREAM


	if(pstStream->u32PackCount > 1) key_f = 1;
    for (i = 0; i < pstStream->u32PackCount; i++)
    {
    		if(SaveMode==1)
    		{
			fwrite(pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
			pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset, 1, fpH265File);

			fflush(fpH265File);
    		}

		if (g_videoPktCount <= 0)
		{
			g_startVideoPts = pstStream->pstPack[i].u64PTS;
			g_lastVideoPts = pstStream->pstPack[i].u64PTS;
		}
		
		int64_t ptsDiff = pstStream->pstPack[i].u64PTS - g_lastVideoPts;
		if ((ptsDiff <= 0) || (ptsDiff > 1000000))
		{
			ptsDiff = 0;
		}
		g_lastVideoPts = pstStream->pstPack[i].u64PTS;
		g_videoPktCount ++;
		
		MPacket pkt;
		memset(&pkt, 0, sizeof(pkt));
		pkt.type = MTYPE_VIDEO;
		pkt.data = pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
		pkt.size = pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
		pkt.duration = ptsDiff;
		pkt.pts = pstStream->pstPack[i].u64PTS;
		pkt.flags = key_f;
		
		/*	
		printf("---###H.265 pkt(%4d, %4d, %lld) pkt count:%d total duration:%d, clock:%d. raw pts:%lld\n",
					pkt.size, pkt.duration, pkt.pts,
					g_videoPktCount, g_videoDuration, clockDuration, pstStream->pstPack[i].u64PTS
					);
		*/
		
		caster_chl_write_video(g_handle[0], &pkt);
		
    }
	
#else
	header h;
	h.type = h265_video;
	video_header vh;
	memset(&vh,0,sizeof(vh));

	axis::shared_buffer sb;
	sb.resize(sizeof(h) + sizeof(vh));

	for (HI_S32 i = 0; i < pstStream->u32PackCount; i++)
	{
		byte * dat = pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
		size_t s=pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
		vh.ts = pstStream->pstPack[i].u64PTS;

		//	int pck_type = pstStream->pstPack[i].DataType.enH265EType;
	
		VENC_PACK_S& p = pstStream->pstPack[i];
		
		if(H265E_NALU_SPS == p.DataType.enH265EType)
		{
			vh.key = 1;
			// printf("find key\n");
			//	printf("  pstStream->u32PackCount=%d\n", pstStream->u32PackCount);
		}
	
		size_t old_size = sb.size();
		sb.resize(old_size + s);
		memcpy(sb.raw_data() + old_size, dat, s);
	
	}
    
	h.size = sb.size();
	memcpy(sb.raw_data(), &h, sizeof(h));
	memcpy(sb.raw_data() + sizeof(h), &vh, sizeof(vh));

	//push_video(s32Chn, dat, tsize, ts);

	if(vh.key ==1)
	{
	//	 printf(" h.size=%d  ", h.size);
	//	 printf("data size=%d\n",(h.size-sizeof(h)-sizeof(vh)));
	}

	// printf("video size=%d\n", h.size);
	for(int i=0;i<udpuser.size();i++)
	{

		if(h.size>65000)
		{
			int mod= h.size/65000 ;
			int modyu=h.size%65000 ;
			for(int j=0;j<=mod;j++)
			{
				printf("mod=%d modyu=%d size=%d\n",mod,modyu,h.size);

				if(j==mod)   //
				{
					udpuser[i].sentclient((char *)sb.raw_data()+j*65000, modyu );
				
					printf("send1 m=%d\n",j);
				}
				else
				{
					udpuser[i].sentclient((char *)sb.raw_data()+j*65000, 65000 );
				
					printf("send2 m=%d\n",j);
				}
			  }

		}
		else  
		{

			udpuser[i].sentclient((char *)sb.raw_data(), h.size );
		}

	}
#endif
	return HI_SUCCESS;

}
/******************************************************************************
* funciton : save stream
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveStream(PAYLOAD_TYPE_E enType,FILE *pFd, VENC_STREAM_S *pstStream)
{
    HI_S32 s32Ret;

    if (PT_H264 == enType)
    {
		//printf("h264 \n");
        s32Ret = SAMPLE_COMM_VENC_SaveH264(pFd, pstStream);
		//printf("h264 end \n");
    }
    else if (PT_MJPEG == enType)
    {
		//printf("PT_MJPEG \n");
     //   s32Ret = SAMPLE_COMM_VENC_SaveMJpeg(pFd, pstStream);
    }
    else if (PT_H265 == enType)
    {
		//printf("h265 \n");
      //  s32Ret = SAMPLE_COMM_VENC_SaveH265(pFd, pstStream);
    }
    else
    {
		//printf("HI_FAILURE");
        return HI_FAILURE;
    }


	//printf("SAMPLE_COMM_VENC_SaveStream \n");
    return s32Ret;
}

/******************************************************************************
* funciton : get stream from each channels and save them
******************************************************************************/
HI_VOID* SAMPLE_COMM_VENC_GetVencStreamProc(HI_VOID *p)
{
	HI_S32 i;
	HI_S32 s32ChnTotal;
	VENC_CHN_ATTR_S stVencChnAttr;
	SAMPLE_VENC_GETSTREAM_PARA_S *pstPara;
	HI_S32 maxfd = 0;
	struct timeval TimeoutVal;
	fd_set read_fds;
	HI_S32 VencFd[VENC_MAX_CHN_NUM];
	HI_CHAR aszFileName[VENC_MAX_CHN_NUM][64];
	FILE *pFile[VENC_MAX_CHN_NUM];
	char szFilePostfix[10];
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	HI_S32 s32Ret;
	VENC_CHN VencChn;
	PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];

	pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S*)p;
	s32ChnTotal = pstPara->s32Cnt;
	printf("s32ChnTotal:%d\n", s32ChnTotal);
	/******************************************
	 step 1:  check & prepare save-file & venc-fd
	******************************************/
	if (s32ChnTotal >= VENC_MAX_CHN_NUM)
	{
		SAMPLE_PRT("input count invaild\n");
		return NULL;
	}
	for (i = 0; i < s32ChnTotal; i++)
	{
		/* decide the stream file name, and open file to save stream */
		VencChn = i;
		s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
		if(s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
				   VencChn, s32Ret);
			return NULL;
		}
		enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

		s32Ret = SAMPLE_COMM_VENC_GetFilePostfix(enPayLoadType[i], szFilePostfix);
		if(s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("SAMPLE_COMM_VENC_GetFilePostfix [%d] failed with %#x!\n", \
				   stVencChnAttr.stVeAttr.enType, s32Ret);
			return NULL;
		}
		sprintf(aszFileName[i], "stream_chn%d%s", i, szFilePostfix);
	#ifdef STORE_STREAM
		pFile[i] = fopen(aszFileName[i], "wb");
		if (!pFile[i])
		{
			SAMPLE_PRT("open file[%s] failed!\n",
				   aszFileName[i]);
			return NULL;
		}
	#endif
		/* Set Venc Fd. */
		VencFd[i] = HI_MPI_VENC_GetFd(i);
		if (VencFd[i] < 0)
		{
			SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
				   VencFd[i]);
			return NULL;
		}
		if (maxfd <= VencFd[i])
		{
			maxfd = VencFd[i];
		}
	}

	/******************************************
	 step 2:  Start to get streams of each channel.
	******************************************/
	while (HI_TRUE == pstPara->bThreadStart)
	{
		FD_ZERO(&read_fds);
		for (i = 0; i < s32ChnTotal; i++)
		{
			FD_SET(VencFd[i], &read_fds);
		}

		TimeoutVal.tv_sec  = 2;
		TimeoutVal.tv_usec = 0;
		s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0)
		{
			SAMPLE_PRT("select failed!\n");
			break;
		}
		else if (s32Ret == 0)
		{
			SAMPLE_PRT("get venc stream time out, exit thread\n");
			continue;
		}
		else
		{
			for (i = 0; i < s32ChnTotal; i++)
			{
				if (FD_ISSET(VencFd[i], &read_fds))
				{
					/*******************************************************
					 step 2.1 : query how many packs in one-frame stream.
					*******************************************************/
					memset(&stStream, 0, sizeof(stStream));
					s32Ret = HI_MPI_VENC_Query(i, &stStat);
					if (HI_SUCCESS != s32Ret)
					{
						SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
						break;
					}

					/*******************************************************
					 step 2.2 : malloc corresponding number of pack nodes.
					*******************************************************/
					stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
					if (NULL == stStream.pstPack)
					{
						SAMPLE_PRT("malloc stream pack failed!\n");
						break;
					}

					/*******************************************************
					 step 2.3 : call mpi to get one-frame stream
					*******************************************************/
					stStream.u32PackCount = stStat.u32CurPacks;
					s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
					if (HI_SUCCESS != s32Ret)
					{
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
							   s32Ret);
						break;
					}
					/*******************************************************
					 step 2.4 : save frame to file
					************************************* ******************/
					//if(i==0)
					{
						//printf("get data\n");
						s32Ret = SAMPLE_COMM_VENC_SaveStream(enPayLoadType[i], pFile[i], &stStream);
						if (HI_SUCCESS != s32Ret)
						{
							free(stStream.pstPack);
							stStream.pstPack = NULL;
							SAMPLE_PRT("save stream failed!\n");
							break;
						}
						//printf("get data end\n");

					}
      

					/*******************************************************
					 step 2.5 : release stream
					*******************************************************/
					s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
					if (HI_SUCCESS != s32Ret)
					{
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						break;
					}
					/*******************************************************
					 step 2.6 : free pack nodes
					*******************************************************/
					free(stStream.pstPack);
					stStream.pstPack = NULL;
				}
			}
		}
	}

	/*******************************************************
	* step 3 : close save-file
	*******************************************************/
#ifdef STORE_STREAM
	for (i = 0; i < s32ChnTotal; i++)
	{
		fclose(pFile[i]);
	}
#endif
	return NULL;
}


/******************************************************************************
* funciton : start get venc stream process thread
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_StartGetStream(HI_S32 s32Cnt)
{
    gs_stPara.bThreadStart = HI_TRUE;
    gs_stPara.s32Cnt = s32Cnt;
//    char* strUrl = "https://api.starrtc.com";
//    char* strPath = "/demo/meeting/arm_info";
//     char* ret = requestMLOCServer1(strUrl, 8080, strPath, "");
	
//	printf("%s", ret);
    char* m_strChannelId = "kvJJAtjDpU5saaGa";
    char* m_strUserId = "arm_1";
    char* m_strAgentId = "BjR6QV3vUJ4d";
    char* m_strTokenId = "SM9B8Y4IM2VG55CCIVB9Q16618BD24AA";
    char* m_strServerIp = "src.starrtc.com";
    int m_ServerPort = 9931;

    //gStartLiveSrc.logIn();
    //startLiveSrc.getServerInfo();
   // gStartLiveSrc.applyForUploading();
//    interface_setLogLevel(1,1);

 //   callbackInit_liveVideoSrc(NULL,NULL, (fun_live_src_apply_upload_channel_ok)fun_live_src_apply_upload_channel_ok1, (fun_live_src_apply_upload_channel_failed)fun_live_src_apply_upload_channel_failed1, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
 // interface_starLiveStartUploadSrcServer(m_strServerIp, m_ServerPort, m_strAgentId, m_strUserId, m_strTokenId, m_strChannelId);

   // interface_startLiveSrcEncoder(STAR_VIDEO_AND_AUDIO_CODEC_CONFIG_H264_AAC, 
//									STAR_VIDEO_ENC_CONFIG_HW_720W_1280H_FPS15_RATE1200_GOP2, 
//									STAR_VIDEO_ENC_CONFIG_NOT_USE, 
//									16000, 
//									1, 
//									32, 
//									0, 
//									STAR_RTC_MEDIA_TYPE_VIDEO_ONLY);

    return pthread_create(&gs_VencPid, 0, SAMPLE_COMM_VENC_GetVencStreamProc, (HI_VOID*)&gs_stPara);
}


int i_dhcp ;
int i_ip ;
int i_mask ;
int i_gateway;
int i_dns1 ;

string g_ip = "192.168.1.22";
string g_netmask = "255.255.255.0";
string g_gateway = "0.0.0.0";
string g_mac = "00:10:79:11:11:10";



#define ETH_NAME "eth0"


void change_mac(){
	char buffer[1024];
	sprintf(buffer, "ifconfig %s down", ETH_NAME);
	system(buffer);

	sprintf(buffer, "ifconfig %s hw ether %s", ETH_NAME, g_mac.c_str());
	printf("%s\n", buffer);
	system(buffer);

	sprintf(buffer, "ifconfig %s up", ETH_NAME);
	system(buffer);
	sleep(2);
} 
//char *ip,char *netmask ,char *gw,char *mac
void change_ip(){
	change_mac();
	printf("kjdfasdlkjasflkj\n\n");
	char buffer[1024];
	sprintf(buffer, "ifconfig %s %s netmask %s", ETH_NAME, g_ip.c_str(), g_netmask.c_str());
	//sprintf(buffer, "ifconfig %s %s netmask %s", ETH_NAME, ip, netmask);
	printf("%s\n", buffer);
	system(buffer);


		sprintf(buffer, "route del default gw %s dev %s", "0.0.0.0", ETH_NAME);
		printf("%s\n", buffer);
		system(buffer);
	
		if(g_gateway != "0.0.0.0"){
			sprintf(buffer, "route add default gw %s dev %s", g_gateway.c_str(), ETH_NAME);
			printf("%s\n", buffer);
			system(buffer);
		}
		else{
			sprintf(buffer, "route add -net 255.255.255.255 netmask 255.255.255.255 dev %s metric 1", ETH_NAME);
			printf("%s\n", buffer);
			system(buffer);
		}
	
}

///////////////////////////////////////////////////////////////////////////////

 /*
 	 ��Ƶ����߻�ȡ ����
 */

///////////////////////////////////////////////////////////////////////////////
#define ACODEC_FILE     "/dev/acodec"  
//#define HI_ACODEC_TYPE_INNER
#define AUDIO_ADPCM_TYPE ADPCM_TYPE_DVI4/* ADPCM_TYPE_IMA, ADPCM_TYPE_DVI4*/
#define G726_BPS MEDIA_G726_40K         /* MEDIA_G726_16K, MEDIA_G726_24K ... */
#define AI_DEV_MAX_NUM          1
#define AO_DEV_MIN_NUM          0
#define AO_DEV_MAX_NUM          1
#define AIO_MAX_NUM             1
#define AIO_MAX_CHN_NUM         16
#define AENC_MAX_CHN_NUM        32
#define ADEC_MAX_CHN_NUM        32

typedef struct tagSAMPLE_AENC_S
{
    HI_BOOL bStart;
    pthread_t stAencPid;
    pthread_t stAACencPid;
    HI_S32  AeChn;
    HI_S32  AdChn;
    FILE    *pfd;
    HI_BOOL bSendAdChn;
} SAMPLE_AENC_S;

typedef struct tagSAMPLE_AI_S
{
    HI_BOOL bStart;
    HI_S32  AiDev;
    HI_S32  AiChn;
    HI_S32  AencChn;
    HI_S32  AoDev;
    HI_S32  AoChn;
    HI_BOOL bSendAenc;
    HI_BOOL bSendAo;
	FILE    *pfd;
    pthread_t stAiPid;
} SAMPLE_AI_S;

typedef struct tagSAMPLE_ADEC_S
{
    HI_BOOL bStart;
    HI_S32 AdChn; 
    FILE *pfd;
    pthread_t stAdPid;
} SAMPLE_ADEC_S;

typedef struct tagSAMPLE_AO_S
{
	AUDIO_DEV AoDev;
	HI_BOOL bStart;
	pthread_t stAoPid;
}SAMPLE_AO_S;




static SAMPLE_AI_S   gs_stSampleAi[AI_DEV_MAX_NUM*AIO_MAX_CHN_NUM];
static SAMPLE_AENC_S gs_stSampleAenc[AENC_MAX_CHN_NUM];
static SAMPLE_ADEC_S gs_stSampleAdec[ADEC_MAX_CHN_NUM];
static SAMPLE_AO_S   gs_stSampleAo[AO_DEV_MAX_NUM];
static CHECK_VI_MODE gs_stCheck;
/******************************************************************************
* function : PT Number to String
******************************************************************************/
static char* SAMPLE_AUDIO_Pt2Str(PAYLOAD_TYPE_E enType)
{
    if (PT_G711A == enType)  
    {
        return "g711a";
    }
    else if (PT_G711U == enType)  
    {
        return "g711u";
    }
    else if (PT_ADPCMA == enType)  
    {
        return "adpcm";
    }
    else if (PT_G726 == enType) 
    {
        return "g726";
    }
    else if (PT_LPCM == enType)  
    {
        return "pcm";
    }
    else 
    {
        return "data";
    }
}

FILE * f_out;// = fopen("out.aac", "wb");
typedef unsigned long   ULONG;
typedef unsigned int    UINT;
typedef unsigned char   BYTE;
typedef char            _TCHAR;

int iPcmBytes=0;
int nBytesRead;
BYTE* pbPCMBuffer;
BYTE* pbAACBuffer;
pthread_mutex_t hMutex = PTHREAD_MUTEX_INITIALIZER;

#define PCMBUF_SIZE 20480



// G711   ����
void *SAMPLE_COMM_AUDIO_AencProc(void *parg)
{
	HI_S32 s32Ret;
	HI_S32 AencFd;
	SAMPLE_AENC_S *pstAencCtl = (SAMPLE_AENC_S *)parg;
	AUDIO_STREAM_S stStream;
	fd_set read_fds;
	struct timeval TimeoutVal;

	FD_ZERO(&read_fds);
	AencFd = HI_MPI_AENC_GetFd(pstAencCtl->AeChn);
	FD_SET(AencFd, &read_fds);



	while (pstAencCtl->bStart)
	{
		TimeoutVal.tv_sec = 1;
		TimeoutVal.tv_usec = 0;
		int nPCMBufferSize=2048;
		
		FD_ZERO(&read_fds);
		FD_SET(AencFd,&read_fds);

		//printf("chn:%d",pstAencCtl->AeChn);

		//printf("ready to read audio files\n" );
		s32Ret = select(AencFd+1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0)
		{
			break;
		}
		else if (0 == s32Ret)
		{
			printf("%s: get aenc stream select time out\n", __FUNCTION__);
			break;
			}

			if (FD_ISSET(AencFd, &read_fds))
			{
            /* get stream from aenc chn */
            			s32Ret = HI_MPI_AENC_GetStream(pstAencCtl->AeChn, &stStream, HI_FALSE);
				if (HI_SUCCESS != s32Ret )
				{
					printf("HI_MPI_AENC_GetStream failed !\n");
					continue;
				}
				/* save audio stream to file */
				//fwrite(stStream.pStream,1,stStream.u32Len, pstAencCtl->pfd);

					
				

				//printf("pcm saving: %d \n",iPcmBytes);
				//printf("save file size: %d",stStream.u32Len);
				//printf("g.711 audio. size:%d, duration:%d, ts:%ld\n", pkt.size, pkt.duration, stStream.u64TimeStamp);
				/* finally you must release the stream */
				s32Ret = HI_MPI_AENC_ReleaseStream(pstAencCtl->AeChn, &stStream);
				if (HI_SUCCESS != s32Ret )
				{
					printf("%s: HI_MPI_AENC_ReleaseStream(%d), failed with %#x!\n",\
						   __FUNCTION__, pstAencCtl->AeChn, s32Ret);
					pstAencCtl->bStart = HI_FALSE;
					return NULL;
				}

				


        }    
    }
    
    fclose(pstAencCtl->pfd);
    pstAencCtl->bStart = HI_FALSE;
    return NULL;
}



#if 0
void *SAMPLE_COMM_AUDIO_AencProc(void *parg)
{
	HI_S32 s32Ret;
	HI_S32 AencFd;
	SAMPLE_AENC_S *pstAencCtl = (SAMPLE_AENC_S *)parg;
	AUDIO_STREAM_S stStream;
	fd_set read_fds;
	struct timeval TimeoutVal;

	FD_ZERO(&read_fds);
	AencFd = HI_MPI_AENC_GetFd(pstAencCtl->AeChn);
	FD_SET(AencFd, &read_fds);
#if 1
	CHP_MEM_FUNC_T mem_func;
	mem_func.chp_malloc = malloc;
	mem_func.chp_free = free;
	mem_func.chp_memset = (CHP_MEMSET)memset;
	mem_func.chp_memcpy = memcpy;

	CHP_AUD_ENC_INFO_T enc_info;
	enc_info.audio_type = CHP_DRI_CODEC_AAC_ADTS;
	enc_info.bit_rate = 128000;
	enc_info.sample_rate = 44100;
	enc_info.sample_size = 16;
	enc_info.channel_mode = 0;

	CHP_U32 aac_handle;

	CHP_RTN_T ret = aac_encoder_init(&mem_func, &enc_info, &aac_handle);
#endif

	char in_buf[3000] = {0};
	int bufStoreSize=0;
	char out_buf[3000];


    pbPCMBuffer = new BYTE [PCMBUF_SIZE];
    pbAACBuffer = new BYTE [44100];

	while (pstAencCtl->bStart)
	{
		TimeoutVal.tv_sec = 1;
		TimeoutVal.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(AencFd,&read_fds);

		//printf("chn:%d",pstAencCtl->AeChn);

		//printf("ready to read audio files\n" );
		s32Ret = select(AencFd+1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0)
		{
			break;
		}
		else if (0 == s32Ret)
		{
			printf("%s: get aenc stream select time out\n", __FUNCTION__);
			break;
			}

			if (FD_ISSET(AencFd, &read_fds))
			{
            /* get stream from aenc chn */
            			s32Ret = HI_MPI_AENC_GetStream(pstAencCtl->AeChn, &stStream, HI_FALSE);
				if (HI_SUCCESS != s32Ret )
				{
					printf("HI_MPI_AENC_GetStream failed !\n");
					continue;
				}
				/* save audio stream to file */
				//fwrite(stStream.pStream,1,stStream.u32Len, pstAencCtl->pfd);

				nBytesRead = stStream.u32Len;//fread(pbPCMBuffer, 1, nPCMBufferSize, fpIn);

				pthread_mutex_lock(&hMutex);
				memcpy((BYTE*)&pbPCMBuffer[iPcmBytes],(BYTE *)stStream.pStream,stStream.u32Len);
				iPcmBytes +=stStream.u32Len;
				pthread_mutex_unlock(&hMutex);

				//printf("pcm saving: %d \n",iPcmBytes);
				//	printf("save file size: %d",stStream.u32Len);
				/* finally you must release the stream */
				s32Ret = HI_MPI_AENC_ReleaseStream(pstAencCtl->AeChn, &stStream);
				if (HI_SUCCESS != s32Ret )
				{
					printf("%s: HI_MPI_AENC_ReleaseStream(%d), failed with %#x!\n",\
						   __FUNCTION__, pstAencCtl->AeChn, s32Ret);
					pstAencCtl->bStart = HI_FALSE;
					return NULL;
				}
         //  if(pstAencCtl->AeChn==1)
           {
#if 1         // 发送需按块来发送
        	   int nPCMBufferSize=2048;
            if(iPcmBytes>=nPCMBufferSize)
            	{

					CHP_AUD_ENC_DATA_T enc_data;
					enc_data.p_in_buf = pbPCMBuffer;              //RAW PCM数据存储的地址
					enc_data.p_out_buf = out_buf;			   //编码后的音频数据存放地址
					enc_data.in_buf_len = nPCMBufferSize;//sizeof(in_buf);			   //RAW PCM缓存的大小
					enc_data.out_buf_len = sizeof(out_buf);		   //压缩缓存区的大小
					enc_data.frame_cnt = 1;			   //需要编码的帧数

					enc_data.used_size = 0;			   //实际使用的RAW PCM存储缓冲区大小
					enc_data.enc_frame_cnt = 0;		   //实际编码的帧数
					enc_data.enc_data_len = 0;		   //压缩后音频数据的长度

					ret = aac_encode(aac_handle, &enc_data);
					if(ret != 0)
						printf("aac_encode:%d\n", ret);
					//printf("stream out size:%d\n",enc_data.enc_data_len);

					fwrite(out_buf, 1, enc_data.enc_data_len, f_out);
					
					MPacket pkt;
					memset(&pkt, 0, sizeof(pkt));
					pkt.type = MTYPE_AUDIO;
					pkt.data = (unsigned char*)out_buf;
					pkt.size = enc_data.enc_data_len;
					pkt.duration = nPCMBufferSize / 2; /// 采样个数
					// do not use !!	
					//rtmpcaster_write(g_handle[0], &pkt);
					//rtmpcaster_write(g_handle[1], &pkt);
					
					printf("audio. iPcmBytes:%d, size:%d, duration:%d, ts:%ld\n", iPcmBytes, pkt.size, pkt.duration, stStream.u64TimeStamp);

					send_audio((char *)out_buf, enc_data.enc_data_len, stStream.u64TimeStamp);

					memcpy(pbPCMBuffer,&pbPCMBuffer[nPCMBufferSize],PCMBUF_SIZE-nPCMBufferSize);//后半部分拷贝到前半部分
					iPcmBytes -= nPCMBufferSize;//未处理数据指针复位

            	}
#endif
           }


        }    
    }
    
    fclose(pstAencCtl->pfd);
    pstAencCtl->bStart = HI_FALSE;
    return NULL;
}


#endif
/******************************************************************************
* function : Ao bind Adec
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_AoBindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn)
{
    MPP_CHN_S stSrcChn,stDestChn;

    stSrcChn.enModId = HI_ID_ADEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = AdChn;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;
    
    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn); 
}

pthread_t stAACencPid;
/******************************************************************************
* function : Create the thread to get stream from aenc and send to adec
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AENC_CHN AeChn, ADEC_CHN AdChn, FILE *pAecFd)
{
    SAMPLE_AENC_S *pstAenc = NULL;

    if (NULL == pAecFd)
    {
        return HI_FAILURE;
    }
    
    pstAenc = &gs_stSampleAenc[AeChn];
    pstAenc->AeChn = AeChn;
    pstAenc->AdChn = AdChn;
    pstAenc->bSendAdChn = HI_TRUE;
    pstAenc->pfd = pAecFd;    
    pstAenc->bStart = HI_TRUE;    


    //pthread_create(&stAACencPid, 0, AUDIO_AACencProc, 0);
    pthread_create(&pstAenc->stAencPid, 0, SAMPLE_COMM_AUDIO_AencProc, pstAenc);
    

    return HI_SUCCESS;
}
/******************************************************************************
* function : Open Aenc File
******************************************************************************/
static FILE * SAMPLE_AUDIO_OpenAencFile(AENC_CHN AeChn, PAYLOAD_TYPE_E enType)
{
    FILE *pfd;
    HI_CHAR aszFileName[128];
    
    /* create file for save stream*/
    sprintf(aszFileName, "audio_chn%d.%s", AeChn, SAMPLE_AUDIO_Pt2Str(enType));
    pfd = fopen(aszFileName, "w+");
    if (NULL == pfd)
    {
        printf("%s: open file %s failed\n", __FUNCTION__, aszFileName);
        return NULL;
    }
    printf("open stream file:\"%s\" for aenc ok\n", aszFileName);
    return pfd;
}

/******************************************************************************
* function : Ao bind Ai
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_AoBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn)
{
    MPP_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32ChnId = AiChn;
    stSrcChn.s32DevId = AiDev;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;

    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}


/******************************************************************************
* function : Start Ao
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StartAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt,
        AIO_ATTR_S *pstAioAttr, AUDIO_SAMPLE_RATE_E enInSampleRate, HI_BOOL bResampleEn)
{
	HI_S32 i;
    HI_S32 s32Ret;

    s32Ret = HI_MPI_AO_SetPubAttr(AoDevId, pstAioAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("%s: HI_MPI_AO_SetPubAttr(%d) failed with %#x!\n", __FUNCTION__, \
               AoDevId,s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_AO_Enable(AoDevId);
    if(HI_SUCCESS != s32Ret)
    {
        printf("%s: HI_MPI_AO_Enable(%d) failed with %#x!\n", __FUNCTION__, AoDevId, s32Ret);
        return HI_FAILURE;
    }

	for (i=0; i<s32AoChnCnt; i++)
    {
	    s32Ret = HI_MPI_AO_EnableChn(AoDevId, i);
	    if(HI_SUCCESS != s32Ret)
	    {
	        printf("%s: HI_MPI_AO_EnableChn(%d) failed with %#x!\n", __FUNCTION__, i, s32Ret);
	        return HI_FAILURE;
	    }
	    
	    if (HI_TRUE == bResampleEn)
	    {
	        s32Ret = HI_MPI_AO_DisableReSmp(AoDevId, i);
	        s32Ret |= HI_MPI_AO_EnableReSmp(AoDevId, i, enInSampleRate);
	        if(HI_SUCCESS != s32Ret)
	        {
	            printf("%s: HI_MPI_AO_EnableReSmp(%d,%d) failed with %#x!\n", __FUNCTION__, AoDevId, i, s32Ret);
	            return HI_FAILURE;
	        }
	    }
	}
	
    return HI_SUCCESS;
}


/******************************************************************************
* function : Start Adec
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StartAdec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType)
{
    HI_S32 s32Ret;
    ADEC_CHN_ATTR_S stAdecAttr;
    ADEC_ATTR_ADPCM_S stAdpcm;
    ADEC_ATTR_G711_S stAdecG711;
    ADEC_ATTR_G726_S stAdecG726;
    ADEC_ATTR_LPCM_S stAdecLpcm;
    
    stAdecAttr.enType = enType;
    stAdecAttr.u32BufSize = 20;
    stAdecAttr.enMode = ADEC_MODE_STREAM;/* propose use pack mode in your app */
        
    if (PT_ADPCMA == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdpcm;
        stAdpcm.enADPCMType = AUDIO_ADPCM_TYPE ;
    }
    else if (PT_G711A == stAdecAttr.enType || PT_G711U == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecG711;
    }
    else if (PT_G726 == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecG726;
        stAdecG726.enG726bps = G726_BPS ;      
    }
    else if (PT_LPCM == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecLpcm;
        stAdecAttr.enMode = ADEC_MODE_PACK;/* lpcm must use pack mode */
    }
    else
    {
        printf("%s: invalid aenc payload type:%d\n", __FUNCTION__, stAdecAttr.enType);
        return HI_FAILURE;
    }     
    
    /* create adec chn*/
    s32Ret = HI_MPI_ADEC_CreateChn(AdChn, &stAdecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: HI_MPI_ADEC_CreateChn(%d) failed with %#x!\n", __FUNCTION__,\
               AdChn,s32Ret);
        return s32Ret;
    }
    return 0;
}


/******************************************************************************
* function : Aenc bind Ai
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_AencBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn)
{
    MPP_CHN_S stSrcChn,stDestChn;

    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32DevId = AiDev;
    stSrcChn.s32ChnId = AiChn;
    stDestChn.enModId = HI_ID_AENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = AeChn;
    
    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}
/******************************************************************************
* function : Start Aenc
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StartAenc(HI_S32 s32AencChnCnt, HI_U32 u32AencPtNumPerFrm, PAYLOAD_TYPE_E enType)
{
    AENC_CHN AeChn;
    HI_S32 s32Ret, i;
    AENC_CHN_ATTR_S stAencAttr;
    AENC_ATTR_ADPCM_S stAdpcmAenc;
    AENC_ATTR_G711_S stAencG711;
    AENC_ATTR_G726_S stAencG726;
    AENC_ATTR_LPCM_S stAencLpcm;
    
    /* set AENC chn attr */
    
    stAencAttr.enType = enType;
    stAencAttr.u32BufSize = 30;
    stAencAttr.u32PtNumPerFrm = u32AencPtNumPerFrm;
		
    if (PT_ADPCMA == stAencAttr.enType)
    {
        stAencAttr.pValue       = &stAdpcmAenc;
        stAdpcmAenc.enADPCMType = AUDIO_ADPCM_TYPE;
    }
    else if (PT_G711A == stAencAttr.enType || PT_G711U == stAencAttr.enType)
    {
        stAencAttr.pValue       = &stAencG711;
    }
    else if (PT_G726 == stAencAttr.enType)
    {
        stAencAttr.pValue       = &stAencG726;
        stAencG726.enG726bps    = G726_BPS;
    }
    else if (PT_LPCM == stAencAttr.enType)
    {
        stAencAttr.pValue = &stAencLpcm;
    }
    else
    {
        printf("%s: invalid aenc payload type:%d\n", __FUNCTION__, stAencAttr.enType);
        return HI_FAILURE;
    }    

    for (i=0; i<s32AencChnCnt; i++)
    {            
        AeChn = i;
        
        /* create aenc chn*/
        s32Ret = HI_MPI_AENC_CreateChn(AeChn, &stAencAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("%s: HI_MPI_AENC_CreateChn(%d) failed with %#x!\n", __FUNCTION__,
                   AeChn, s32Ret);
            return s32Ret;
        }        
    }
    
    return HI_SUCCESS;
}

/******************************************************************************
* function : Start Ai
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_StartAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
        AIO_ATTR_S *pstAioAttr, AUDIO_SAMPLE_RATE_E enOutSampleRate, HI_BOOL bResampleEn, AI_VQE_CONFIG_S *pstAiVqeAttr)
{
    HI_S32 i;
    HI_S32 s32Ret;
	
    s32Ret = HI_MPI_AI_SetPubAttr(AiDevId, pstAioAttr);
    if (s32Ret)
    {
        printf("%s: HI_MPI_AI_SetPubAttr(%d) failed with %#x\n", __FUNCTION__, AiDevId, s32Ret);
        return s32Ret;
    }
	
    s32Ret = HI_MPI_AI_Enable(AiDevId);
	if (s32Ret)
    {
        printf("%s: HI_MP00I_AI_Enable(%d) failed with %#x\n", __FUNCTION__, AiDevId, s32Ret);
        return s32Ret;
    }   
	
    for (i=0; i<s32AiChnCnt; i++)
    {
    	//if(i==1)
    	//	s32Ret = HI_MPI_AI_EnableChn(AiDevId, 4);
    	//else
    		s32Ret = HI_MPI_AI_EnableChn(AiDevId, i);
		if (s32Ret)
        {
            printf("%s: HI_MPI_AI_EnableChn(%d,%d) failed with %#x\n", __FUNCTION__, AiDevId, i, s32Ret);
            return s32Ret;
        }        

        if (HI_TRUE == bResampleEn)
        {
            s32Ret = HI_MPI_AI_EnableReSmp(AiDevId, i, enOutSampleRate);
			if (s32Ret)
            {
                printf("%s: HI_MPI_AI_EnableReSmp(%d,%d) failed with %#x\n", __FUNCTION__, AiDevId, i, s32Ret);
                return s32Ret;
            }
        }

		if (NULL != pstAiVqeAttr)
        {
			s32Ret = HI_MPI_AI_SetVqeAttr(AiDevId, i, SAMPLE_AUDIO_AO_DEV, i, pstAiVqeAttr);
			if (s32Ret)
		    {
		        printf("%s: HI_MPI_AI_SetVqeAttr(%d,%d) failed with %#x\n", __FUNCTION__, AiDevId, i, s32Ret);
		        return s32Ret;
		    }
			
			s32Ret = HI_MPI_AI_EnableVqe(AiDevId, i);
			if (s32Ret)
		    {
		        printf("%s: HI_MPI_AI_EnableVqe(%d,%d) failed with %#x\n", __FUNCTION__, AiDevId, i, s32Ret);
		        return s32Ret;
		    }
        }
    }
    
    return HI_SUCCESS;
}

HI_S32 SAMPLE_INNER_CODEC_CfgAudio(AUDIO_SAMPLE_RATE_E enSample, HI_BOOL bMicin)
{
    HI_S32 fdAcodec = -1;
    HI_S32 ret = HI_SUCCESS;
    unsigned int i2s_fs_sel = 0;
	int iAcodecInputVol = 0;
	
    fdAcodec = open(ACODEC_FILE,O_RDWR);
    if (fdAcodec < 0) 
    {
        printf("%s: can't open Acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        ret = HI_FAILURE;
    }
    if(ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL))
    {
    	printf("Reset audio codec error\n");
    }

    if ((AUDIO_SAMPLE_RATE_8000 == enSample)
        || (AUDIO_SAMPLE_RATE_11025 == enSample)
        || (AUDIO_SAMPLE_RATE_12000 == enSample)) 
    {
        i2s_fs_sel = 0x18;
    } 
    else if ((AUDIO_SAMPLE_RATE_16000 == enSample)
        || (AUDIO_SAMPLE_RATE_22050 == enSample)
        || (AUDIO_SAMPLE_RATE_24000 == enSample)) 
    {
        i2s_fs_sel = 0x19;
    } 
    else if ((AUDIO_SAMPLE_RATE_32000 == enSample)
        || (AUDIO_SAMPLE_RATE_44100 == enSample)
        || (AUDIO_SAMPLE_RATE_48000 == enSample)) 
    {
        i2s_fs_sel = 0x1a;
    } 
    else 
    {
        printf("%s: not support enSample:%d\n", __FUNCTION__, enSample);
        ret = HI_FAILURE;
    }

    if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel)) 
    {
        printf("%s: set acodec sample rate failed\n", __FUNCTION__);
        ret = HI_FAILURE;
    }
    
    if (HI_TRUE == bMicin)
    {
		/******************************************************************************************
		The input volume range is [-87, +86]. Both the analog gain and digital gain are adjusted.
		A larger value indicates higher volume. 
		For example, the value 86 indicates the maximum volume of 86 dB, 
		and the value -87 indicates the minimum volume (muted status). 
		The volume adjustment takes effect simultaneously in the audio-left and audio-right channels. 
		The recommended volume range is [+10, +56]. 
		Within this range, the noises are lowest because only the analog gain is adjusted, 
		and the voice quality can be guaranteed.
		*******************************************************************************************/
		iAcodecInputVol = 30; 
        if (ioctl(fdAcodec, ACODEC_SET_INPUT_VOL, &iAcodecInputVol))
        {
            printf("%s: set acodec micin volume failed\n", __FUNCTION__);
            return HI_FAILURE;
        }
        
    }
    
    close(fdAcodec);
    return ret;
}


/* config codec */ 
HI_S32 SAMPLE_COMM_AUDIO_CfgAcodec(AIO_ATTR_S *pstAioAttr, HI_BOOL bMicIn)
{
    HI_S32 s32Ret = HI_SUCCESS;
#ifdef HI_ACODEC_TYPE_AK7756
    /*** ACODEC_TYPE_AK7756EN ***/ 
    s32Ret = SAMPLE_Ak7756en_CfgAudio(pstAioAttr->enWorkmode, pstAioAttr->enSamplerate);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: SAMPLE_Ak7756en_CfgAudio failed\n", __FUNCTION__);
        return s32Ret;
    }
#endif

#ifdef HI_ACODEC_TYPE_INNER
    /*** INNER AUDIO CODEC ***/
    printf("in in in ...codec\n");
    s32Ret = SAMPLE_INNER_CODEC_CfgAudio(pstAioAttr->enSamplerate, bMicIn); 
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s:SAMPLE_INNER_CODEC_CfgAudio failed\n", __FUNCTION__);
        return s32Ret;
    }
#endif

#ifdef HI_ACODEC_TYPE_TLV320AIC31    
    /*** ACODEC_TYPE_TLV320 ***/ 
    s32Ret = SAMPLE_Tlv320_CfgAudio(pstAioAttr->enWorkmode, pstAioAttr->enSamplerate);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: SAMPLE_Tlv320_CfgAudio failed\n", __FUNCTION__);
        return s32Ret;
    }
#endif    
    return HI_SUCCESS;
}

/******************************************************************************
* function : get frame from Ai, send it  to Aenc or Ao
******************************************************************************/
void *SAMPLE_COMM_AUDIO_AiProc(void *parg)
{
    HI_S32 s32Ret;
    HI_S32 AiFd;
    SAMPLE_AI_S *pstAiCtl = (SAMPLE_AI_S *)parg;
    AUDIO_FRAME_S stFrame; 
	AEC_FRAME_S   stAecFrm;
    fd_set read_fds;
    struct timeval TimeoutVal;
    AI_CHN_PARAM_S stAiChnPara;

    s32Ret = HI_MPI_AI_GetChnParam(pstAiCtl->AiDev, pstAiCtl->AiChn, &stAiChnPara);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: Get ai chn param failed\n", __FUNCTION__);
        return NULL;
    }
    
    stAiChnPara.u32UsrFrmDepth = 30;
    
    s32Ret = HI_MPI_AI_SetChnParam(pstAiCtl->AiDev, pstAiCtl->AiChn, &stAiChnPara);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: set ai chn param failed\n", __FUNCTION__);
        return NULL;
    }
    
    FD_ZERO(&read_fds);
    AiFd = HI_MPI_AI_GetFd(pstAiCtl->AiDev, pstAiCtl->AiChn);
    FD_SET(AiFd,&read_fds);

	byte  tmp[1024];
    while (pstAiCtl->bStart)
    {     
        TimeoutVal.tv_sec = 1;
        TimeoutVal.tv_usec = 0;
        
        FD_ZERO(&read_fds);
        FD_SET(AiFd,&read_fds);
        
        s32Ret = select(AiFd+1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0) 
        {
            break;
        }
        else if (0 == s32Ret) 
        {
            printf("%s: get ai frame select time out\n", __FUNCTION__);
            continue;
        }        
        
        if (FD_ISSET(AiFd, &read_fds))
        {
			HI_BOOL flag;
			HI_U32 data_len;

            /* get frame from ai chn */
			memset(&stAecFrm, 0, sizeof(AEC_FRAME_S));
            s32Ret = HI_MPI_AI_GetFrame(pstAiCtl->AiDev, pstAiCtl->AiChn, &stFrame, &stAecFrm, HI_FALSE);
            if (HI_SUCCESS != s32Ret )
            {
            	#if 0
                printf("%s: HI_MPI_AI_GetFrame(%d, %d), failed with %#x!\n",\
                       __FUNCTION__, pstAiCtl->AiDev, pstAiCtl->AiChn, s32Ret);
                pstAiCtl->bStart = HI_FALSE;
                return NULL;
				#else
				continue;
				#endif
            }
			
	if(AudioEncType !=2)  //��Ƶ����
	{
		if(AudioEncType == 0)
		{
			g711_encode((byte *)stFrame.pVirAddr[0],stFrame.u32Len,tmp);
			//fwrite(tmp,1,stFrame.u32Len/2, pstAiCtl->pfd);

			g_audioAllSize+=stFrame.u32Len/2;

			if (g_audioPktCount <= 0)
			{
				g_startAudioPts = stFrame.u64TimeStamp;
				g_lastAudioPts = stFrame.u64TimeStamp;
			}
			
			if (g_startAudioPts == 0)
			{
				g_startAudioPts = stFrame.u64TimeStamp;
			}
			
			int64_t ptsDiff = stFrame.u64TimeStamp - g_lastAudioPts;
			if ((ptsDiff < 0) || (ptsDiff > 1000000))
			{
				ptsDiff = 0;
			}
			g_lastAudioPts = stFrame.u64TimeStamp;
		
			if(g_audioPtsPre==0)
				g_audioPtsPre=stFrame.u64TimeStamp;

			g_audioPts=	stFrame.u64TimeStamp-g_audioPtsPre;
			g_audioPktCount ++;
			
			MPacket pkt;
			memset(&pkt, 0, sizeof(pkt));
			pkt.type = MTYPE_AUDIO;
			pkt.data = (unsigned char*)tmp;
			pkt.size = stFrame.u32Len/2;
			pkt.duration = stFrame.u32Len/2;		/// 采样个数
			pkt.pts = stFrame.u64TimeStamp; 
			
			g_audioDuration += pkt.size;
			
			int clockDuration = 0;
			if (g_audioPktCount <= 1)
			{
				clock_gettime(CLOCK_REALTIME, &g_audioTimespec);
			}
			else
			{
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				clockDuration = (ts.tv_sec - g_audioTimespec.tv_sec) * 1000 + (ts.tv_nsec - g_audioTimespec.tv_nsec) / 1000000;
			}
	/*
			printf("---###audio pkt(%4d, %4d, %lld) pkt count:%d total duration:%d, clock:%d. total size:%d raw pts:%lld\n",
					pkt.size, pkt.duration, pkt.pts,
					g_audioPktCount, g_audioDuration, clockDuration,
					g_audioAllSize, stFrame.u64TimeStamp);
	*/		
			
			caster_chl_write_audio(g_handle[0], &pkt);
			
			
		}
		
		else if(AudioEncType == 1)
		{

			if(m_aac.G7112Aac((byte *)stFrame.pVirAddr[0], stFrame.u32Len) > 0)
			{
				int offset = 0;   // 7
				memcpy(tmp, &(m_aac.m_pOutAACBuffer[offset]), m_aac.aacSize - offset);
				g_audioAllSize += (m_aac.aacSize - offset);
				flag = HI_TRUE;
				data_len = m_aac.aacSize - offset;
			}
			else
			{
				flag = HI_FALSE;    // no data from AAC encoder !
			}
		
			MPacket pkt;
			memset(&pkt, 0, sizeof(pkt));
			pkt.type = MTYPE_AUDIO;
			pkt.data = (unsigned char*)tmp;
			pkt.size = data_len;
			pkt.duration = data_len;		/// 采样个数
			pkt.pts = stFrame.u64TimeStamp; 

			if(flag)	// data valid
			{
				caster_chl_write_audio(g_handle[0], &pkt);
			}
		}
			
	}
	#ifdef USE_HI_AENC		
            /* send frame to encoder */
            if (HI_TRUE == pstAiCtl->bSendAenc)
            {
                s32Ret = HI_MPI_AENC_SendFrame(pstAiCtl->AencChn, &stFrame, &stAecFrm);
                if (HI_SUCCESS != s32Ret )
                {
                    printf("%s: HI_MPI_AENC_SendFrame(%d), failed with %#x!\n",\
                           __FUNCTION__, pstAiCtl->AencChn, s32Ret);
                    pstAiCtl->bStart = HI_FALSE;
                    return NULL;
                }
            }
         
            /* send frame to ao */
            if (HI_TRUE == pstAiCtl->bSendAo)
            {
                s32Ret = HI_MPI_AO_SendFrame(pstAiCtl->AoDev, pstAiCtl->AoChn, &stFrame, 1000);
                if (HI_SUCCESS != s32Ret )
                {
                    printf("%s: HI_MPI_AO_SendFrame(%d, %d), failed with %#x!\n",\
                           __FUNCTION__, pstAiCtl->AoDev, pstAiCtl->AoChn, s32Ret);
                    pstAiCtl->bStart = HI_FALSE;
                    return NULL;
                }
                
            }
 #endif  
            /* finally you must release the stream */
            s32Ret = HI_MPI_AI_ReleaseFrame(pstAiCtl->AiDev, pstAiCtl->AiChn, &stFrame, &stAecFrm);
            if (HI_SUCCESS != s32Ret )
            {
                printf("%s: HI_MPI_AI_ReleaseFrame(%d, %d), failed with %#x!\n",\
                       __FUNCTION__, pstAiCtl->AiDev, pstAiCtl->AiChn, s32Ret);
                pstAiCtl->bStart = HI_FALSE;
                return NULL;
            }
            
        }
    }
    
    pstAiCtl->bStart = HI_FALSE;
	fclose(pstAiCtl->pfd);
    return NULL;
}




/******************************************************************************
* function : Create the thread to get frame from ai and send to ao
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_CreatTrdAiAENC(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AencChn)
{
    SAMPLE_AI_S *pstAi = NULL;
    
    pstAi = &gs_stSampleAi[AiDev*AIO_MAX_CHN_NUM + AiChn];
    pstAi->bSendAenc = HI_TRUE;
    pstAi->bSendAo = HI_FALSE;
    pstAi->bStart= HI_TRUE;
    pstAi->AiDev = AiDev;
    pstAi->AiChn = AiChn;
    pstAi->AencChn= AencChn;

    HI_CHAR aszFileName[128];
 #if 0  
    /* create file for save stream*/
    pstAi->pfd= fopen("G711Stream.g711", "w+");
    if (NULL == pstAi->pfd)
    {
        printf("g711 open file failed\n");
        return NULL;
    }
#endif

    pthread_create(&pstAi->stAiPid, 0, SAMPLE_COMM_AUDIO_AiProc, pstAi);
    
    return HI_SUCCESS;
}



static PAYLOAD_TYPE_E gs_enPayloadType = PT_LPCM ;
static HI_BOOL gs_bMicIn = HI_TRUE;
static HI_BOOL gs_bAioReSample  = HI_FALSE;
static HI_BOOL gs_bUserGetMode  = HI_FALSE;
static HI_BOOL gs_bAoVolumeCtrl = HI_TRUE;
static AUDIO_SAMPLE_RATE_E enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
static AUDIO_SAMPLE_RATE_E enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
static HI_U32 u32AencPtNumPerFrm = 0;

HI_S32 SAMPLE_AUDIO_AiAenc(HI_VOID)
{
	 HI_S32 i, s32Ret;
	AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
	AI_CHN      AiChn;
	AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
	AO_CHN      AoChn = 0;
	ADEC_CHN    AdChn = 0;
	HI_S32      s32AiChnCnt;
	HI_S32      s32AoChnCnt;
	HI_S32      s32AencChnCnt;
	AENC_CHN    AeChn;
	HI_BOOL     bSendAdec = HI_TRUE;
	FILE        *pfd = NULL;
	AIO_ATTR_S stAioAttr;
	

	if(Vi_Mode == 0)
	{
		stAioAttr.enSamplerate   = AUDIO_FREQ;
		stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
		stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
		stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
		stAioAttr.u32EXFlag      = 0;
		stAioAttr.u32FrmNum      = 30;
		stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
		stAioAttr.u32ChnCnt      = 2;
		stAioAttr.u32ClkSel      = 0;
	}
	else
	{
		stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_48000;
		stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
		stAioAttr.enWorkmode     = AIO_MODE_I2S_SLAVE;   // SiI9135 must set as slave mode
		stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
		stAioAttr.u32EXFlag      = 1;
		stAioAttr.u32FrmNum      = 30;
		stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM*2;
		stAioAttr.u32ChnCnt      = 1;
		stAioAttr.u32ClkSel      = 0;

	}

	gs_bAioReSample = HI_FALSE;
	enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
	enOutSampleRate = AUDIO_FREQ; 
	u32AencPtNumPerFrm = stAioAttr.u32PtNumPerFrm;

	g711_init();

    /********************************************
      step 1: config audio codec
    ********************************************/
	s32Ret = SAMPLE_INNER_CODEC_CfgAudio(stAioAttr.enSamplerate, gs_bMicIn); 
	if (HI_SUCCESS != s32Ret)
	{
		printf("%s:SAMPLE_INNER_CODEC_CfgAudio failed\n", __FUNCTION__);
		return s32Ret;
	}

  
	/********************************************
	  step 2: start Ai
	********************************************/
	s32AiChnCnt = stAioAttr.u32ChnCnt;
	s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, &stAioAttr, enOutSampleRate, gs_bAioReSample, NULL);
	if (s32Ret != HI_SUCCESS)
	{
	  //  SAMPLE_DBG(s32Ret);
		return HI_FAILURE;
	}

		s32AoChnCnt = stAioAttr.u32ChnCnt;
	s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AoChnCnt, &stAioAttr, enInSampleRate, HI_FALSE);
	if (s32Ret != HI_SUCCESS)
	{
	//	SAMPLE_DBG(s32Ret);
		return HI_FAILURE;
	}

	for(int i=0;i<stAioAttr.u32ChnCnt;i++)
	{
		AiChn=AoChn=i;
		/* bind AI to AO channle */
		s32Ret = SAMPLE_COMM_AUDIO_AoBindAi(AiDev, AiChn, AoDev, AoChn);
		if (s32Ret != HI_SUCCESS)
		{
		//	SAMPLE_DBG(s32Ret);
			return HI_FAILURE;
		}
		//printf("ai(%d,%d) bind to ao(%d,%d) ok\n", AiDev, AiChn, AoDev, AoChn);
	}
	
   /********************************************
      step 3: start Aenc
    ********************************************/

#ifdef USE_HI_AENC
    s32AencChnCnt = 2;
    s32Ret = SAMPLE_COMM_AUDIO_StartAenc(s32AencChnCnt, u32AencPtNumPerFrm, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
      //  SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
#endif

  /* manu send AI to AENC channle */
  gs_bUserGetMode= HI_TRUE;
	  AiChn=0;
	AeChn=0;
  
    if (HI_TRUE == gs_bUserGetMode)
    {
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAENC(AiDev, AiChn,  AeChn);
        if (s32Ret != HI_SUCCESS)
        {
           // SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
    }

	

#if 0
   /********************************************
      step 4: Aenc bind Ai Chn
    ********************************************/
	for (i=0; i<s32AencChnCnt; i++)
	{
		AeChn = i;
		AiChn = i;

		s32Ret = SAMPLE_COMM_AUDIO_AencBindAi(AiDev, AiChn, AeChn);
		if (s32Ret != HI_SUCCESS)
		{

			return s32Ret;
		}

		printf("Ai(%d,%d) bind to AencChn:%d ok!\n",AiDev , AiChn, AeChn);
    }
   #endif


   
	/********************************************
	step 5: start Adec & Ao. ( if you want )
	********************************************/
	if (HI_TRUE == bSendAdec)
	{
#if 0
        s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
        if (s32Ret != HI_SUCCESS)
        {
         //   SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

		s32AoChnCnt = stAioAttr.u32ChnCnt;
        s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AoChnCnt, &stAioAttr, enInSampleRate, gs_bAioReSample);
        if (s32Ret != HI_SUCCESS)
        {
           // SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

		pfd = SAMPLE_AUDIO_OpenAencFile(AdChn, gs_enPayloadType);
		if (!pfd)
		{
		   // SAMPLE_DBG(HI_FAILURE);
			return HI_FAILURE;
		}
#endif
		//for (i=0; i<s32AencChnCnt; i++)
		//{
#ifdef USE_HI_AENC
		AeChn=0;
		s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AeChn, AdChn, pfd);
		if (s32Ret != HI_SUCCESS)
		{
		  //  SAMPLE_DBG(s32Ret);
			return HI_FAILURE;
		}
#endif
		//}

		//s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
		//if (s32Ret != HI_SUCCESS)
		//{
		   // SAMPLE_DBG(s32Ret);
		//	return HI_FAILURE;
		//}

		printf("bind adec:%d to ao(%d,%d) ok \n", AdChn, AoDev, AoChn);
	 }

}


/******************************************************************************
* function :  H.264@1080p@30fps+H.265@1080p@30fps+H.264@D1@30fps
******************************************************************************/
bool inipara()
{

	  //��ȡ�����ļ�
//////////////////////////////////////////////////����
	reader.load_ini("box.ini");

	string str="";
	string str1="";
    string str2="";
    string strfbl="";

	reader.get_value("frame",str);
	if(str!="")
	{
		
	}
    reader.get_value("rate",str1);
	if(str1!="")
	{
	
	}

	reader.get_value("fbl",strfbl);
	if(strfbl!="")
	{
		
	}

	rl_frame = atoi(str.c_str());
	rl_rate = atoi(str1.c_str());
	rl_fbl = atoi(strfbl.c_str());


	printf("frame=%d rate=%d rl_fbl=%d\n",rl_frame,rl_rate,rl_fbl);  

	if((rl_frame<5)||(rl_rate<=100) )
	{
		printf("rl_frame rl_rate must >0\n");  
		rl_frame=25;
		rl_rate=1024 ;

	}
	if((rl_frame>60)||(rl_rate>1024*8))
	{
		printf("rl_frame>=5 and rate<=1024*6 ");
		rl_frame=25;
		rl_rate=1024 ;
	}   
    

	// 	 GetVideoPara();
	 	 //GetNetInfor();   // ������ڲ���ɾ��
//string g_ip = "192.168.1.22";
//string g_netmask = "255.255.255.0";
//string g_gateway = "0.0.0.0";
//string g_mac = "00:10:79:11:11:10";


//////////////////////////////////////////////////����
	string  strip;
	string  strnetmask;
	string  strgw;
	string  strmac;
	reader.get_value("ip",strip);
	if(strip!="")
	{
		printf("strip=%s\n",strip.c_str());
		g_ip=strip ;
		cout<<g_ip<<endl ;
	}
	else
	{
		strip= "192.168.1.22";
		cout<<g_ip<<endl ;
	}

	reader.get_value("netmask",strnetmask);
	if(strnetmask!="")
	{
		printf("  strnetmask=%s\n",strnetmask.c_str());
		g_netmask=strnetmask ;
	}
	else
	{
		strnetmask="255.255.0.0";
	}


	reader.get_value("gateway",strgw);
	if(strgw!="")
	{
		printf("strgw=%s\n",strgw.c_str());
		g_gateway=strgw ;
	}
	else
	{
		strgw="192.168.1.1";
	}

	reader.get_value("mac",strmac);
	if(strmac!="")
	{
		printf("strmac=%s\n",strmac.c_str());
		g_mac=strmac ;
	}
	else
	{
		strmac= "00:10:79:11:11:10";
	}

	//change_ip();
	////////////////////////////////////////ͼ�����

	string  strld;
	string  strsd;
	string  strbhd;
	string  strdbd;
	reader.get_value("ld",strld);
	if(strld!="")
	{
		printf("strld=%s\n",strld.c_str());
	 
        g_ld = atoi(strld.c_str());
		cout<<g_ld<<endl;
		
	}
	else
	{
		g_ld= 50 ;
	}


	reader.get_value("sd",strsd);
	if(strsd!="")
	{
		printf("strld=%s\n",strsd.c_str());

		g_sd = atoi(strsd.c_str());
		cout<<g_sd<<endl;

	}
	else
	{
		g_sd= 50 ;
		cout<<"Not find sd\n"<<endl;
	}

    reader.get_value("bhd",strbhd);
	if(strbhd!="")
	{
		printf("strbhd=%s\n",strbhd.c_str());

		g_bhd = atoi(strbhd.c_str());
		cout<<g_bhd<<endl;

	}
	else
	{
		g_bhd= 50 ;
	}

	  reader.get_value("dbd",strdbd);
	if(strbhd!="")
	{
		printf("strdbd=%s\n",strdbd.c_str());

		g_dbd = atoi(strdbd.c_str());
		cout<<g_dbd<<endl;

	}
	else
	{
		g_dbd= 50 ;
	}

	return true ;
}



//���� 
HI_U32 u32Profile;
VPSS_GRP VpssGrp;
VPSS_CHN VpssChn;
VENC_CHN VencChn;
SAMPLE_RC_E enRcMode;
PAYLOAD_TYPE_E  enPayLoad[3];
PIC_SIZE_E enSize[3];
SAMPLE_VI_CONFIG_S stViConfig ;
HI_S32 s32ChnNum;


HI_S32 SAMPLE_COMM_VENC_StopGetStream()
{
    if (HI_TRUE == gs_stPara.bThreadStart)
    {
        gs_stPara.bThreadStart = HI_FALSE;
        pthread_join(gs_VencPid, 0);
    }
    return HI_SUCCESS;
}




HI_S32 SAMPLE_COMM_VENC_UnBindVpss(VENC_CHN VeChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

/******************************************************************************
* funciton : Stop venc ( stream mode -- H264, MJPEG )
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_Stop(VENC_CHN VencChn)
{
    HI_S32 s32Ret;

    /******************************************
     step 1:  Stop Recv Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 2:  Distroy Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : Vi chn unbind vpss group
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_UnBindVpss(SAMPLE_VI_MODE_E enViMode)
{
    HI_S32 i, j, s32Ret;
    VPSS_GRP VpssGrp;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    SAMPLE_VI_PARAM_S stViParam;
    VI_DEV ViDev;
    VI_CHN ViChn;

    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    VpssGrp = 0;    
    for (i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;

        for (j=0; j<stViParam.s32ViChnCnt; j++)
        {
            ViChn = j * stViParam.s32ViChnInterval;
            
            stSrcChn.enModId = HI_ID_VIU;
            stSrcChn.s32DevId = ViDev;
            stSrcChn.s32ChnId = ViChn;
        
            stDestChn.enModId = HI_ID_VPSS;
            stDestChn.s32DevId = VpssGrp;
            stDestChn.s32ChnId = 0;
        
            s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
            
            VpssGrp ++;
        }
    }
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VPSS_DisableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret;

    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM)
    {
        printf("VpssGrp%d is out of rang[0,%d]. \n", VpssGrp, VPSS_MAX_GRP_NUM);
        return HI_FAILURE;
    }

    if (VpssChn < 0 || VpssChn > VPSS_MAX_CHN_NUM)
    {
        printf("VpssChn%d is out of rang[0,%d]. \n", VpssChn, VPSS_MAX_CHN_NUM);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("%s failed with %#x\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VPSS_StopGroup(VPSS_GRP VpssGrp)
{
    HI_S32 s32Ret;

    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM)
    {
        printf("VpssGrp%d is out of rang[0,%d]. \n", VpssGrp, VPSS_MAX_GRP_NUM);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("%s failed with %#x\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("%s failed with %#x\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}



HI_S32 SAMPLE_COMM_VI_StopBT656(SAMPLE_VI_MODE_E enViMode)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;

    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }

    /*** Stop VI Chn ***/
    for(i=0;i<stViParam.s32ViChnCnt;i++)
    {
        /* Stop vi phy-chn */
        ViChn = i * stViParam.s32ViChnInterval;
        s32Ret = HI_MPI_VI_DisableChn(ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StopChn failed with %#x\n",s32Ret);
            return HI_FAILURE;
        }
    }

    /*** Stop VI Dev ***/
    for(i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;
        s32Ret = HI_MPI_VI_DisableDev(ViDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StopDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_COMM_VI_StopVi(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_VI_MODE_E enViMode;

    if(!pstViConfig)
    {
        SAMPLE_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }
    enViMode = pstViConfig->enViMode;
    
    s32Ret = SAMPLE_COMM_VI_StopBT656(enViMode);        
    
    return s32Ret;
}

HI_VOID SAMPLE_COMM_SYS_Exit(void)
{
    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();
    return;
}

HI_S32 SAMPLE_COMM_AUDIO_AencUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn)
{
    MPP_CHN_S stSrcChn,stDestChn;

    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32DevId = AiDev;
    stSrcChn.s32ChnId = AiChn;
    stDestChn.enModId = HI_ID_AENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = AeChn;
    
    return HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);      
}


HI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(AENC_CHN AeChn)
{
    SAMPLE_AENC_S *pstAenc = NULL;

    pstAenc = &gs_stSampleAenc[AeChn];
    if (pstAenc->bStart)
    {
        pstAenc->bStart = HI_FALSE;
        pthread_cancel(pstAenc->stAencPid);
        pthread_join(pstAenc->stAencPid, 0);
    }
    
    
    return HI_SUCCESS;
}


HI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAenc()
{
	HI_U32 u32ChnId;

    for (u32ChnId = 0; u32ChnId < AENC_MAX_CHN_NUM; u32ChnId ++)
    {
        SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(u32ChnId);
    }    

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_AUDIO_StopAenc(HI_S32 s32AencChnCnt)
{
    HI_S32 i;
    HI_S32 s32Ret;
    
    for (i=0; i<s32AencChnCnt; i++)
    {
        s32Ret = HI_MPI_AENC_DestroyChn(i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("%s: HI_MPI_AENC_DestroyChn(%d) failed with %#x!\n", __FUNCTION__,
                   i, s32Ret);
            return s32Ret;
        }
        
    }
    
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_AUDIO_StopAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
        HI_BOOL bResampleEn, HI_BOOL bVqeEn)
{
    HI_S32 i; 
    HI_S32 s32Ret;
    
    for (i=0; i<s32AiChnCnt; i++)
    {
        if (HI_TRUE == bResampleEn)
        {
            s32Ret = HI_MPI_AI_DisableReSmp(AiDevId, i);
            if(HI_SUCCESS != s32Ret)
            {
                printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
                return s32Ret;
            }
        }

		if (HI_TRUE == bVqeEn)
        {
            s32Ret = HI_MPI_AI_DisableVqe(AiDevId, i);
            if(HI_SUCCESS != s32Ret)
            {
                printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
                return s32Ret;
            }
        }
        
        s32Ret = HI_MPI_AI_DisableChn(AiDevId, i);
        if(HI_SUCCESS != s32Ret)
        {
            printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
            return s32Ret;
        }
    }  
    
    s32Ret = HI_MPI_AI_Disable(AiDevId);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
        return s32Ret;
    }
    
    return HI_SUCCESS;
}
HI_S32 SAMPLE_COMM_SYS_GetPicSize(SIZE_S *pstSize)
{
	hiPIC_SIZE_E enSize;
	if(Vi_Input == 0)
	{
		enSize=PIC_HD720; 
	}
	else if(Vi_Input == 1)
	{
		enSize=PIC_HD1080; 

	}
	else if(Vi_Input == 2)
	{
		enSize=PIC_HD1080; 

	}
	else if(Vi_Input == 3)
	{
		enSize=PIC_D1;
	}

	return SAMPLE_COMM_SYS_GetPicSize(gs_enNorm,enSize,pstSize);
}
 
bool gloableSettingStarrtc()
{
	bool bRet = true;
	int nWidth = 1920;
	int nHeight = 1080;
	if(Vi_Input == 0 && videoParam.VideoType > 3)
		videoParam.VideoType = 3;


	 if(videoParam.VideoType==0)  //CIF
	{
		nWidth = 352;
		nHeight = 288;
	}
	 else if(videoParam.VideoType==1) //VGA
	{
		nWidth = 640;
		nHeight = 480;
	}
	else if(videoParam.VideoType==2) //D1
	{
		nWidth=720;
		nHeight=576;
	}
	else if(videoParam.VideoType==3)//720P
	{
		nWidth=1280;
		nHeight=720; 
	}
	else if(videoParam.VideoType==4)//1080P
	{
		nWidth=1920;
		nHeight=1080;
	}

	if(StRotate == 1||StRotate == 3)
	{
		int tmp = nWidth;
		nWidth = nHeight;
		nHeight = tmp;
	}
	/*HI_S32 s32Ret = SAMPLE_COMM_SYS_GetPicSize(&stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("get picture size failed!\n");
		bRet = false;
	}
	else*/
	//{
	//	gStartLiveSrc.globalSetting(nWidth, nHeight,videoParam.VideoFrameRate, videoParam.VideoBitRate);
	//}
	g_pXHLiveManager->m_Param.videoParam.w = nWidth;
	g_pXHLiveManager->m_Param.videoParam.h = nHeight;
	g_pXHLiveManager->m_Param.videoParam.fps = videoParam.VideoFrameRate;
	g_pXHLiveManager->m_Param.videoParam.bitrate = videoParam.VideoBitRate;
	return bRet;
}

HI_S32 StartVideoEnc()
{

	HI_U8 aac_config[60];	
	VB_CONF_S stVbConf;

	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr;
	VPSS_CHN_MODE_S stVpssChnMode;

	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	SIZE_S stSize;


	u32Profile = 0;
	

	if(Vi_Input == 0)
	{
		enSize[0]=PIC_HD720; 
		enSize[1]=PIC_D1; 
		enSize[2]=PIC_CIF;
		printf("720P HDMI MODE\n");
	}
	else if(Vi_Input == 1)
	{
		enSize[0]=PIC_HD1080; 
		enSize[1]=PIC_HD720; 
		enSize[2]=PIC_D1;
		printf("1080P HDMI MODE\n");
	}
	else if(Vi_Input == 2)
	{
		enSize[0]=PIC_HD1080; 
		enSize[1]=PIC_HD720; 
		enSize[2]=PIC_D1;
		printf("1080i HDMI MODE\n");
	}
	else if(Vi_Input == 3)
	{
		system("himm 0x20120004  0x400000");

		enSize[0]=PIC_D1;
		enSize[1]=PIC_D1;
		enSize[2]=PIC_CIF;

		printf("CVBS MODE\n");
	}

	
	if(videoParam.VideoEncType==0)
	{
		enPayLoad[0]=PT_H264;
		enPayLoad[1]=PT_H264;
		enPayLoad[2]=PT_H264;
	}
	else if(videoParam.VideoEncType==1)
	{
		enPayLoad[0]=PT_H265;
		enPayLoad[1]=PT_H265; 
		enPayLoad[2]=PT_H265;
	}


	
	rl_frame=videoParam.VideoFrameRate;
	rl_rate=videoParam.VideoBitRate;

	// use channel number 
	s32ChnNum = 1;

	g_ld=50 ;
	g_sd=50 ;
	g_bhd=50 ;
	g_dbd=50 ;


 /******************************************
     step  1: init sys variable 
    ******************************************/
	memset(&stVbConf,0,sizeof(VB_CONF_S));

	SAMPLE_COMM_VI_GetSizeBySensor(&enSize[0]);
	switch(InputMode[Vi_Input])
	{
		case SONY_IMX178_LVDS_5M_30FPS:
		case APTINA_AR0330_MIPI_1536P_25FPS:
		case APTINA_AR0330_MIPI_1296P_25FPS:
			enSize[1] = PIC_VGA;
			break;
		default:
			break;
	}

	stVbConf.u32MaxPoolCnt = 128;

	/*video buffer*/
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
				enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = 20;

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
				enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt =20;

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
				enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[2].u32BlkCnt = 20;


    /******************************************
     step 2: mpp system init. 
    ******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		return HI_FAILURE;
	}

	 /******************************************
	 step 3: start vi dev & chn to capture
	******************************************/

	stViConfig.enViMode = InputMode[Vi_Input];

#if 1
	if(StRotate==1)
	{
		stViConfig.enRotate = ROTATE_90;
	}
	else if(StRotate==2)
	{
		stViConfig.enRotate = ROTATE_180;
	}
	else if(StRotate==3)
	{
		stViConfig.enRotate = ROTATE_270;
	}
	else
	{
		stViConfig.enRotate = ROTATE_NONE;
	}
#endif
	stViConfig.enNorm 	= VIDEO_ENCODING_MODE_AUTO;
	stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
	stViConfig.enWDRMode = WDR_MODE_NONE;

	 
	s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig); 
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		return HI_FAILURE;
	}
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/

	
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		return HI_FAILURE;
	}

	VpssGrp = 0;
	stVpssGrpAttr.u32MaxW = stSize.u32Width;
	stVpssGrpAttr.u32MaxH = stSize.u32Height;
	stVpssGrpAttr.bIeEn = HI_FALSE;
	stVpssGrpAttr.bNrEn = HI_TRUE;
	stVpssGrpAttr.bHistEn = HI_FALSE;
	stVpssGrpAttr.bDciEn = HI_FALSE;
	stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

	 s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
	  	return HI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
	  	return HI_FAILURE;
	}

	VpssChn = 0;
	stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
	stVpssChnMode.bDouble        = HI_FALSE;
	stVpssChnMode.enPixelFormat  = SAMPLE_PIXEL_FORMAT;
	stVpssChnMode.u32Width       = stSize.u32Width;
	stVpssChnMode.u32Height      = stSize.u32Height;
	stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
	memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
	stVpssChnAttr.s32SrcFrameRate = -1;
	stVpssChnAttr.s32DstFrameRate = -1;
	
	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
		return HI_FAILURE;
	}


	VpssChn = 1;
	stVpssChnMode.enChnMode       = VPSS_CHN_MODE_USER;
	stVpssChnMode.bDouble         = HI_FALSE;
	stVpssChnMode.enPixelFormat   = SAMPLE_PIXEL_FORMAT;
	stVpssChnMode.u32Width        = stSize.u32Width;
	stVpssChnMode.u32Height       = stSize.u32Height;
	stVpssChnMode.enCompressMode  = COMPRESS_MODE_SEG;
	stVpssChnAttr.s32SrcFrameRate = -1;
	stVpssChnAttr.s32DstFrameRate = -1;

	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
	  	return HI_FAILURE;
	}
#if 0
	VpssChn = 2;
	stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
	stVpssChnMode.bDouble		= HI_FALSE;
	stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
	stVpssChnMode.u32Width		= 720;
	stVpssChnMode.u32Height 	= (VIDEO_ENCODING_MODE_PAL==gs_enNorm)?576:480;;
	stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

	stVpssChnAttr.s32SrcFrameRate = -1;
	stVpssChnAttr.s32DstFrameRate = -1;
#endif

	VO_PUB_ATTR_S stVoPubAttr;
	VO_INTF_TYPE_E g_enVoIntfType;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN VoChn = 0;
	VO_LAYER VoLayer = 0;
	VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;
	g_enVoIntfType=VO_INTF_CVBS;
	
	/*	cvbs vi--->vo  */
	stVoPubAttr.enIntfType = g_enVoIntfType;
	stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
	stVoPubAttr.u32BgColor = 0x000000ff;

	/* In HD, this item should be set to HI_FALSE */
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
		return HI_FAILURE;
	}

	stLayerAttr.bClusterMode = HI_FALSE;
	stLayerAttr.bDoubleFrame = HI_FALSE;
	stLayerAttr.enPixFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stLayerAttr.stDispRect.s32X = 0;
	stLayerAttr.stDispRect.s32Y = 0;
    
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, 
	                        &stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height,
	                        &stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_GetWH failed!\n");
		return HI_FAILURE;
	}   

	stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
	stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr, HI_TRUE);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
		return HI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, VO_MODE_1MUX);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
		return HI_FAILURE;
	}


	VpssChn=1;
	s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss(vo:%d)-(VpssChn:%d) failed with %#x!\n", VoDev, VoChn, s32Ret);
		return HI_FAILURE;
	}


	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
		return HI_FAILURE;
	}

	if(RcMode==0)
	{
		enRcMode = SAMPLE_RC_CBR;
	}
	else	
		enRcMode = SAMPLE_RC_VBR;
	
	VpssGrp = 0;
	VpssChn = 0;
	VencChn = 0;
	s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0],\
								   gs_enNorm, enSize[0], enRcMode,u32Profile);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		return HI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		return HI_FAILURE;
	}

	if(videoParam.VideoEncType==0)   //h264
	{
		memset(&g_format[0], 0, sizeof(MFormat));
		g_format[0].codec = MCODEC_H264;
		g_format[0].width = stVpssChnMode.u32Width;
		g_format[0].height = stVpssChnMode.u32Height;
		g_format[0].framerate = rl_frame;
		g_format[0].profile = 66; // 66= baseline, 77=main, 100=high


		if(AudioEncType ==0 )
		{
			g_format[0].audioCodec = MCODEC_G711A; 
		}
		else if(AudioEncType ==1 )
		{
			my_aac_init();	

			g_format[0].audioProfile = 1;
			g_format[0].audioCodec = MCODEC_AAC;
			g_format[0].configSize = 2;
			g_format[0].config = &aac_config[0];
			memcpy(&aac_config[0], &(m_aac.aac_config), g_format[0].configSize);

			printf("MFormat: aac config[0]:0x%02x, aac config[1]:0x%02x\n", aac_config[0], aac_config[1]);
		}


		g_format[0].channels = 1;
		g_format[0].sampleRate = AUDIO_FREQ; //audio_freq;
		g_format[0].clockRate = 1000000; /// 90000
		g_format[0].audioRate = 1000000; /// g_format[0].sampleRate;

	
		caster_chl_open(&g_handle[0], "0", &g_format[0]);
	
	}


	
	else if(videoParam.VideoEncType==1)   //h265
	{
		memset(&g_format[0], 0, sizeof(MFormat));
		g_format[0].codec = MCODEC_H265;
		g_format[0].width = stVpssChnMode.u32Width;
		g_format[0].height = stVpssChnMode.u32Height;
		g_format[0].framerate = rl_frame;
		g_format[0].profile = 66; // 66= baseline, 77=main, 100=high

		if(AudioEncType == 0)
		{
			g_format[0].audioCodec = MCODEC_G711A; 
		}
		else if(AudioEncType == 1)
		{
			my_aac_init();	

			g_format[0].audioProfile = 1;
			g_format[0].audioCodec = MCODEC_AAC;
			g_format[0].configSize = 2;
			g_format[0].config = &aac_config[0];
			memcpy(&aac_config[0], &(m_aac.aac_config), g_format[0].configSize);

			printf("MFormat: aac config[0]:0x%02x, aac config[1]:0x%02x\n", aac_config[0], aac_config[1]);

		}
		
		g_format[0].channels = 1;
		g_format[0].sampleRate = AUDIO_FREQ; //audio_freq; 
		g_format[0].clockRate = 1000000; 
		g_format[0].audioRate = 1000000; /// g_format[0].sampleRate;

		caster_chl_open(&g_handle[0], "0", &g_format[0]);
		
	}
	
#if 0
	/*** 1080p **/
	VpssChn = 1;
	VencChn = 1;
	s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[1], \
									gs_enNorm, enSize[1], enRcMode,u32Profile);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
	  //  goto END_VENC_1080P_CLASSIC_5;
	}

	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
	  //  goto END_VENC_1080P_CLASSIC_5;
	}
	VpssChn = 2;
	VencChn = 2;
	s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[2], \
									gs_enNorm, enSize[2], enRcMode,u32Profile);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
	  //  goto END_VENC_1080P_CLASSIC_5;
	}

	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
	  //  goto END_VENC_1080P_CLASSIC_5;
	}
	
	
	memset(&g_format[1], 0, sizeof(MFormat));
	g_format[1].codec = MCODEC_H265;
	g_format[1].width = stVpssChnMode.u32Width;
	g_format[1].height = stVpssChnMode.u32Height;
	g_format[1].framerate = 25;
	g_format[1].profile = 0;
	g_format[1].audioCodec = MCODEC_G711A; //MCODEC_NONE;//MCODEC_G711A;
	g_format[1].channels = 1;
	g_format[1].sampleRate = 8000;
	g_format[1].clockRate = 1000000;
	g_format[1].audioRate = 1000000;
	
	//rtmpcaster_open(&g_handle[1], 0, 0, &g_format[1]);
    
	
#endif
	/******************************************
	 step 6: stream venc process -- get stream, then save it to file.
	******************************************/
	s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		return HI_FAILURE;
	}

}

HI_S32 StopVideoEnc()
{
  /******************************************
	 step 7: exit process
	******************************************/
	SAMPLE_COMM_VENC_StopGetStream();
	
END_VENC_1080P_CLASSIC_5:
	VpssGrp = 0;
	
	VpssChn = 0;  
	VencChn = 0;
	SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
	SAMPLE_COMM_VENC_Stop(VencChn);

	VpssChn = 1;   
	VencChn = 1;
	SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
	SAMPLE_COMM_VENC_Stop(VencChn);

	
	if(SONY_IMX178_LVDS_5M_30FPS != InputMode[Vi_Input])
	{
		VpssChn = 2;   
		VencChn = 2;
		SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_Stop(VencChn);
	}

	SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
	VpssGrp = 0;
	VpssChn = 0;
	SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	VpssChn = 1;
	SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	if(SONY_IMX178_LVDS_5M_30FPS != InputMode[Vi_Input])
	{
		VpssChn = 2;
		SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	}
END_VENC_1080P_CLASSIC_3:	 //vpss stop	   
	SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:	 //vpss stop   
	SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
	SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	
	//rtmpcaster_close(g_handle[0]);
	//rtmpcaster_close(g_handle[1]);
	
	return 0;

}

HI_S32 StopAudioEnc()
{
	HI_S32 i, s32Ret;

	HI_S32		s32AencChnCnt;

	s32AencChnCnt=2;
	AI_CHN      AiChn;
	AENC_CHN    AeChn;

    /********************************************
      step 6: exit the process
    ********************************************/

	SAMPLE_COMM_AUDIO_DestoryTrdAenc();
	
    for (i=0; i<s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

     
        s32Ret = SAMPLE_COMM_AUDIO_AencUnbindAi(0, AiChn, AeChn);
        if (s32Ret != HI_SUCCESS)
        {
            return HI_FAILURE;
        }
        
    }
    
    s32Ret = SAMPLE_COMM_AUDIO_StopAenc(s32AencChnCnt);
    if (s32Ret != HI_SUCCESS)
    {
       
        return HI_FAILURE;
    }
    
    s32Ret = SAMPLE_COMM_AUDIO_StopAi(0, 2, gs_bAioReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        
        return HI_FAILURE;
    }

    return HI_SUCCESS;

}


void UDP_THREAD(void)
{
#if 1
	printf("udp start\n") ;
	pthread_t udpid;
	int	ret = pthread_create (&udpid, NULL,udprecv, NULL);	 //
	if (ret != 0)
	{
		 perror ("Create timer faile!\n");
		 exit (1);
	}  

	pthread_t udpsetid;
	ret = pthread_create (&udpsetid, NULL,udprecvset, NULL);  //
	if (ret != 0)
	{
		 perror ("Create timer faile!\n");
		 exit (1);
	}  
#endif
}

void * HTX_CheckViMode(void *parg)
{

	int ret;
	CHECK_VI_MODE * pstCheckCtl=(CHECK_VI_MODE *) parg;
	
	HI_U32 pixel,INTRL;

	while(pstCheckCtl->bStart)
	{
		ret=ioctl(pstCheckCtl->fd, 0, &pixel);
		if( ret < 0)
		{
			SAMPLE_PRT("ioctl Get HDMI Pixel fail\n");
		}

		if(pixel == 1280)
		{
			//printf("Check Input Pixel=720P\n");
			Vi_Input=0;
		}
		else if(pixel == 1920)
		{
			ret=ioctl(pstCheckCtl->fd, 1, &INTRL);	
			if (ret < 0)
			{
				printf("ioctl Get HDMI Intrl fail\n");
			}
			if(INTRL == 0)
			{
				Vi_Input=1;
				//printf("Check Input Pixel=1080P\n");
			}
			else if(INTRL == 1)
			{
				Vi_Input=2;
				//printf("Check Input Pixel=1080I\n");
			}
			else
				printf("get intrl error\n");
		}
		else 
			printf("get hdmi pixel error\n");

		usleep(500);
	}

	 close(pstCheckCtl->fd);
	 
	 return HI_SUCCESS;
}




HI_U32 HTX_CreatTrdCheckViMode(void)
{
	CHECK_VI_MODE * pstCheck;
	pstCheck=&gs_stCheck;
	
	pstCheck->fd= open(DEV_FILE, O_RDWR);
	if (pstCheck->fd < 0)
	{
		SAMPLE_PRT("open %s fail\n", DEV_FILE);
		return HI_FAILURE;
	}

	pstCheck->bStart=HI_TRUE;

	pthread_create(&pstCheck->checkid,0,HTX_CheckViMode,pstCheck);

	return 0;
}


HI_S32 Load_Param(HI_VOID)
{
	HI_S32 ret,i;
	string ViModeStr,VideoEncTypeStr,VideoTypeStr,VideoFrameRateStr,VideoBitRateStr,RcModeStr,SaveModeStr,AudioEncTypeStr,SDI_InputStr;
	
	ret = reader.load_ini(FILENAME);
	if(ret!=-1)
	{
		reader.get_value("ViMode", ViModeStr);  
		reader.get_value("SDI_Input", SDI_InputStr);  
		reader.get_value("RcMode", RcModeStr);    
		reader.get_value("VideoEncType", VideoEncTypeStr);
		reader.get_value("VideoType", VideoTypeStr);
		reader.get_value("VideoFrameRate", VideoFrameRateStr);
		reader.get_value("VideoBitRate", VideoBitRateStr);
		reader.get_value("SaveMode", SaveModeStr);
		reader.get_value("AudioEncType", AudioEncTypeStr);

	
		Vi_Mode = atoi(ViModeStr.c_str());
		SDI_Input = atoi(SDI_InputStr.c_str());
		RcMode = atoi(RcModeStr.c_str());
		SaveMode = atoi(SaveModeStr.c_str());
		AudioEncType = atoi(AudioEncTypeStr.c_str());
		
		
		videoParam.VideoEncType = atoi(VideoEncTypeStr.c_str());
		videoParam.VideoType = atoi(VideoTypeStr.c_str());
		videoParam.VideoFrameRate = atoi(VideoFrameRateStr.c_str());
		videoParam.VideoBitRate = atoi(VideoBitRateStr.c_str());

		if(Vi_Mode==0&&videoParam.VideoType > 2)
			videoParam.VideoType=2;
	
	}
	
	printf("########\nViMode=%d\nSDI_Input=%d\nRcMode=%d\nVideoEncType=%d\nVideoType=%d\nVideoFrameRate=%d\nVideoBitRate=%d\nSaveMode=%d\nAudioEncType=%d\n",\
		Vi_Mode,SDI_Input,RcMode,videoParam.VideoEncType,	videoParam.VideoType,videoParam.VideoFrameRate,videoParam.VideoBitRate,SaveMode,AudioEncType);

	return HI_SUCCESS;
}


HI_U32 SetFrameRateAndBitRate(HI_U32 nFrameRate, HI_U32 nBitRate)
{
	VENC_CHN_ATTR_S stVencChnAttr;
	HI_S32 ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
	if(ret != HI_SUCCESS)
	{
		return ret;
	}

	stVencChnAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate = nFrameRate;
	stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = nBitRate;

	ret = HI_MPI_VENC_SetChnAttr(VencChn, &stVencChnAttr);
	return ret;
}

void my_aac_init(void)
{
	m_aac.init(/*audio_freq*/AUDIO_FREQ, 1, 16, 128000); // 8K, 1 channel, 16 bits, 128000
	
}



