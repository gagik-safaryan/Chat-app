#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8888
#define SA struct sockaddr
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
typedef struct{
    char from[MAX];
    char message[180];
}Msg_t;
Msg_t messages[300];
char cname[MAX];
int nmess = 0,msgcount=0;
pthread_t rec,men;
void Menu(int sockfd);
void intHandler(int sockfd)
{
    close(sockfd);
    exit(0);
}

void Send(int sockfd)
{
    for(;;)
    {
    printf("Write Exit after sending message to exit\n");
    char to[MAX], message[100],fmsg[181],extra[MAX];
    bzero(&to, sizeof(to));
    bzero(&message, sizeof(message));
    bzero(&fmsg, sizeof(fmsg));
    int count = 0;
    char exits[] = "Exit";
    fgets(extra,MAX,stdin);
    printf("\nTo: ");
    fgets(to,MAX,stdin);
    printf("Message: ");
    fgets(message,100,stdin);
    //Exit status

    //Encode
    for(int i = 0; to[i] != 0; i++)
    {
        fmsg[i] = to[i];
        count  = i;
    }
    fmsg[count+1] = ';';
    for(int i = 0; message[i] != 0; i++)
    {
        fmsg[count+i+2] = message[i];
    }
    write(sockfd, fmsg, sizeof(fmsg));
    bzero(fmsg,sizeof(fmsg));
    extra[strcspn(extra, "\n")] = '\0';
    if(strcmp(exits,extra) == 0)
    {
        Menu(sockfd);
    }
    bzero(extra,sizeof(extra));
    usleep(200);
    }
}

void *Recieve(int sockfd)
{
    for(;;)
    {
        char fmsg[181];
        read(sockfd,fmsg,sizeof(fmsg));
        msgcount++;
        //Decode
        char decoded[181];
        char *p;
        bzero(decoded,sizeof(decoded));
        for(int i = 0; fmsg[i] != '\0'; i++)
        {
            decoded[i] = fmsg[i];
            if(fmsg[i+1] == ';')
            {
                strcpy(fmsg,decoded);
                strcpy(messages[msgcount].from,fmsg);
                p = fmsg+i+2;
            }
        }
        strcpy(messages[msgcount].message,p);
        if(strcmp(fmsg,"\nUsername exists.\n") == 0)
        {
            close(sockfd);
            exit(0);
        }
        bzero(fmsg,sizeof(fmsg));
        usleep(200);
    }
    return NULL;
}

void Mailbox(int sockfd)
{
    char exits[] = "Exit";
    system("clear");
    printf("Enter anything to refresh\n");
    for(;;)
    {
        system("clear");
        char extra[MAX];
        for(int i = 1; i < msgcount; i++)
        {
            printf("From: %s\n",messages[i].from);
            printf("---------------\n%s\n",messages[i].message);
            // if(nmess+2 == msgcount)
            // {
            //     system("clear");
            //     i = 1;
            //     nmess = msgcount;
            // }
        }
        fgets(extra,MAX,stdin);
        extra[strcspn(extra, "\n")] = '\0';
        if(strcmp(exits,extra) == 0)
        {
            Menu(sockfd);
        }
        bzero(extra,sizeof(extra));

    }
}

void Menu(int sockfd)
{
    system("clear");
    int option = 0;
    printf("\nWelcome to GagoChat dear %s\n",cname);
    printf("[1]Mailbox\n");
    printf("[2]Send\n");
    printf("[3]Exit\n");
    scanf("%d",&option);
    if(option == 1)
    {
        system("clear");
        Mailbox(sockfd);

    }else if (option == 2) {
        system("clear");
        Send(sockfd);
}
    else{system("clear");
        write(sockfd,"Exit",sizeof("Exit"));
        close(sockfd);
        exit(0);
    }
}

int main()
{
	int sockfd;
	struct sockaddr_in servaddr;
    printf("Enter your name: ");
    scanf("%s",cname);
	// socket create
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else{}
	bzero(&servaddr, sizeof(servaddr));


	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
		!= 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else{}
    char buff[MAX];
    strcpy( buff,cname);
    write(sockfd, buff, sizeof(buff));
    bzero(&buff, sizeof(buff));
    pthread_create(&rec, NULL, Recieve, sockfd);
    Menu(sockfd);

	// close the socket
	close(sockfd);
}


