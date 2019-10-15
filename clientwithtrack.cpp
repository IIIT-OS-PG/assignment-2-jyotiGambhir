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

pthread_mutex_t locka;

struct threadData{
	int fd;
	//string filename;
	// int address;
	// struct sockaddr_in sa;
};

struct csData{
	int port;
	//string filename;
	int clientport;
};

struct clientHandleDetails
{
	char* ipaddr;
	int portnum;
	char* filename;
	int chunknum;
	char* newfilename;
	//SHA
};

void* clientRequestServe(void* threadarg)
{	struct threadData *threadStruct;
	//cout<<((struct threadData *)threadarg)->filename<<"\n";
	threadStruct=(struct threadData *)threadarg;
	int connectfd=threadStruct->fd;
	// cout<<" received "<<threadStruct->filename<<endl;
	// string filename=threadStruct->filename;
	// struct sockaddr_in serveraddr=threadStruct->sa;
	// int addrlen=threadStruct->address;
	// int connectfd;
	// if((connectfd=accept(socketfd,(struct sockaddr*)&serveraddr,(socklen_t*)&addrlen)) < 0)
	// {
	// 	perror("Error in accepting");
	// 	exit(EXIT_FAILURE);
	// }
	//cout<<"accepted"<<endl;
	cout<<"accepted in server"<<endl;
	//cout<<filename<<" in serve request"<<endl;
	
	// fseek(fp,0,SEEK_END);
	// int size=ftell(fp);
	// rewind(fp);
	char filename[100];
	int c;
	//recv(connectfd,filename,sizeof(filename),0);
	// recv(connectfd,&filename,sizeof(filename),0);
	cout<<"received filename "<<filename<<endl;
	
	if(recv(connectfd,&c,sizeof(c),0) < 0)
	{
		perror("error in receiving");
		exit(EXIT_FAILURE);
	}
	recv(connectfd,filename,sizeof(filename),0);
	cout<<"received filename "<<filename<<endl;
	cout<<"received chunkum "<<c<<endl;
	FILE *fp=fopen(filename,"rb");

	fseek(fp,(c-1)*512,SEEK_SET);
	//send(connectfd,&size,sizeof(size),0);
	int n;
	int size=512;
	char buf[256]={0};
	while((n=fread(buf,sizeof(char),256,fp)) > 0 && size>0)
	{
		send(connectfd,buf,n,0);
		cout<<"inn buff"<<buf<<endl;
		memset(buf,'\0',256);
		size=size-n;
	}
	fclose(fp);
	close(connectfd);
	//close(socketfd);
}

void* clientDownloadChunk(void* clientDetails)
{	cout<<"entered clientDownloadChunk:"<<endl;
	struct clientHandleDetails *cdetails;
	cdetails=(struct clientHandleDetails*)clientDetails;
	int socketfd=socket(AF_INET,SOCK_STREAM,0);
	
	if(socketfd<0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(cdetails->portnum);
	serveraddr.sin_addr.s_addr=inet_addr(cdetails->ipaddr);
	char buf[256]={0};
	int m;
	// cout<<"Press 1 to conitnue downloading"<<endl;
	// cin>>m;
	if(connect(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("unable to connect");
		exit(EXIT_FAILURE);
	}
	cout<<"connected"<<endl;
	//cout<<"file in client thread "<<cdetails->filename<<endl;
	//cout<<"chunknum is "<<cdetails->chunknum<<endl;
	//char* st="hi";
	//send(socketfd,&st,sizeof(st),0);
	char* filedwn=cdetails->filename;
	
	//send(socketfd,filedwn,sizeof(filedwn),0);
	int cnum=cdetails->chunknum;
	cout<<"cnum is "<<cnum<<endl;
	//int pp=3;
	if(send(socketfd,&cnum,sizeof(cnum),0)<0)
	{	perror("error in sending");
		exit(EXIT_FAILURE);
	}
	send(socketfd,filedwn,sizeof(filedwn),0);
	cout<<"file name is "<<filedwn<<endl;
	//send(socketfd,&cnum,sizeof(cnum),0);
	pthread_mutex_lock(&locka);
	FILE *fp=fopen(cdetails->newfilename,"ab+");
	int chunksize=512;
	int n;

	
	rewind(fp);
	fseek(fp,(cdetails->chunknum-1)*512,SEEK_SET);
	
	
	while((n=recv(socketfd,buf,256,0)) > 0 && chunksize > 0)
	{
		fwrite(buf,sizeof(char),n,fp);
		cout<<"Buff"<<buf<<endl;
		memset(buf,'\0',256);

		chunksize=chunksize-n;
		cout<<"CHUNKSIZE "<<chunksize<<endl;
		fclose(fp);
	} 
	cout<<"releasing lock"<<endl;
	pthread_mutex_unlock(&locka);
	cout<<"lock released"<<endl;
	
	close(socketfd);

}


void* clientFunc(void* threadarg)
{	struct csData *d;
	d=(struct csData*)threadarg;
	cout<<"in client"<<endl;
	int socketfd=socket(AF_INET,SOCK_STREAM,0);
	
	if(socketfd<0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(d->port);
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int m;
	if(connect(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("unable to connect");
		exit(EXIT_FAILURE);
	}
	cout<<"connected in client"<<endl;
	char inputcom[1024];
	cout<<"enter command"<<endl;
	cin>>inputcom;
	char* comm=strtok(inputcom," ");
	if(strcmp(comm,"create_user")==0)
	{
		char* userid=strtok(NULL," ");
		char* passwd=strtok(NULL," ");
		char* portnum;
		sprintf(portnum,"%d",d->clientport);
		char* ipnum="127.0.0.1";
		char comp[1024];
		strcpy(comp,userid);
		strcat(comp,";");
		strcat(comp,passwd);
		strcat(comp,";");
		strcat(comp,ipnum);
		strcat(comp,";");
		strcat(comp,portnum);
		cout<<comp<<endl;
		//send(socketfd,comp,sizeof(comp),0);
		//cout<<"sent"<<endl;


	}
	else if(strcmp(comm,"yeye")==0)
	{


	char fileDownload[1024];
	cout<<"enter filename to download"<<endl;
	cin>>fileDownload;
	char buf[256]={0};
	
	send(socketfd,fileDownload,sizeof(fileDownload),0);
	int file_size;
	recv(socketfd,&file_size,sizeof(file_size),0);
	cout<<file_size<<" in peer1"<<endl;
	char newFile[100];
	cout<<"enter filename for new file"<<endl;
	cin>>newFile;
	FILE *fp=fopen(newFile,"wb+");
	// int file_size;
	// recv(connectfd,&file_size,sizeof(file_size),0);
	int n;
	memset(buf,'\0',256);
	int tempsize;
	tempsize=file_size;
	while(tempsize > 0)
	{	if(tempsize>=256)
			fwrite(buf,sizeof(char),256,fp);
		//cout<<"Buff"<<buf<<endl;
		else
			fwrite(buf,sizeof(char),tempsize,fp);
		tempsize=tempsize-256;
	}
	fclose(fp);
	int clientnum=3;
	int portnum;
	pthread_t clientAvail[clientnum];
	int arr[3]={6000,7000,8000};
	int ch[3]={3,2,1};
	int i=0;
	while(clientnum--)
	{	struct clientHandleDetails cd;
		// cout<<"Enter client port details to fetch data from:"<<endl;
		// cin>>cd.portnum;
		// cout<<"Enter chunknum"<<endl;
		// //int a;
		// cin>>cd.chunknum;
		cd.portnum=arr[i];
		cd.chunknum=ch[i];
		cd.ipaddr="127.0.0.1";
		cd.filename=fileDownload;
		cd.newfilename=newFile;
		//pthread_t clientAvail[i];
		cout<<"in main file to be downloaded "<<cd.filename<<endl;
		cout<<"new ile to be created "<<cd.newfilename<<endl;
		cout<<"new chunk "<<cd.chunknum<<endl;
		pthread_create(&clientAvail[i],NULL,clientDownloadChunk,(void*)&cd);
		i++;
		//pthread_join(clientAvail[i],NULL);
	}
	for(int i=0; i<3; i++)
		pthread_join(clientAvail[i],NULL);
	// while((n=recv(socketfd,buf,2048,0)) > 0 && file_size > 0)
	// {
	// 	fwrite(buf,sizeof(char),n,fp);
	// 	//cout<<"Buff"<<buf<<endl;
	// 	memset(buf,'\0',2048);
	// 	file_size=file_size-n;
	// } 
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
	}
	close(socketfd);

}

void* serverFunc(void* threadarg)
{	struct csData *d;
	d=(struct csData*)threadarg;
	// cout<<"in server"<<endl;
	// cout<<d->filename<<" filename"<<endl;
	int socketfd=socket(PF_INET,SOCK_STREAM,0);
	
	if(socketfd<0)
	{
		perror("socket failed");
		pthread_exit(NULL);
	}
	struct sockaddr_in serveraddr;
	
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(d->port);
	//cout<<INADDR_ANY;
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	// //cout<<serveraddr.sfamily<<endl;
	// //cout<<serveraddr.sport<<endl;
	// //cout<<serveraddr.saddr.addr<<endl;
	int addrlen=sizeof(serveraddr);
	if(bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("Error in Binding");
		pthread_exit(NULL);
	}
	cout<<"binded in server"<<endl;
	if(listen(socketfd,5) < 0)
	{
		perror("ECONNREFUSED");
		pthread_exit(NULL);
	}
	cout<<"listening in server"<<endl;
	int connectfd;
	//while(1)
	//{
		while((connectfd=accept(socketfd,(struct sockaddr*)&serveraddr,(socklen_t*)&addrlen)) > 0)
		{	cout<<"entered"<<endl;
			struct threadData th;
			th.fd=connectfd;
			//th.filename=d->filename;
			// th.sa=serveraddr;
			// th.address=addrlen;
			//cout<<th.filename<<" craeting thread"<<endl;
			pthread_t newThread;
			pthread_create(&newThread,NULL,clientRequestServe,(void*)&th);
			pthread_detach(newThread);
		// perror("Error in accepting");
		// exit(EXIT_FAILURE);
		}
	//} 
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
	//close(connectfd);
	close(socketfd);
	// cout<<"Sent";*/
}

int main()
{	pthread_t clientThread,serverThread;
	cout<<"initialized"<<endl;
	struct csData serverDetails;
	cout<<"Port number for server"<<endl;
	cin>>serverDetails.port;
	// cout<<"enter file path to be downloaded"<<endl;
	// cin>>serverDetails.filename;
	//cout<<serverDetails.filename<<" filename"<<endl;
	struct csData clientDetails;
	cout<<"Port number for client to connect tracker"<<endl;
	cin>>clientDetails.port;
	clientDetails.clientport=serverDetails.port;
	if(pthread_mutex_init(&locka,NULL) != 0)
	{
		cout<<"mutex init failed"<<endl;
	}
	// cout<<"enter new file path"<<endl;
	// cin>>clientDetails.filename;
	pthread_create(&clientThread,NULL,clientFunc,(void*)&clientDetails);

	cout<<"client thread returned"<<endl;
	// thread clientThread(clientFunc);
	// thread serverThread(serverFunc);
	pthread_create(&serverThread,NULL,serverFunc,(void*)&serverDetails);
	cout<<"server thread returned"<<endl;
	pthread_join(clientThread,NULL);
	pthread_join(serverThread,NULL);
	// clientThread.join();
	// serverThread.join();
}