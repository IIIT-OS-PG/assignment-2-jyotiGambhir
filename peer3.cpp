#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <thread>
#include <pthread.h>

using namespace std;

struct threadData{
	int fd;
	int address;
	struct sockaddr_in sa;
};

void* clientRequestServe(void* threadarg)
{	struct threadData *threadStruct;
	threadStruct=(struct threadData *)threadarg;
	int socketfd=threadStruct->fd;
	struct sockaddr_in serveraddr=threadStruct->sa;
	int addrlen=threadStruct->address;
	int connectfd;
	if((connectfd=accept(socketfd,(struct sockaddr*)&serveraddr,(socklen_t*)&addrlen)) < 0)
	{
		perror("Error in accepting");
		exit(EXIT_FAILURE);
	}
	//cout<<"accepted"<<endl;
	cout<<"accepted in server"<<endl;
	FILE *fp=fopen("/home/jyoti/Desktop/os/peer_to_peer/client.cpp","rb");
	fseek(fp,0,SEEK_END);
	int size=ftell(fp);
	rewind(fp);
	send(connectfd,&size,sizeof(size),0);
	fclose(fp);
	close(connectfd);
	//close(socketfd);
}


void* clientFunc(void* threadarg)
{
	cout<<"in client"<<endl;
	int socketfd=socket(AF_INET,SOCK_STREAM,0);
	
	if(socketfd<0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(2020);
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int m;
	cout<<"enter for connect"<<endl;
	cin>>m;
	char buf[2048]={0};
	if(connect(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("unable to connect");
		exit(EXIT_FAILURE);
	}
	cout<<"connected in client"<<endl;
	int file_size;
	recv(socketfd,&file_size,sizeof(file_size),0);
	cout<<file_size<<" in peer1"<<endl;
	FILE *fp=fopen("copy_peer3.txt","wb");
	// int file_size;
	// recv(connectfd,&file_size,sizeof(file_size),0);
	int n;
	while((n=recv(socketfd,buf,2048,0)) > 0 && file_size > 0)
	{
		fwrite(buf,sizeof(char),n,fp);
		//cout<<"Buff"<<buf<<endl;
		memset(buf,'\0',2048);
		file_size=file_size-n;
	} 
	// if(read(socketfd,buf,2048) < 0)
	// {
	// 	perror("unable to read from buffer");
	//  	exit(EXIT_FAILURE);
	// }
	//cout<<"inpeer1 "<<buf;
	//char *hell="Hello I am from Client";
	// char buf[2048];
	// FILE *fp=fopen("/home/jyoti/Desktop/os/peer_to_peer/admin.pdf","rb");
	// fseek(fp,0,SEEK_END);
	// int size=ftell(fp);
	// rewind(fp);
	// send(socketfd,&size,sizeof(size),0);
	// int n;
	// while((n=fread(buf,sizeof(char),2048,fp)) > 0 && size>0)
	// {
	// 	send(socketfd,buf,n,0);
	// 	cout<<"inn buff"<<buf<<endl;
	// 	memset(buf,'\0',2048);
	// 	size=size-n;
	// }
	fclose(fp);
	close(socketfd);

}

void* serverFunc(void* threadarg)
{
	cout<<"in server"<<endl;
	int socketfd=socket(PF_INET,SOCK_STREAM,0);
	
	if(socketfd<0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(2040);
	//cout<<INADDR_ANY;
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	// //cout<<serveraddr.sfamily<<endl;
	// //cout<<serveraddr.sport<<endl;
	// //cout<<serveraddr.saddr.addr<<endl;
	int addrlen=sizeof(serveraddr);
	if(bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("Error in Binding");
		exit(EXIT_FAILURE);
	}
	cout<<"binded in server"<<endl;
	if(listen(socketfd,2) < 0)
	{
		perror("ECONNREFUSED");
		exit(EXIT_FAILURE);
	}
	cout<<"listening in server"<<endl;
	int connectfd;
	while((connectfd=accept(socketfd,(struct sockaddr*)&serveraddr,(socklen_t*)&addrlen)) > 0)
	{	struct threadData th;
		th.fd=connectfd;
		th.sa=serveraddr;
		th.address=addrlen;
		pthread_t newThread;
		pthread_create(&newThread,NULL,clientRequestServe,(void*)&th);
		pthread_join(newThread,NULL);
		// perror("Error in accepting");
		// exit(EXIT_FAILURE);
	}
	// FILE *fp=fopen("/home/jyoti/Desktop/os/peer_to_peer/admin.pdf","rb");
	// fseek(fp,0,SEEK_END);
	// int size=ftell(fp);
	// rewind(fp);
	// send(connectfd,&size,sizeof(size),0);
	// char buf[2048]={0};
	// int n;
	// while((n=fread(buf,sizeof(char),2048,fp)) > 0 && size>0)
	// {
	// 	send(connectfd,buf,n,0);
	// 	//cout<<"inn buff"<<buf<<endl;
	// 	memset(buf,'\0',2048);
	// 	size=size-n;
	// }
	// fclose(fp);

	/*while(listen(socketfd,2) >= 0)
	{
		thread clientRequestServe(clientRequestServeFunc,socketfd,serveraddr,addrlen);
		clientRequestServe.join();
	}*/
	
	// if((connectfd=accept(socketfd,(struct sockaddr*)&serveraddr,(socklen_t*)&addrlen)) < 0)
	// {
	// 	perror("Error in accepting");
	// 	exit(EXIT_FAILURE);
	// }
	// char buf[2048]={0};
	// FILE *fp=fopen("songs1.mp3","wb");
	// int file_size;
	// recv(connectfd,&file_size,sizeof(file_size),0);
	// int n;
	// while((n=recv(connectfd,buf,2048,0)) > 0 && file_size > 0)
	// {
	// 	fwrite(buf,sizeof(char),n,fp);
	// 	//cout<<"Buff"<<buf<<endl;
	// 	memset(buf,'\0',2048);
	// 	file_size=file_size-n;
	// } 
	
	//fclose(fp);
	// char *hell="Hello sent form server";
	// if(read(connectfd,buf,1024) < 0)
	// {
	// 	perror("unable to read from buffer");
	// 	exit(EXIT_FAILURE);
	// }
	// string st=buf;
	// cout<<st;
	// cout<<"accepted in server"<<endl;
	// FILE *fp=fopen("/home/jyoti/Desktop/os/peer_to_peer/client.cpp","rb");
	// fseek(fp,0,SEEK_END);
	// int size=ftell(fp);
	// rewind(fp);
	// send(connectfd,&size,sizeof(size),0);
	close(connectfd);
	close(socketfd);
	// cout<<"Sent";*/
}

int main()
{	pthread_t clientThread,serverThread;
	cout<<"initialized"<<endl;
	pthread_create(&clientThread,NULL,clientFunc,NULL);
	cout<<"client thread returned"<<endl;
	// thread clientThread(clientFunc);
	// thread serverThread(serverFunc);
	pthread_create(&serverThread,NULL,serverFunc,NULL);
	cout<<"server thread returned"<<endl;
	pthread_join(clientThread,NULL);
	pthread_join(serverThread,NULL);
	// clientThread.join();
	// serverThread.join();
}