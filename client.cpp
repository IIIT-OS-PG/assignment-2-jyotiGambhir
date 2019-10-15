#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

int main()
{
	int socketfd=socket(AF_INET,SOCK_STREAM,0);
	
	if(socketfd<0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(2000);
	serveraddr.sin_addr.s_addr=INADDR_ANY;
	if(connect(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("unable to connect");
		exit(EXIT_FAILURE);
	}
	//char *hell="Hello I am from Client";
	char buf[2048];
	FILE *fp=fopen("/home/jyoti/Desktop/os/peer_to_peer/admin.pdf","rb");
	fseek(fp,0,SEEK_END);
	int size=ftell(fp);
	rewind(fp);
	send(socketfd,&size,sizeof(size),0);
	int n;
	while((n=fread(buf,sizeof(char),2048,fp)) > 0 && size>0)
	{
		send(socketfd,buf,n,0);
		cout<<"inn buff"<<buf<<endl;
		memset(buf,'\0',2048);
		size=size-n;
	}
	fclose(fp);
	close(socketfd);
	// send(socketfd,hell,strlen(hell),0);
	// cout<<"Hello sent"<<endl;
	// if(read(socketfd,buf,1024) < 0)
	// {
	// 	perror("unable to read from buffer");
	// 	exit(EXIT_FAILURE);
	// }
	// string st=buf;
	// cout<<buf;

}