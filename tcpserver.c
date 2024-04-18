#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define servportno 45566

typedef struct pmstr
{
	int sockfd;
} pmstr_t;

typedef struct chatroome
{
	int sockfd;
	char *handler;
	struct  chatroome *next;
} chatroommember;

chatroommember *proom1, *proom2, *proom3;

pthread_mutex_t lock;
void *client_routine(pmstr_t *pmstrpara);

void chatroominsert(int roomno, int sockfd, char *handler);
void chatroomdelete(int roomno, int sockfd);
int chatroomcheck(int roomno, int sockfd); // return 1: successful; return 0: failed


int main(int argc, char *argv[])
{
     fprintf(stderr,"Server is initializing......\n");
     int sockfd, newsockfd, portno, clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int i,n;
     pthread_t clientth[100];
     pmstr_t *pmthread;

     n=0;
     proom1 = NULL;
     proom2 = NULL;
     proom3 = NULL;
     pthread_mutex_init(&lock, NULL);

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
        fprintf(stderr,"ERROR opening socket");
        exit(0);
	 }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = servportno;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
     	fprintf(stderr,"ERROR on binding");
        exit(0);
	 }

	 fprintf(stderr, "Server is up now.\n");

     while (1) {
     	listen(sockfd,5);
     	clilen = sizeof(cli_addr);
     	newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
     	if (newsockfd < 0)
            fprintf(stderr,"ERROR on accept");
        else {
			pmthread = (pmstr_t *) malloc(sizeof(pmstr_t));
        	pmthread->sockfd = newsockfd;
        	pthread_create(&clientth[n++], NULL, (void *)client_routine, (void *)pmthread);
		}
	 }

	 for(i=0;i<n;i++)
	 	pthread_join(clientth[i],NULL);

     return 0;
}

void *client_routine(pmstr_t *pmstrpara)
{
	int n,i,j,roomno,joined,usedhandle;
	char buffer[256],part1[50],part2[50],part3[256];
	char *texttogo, *handler;
	chatroommember *p;

    while (1) {
    	bzero(buffer,256);
    	n = read(pmstrpara->sockfd,buffer,255);
    	if (n < 0)
    		fprintf(stderr,"ERROR reading from socket");
    	else {
			i=j=0;
			do {
				part1[j++]=buffer[i++];
			}while((buffer[i]!=' ') && (buffer[i]!=0) );
			part1[j]=0;

			i++;
			j=0;
			do {
				part2[j++]=buffer[i++];
			}while((buffer[i]!=' ') && (buffer[i]!=0));
			part2[j]=0;
			roomno=atoi(part2);

			i++;
			j=0;
			do {
				part3[j++]=buffer[i++];
			}while(buffer[i]!=0);
			part3[j]=0;

			if(!strcmp(part1,"join")) 
			{
				joined = chatroomcheck(roomno, pmstrpara->sockfd);
				if(joined) 
				{
					n = write(pmstrpara->sockfd, "Already joined this room", 24);
					if (n < 0)
					{
						fprintf(stderr, "ERROR writing to socket\n");
					}
				} 
				else 
				{
					chatroominsert(roomno, pmstrpara->sockfd, handler);
					n = write(pmstrpara->sockfd, "Joined the room successfully", 30);
					if (n < 0)
					{
						fprintf(stderr, "ERROR writing to socket\n");
					}
				}
			}
			else if (!strcmp(part1,"exit")) 
			{
				chatroomdelete(roomno, pmstrpara->sockfd);
				n = write(pmstrpara->sockfd, "Exited the room", 15);
				if (n < 0)
				{
					fprintf(stderr, "ERROR writing to socket\n");
				}				
			}
			else if (!strcmp(part1,"msg")) 
			{
				int roomNumber = atoi(part2);
				char *messageText = part3;

				chatroommember *currentRoom = NULL;
				switch (roomNumber)
				{
					case 1: currentRoom = proom1; break;
					case 2: currentRoom = proom2; break;
					case 3: currentRoom = proom3; break;
					default:
						n = write(pmstrpara->sockfd, "Invalid room number", 20);
						if(n < 0)
						{
							fprintf(stderr, "ERROR writing to socket\n");
						}
						break;
				}

				if (currentRoom != NULL)
				{
					while (currentRoom != NULL)
					{
						if (currentRoom->sockfd == pmstrpara->sockfd)
						{
							handler = currentRoom->handler;
						}
						if (currentRoom->sockfd != pmstrpara->sockfd)
						{
							char messageToSend[512];
							snprintf(messageToSend, sizeof(messageToSend), "[%s, Room %d]: %s", handler, roomNumber, messageText);
							n = write(currentRoom->sockfd, messageToSend, strlen(messageToSend));
							if (n < 0)
							{
								fprintf(stderr, "ERROR writing to socket\n");
							}
						}
						currentRoom = currentRoom->next;
					}
					n = write(pmstrpara->sockfd, "Your message has been successfully distrubuted", 46);
					if (n < 0)
					{
						fprintf(stderr, "ERROR writing to socket\n");
					}
				}
			}
			else {
				n = write(pmstrpara->sockfd,"Unrecognized command",20);
		    	if (n < 0)
		    		fprintf(stderr,"ERROR writing to socket\n");
			}
		}

	}

	close(pmstrpara->sockfd);
}

void chatroominsert(int roomno, int sockfd, char *handler)
{
	chatroommember *p;
	p=(chatroommember *)malloc(sizeof(chatroommember));
	p->sockfd = sockfd;
	p->handler = strdup(handler);

	switch (roomno) {
		case 1: p->next = proom1; proom1 = p; break;
		case 2: p->next = proom2; proom2 = p; break;
		case 3: p->next = proom3; proom3 = p; break;
		default: break;
	}
}

void chatroomdelete(int roomno, int sockfd)
{
	chatroommember *proom, *p, *pprevious;
	switch(roomno) {
		case 1: proom = proom1; break;
		case 2: proom = proom2; break;
		case 3: proom = proom3; break;
		default:break;
	}
	p = proom;
	pprevious = p;
	while(p)
	{
		if ((p->sockfd)==(sockfd))
			break;
		else
		{
			pprevious = p;
			p=p->next;
		}
	}

	if(p==proom)
		proom=p->next;
	else
		pprevious->next=p->next;

	switch(roomno) {
			case 1: proom1 = proom; break;
			case 2: proom2 = proom; break;
			case 3: proom3 = proom; break;
			default:break;
	}

	free(p);
}

int chatroomcheck(int roomno, int sockfd)
{
	chatroommember *p;
	switch(roomno) {
		case 1: p = proom1; break;
		case 2: p = proom2; break;
		case 3: p = proom3; break;
		default:break;
	}

	while(p)
	{
		if ((p->sockfd)==(sockfd))
			return 1;
		else
			p=p->next;
	}
	return 0;
}
