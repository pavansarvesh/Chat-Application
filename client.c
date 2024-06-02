#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>

int createTCPIpv4Socket() { return socket(AF_INET, SOCK_STREAM, 0); }
struct sockaddr_in *createIPv4Address(char *ip, int port)
{
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if (strlen(ip) == 0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);

    return address;
}

void startListeningAndPrintMessagesOnNewThread(int fd);
void listenAndPrint(int socketFD);
void readConsoleEntriesAndSendToServer(int socketFD);
int main()
{
    int socketFD = createTCPIpv4Socket();
    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);
    int result = connect(socketFD, address, sizeof(*address));
    if (result == 0)
        printf("connection was successful\n");
    startListeningAndPrintMessagesOnNewThread(socketFD);
    readConsoleEntriesAndSendToServer(socketFD);
    close(socketFD);
    return 0;
}

void readConsoleEntriesAndSendToServer(int socketFD)
{
    char *name = NULL;
    size_t nameSize = 0;
    printf("please enter your name : \n");
    ssize_t nameCount = getline(&name, &nameSize, stdin);
    name[nameCount - 1] = 0;
    char *line = NULL;
    size_t lineSize = 0;
    printf("type and we will send(type exit) : \n");
    char buffer[1024];
    while (true)
    {
        ssize_t charCount = getline(&line, &lineSize, stdin);
        line[charCount - 1] = 0;
        sprintf(buffer, "%s:%s", name, line);
        if (charCount > 0)
        {
            if (strcmp(line, "exit") == 0)
                break;

            ssize_t amountWasSent = send(socketFD,
                                         buffer,
                                         strlen(buffer), 0);
        }
    }
}

void startListeningAndPrintMessagesOnNewThread(int socketFD)
{

    pthread_t id;
    pthread_create(&id, NULL, listenAndPrint, socketFD);
}

void listenAndPrint(int socketFD)
{
    char buffer[1024];

    while (true)
    {
        ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);

        if (amountReceived > 0)
        {
            buffer[amountReceived] = 0;
            printf("Response was %s\n ", buffer);
        }

        if (amountReceived == 0)
            break;
    }

    close(socketFD);
}