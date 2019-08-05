#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "V3516A.h"
#include "send_searchIp.h"
#include <pthread.h>

#include "RtspCaster.h"
#include "XHLiveManager.h"
#include "CLogin.h"
#include "XHChatroomType.h"
#include "CInterfaceUrls.h"

extern int Vi_Mode,Vi_Input,SDI_Input;



 int main( int argc, char **argv )
{ 

	//初始化 RTSP端口
	caster_init(554);

	Load_Param();

	if(Vi_Mode == 0)   //CVBS MODE
	{
		Vi_Input = 3;
	}
	else if(Vi_Mode ==1 )  //HDMI MODE
	{
		//启动检测VI模式的线程 修改Vi_Input
		HTX_CreatTrdCheckViMode(); 
	}
	else if(Vi_Mode == 2)  //SDI MODE
	{
		Vi_Input = SDI_Input;
	}

	CUserManager* pUserManager = new CUserManager();
	CLogin login(pUserManager);

	bool bRet = login.logIn();

	while(bRet == false)
	{
		usleep( 1000 );
		bRet = login.logIn();
	}
	string strName = "armLive" + pUserManager->m_ServiceParam.m_strUserId;
	XH_CHATROOM_TYPE chatRoomType = XH_CHATROOM_TYPE_GLOBAL_PUBLIC;
	XH_LIVE_TYPE channelType = XH_LIVE_TYPE_GLOBAL_PUBLIC;
	g_pXHLiveManager = new XHLiveManager(pUserManager);

	list<ChatroomInfo> listData;
	string strLiveId = "";

	char strListType[10] = { 0 };
	sprintf(strListType, "%d,%d", CHATROOM_LIST_TYPE_LIVE, CHATROOM_LIST_TYPE_LIVE_PUSH);

	if(pUserManager->m_bAEventCenterEnable)
	{
		CInterfaceUrls::demoQueryList(strListType, listData);
	}
	else
	{
		XHLiveManager::getLiveList(pUserManager, "", strListType, listData);
	}
	list<ChatroomInfo>::iterator iter = listData.begin();

	for (; iter != listData.end(); iter++)
	{
		if(iter->m_strCreaterId == pUserManager->m_ServiceParam.m_strUserId && strName == iter->m_strName)
		{
			strLiveId = iter->m_strRoomId;
			break;
		}
	}
	if(strLiveId == "")
	{
		strLiveId = g_pXHLiveManager->createLive(strName, chatRoomType, channelType);
		if(strLiveId != "")
		{
			string strInfo = "{\"id\":\"";
			strInfo += strLiveId;
			strInfo += "\",\"creator\":\"";
			strInfo += pUserManager->m_ServiceParam.m_strUserId;
			strInfo += "\",\"name\":\"";
			strInfo += strName;
			strInfo += "\"}";
			bool bAec = true;
			if(bAec)
			{
				printf("%s\n", (char*)strInfo.c_str());
				CInterfaceUrls::demoSaveToList(pUserManager->m_ServiceParam.m_strUserId, CHATROOM_LIST_TYPE_LIVE, strLiveId, strInfo);
			}
			else
			{
				g_pXHLiveManager->saveToList(pUserManager->m_ServiceParam.m_strUserId, CHATROOM_LIST_TYPE_LIVE, strLiveId, strInfo);
			}
		}
	}

	if(strLiveId != "")
	{
		if(!gloableSettingStarrtc())
		{
			printf("gloableSettingStarrtc failed \n");
		}
		else
		{
			StartVideoEnc();

			while(g_pXHLiveManager->m_Param.videoParam.ppsData == NULL || g_pXHLiveManager->m_Param.videoParam.spsData == NULL)
			{
				usleep( 1000 );
			}

			StopVideoEnc();
			
			bRet = g_pXHLiveManager->startLive(strLiveId);
			if(bRet)
			{
				g_bInit = true;
				StartVideoEnc();
				while (true)
				{
					usleep( 800 );
				}

				StopVideoEnc();
				
			}
			else
			{
				printf("startLive failed \n");
			}
		}
		
	}
	else
	{
		printf("createLive failed \n");
	}	
	caster_quit();
 }
