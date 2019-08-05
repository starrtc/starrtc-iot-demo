#ifdef WIN32

#define socklen_t int

#else

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <netdb.h>

#define SD_BOTH 2
#define closesocket close

#endif
#include <signal.h>
//#include <vector>
#define  INVALID_SOCKET -1
#include <iostream>
using namespace std ;
class simple_tcp{
private:
	int  s;
public:
	simple_tcp():s(INVALID_SOCKET){
		
	}
	~simple_tcp(){
		clear();
	}
	bool listen(const char * addr, int port){
		struct hostent *he = gethostbyname(addr);
		if(he == 0)return false;
        signal(SIGPIPE,SIG_IGN); 

		struct sockaddr_in host;
		host.sin_port = htons(port);
		host.sin_family = AF_INET;
		host.sin_addr = *((struct in_addr *)he->h_addr);

		if(s && s != INVALID_SOCKET)closesocket(s);
		
		s = socket(AF_INET, SOCK_STREAM, 0);
		
		if(s == INVALID_SOCKET)return false;

		if(INVALID_SOCKET == bind(s, (struct sockaddr *)&host, sizeof(host))){
			closesocket(s);
			return false;
		}

		if(::listen(s, 1) == INVALID_SOCKET){
			closesocket(s);
			return false;
		}
		return true;
	}
	void accept(simple_tcp& st){
		st.s = ::accept(s, 0, 0);
	}
	bool connect(const char * addr, int port){
		struct hostent *he = gethostbyname(addr);
		if(he == 0)return false;

		struct sockaddr_in host;
		host.sin_port = htons(port);
		host.sin_family = AF_INET;
		host.sin_addr = *((struct in_addr *)he->h_addr);

		if(s && s != INVALID_SOCKET)closesocket(s);
		s = socket(AF_INET, SOCK_STREAM, 0);
		if(s == INVALID_SOCKET)return false;

		int ret = ::connect(s, (struct sockaddr *) &host, sizeof(host));
		if(ret == -1){
			closesocket(s);
			s = INVALID_SOCKET;
			return false;
		}
		return true;
	}
	void clear(){
		if (s && s != INVALID_SOCKET) {
			shutdown(s, SD_BOTH);
			closesocket(s);
			s = INVALID_SOCKET;
		}
	}
	bool readable(){
		fd_set set;
		FD_ZERO(&set);
		FD_SET(s, &set);

		timeval tv = {0, 0};

		return 1 == select(0, &set, 0, 0, &tv);
	}
	bool recv_some(void *buffer, int *length){
		int ret = ::recv(s, (char*)buffer, *length, 0);
		if(ret == -1 || ret == 0) return false;
		*length = ret;
		return true;
	}
	bool recv(void *buffer, int length){
		int real_recv = 0;
		while(length != 0){
			int ret = ::recv(s, (char*)buffer + real_recv, length, 0);
			if(ret == -1 || ret == 0) return false;
			length -= ret;
			real_recv += ret;
		}
		return true;
	}
	bool send(const void *buffer, int length){
		int real_send = 0;
		while(length != 0){
			int ret = ::send(s, (char*)buffer + real_send, length, 0);
			if(ret == -1 || ret == 0){
		
			 
			return false;
		}
			length -= ret;
			real_send += ret;
		}  
		return true;
	}
};
//vector <sockaddr_in> users ;
class simple_udp{
public:
	int  s;
    struct sockaddr_in clientAddr;
public:
	simple_udp():s(INVALID_SOCKET){
	
	}
	~simple_udp(){
	}
   bool create_socket(int port)
   {
   	struct sockaddr_in host;
    host.sin_family = AF_INET;
    host.sin_port = htons(port);
    host.sin_addr.s_addr = htonl(INADDR_ANY);

    
    if ( (s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }
    if (bind(s, (struct sockaddr *)&host, sizeof(host)) < 0)
    {
        perror("bind");
        exit(1);
    }
 
   }
  
   int  recvclient(char  * buf)
   {
	   int len = sizeof(clientAddr);
	   char mesg[2048];
	   memset(mesg,0,2048);
	   int  n = recvfrom(s, mesg, sizeof(mesg), 0, (struct sockaddr*)&clientAddr, (socklen_t*)&len);

		if(n<0)
			return -1;
	   memcpy(buf,mesg,n);
	   //printf("recv n=%d\n",n);
	   return n;
	   //printf("mesg=%s\n",mesg);
	  //cout<<"mesg"<<mesg[0]<<endl;
#if 0
	   for(int i=0;i<n;i++)
	   {
		  
		   printf("%0x: ",mesg[i]);
	   }
	   if (n>0)
	   {
		   printf("%s %u says: %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), mesg);
		   return n ;
	   }
#endif
   }
     
     int sentclient(void * buf,int length)
     {
      
		    int m = sendto(s, buf,length, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

            if (m < 0)
            {
              perror("sendto");
			  printf("length=%d\n",length);
              
            }
			else
			{
			  
			}

     }
};
