#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/sha.h>
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
	int client_port;
	string client_ip;
	//string filename;
	int tracker_port;
	string tracker_ip;
};

struct serverData{
	int server_port;
	string server_ip;
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

string sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH])
{
   stringstream ss;
   for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string sha256_of_chunks(FILE* f,int size)
{	cout<<"entered to calculate sha";
	if(!f)
		return NULL;

	string finalSha="";
	unsigned char hashcal[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	const int buf=512;
	unsigned char* buffer=(unsigned char*)malloc(buf+1);
	int n=0;
	if(!buffer)
		return NULL;
	while((n=fread(buffer,sizeof(char),buf,f)) > 0 && size>0)
	{
		SHA256_Update(&sha256,buffer,n);
		SHA256_Final(hashcal,&sha256);
		string outBuf=sha256_hash_string(hashcal);
		string ans=outBuf.substr(0,20);
		finalSha += ans;
		memset(buffer,'\0',512);
		size -= n;
	}
	fclose(f);
	return finalSha;
}

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
	
	if(recv(connectfd,&c,sizeof(c),0) < 0)
	{
		perror("error in receiving");
		exit(EXIT_FAILURE);
	}
	int ack;
	send(connectfd,&ack,sizeof(ack),0);
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
	int ack1;
	recv(connectfd,&ack1,sizeof(ack1),0);
	fclose(fp);
	close(connectfd);
	//close(socketfd);
}

void* clientDownloadChunk(void* clientDetails)
{	cout<<"entered clientDownloadChunk:"<<endl;
	struct clientHandleDetails* cdetails;
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
		cout<<"port is"<<cdetails->portnum<<endl;
		cout<<"filename is "<<cdetails->filename;
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
	int ack;
	recv(socketfd,&ack,sizeof(ack),0);
	send(socketfd,filedwn,sizeof(filedwn),0);
	cout<<"file name is "<<filedwn<<endl;
	//send(socketfd,&cnum,sizeof(cnum),0);
	pthread_mutex_lock(&locka);
	FILE *fp=fopen(cdetails->newfilename,"rab+");
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
		
	} 
	fclose(fp);
	int ack2=1;
	send(socketfd,&ack2,sizeof(ack2),0);
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
	int nw=1;
	if(socketfd<0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(d->tracker_port);
	serveraddr.sin_addr.s_addr=inet_addr(d->tracker_ip.c_str());
	struct sockaddr_in clientaddr;
	clientaddr.sin_family=AF_INET;
	clientaddr.sin_port=htons(d->client_port);
	clientaddr.sin_addr.s_addr=inet_addr(d->client_ip.c_str());
	if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &nw, sizeof(nw)))
	{
		cout<<"set socket opt"<<endl;
		pthread_exit(NULL);
	}
	if(bind(socketfd,(struct sockaddr*)&clientaddr,sizeof(clientaddr)) < 0)
	{
		perror("Error in Binding");
		pthread_exit(NULL);
	}
	int m;
	cout<<"press 1 to connect tracker"<<endl;
	cin>>m;
	if(connect(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("unable to connect");
		exit(EXIT_FAILURE);
	}
	cout<<"connected in client"<<endl;
	int ack=1;
	//char inputcom[1024];
	cin.ignore();
	char msg[2048];
	string username="";
		while(1)
		{	string command;
			cout<<"enter command"<<endl;
			
			getline(cin,command);
		
			string command1=command;
			char* comm=strtok((char*)command1.c_str()," ");
			cout<<"command is "<<comm<<endl;
			if(strcmp(comm,"create_user")==0)
			{	cout<<"entered create_user"<<command<<endl;
				//send userid and password
				send(socketfd,(char*)command.c_str(),command.length(),0);
				
				recv(socketfd,&msg,2048,0);
				//string msg1=msg;
				cout<<msg<<endl;
			}

			else if(strcmp(comm,"login")==0)
			{	//set username for this thread
				username=strtok(NULL," ");
				cout<<"in login user"<<endl;
				cout<<"user logged in is "<<username<<endl;
				// send userid and password
				send(socketfd,(char*)command.c_str(),command.length(),0);
				recv(socketfd,&msg,2048,0);
				cout<<msg<<endl;
			}

			else if(strcmp(comm,"create_group")== 0)
			{
				send(socketfd,(char*)command.c_str(),command.length(),0);
				recv(socketfd,&msg,2048,0);
				cout<<msg<<endl;
			}

			else if(strcmp(comm,"join_group") == 0)
			{
				cout<<"in join group req"<<endl;
				send(socketfd,(char*)command.c_str(),command.length(),0);
				cout<<"waiting for msg"<<endl;
				recv(socketfd,&msg,2048,0);	
				cout<<msg<<endl;
				//cout<<"msg received"<<endl;
			}

			else if(strcmp(comm,"leave_group") == 0)
			{
				cout<<"in leave group"<<endl;
				send(socketfd,(char*)command.c_str(),command.length(),0);
				recv(socketfd,&msg,2048,0);
				cout<<msg<<endl;
			}

			else if(strcmp(comm,"list_requests") == 0)
			{	//char* msg;
				cout<<"in list req"<<endl;
				send(socketfd,(char*)command.c_str(),command.length(),0);
				char req[2048];
				cout<<"List of Pending Requests"<<endl;
				recv(socketfd,&req,2048,0);
				cout<<req<<endl;
				if(strcmp(req,"Group_Id_Doesn't_Exists")==0)
				{	//cout<<"sending ack"<<endl;
					send(socketfd,&ack,sizeof(ack),0);
					//cout<<"ack sent "<<endl;
					//recv(socketfd,msg,2048,0);
					//int pp=0;
					//char jk[2048];
					recv(socketfd,&msg,2048,0);
					cout<<msg<<endl;
					//cout<<msg<<endl;
					//send(socketfd,&ack,sizeof(ack),0);

				}
				else
				{	int mp;
					send(socketfd,&ack,sizeof(ack),0);
					recv(socketfd,&mp,sizeof(mp),0);
					cout<<"n is "<<mp<<endl;
					send(socketfd,&ack,sizeof(ack),0);
					memset(req,'\0',1024);
					while(mp--)
					{
						recv(socketfd,&req,1024,0);
						cout<<req<<endl;
						memset(req,'\0',1024);
						send(socketfd,&ack,ack,0);
					}
					recv(socketfd,&ack,sizeof(ack),0);
				}
				

			}

			else if(strcmp(comm,"accept_request")==0)
			{	//char* msg;
				send(socketfd,(char*)command.c_str(),command.length(),0);
				recv(socketfd,&msg,2048,0);
				cout<<msg<<endl;
			}

			else if(strcmp(comm,"list_groups")==0)
			{	
				send(socketfd,(char*)command.c_str(),command.length(),0);
				
				int n;
				recv(socketfd,&n,sizeof(n),0);
				//cout<<"n is "<<n<<endl;
				send(socketfd,&ack,sizeof(ack),0);
				while(n--)
				{	memset(msg,'\0',2048);
					recv(socketfd,&msg,2048,0);
					cout<<msg<<endl;
					
					send(socketfd,&ack,sizeof(ack),0);
				}
				//cout<<"ack received "<<endl;
				recv(socketfd,&ack,sizeof(ack),0);
				//send(socketfd,&ack,sizeof(ack),0);
				//}
			}

			else if(strcmp(comm,"upload_file")==0)
			{
				cout<<"entered upload file command";
				//send(socketfd,(char*)command.c_str(),command.length(),0);
				//string file_to_be_uploaded;
				char* file_to_be_uploaded=strtok(NULL," ");
				FILE *fp=fopen(file_to_be_uploaded,"rb");
				fseek(fp,0,SEEK_END);
				int size=ftell(fp);
				rewind(fp);
				//fclose(fp);
				string shaRecv=sha256_of_chunks(fp,size);
				string sendData=command+" "+to_string(size);
				cout<<sendData<<endl;
				cout<<shaRecv.length()<<" "<<shaRecv<<endl;
				send(socketfd,(char*)sendData.c_str(),sendData.length(),0);
				int m;
				recv(socketfd,&m,sizeof(m),0);
				if(m==1)
				{	cout<<"received m"<<m<<endl;
					cout<<"Group Id not present"<<endl;
					continue;
				}
				if(m==2)
				{	cout<<"received m"<<m<<endl;
					cout<<"User Id not present in Group"<<endl;
					continue;
				}
				else
				{
					cout<<"received m"<<m<<endl;
					recv(socketfd,&ack,sizeof(ack),0);
					int n=shaRecv.length();
					int i=0;
					while(i!=shaRecv.length())
					{	cout<<"sending "<<shaRecv.substr(i,20)<<endl;
						send(socketfd,(char*)shaRecv.substr(i,20).c_str(),20,0);
						recv(socketfd,&ack,sizeof(ack),0);
						i+=20;
					}
					char* msg="end";
					send(socketfd,msg,sizeof(msg),0);
					recv(socketfd,&ack,sizeof(ack),0);
				}
			}

			/*else if(strcmp(comm,"download_file")==0)
			{	send(socketfd,(char*)command.c_str(),command.length(),0);
				char* gid;
				gid=strtok(NULL," ");
				char* fileDownload;
				//cout<<"enter filename to download"<<endl;
				//cin>>fileDownload;
				fileDownload=strtok(NULL," ");
				// char* dest;
				// dest=strtok(NULL," ");
				char buf[256]={0};
				
				//send(socketfd,fileDownload,sizeof(fileDownload),0);
				//int ack;
				int file_size;
				recv(socketfd,&file_size,sizeof(file_size),0);
				cout<<file_size<<" in peer1"<<endl;
				char* newFile;
				//cout<<"enter filename for new file"<<endl;
				newFile=strtok(NULL," ");
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
				cout<<"Null initialised file created"<<endl;
				int chunksize=512;
				int no_of_chunks=ceil((float)file_size/chunksize);
				int p=no_of_chunks;
				cout<<"p is "<<p<<endl;
				send(socketfd,&ack,sizeof(ack),0);
				string recvSha="";
				memset(msg,'\0',2048);
				while(p--)
				{	cout<<"p is "<<p<<endl;
					recv(socketfd,&msg,2048,0);
					recvSha+=msg;
					memset(msg,'\0',2048);
					send(socketfd,&ack,sizeof(ack),0);
				}
				cout<<recvSha<<endl;
				/*int clientnum=3;
				int portnum;
				pthread_t clientAvail[clientnum];
				int arr[3]={6000,7000,8900};
				int ch[3]={3,2,1};
				int i=0;
				while(clientnum--)
				{	//struct clientHandleDetails cd;
					struct clientHandleDetails *cd = (struct  clientHandleDetails*)malloc(sizeof(struct clientHandleDetails));
					// cout<<"Enter client port details to fetch data from:"<<endl;
					// cin>>cd.portnum;
					// cout<<"Enter chunknum"<<endl;
					// //int a;
					// cin>>cd.chunknum;
					cd->portnum=arr[i];
					cd->chunknum=ch[i];
					cd->ipaddr="127.0.0.1";
					cd->filename=fileDownload;
					cd->newfilename=newFile;
					//pthread_t clientAvail[i];
					// cout<<"in main port number "<<cd.portnum;
					// cout<<"in main ip "<<cd.ipaddr;
					// cout<<"in main file to be downloaded "<<cd.filename<<endl;
					// cout<<"new ile to be created "<<cd.newfilename<<endl;
					// cout<<"new chunk "<<cd.chunknum<<endl;
					pthread_create(&clientAvail[i],NULL,&clientDownloadChunk,(void*)cd);
						//	pthread_join(clientAvail[i],NULL);

					i++;
					//pthread_join(clientAvail[i],NULL);
				}
				for(int i=0; i<3; i++)
					pthread_join(clientAvail[i],NULL);*/
		
			//}
			cout<<"executed command"<<endl;
		}
	close(socketfd);

}

void* serverFunc(void* threadarg)
{	struct serverData *d;
	d=(struct serverData*)threadarg;
	// cout<<"in server"<<endl;
	// cout<<d->filename<<" filename"<<endl;
	int socketfd=socket(PF_INET,SOCK_STREAM,0);
	int nw=1;
	if(socketfd<0)
	{
		perror("socket failed");
		pthread_exit(NULL);
	}
	struct sockaddr_in serveraddr;
	
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(d->server_port);
	//cout<<INADDR_ANY;
	serveraddr.sin_addr.s_addr=inet_addr(d->server_ip.c_str());
	// //cout<<serveraddr.sfamily<<endl;
	// //cout<<serveraddr.sport<<endl;
	// //cout<<serveraddr.saddr.addr<<endl;
	if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&nw,sizeof(nw))<0)
	{
		cout<<"set socket opt"<<endl;
		pthread_exit(NULL);
	}


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
	struct serverData serverDetails;
	cout<<"Port number for server"<<endl;
	cin>>serverDetails.server_port;
	cout<<"IP for server"<<endl;
	cin>>serverDetails.server_ip;
	// cout<<"enter file path to be downloaded"<<endl;
	// cin>>serverDetails.filename;
	//cout<<serverDetails.filename<<" filename"<<endl;
	struct csData clientDetails;
	clientDetails.client_port=serverDetails.server_port;
	clientDetails.client_ip=serverDetails.server_ip;
	cout<<"Port number for client to connect tracker"<<endl;
	cin>>clientDetails.tracker_port;
	cout<<"IP for tracker"<<endl;
	//clientDetails.clientport=serverDetails.port;
	cin>>clientDetails.tracker_ip;
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