#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define MAX_MESSAGE_SIZE 256

#define servportno 45566



void *routine_write( );
void *routine_read();

//Global variable
int sockfd;


int main(int argc, char *argv[])
{
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    pthread_t thread_r, thread_w;

    portno = servportno;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr,"ERROR opening socket\n");
        exit(0);
	}

    server = gethostbyname("turing.csce.uark.edu");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        fprintf(stderr,"ERROR connecting\n");
        exit(0);
	}
    else
        fprintf(stderr,"\nThe communication channel has been connected successfully! Ready to go!\n");

	fprintf(stderr,"\nWelcome to the Chat Center\n");
	fprintf(stderr,"There are three chat room available and you can join anyone freely.\n");
	fprintf(stderr,"To operate properly, please follow three following formats when you join, exit, or send message. Otherwise, your request will be rejected by the server\n");
	fprintf(stderr,"1, to join one chat room: join <chat room number> <handler>\n");
	fprintf(stderr,"2, to exit one chat room: exit <chat room number>\n");
	fprintf(stderr,"3, to send one message to one chat room: msg <chat room number> <message>\n");
	fprintf(stderr,"THE COMMANDS, i.e. join, exit, msg, SHOULD BE IN lowcase. Enjoy:)\n\n");

    // create one thread to write to the socket
    if(pthread_create(&thread_w, NULL, routine_write, NULL))
    {
        fprintf(stderr, "Error creating thread for writing\n");
        exit(1);
    }

    // create one thread to read from the socket
    if(pthread_create(&thread_r, NULL, routine_read, NULL))
    {
        fprintf(stderr, "Error creating thread for reading\n");
        exit(1);
    }





    //join or cancel threads

    pthread_join(thread_w, NULL);
    pthread_join(thread_r, NULL);

	close(sockfd);

    return 0;
}

void *routine_write()
{
    char buffer[MAX_MESSAGE_SIZE];

    while (1) {
        // Read user input
        fgets(buffer, MAX_MESSAGE_SIZE, stdin);

        // Remove newline character from input
        buffer[strcspn(buffer, "\n")] = 0;

        // Send the message to the server
        if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            exit(1);
        }
    }

    return NULL;

}

void *routine_read()
{
    char buffer[MAX_MESSAGE_SIZE];
    int bytes_received;

    while (1) {
        // Receive messages from the server
        bytes_received = recv(sockfd, buffer, MAX_MESSAGE_SIZE - 1, 0);
        if (bytes_received == -1) {
            perror("recv");
            exit(1);
        } else if (bytes_received == 0) {
            printf("Server closed the connection\n");
            exit(0);
        } else {
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
        }
    }

    return NULL;

}



