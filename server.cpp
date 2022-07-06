/* File Name: server.cpp
*  Author: Jaron Baldazo
*  Date: October 21, 2021
*  Compile: g++ server.cpp -o server
*  Run: ./server
*/

#include <stdio.h>         //input/ouput
#include <string.h>        //string
#include <sys/socket.h>    //socket
#include <arpa/inet.h>     //inet_addr
#include <unistd.h>        //close()
#include <stdlib.h>        //exit()
#include <time.h>          //timer for microserver

#define PORT 22334
#define TRANSLATORPORT 22333
#define CURRENCYPORT 22332
#define VOTEPORT 22331
#define MESSAGELENGTH 1000

void errorMessage(const char *msg){
    puts(msg);
    exit(EXIT_FAILURE);
}

int main (){
    int parentSocketDesc, childSocketDesc;           //file descriptor for server socket
    int c;                                           //size of structure sockaddr_in
    struct sockaddr_in server, client;               //structure sockaddr_in for server
    int pid;                                         //pid of child processes from fork()
    char clientRequest[MESSAGELENGTH];               //request from client
    int creqSize;                                    //client request size in bytes
    char serverReply[MESSAGELENGTH];                 //reply from server
    int srepSize;                                    //server reply size in bytes
    int doneFlag;                                    //flag for when to stop receiving from client

    //initialize sockaddr server structure
    server.sin_family = AF_INET;                //family type
    server.sin_port = htons(PORT);              //port number of server
    server.sin_addr.s_addr = htonl(INADDR_ANY); //bind to ip

    //create server socket
    parentSocketDesc = socket (PF_INET, SOCK_STREAM, 0);

    //can't create server socket
    if (parentSocketDesc == -1){
        errorMessage("Cant create main socket.");
    }

    //attempts to bind server socket to speciofic address and port
    if (bind(parentSocketDesc, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
        errorMessage("Can't bind main socket to specific address and port");
    }
    else {
        puts("Successfully binded main socket.");
    }

    //attempts to listen for incoming connections from clients
    if (listen(parentSocketDesc, 10) == -1){
        errorMessage("Failed to put main socket in listening mode");
    }
    else {
        puts("Server listening on TCP port 22334");
    }
    
    c = sizeof(struct sockaddr_in);         //size of sockaddr_in structure
    bzero(clientRequest, MESSAGELENGTH);    //clear clientRequest string
    bzero(serverReply, MESSAGELENGTH);      //clear serverReply string

    //keeps accepting new connections until program is terminated
    for ( ; ; ){
        //attempts to accept a new connection on server socket
        childSocketDesc = accept(parentSocketDesc, (struct sockaddr *)&client, (socklen_t *) &c);
        //fail to accept a new connection
        if (childSocketDesc == -1){
            errorMessage("Waiting for new connections...");
        }
        else {
            puts("Accepted a connection from client");
        }

        pid = fork();   //create a child process for new client
        //failed to create child process
        if (pid < 0){
            errorMessage("Failed to create child process.");
        }
        //child process created
        else if (pid == 0){
            close(parentSocketDesc);
            doneFlag = 0;
            while (!doneFlag){
                creqSize = recv(childSocketDesc, clientRequest, MESSAGELENGTH, 0);
                if (creqSize == -1){
                    errorMessage("Failed to receive a request from client");
                }

                puts ("The client requested:");
                puts (clientRequest);
                
                //translator
                if (strncmp(clientRequest, "TRANSLATOR", 10) == 0){
                    bzero(clientRequest, MESSAGELENGTH);
                    sprintf(serverReply, "\nAvailable English words: Hello || Goodbye || Library || Thank You || Dog\nEnter an English word: ");
                    puts ("The server reply:");
                    puts (serverReply);
                    srepSize = send(childSocketDesc, serverReply, strlen(serverReply), 0);
                    //failed to send serverReply to client
                    if (srepSize == -1){
                        puts("Failed to send server reply to client.");
                    }
                    //receive English word
                    recv(childSocketDesc, clientRequest, MESSAGELENGTH, 0);
                    
                    int translatorDesc;                     //file descriptor for translator socket
                    char frenchWord[MESSAGELENGTH];         //French word from translator
                    char* englishWord = clientRequest;      //point to English word
                    int frLength;                           //French word length
                    struct sockaddr_in translatorServer;    //structure sockaddr_in for socket
                    int a;                                  //size of structure sockaddr_in
                    struct timeval tv;                      //structure timeout

                    translatorServer.sin_family = AF_INET;                             //family type  
                    translatorServer.sin_port = htons(TRANSLATORPORT);                 //port number of server
                    translatorServer.sin_addr.s_addr = inet_addr("136.159.5.27");      //bind to ip

                    tv.tv_sec = 3;  //3 sec
                    tv.tv_usec = 0; //0 microsec

                    //create translator socket
                    translatorDesc = socket (PF_INET, SOCK_DGRAM, 0);

                    //can't create translator socket
                    if (translatorDesc == -1){
                        errorMessage("Cant create translator socket.");
                    }

                    //timer for UDP recvfrom()
                    if(setsockopt(translatorDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0){
                        errorMessage("Timer error.");
                    }

                    sendto(translatorDesc, englishWord, strlen(englishWord), MSG_CONFIRM, (struct sockaddr*)&translatorServer, sizeof(translatorServer));
                    // if (sendto(translatorDesc, englishWord, strlen(englishWord), MSG_CONFIRM, (struct sockaddr*)&translatorServer, sizeof(translatorServer)) == -1){
                    //     translatorServer.sin_addr.s_addr = inet_addr("136.159.5.25");
                    //     sendto(translatorDesc, englishWord, strlen(englishWord), MSG_CONFIRM, (struct sockaddr*)&translatorServer, sizeof(translatorServer));
                    // }
                    puts("English word sent.");

                    a = sizeof(translatorServer);

                    frLength = recvfrom(translatorDesc, (char*)frenchWord, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&translatorServer, (socklen_t*)&a);

                    //if no responds from UDP, send "MICRO OFFLINE"
                    if (frLength == -1){
                        srepSize = send(childSocketDesc, "MICRO OFFLINE", strlen("MICRO OFFLINE"), 0);
                        close(translatorDesc);
                        continue;
                    }
                    //send french word
                    else {
                        frenchWord[frLength] = '\0';
                        close(translatorDesc);
                        srepSize = send(childSocketDesc, frenchWord, strlen(frenchWord), 0);
                        //failed to send serverReply to client
                        if (srepSize == -1){
                            puts("Failed to send server reply to client.");
                        }
                    }
                }
                //currency
                else if (strncmp(clientRequest, "CURRENCY CONVERTER", 18) == 0){
                    bzero(clientRequest, MESSAGELENGTH);
                    sprintf(serverReply, "\nEnter amount to be converted in the format <AMOUNT> <SOURCE CURRENCY> <DESTINATION CURRENCY>:\n\nSupported Currencies:\n CAD\n USD\n EUR\n GBP\n BTC");
                    puts ("The server reply:");
                    puts (serverReply);
                    srepSize = send(childSocketDesc, serverReply, strlen(serverReply), 0);
                    //failed to send serverReply to client
                    if (srepSize == -1){
                        puts("Failed to send server reply to client.");
                    }
                    //receive currency to be converted
                    recv(childSocketDesc, clientRequest, MESSAGELENGTH, 0);

                    int currencyDesc;                       //file descriptor for translator socket
                    char converted[MESSAGELENGTH];          //converted currency request
                    char* convertRequest = clientRequest;   //point to currency request
                    int conLength;                          //converted request length
                    struct sockaddr_in currencyServer;      //structure sockaddr_in for socket
                    int b;                                  //size of structure sockaddr_in
                    struct timeval tv;                      //structure timeout


                    currencyServer.sin_family = AF_INET;                             //family type  
                    currencyServer.sin_port = htons(CURRENCYPORT);                   //port number of server
                    currencyServer.sin_addr.s_addr = inet_addr("136.159.5.27");      //bind to ip

                    tv.tv_sec = 3;  //3 sec
                    tv.tv_usec = 0; //0 microsec

                    //create translator socket
                    currencyDesc = socket (PF_INET, SOCK_DGRAM, 0);

                    //can't create translator socket
                    if (currencyDesc == -1){
                        errorMessage("Cant create currency converter socket.");
                    }

                    //timeout for recvfrom()
                    if(setsockopt(currencyDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0){
                        errorMessage("Timer error.");
                    }

                    sendto(currencyDesc, convertRequest, strlen(convertRequest), MSG_CONFIRM, (struct sockaddr*)&currencyServer, sizeof(currencyServer));

                    puts("Currency to be converted sent.");

                    b = sizeof(currencyServer);

                    conLength = recvfrom(currencyDesc, (char*)converted, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&currencyServer, (socklen_t*)&b);
                    //UDP server offline, send "MICRO OFFLINE"
                    if (conLength == -1){
                        srepSize = send(childSocketDesc, "MICRO OFFLINE", strlen("MICRO OFFLINE"), 0);
                        close(currencyDesc);
                        continue;
                    }
                    converted[conLength] = '\0';
                    close(currencyDesc);
                    srepSize = send(childSocketDesc, converted, strlen(converted), 0);
                    //failed to send serverReply to client
                    if (srepSize == -1){
                        puts("Failed to send server reply to client.");
                    }
                }
                //vote
                else if (strncmp(clientRequest, "VOTE", 4) == 0){
                    sprintf(serverReply, "\nWelcome to Secure Voting.\n\nThe following commands are supported:\n\n SHOW CANDIDATES\n VOTE\n VOTING SUMMARY\n MAIN\n");
                    puts ("The server reply:");
                    puts (serverReply);
                    //sends welcome to voting message
                    srepSize = send(childSocketDesc, serverReply, strlen(serverReply), 0);
                    //failed to send serverReply to client
                    if (srepSize == -1){
                        puts("Failed to send server reply to client.");
                    }
                    int voteFlag = 0;
                    while(!voteFlag){
                        bzero(clientRequest, MESSAGELENGTH);
                        // bzero(serverReply, MESSAGELENGTH);
                        //receive vote command
                        recv(childSocketDesc, clientRequest, MESSAGELENGTH, 0);

                        // puts("-------------------------");
                        // puts(clientRequest);
                        // puts("-------------------------");

                        int voteDesc;                           //file descriptor for vote socket
                        char voteReply[MESSAGELENGTH];          //reply from vote server
                        char* voteRequest = clientRequest;      //point to vote request
                        int voteReqLength;                      //vote request length
                        struct sockaddr_in voteServer;          //structure sockaddr_in for socket
                        int d;                                  //size of structure sockaddr_in
                        struct timeval tv;                      //structure timeout


                        voteServer.sin_family = AF_INET;                             //family type  
                        voteServer.sin_port = htons(VOTEPORT);                   //port number of server
                        voteServer.sin_addr.s_addr = inet_addr("136.159.5.27");      //bind to ip

                        tv.tv_sec = 3;  //3 sec
                        tv.tv_usec = 0; //0 microsec

                        //create vote socket
                        voteDesc = socket (PF_INET, SOCK_DGRAM, 0);

                        //can't create vote socket
                        if (voteDesc == -1){
                            errorMessage("Cant create currency converter socket.");
                        }

                        //timeout operation
                        if(setsockopt(voteDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0){
                            errorMessage("Timer error.");
                        }
                        
                        bzero(voteReply, MESSAGELENGTH);

                        //vote
                        if (strncmp(voteRequest, "VOTE", 13) == 0){
                            sendto(voteDesc, voteRequest, strlen(voteRequest), MSG_CONFIRM, (struct sockaddr*)&voteServer, sizeof(voteServer));

                            d = sizeof(voteServer);

                            bzero(voteReply,MESSAGELENGTH);
                            voteReqLength = recvfrom(voteDesc, (char*)voteReply, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&voteServer, (socklen_t*)&d);
                            if (voteReqLength == -1){
                                srepSize = send(childSocketDesc, "MICRO OFFLINE", strlen("MICRO OFFLINE"), 0);
                                close(voteDesc);
                                break;
                            }
                            puts(voteReply);
                            voteReply[voteReqLength] = '\0';

                            srepSize = send(childSocketDesc, voteReply, strlen(voteReply), 0);
                            //failed to send serverReply to client
                            if (srepSize == -1){
                                puts("Failed to send server reply to client.");
                            }
                            
                            bzero(clientRequest, MESSAGELENGTH);
                            //receive encrypted vote
                            recv(childSocketDesc, clientRequest, MESSAGELENGTH, 0);

                            //send encrypted vote to micro
                            sendto(voteDesc, voteRequest, strlen(voteRequest), MSG_CONFIRM, (struct sockaddr*)&voteServer, sizeof(voteServer));
                            d = sizeof(voteServer);

                            bzero(voteReply, MESSAGELENGTH);
                            //receive response from micro
                            voteReqLength = recvfrom(voteDesc, (char*)voteReply, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&voteServer, (socklen_t*)&d);
                            if (voteReqLength == -1){
                                errorMessage("Failed to receive vote reply.");
                            }
                            puts(voteReply);
                            voteReply[voteReqLength] = '\0';
                            close(voteDesc);

                            srepSize = send(childSocketDesc, voteReply, strlen(voteReply), 0);
                            //failed to send serverReply to client
                            if (srepSize == -1){
                                puts("Failed to send server reply to client.");
                            }
                        }
                        //main
                        else if (strncmp(voteRequest, "MAIN", 4) == 0){
                            voteFlag = 1;
                            continue;
                        }
                        //other command
                        else {
                            // puts("-------------------------");
                            // puts(voteRequest);
                            // puts("-------------------------");
                            sendto(voteDesc, voteRequest, strlen(voteRequest), MSG_CONFIRM, (struct sockaddr*)&voteServer, sizeof(voteServer));

                            d = sizeof(voteServer);

                            voteReqLength = recvfrom(voteDesc, (char*)voteReply, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&voteServer, (socklen_t*)&d);
                            if (voteReqLength == -1){
                                srepSize = send(childSocketDesc, "MICRO OFFLINE", strlen("MICRO OFFLINE"), 0);
                                close(voteDesc);
                                break;
                            }
                            puts(voteReply);
                            voteReply[voteReqLength] = '\0';
                            close(voteDesc);
                            srepSize = send(childSocketDesc, voteReply, strlen(voteReply), 0);
                            //failed to send serverReply to client
                            if (srepSize == -1){
                                puts("Failed to send server reply to client.");
                            }  
                        }
                    }
                }
                //help
                else if (strncmp(clientRequest, "HELP", 4) == 0){
                    sprintf(serverReply, "\nSupported commands:\n TRANSLATOR\n CURRENCY CONVERTER\n VOTE\n HELP\n EXIT\n");
                    puts ("The server reply:");
                    puts (serverReply);
                    srepSize = send(childSocketDesc, serverReply, strlen(serverReply), 0);
                    //failed to send serverReply to client
                    if (srepSize == -1){
                        puts("Failed to send server reply to client.");
                    }
                }
                //exit
                else if (strncmp(clientRequest, "EXIT", 4) == 0){
                    sprintf(serverReply, "\nThanks for using the following services. Bye!\n");
                    doneFlag = 1;
                    puts ("The server reply:");
                    puts (serverReply);
                    srepSize = send(childSocketDesc, serverReply, strlen(serverReply), 0);
                    //failed to send serverReply to client
                    if (srepSize == -1){
                        puts("Failed to send server reply to client.");
                    }
                }
                //invalid command
                else {
                    sprintf(serverReply, "\nUnrecognized command. Use command HELP to see available commands.\n");
                    puts ("The server reply:");
                    puts (serverReply);
                    srepSize = send(childSocketDesc, serverReply, strlen(serverReply), 0);
                    //failed to send serverReply to client
                    if (srepSize == -1){
                        puts("Failed to send server reply to client.");
                    }
                }

                bzero(clientRequest, MESSAGELENGTH);    //clear clientRequest string
                bzero(serverReply, MESSAGELENGTH);      //clear serverReply string
            }
            close(childSocketDesc);
        }
        else {
            puts("Created child process.");
            puts ("Going back to listening.");
            close (childSocketDesc);
        }
    }
    return 0;   
}