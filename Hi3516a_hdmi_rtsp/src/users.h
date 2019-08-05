#include "udptcp.h"
#include <list>
#include<set>
#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <string.h>
#include <string>
#include <sys/time.h> 

#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include<string.h>
#include <vector>

#include "V3516A.h"
#include <iostream>
#include <unistd.h>
#include <string>
using namespace std;


extern HI_S32 StopVideoEnc() ;
extern HI_S32 StartVideoEnc() ;
extern HI_S32 SAMPLE_AUDIO_AiAenc(HI_VOID);
extern HI_S32 StopAudioEnc();



typedef struct _VIDEO_PARAM{
	HI_U8 VideoEncType;	
	HI_U8 VideoType;	
	HI_U8 VideoFrameRate; 
	HI_U16 VideoBitRate;
	HI_U8 ViRrame;
} VIDEO_PARAM;
const HI_U8 video_params = 30;


#pragma pack(push)  //
#pragma pack(1)

typedef struct _HDMI_INFO
{
	unsigned int intrl;
	unsigned int pixel;
	unsigned int fps;
	unsigned int audiosample;
	
}HDMIINFO;

#pragma pack(pop)


typedef simple_tcp * ptcp;
class x_vdevice;
x_vdevice * g_vdevice; 
set <ptcp> m_users;

const char * http_ok =
"HTTP/1.0 200 OK\r\n"
"Content-type: application/octet-stream\r\n"
"Cache-Control: no-cache\r\n"
"\r\n";
extern void change_ip(char *ip,char *netmask ,char *gw,char *mac) ;
extern ini_reader  reader;
class x_vdevice
{	
private:

public:	x_vdevice( ) 
	{
	    g_vdevice = this;
	 }

	void add_user(ptcp user)
	{
		m_users.insert(user);
	}
	void del_user(ptcp user)
	{
		m_users.erase(user);
	}

} ;
simple_tcp s;
void * usersock(void *){
	
 int   sock;
 struct   sockaddr_in   sin;
 struct   ifreq   ifr;

 sock   =   socket(AF_INET,   SOCK_DGRAM,   0);
 if   (sock   ==   -1)
 {
 perror("socket");

 }

 strncpy(ifr.ifr_name,   "eth0",   IFNAMSIZ);
 ifr.ifr_name[IFNAMSIZ   -   1]   =   0;

  if   (ioctl(sock,   SIOCGIFADDR,   &ifr)   <   0)
 {
    perror("ioctl");

  }

   memcpy(&sin,   &ifr.ifr_addr,   sizeof(sin));


    printf("listen ip=%s", inet_ntoa(sin.sin_addr));
 	
	bool b = s.listen(inet_ntoa(sin.sin_addr), 8081);
		if(!b)
	 {
	 	printf("listen faile\n") ;
	    	
	 }
while(true){	
   simple_tcp * s2 = new simple_tcp;
   s.accept(*s2);
   g_vdevice->add_user(s2);
   printf("add user\n");
// s2->send(http_ok, strlen(http_ok));
 }
}
#define RECVPORT  9091
simple_udp   sudp ;
vector<simple_udp > udpuser;
VIDEO_PARAM videoParam,recvVideoParam;


void * udprecv(void *)
{

	printf("udp thread start\n");
	sudp.create_socket(RECVPORT); 
	char buff[100];
	memset(&videoParam,0,sizeof(VIDEO_PARAM));
	memset(&recvVideoParam,0,sizeof(VIDEO_PARAM));

	while(1){
		printf("in while(1)\n");
		sudp.recvclient(buff); 

		if(buff[0]=='a'&&buff[1]=='b')  //删除已登入的套接字
		{
			printf("recv delete\n");
			vector<simple_udp>::iterator it=udpuser.begin(); 
			for(int i=0;i<udpuser.size();i++)
			{
			    it=udpuser.begin() ;


				if( udpuser[i].clientAddr.sin_port=sudp.clientAddr.sin_port)
				{
					it=it+i;
					udpuser.erase(it);

					printf("delete udp\n");
					printf("udpuser size=%d\n",udpuser.size());
					break ;
				}
				else
				{
					printf("udpsock is not same\n");
				}
			}
		}
		else if(buff[0]==video_params) // 接收到网络命令
		{
			memset(&recvVideoParam,0,sizeof(VIDEO_PARAM));
			memcpy((char*)&recvVideoParam,buff,sizeof(VIDEO_PARAM));

			// 处理参数
			if((recvVideoParam.VideoType!=videoParam.VideoType)|| \
				(recvVideoParam.VideoFrameRate!=videoParam.VideoFrameRate)|| \
				(recvVideoParam.VideoEncType!=videoParam.VideoEncType) || \
				(recvVideoParam.VideoBitRate!=videoParam.VideoBitRate))
			{
				// 先停止采集
				printf("changing paramenters\n");
				memcpy((char*)&videoParam,(char*)&recvVideoParam,sizeof(VIDEO_PARAM));

				StopAudioEnc();
				StopVideoEnc();

				StartVideoEnc();
				SAMPLE_AUDIO_AiAenc();


			}

		}
		else
		{
			printf("add user\n");
			udpuser.push_back(sudp);
			printf("recv users size=%d\n",udpuser.size()); 
		    ////////////////////////////////////////////////////
			string decodeip=inet_ntoa(sudp.clientAddr.sin_addr) ;
	
			reader.set_value("decode",decodeip) ;
			
			reader.save_ini("box.ini") ;

		}
	}
}
simple_udp   sudpset ;
//extern void change_ip()  ;

struct  SETNET{
	char  type ;
	char ipp[16] ;
	char netmask[16];
	char gateway[16];
	char mac[18];
};

struct  SETENC{
	char  type ;
	char  frame ;
	char  fbl;
	int   rate ;
};

struct  SETPIC{
	char  type ;
	char  ld;
	char  sd;
	char  bhd;
	char  dbd;
};

struct GETPARA{
  char type;
  char ld;
  char sd;
  char bhd;
  char dbd;
  char fbl;
  char frame;
  int rate ;
  char ipp[16] ;
  char netmask[16];
  char gateway[16];
  char mac[18];
};
 GETPARA  getpara ;

 
void * udprecvset(void *){

    printf("audio recv thread start\n");
	sudpset.create_socket(7000); 
	char buff[2048];
	FILE *pFile;



	while(1)
	{
		int num=sudpset.recvclient(buff); 

		if(num>0)
		{
			printf("receive audio pkt size:%d \n",num);
			//fwrite(buff, 1, num, pFile);
		}
		//else
		//	sleep(2);
	}

	//fclose(pFile);

}
