 
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 80
#define PORT 8888
#define SA struct sockaddr
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

int count = 0;
int msgcount = 0;
char fmsg[181];
char buff[181];
pthread_t chat;

typedef struct{
    char name[MAX];
    int fd;
    bool online;
} client_t;
typedef struct{
    char from[MAX];
    char to[MAX];
    char message[180];
    bool status;
}Msg_t;


client_t client[100];
Msg_t messages[300];

void ClientInfo(client_t cl)
{
    printf("\nName: %s\n",cl.name);
    printf("ID: %i\n",cl.fd);
}
void MSGinfo(Msg_t msg)
{
    printf("\nFrom: %s\n",msg.from);
    printf("To: %s",msg.to);
    printf("Message: %s\n",msg.message);
}

void DataEnter(client_t cl)
{
   FILE *file = fopen("users.txt", "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return;
    }

    bool username_exists = false;
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character from the line
        line[strcspn(line, "\n")] = 0;

        if (strstr(line, cl.name) != NULL) {
            username_exists = true;
            write(cl.fd,"\nUsername exists.\n", sizeof("\nUsername exists.\n"));
            break;
        }
    }

    fclose(file);
    if (!username_exists) {
        file = fopen("users.txt", "a");
        if (file == NULL) {
            printf("Failed to open the file.\n");
            return;
        }
        fprintf(file, "%i-%s\n",cl.fd ,cl.name);
        fclose(file);

    } else {
        printf("Username already exists in the file.\n");
    }

}
int FDfinder(Msg_t msg,client_t cl)
{
	FILE *fp;
	int find_result = 0;
	char temp[512];
    char fd;
	fp = fopen("users.txt", "r");

	while(fgets(temp, 512, fp) != NULL) {
		if((strstr(temp, msg.to)) != NULL) {
            fd = temp[find_result];
			find_result++;
		}
	}
	if(find_result == 0) {
        write(cl.fd,"\ncouldn't find a user.\n", sizeof("\ncouldn't find a user.\n"));
	}
	//Close the file if still open.
	if(fp) {
		fclose(fp);
	}
    fd = fd - '0';
   	return(fd);
}

void *Mthread(void* abc)
{
    for(;;)
    {
        int lcount = 0;
        if(messages[msgcount].status == true)
        {
        for(int i = 0; messages[msgcount].from[i] != 0; i++)
        {
            fmsg[i] = messages[msgcount].from[i];
            lcount  = i;
        }
        fmsg[lcount+1] = ';';
        for(int i = 0; messages[msgcount].message[i] != 0; i++)
        {
            fmsg[lcount+i+2] = messages[msgcount].message[i];
        }

        write(FDfinder(messages[msgcount],client[count]), fmsg, sizeof(fmsg));
        bzero(fmsg,sizeof(fmsg));
        bzero(&buff, sizeof(buff));
        messages[msgcount].status = false;

        }
    }
    return NULL;
}


// Message reader
void *func(int connfd)
{
    read(connfd, buff, sizeof(buff));
    client[count].fd = connfd;
    strcpy( client[count].name, buff);
    DataEnter(client[count]);
    int const cnt = count;
    client[cnt].online = true;
    //printf("THREAD ID:%d\n",pthread_self())
    for(;;)
    {
        if(client[cnt].online == true)
        {
        if(read(client[cnt].fd, buff, sizeof(buff)) >= 0)
        {
            msgcount++;
            if(strstr(buff ,"Exit"))
            {
                printf("User %s left\n",client[cnt].name);
                close(client[cnt].fd);
                client[cnt].online = false;
            }else{
                strcpy(messages[msgcount].from,client[cnt].name);

                            //Decode
                char decoded[181];
                char *p;
                bzero(decoded,sizeof(decoded));
                for(int i = 0; buff[i] != '\0'; i++)
                {
                    decoded[i] = buff[i];
                    if(buff[i+1] == ';')
                    {
                        strcpy(messages[msgcount].to,decoded);
                        p = buff+i+2;
                    }
                }
                strcpy(messages[msgcount].message,p);

                messages[msgcount].status = true;
        }
        }
        }
    }
    return NULL;
}


// Driver function
int main()
{
    pthread_t clienth;
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;
    pthread_create(&chat, NULL, Mthread, NULL);
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));


	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Bind
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");
    for(;;)
    {
	//listen
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	// Accept
	connfd = accept(sockfd, (SA*)&cli, &len);

	if (connfd < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");


    pthread_create(&clienth, NULL, func, connfd);
    count++;
    }
	// close the socket
	close(sockfd);
}

