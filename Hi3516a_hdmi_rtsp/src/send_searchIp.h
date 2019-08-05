#ifndef    _SEND_SEARCH
#define    _SEND_SEARCH 
 
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include "swap.h"
#include <sstream>
#include <string>
#include<string.h>
using namespace std;

#include <unistd.h>   
#include <sys/ioctl.h>

char *c_mac;
const char* cip="224.1.1.3";
#define bc_port 10100

bool local_mac(unsigned char * mac, const char * eth_name)
{
//	printf("local mac\n");
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("%s: Get Local MAC Fail!(socket)\r\n", eth_name);
		return false;
	}

	ifreq ifr;
	strncpy(ifr.ifr_name, eth_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
    //printf("local mac ioctl\n");
	if(ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		printf("%s: Get Local MAC Fail!(ioctl)\r\n", eth_name);
		close(sock);
		return false;
	}
     
   // printf("memcpy(mac, ifr.ifr_addr.sa_data, 6);\n");
	memcpy(mac, ifr.ifr_addr.sa_data, 6);
//	printf("mac:%.2X:%.2X:%.2X:%.2X:%0.2X:%0.2X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
   
	close(sock);
	return true;
}
bool local_netmask(unsigned long & netmask, const char * eth_name)   
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("Error: Get Local NETMASK Fail!(socket)\r\n");
		return false;
	}

	ifreq ifr;
	strncpy(ifr.ifr_name, eth_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if(ioctl(sock, SIOCGIFNETMASK, &ifr) < 0)
	{
		printf("Error: Get Local NETMASK Fail!(ioctl)\r\n");
		close(sock);
		return false;
	}

	memcpy(&netmask, &(((sockaddr_in *)&ifr.ifr_netmask)->sin_addr), sizeof(netmask));

//	printf("netmask:0x%x\r\n", netmask);
	
	close(sock);
	return true;
}

bool local_broadaddr(unsigned long & broadaddr, const char * eth_name)   
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("Error: Get Local BROADADDR Fail!(socket)\r\n");
		return false;
	}

	ifreq ifr;
	strncpy(ifr.ifr_name, eth_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if(ioctl(sock, SIOCGIFBRDADDR, &ifr) < 0)
	{
		printf("Error: Get Local BROADADDR Fail!(ioctl)\r\n");
		close(sock);
		return false;
	}

	memcpy(&broadaddr, &(((sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr), sizeof(broadaddr));

//	printf("broadaddr:0x%x\r\n", broadaddr);
	close(sock);
	return true;
}

bool local_addr(unsigned long & ipaddr, const char * eth_name)   
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
//		printf("Error: Get Local IP Fail!(socket)\r\n");
		return false;
	}

	ifreq ifr;
	strncpy(ifr.ifr_name, eth_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
//		printf("Error: Get Local IP Fail!(ioctl)\r\n");
		close(sock);
		return false;
	}

	memcpy(&ipaddr, &(((sockaddr_in *)&ifr.ifr_addr)->sin_addr), sizeof(ipaddr));
	
//	ipaddr=swap(ipaddr);
//	printf("ip:0x%x\r\n", ipaddr);

	close(sock);
	return true;
}
  
  
  
  
  
#include <arpa/inet.h>  //for in_addr  
#include <linux/rtnetlink.h>    //for rtnetlink  
#include <net/if.h> //for IF_NAMESIZ, route_info  
#include <stdlib.h> //for malloc(), free()  
#include <string.h> //for strstr(), memset()  
  
#include <string>  
  
#define BUFSIZE 8192  
   
struct route_info{  
 u_int dstAddr;  
 u_int srcAddr;  
 u_int gateWay;  
 char ifName[IF_NAMESIZE];  
};  
int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)  
{  
  struct nlmsghdr *nlHdr;  
  int readLen = 0, msgLen = 0;  
  do{  
    //�յ��ں˵�Ӧ��  
    if((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0)  
    {  
      perror("SOCK READ: ");  
      return -1;  
    }  
     
    nlHdr = (struct nlmsghdr *)bufPtr;  
    //���header�Ƿ���Ч  
    if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))  
    {  
      perror("Error in recieved packet");  
      return -1;  
    }  
     
      
    if(nlHdr->nlmsg_type == NLMSG_DONE)   
    {  
      break;  
    }  
    else  
    {  
        
      bufPtr += readLen;  
      msgLen += readLen;  
    }  
     
      
    if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)   
    {  
        
     break;  
    }  
  } while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));  
  return msgLen;  
}  
//�������ص�·����Ϣ  
void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo,char *gateway)  
{  
  struct rtmsg *rtMsg;  
  struct rtattr *rtAttr;  
  int rtLen;  
  char *tempBuf = NULL;  
  struct in_addr dst;  
  struct in_addr gate;  
    
  tempBuf = (char *)malloc(100);  
  rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);  
  // If the route is not for AF_INET or does not belong to main routing table  
  //then return.   
  if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))  
  return;  
    
  rtAttr = (struct rtattr *)RTM_RTA(rtMsg);  
  rtLen = RTM_PAYLOAD(nlHdr);  
  for(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen)){  
   switch(rtAttr->rta_type) {  
   case RTA_OIF:  
    if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);  
    break;  
   case RTA_GATEWAY:  
    rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);  
    break;  
   case RTA_PREFSRC:  
    rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);  
    break;  
   case RTA_DST:  
    rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);  
    break;  
   }  
  }  
  dst.s_addr = rtInfo->dstAddr; 
 // printf("start get  gateway\n"); 
  if (strstr((char *)inet_ntoa(dst), "0.0.0.0"))  
  {  
   // printf("oif:%s\n",rtInfo->ifName);  
    gate.s_addr = rtInfo->gateWay;  
    sprintf(gateway, (char *)inet_ntoa(gate));  
  //  printf("gateway:%s\n",gateway);  
  /*gate.s_addr = rtInfo->srcAddr;  
    printf("src:%s\n",(char *)inet_ntoa(gate));  
    gate.s_addr = rtInfo->dstAddr;  
    printf("dst:%s\n",(char *)inet_ntoa(gate)); */  
  }  
  free(tempBuf);  
  return;  
}  
  
int get_gateway(char *gateway)  
{  
 struct nlmsghdr *nlMsg;  
 struct rtmsg *rtMsg;  
 struct route_info *rtInfo;  
 char msgBuf[BUFSIZE];  
   
 int sock, len, msgSeq = 0;  
  
 if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)  
 {  
  perror("Socket Creation: ");  
  return -1;  
 }  
   
   
 memset(msgBuf, 0, BUFSIZE);  
   
   
 nlMsg = (struct nlmsghdr *)msgBuf;  
 rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);  
   
   
 nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.  
 nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .  
   
 nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.  
 nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.  
 nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.  
   
   
 if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0){  
  printf("Write To Socket Failed��\n");  
  return -1;  
 }  
   
   
 if((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {  
  printf("Read From Socket Failed��\n");  
  return -1;  
 }  
   
 rtInfo = (struct route_info *)malloc(sizeof(struct route_info));  
 for(;NLMSG_OK(nlMsg,len);nlMsg = NLMSG_NEXT(nlMsg,len)){  
  memset(rtInfo, 0, sizeof(struct route_info));  
  parseRoutes(nlMsg, rtInfo,gateway);  
 }  
 free(rtInfo);  
 close(sock);  
 return 0;  
}  


int get_host_info( string& local_ip, string& local_mask )
{
	struct sockaddr_in *my_ip;
	struct sockaddr_in *addr;
	struct sockaddr_in  myip;

	my_ip = &myip;
	struct ifreq ifr;
	int sock;
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return -1;
	}
	strcpy( (char*)(ifr.ifr_name), "eth0" );
	// get local ip
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		return -1;
	}
	my_ip->sin_addr = ((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr;
	local_ip = inet_ntoa(my_ip->sin_addr);

	// get local mask
	if( ioctl( sock, SIOCGIFNETMASK, &ifr) == -1 )
	{
		perror("[-] ioctl");
		return -1;
	}
	addr = (struct sockaddr_in *) & (ifr.ifr_addr);
	local_mask = inet_ntoa( addr->sin_addr);
	close(sock);
	return 0;
}

void * send_ipinfo(void *arg){

	string         local_ip;
	string         local_mask;
	unsigned short local_port;

	if ( get_host_info( local_ip, local_mask ) < 0 )
	{

		perror("get local ip error!\r\n");
		return ((void *)-1);
	}


	int snd_fd = -1;
	snd_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(snd_fd < 0)
	{
		perror("socket");
		exit(1);
	}
		
	struct sockaddr_in mc_addr;
	memset(&mc_addr, 0 , sizeof(mc_addr));
	mc_addr.sin_family = AF_INET;
	mc_addr.sin_port = htons(7890);
	mc_addr.sin_addr.s_addr = inet_addr("224.100.100.100");
	
	while(1)
	{   
		unsigned short len;
		unsigned long  broadaddr;
		unsigned long  ipaddr;
		string str="eth0";
		unsigned char mac[6];
		unsigned long netmask;
		char buff[256]={0};
		unsigned char data[256]={0};
		
		struct in_addr in_netmask;  
		struct in_addr in_localaddr;
		struct in_addr in_broadaddr;
		char  *c_netmask=new char[32];
		char  *c_broadaddr=new char[32];
		char  *c_ipaddr=new char[32];
		char sendbuf[256]={0};
		memset(c_netmask,0,32);
		memset(c_broadaddr,0,32);
		memset(c_ipaddr,0,32);
		memset(data,0,256);
		memset(buff,0,256);
		memset(sendbuf,0,256);
		/*****************************
		mac��ַ
		******************************/
		//  printf("get_mac\n");
		local_mac(mac,str.c_str()); 
		/*****************************
		netmask
		******************************/
		local_netmask(netmask,str.c_str()) ;
		in_netmask.s_addr=netmask; 
		c_netmask=(char *)inet_ntoa(in_netmask);


		sprintf(sendbuf,"0x32-%.2X:%.2X:%.2X:%.2X:%0.2X:%0.2X-%s-", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],c_netmask);
        //printf("sendbuf=%s\n",sendbuf);
        //printf("\n");
        /*****************************
		 �㲥��ַ
		 ******************************/
		local_broadaddr(broadaddr, "eth0") ;
		in_broadaddr.s_addr=broadaddr;
		c_broadaddr=(char *)inet_ntoa(in_broadaddr); 
		c_broadaddr[strlen(c_broadaddr)]='-';
		strcat(sendbuf,c_broadaddr);
		//printf("sendbuf=%s\n",sendbuf);
		//printf("\n");
		/*****************************
		IP ��ַ
		******************************/
		local_addr(ipaddr, "eth0") ;
		in_localaddr.s_addr=ipaddr;
		c_ipaddr=(char *)inet_ntoa(in_localaddr);
		c_ipaddr[strlen(c_ipaddr)]='-';
		strcat(sendbuf,c_ipaddr);  
            
       // printf("sendbuf=%s\n",sendbuf);
       // printf("\n");
        
        /*****************************
		Ĭ�����
		******************************/
		
        get_gateway(buff); 
     // printf("gateway=%s\n",buff); 
        strcat(sendbuf,buff);  
//      printf("sendbuf=%s\n",sendbuf);  
  //      printf("\n");
        /***********************************
         �����ַ�
        ************************************/
        len =strlen(sendbuf);
	//	printf("send net info:%s\n",sendbuf);
	  if(snd_fd>0)
	  {
			int num = sendto(snd_fd, sendbuf, strlen(sendbuf), 0, 
				(struct sockaddr *)&mc_addr, sizeof(mc_addr));
			if(num < 0)
			{
				perror("sendto:::");
				exit(1);
			}
	  }
	  else
	  {
		  printf("snd_fd <0\n");
	  }
	    sleep(1);
	
	}
		 
 	  close(snd_fd);
 	  return ((void *)0);
 }


#endif
