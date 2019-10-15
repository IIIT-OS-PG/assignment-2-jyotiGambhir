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
	int socketfd=socket(PF_INET,SOCK_STREAM,0);
	
	if(socketfd<0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(2000);
	//cout<<INADDR_ANY;
	serveraddr.sin_addr.s_addr=inet_addr("10.1.100.250");
	// //cout<<serveraddr.sfamily<<endl;
	// //cout<<serveraddr.sport<<endl;
	// //cout<<serveraddr.saddr.addr<<endl;
	int addrlen=sizeof(serveraddr);
	if(bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("Error in Binding");
		exit(EXIT_FAILURE);
	}
	if(listen(socketfd,5) < 0)
	{
		perror("ECONNREFUSED");
		exit(EXIT_FAILURE);
	}
	int connectfd;
	
	if((connectfd=accept(socketfd,(struct sockaddr*)&serveraddr,(socklen_t*)&addrlen)) < 0)
	{
		perror("Error in accepting");
		exit(EXIT_FAILURE);
	}
	char buf[2048]={0};
	FILE *fp=fopen("songs1.mp3","wb");
	int file_size;
	recv(connectfd,&file_size,sizeof(file_size),0);
	int n;
	while((n=recv(connectfd,buf,2048,0)) > 0 && file_size > 0)
	{
		fwrite(buf,sizeof(char),n,fp);
		//cout<<"Buff"<<buf<<endl;
		memset(buf,'\0',2048);
		file_size=file_size-n;
	} 
	close(connectfd);
	close(socketfd);
	fclose(fp);
	// char *hell="Hello sent form server";
	// if(read(connectfd,buf,1024) < 0)
	// {
	// 	perror("unable to read from buffer");
	// 	exit(EXIT_FAILURE);
	// }
	// string st=buf;
	// cout<<st;
	// send(connectfd,hell,strlen(hell),0);
	// cout<<"Sent";
}
