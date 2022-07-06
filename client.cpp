/* File Name: client.cpp
*  Author: Jaron Baldazo
*  Date: October 21, 2021
*  Compile: g++ client.cpp -o client
*  Run: ./client
*/

#include <stdio.h>         //input/ouput
#include <string.h>        //string
#include <sys/socket.h>    //socket
#include <arpa/inet.h>     //inet_addr
#include <unistd.h>        //close()
#include <stdlib.h>        //exit()
#include <string>          //string
#include <chrono>          //timer
#include <iostream>        //cout

using namespace std;

#define MESSAGELENGTH 1000
#define MAXTIME 60  //60 seconds for timer

//error message function
void errorMessage(const char *msg){
    puts(msg);
    exit(EXIT_FAILURE);
}

int main (int argc, char *argv[]){

    //take in arguments for ip and port
    if (argc != 3){
        puts("Usage: ./client <ip adress> <port #>");
        return 0;
    }
    
    //initialize ip and port number of server
    char* ip = argv[1];
    char* port = argv[2];
    int portNumber = atoi(port);

    int clientSocketDesc;               //file descriptor for client socket
    struct sockaddr_in server;          //structure sockaddr_in for server
    int doneFlag;                       //flag for when to stop sending
    int length;                         //length of command
    char ch;                            //character of command
    char command [MESSAGELENGTH];       //command to be sent to server
    int commandSize;                    //command size in bytes
    char reply [MESSAGELENGTH];         //reply from server
    int replySize;                      //reply size in bytes   
    int hasVoted;                       //keep track if client has voted

    server.sin_family = AF_INET;                //family type
    server.sin_port = htons(portNumber);        //port number of server
    server.sin_addr.s_addr = inet_addr(ip);     //bind to ip of server

    //create server socket
    clientSocketDesc = socket (PF_INET, SOCK_STREAM, 0);

    //can't create server socket
    if (clientSocketDesc == -1){
        errorMessage("Cant create client socket.");
    }

    //attempt to connect to server
    if (connect(clientSocketDesc, (struct sockaddr *)&server, sizeof(server)) == -1){
        errorMessage("Can't connect to local server.");

    }
    puts("Connected to Indirection Server\n");
    // printf("The following commands are supported:\n\n");
    // printf("TRANSLATOR - Translates one word from English to French.\n");
    // printf("CURRENCY CONVERTER - Converts CAD to USD, EURO, GBP, or BTC.\n");
    // printf("VOTE - Vote candidate using secure voting.\n");
    // printf("HELP - Show supported commands.\n");
    // printf("EXIT - Exits the program.\n");
    printf("NOTE: VOTING POLLS WILL CLOSE IN 60 SECONDS.\n");


    //time starts
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    doneFlag = 0;   //keep looping until client exits
    hasVoted = 0;   //check if client has voted

    while(!doneFlag){
        printf("\nThe following commands are supported:\n\n");
        printf(" TRANSLATOR - Translates one word from English to French.\n");
        printf(" CURRENCY CONVERTER - CAD, USD, EURO, GBP, or BTC.\n");
        printf(" VOTE - Vote candidate using secure voting.\n");
        printf(" TIME - Show time left to vote.\n");
        printf(" HELP - Show supported commands.\n");
        printf(" EXIT - Exits the program.\n");

        //prompts user for a supported command
        printf("\nEnter supported command: ");
        length = 0;

        //puts client command request into command[] char by char
        while((ch = getchar()) != '\n'){
            command[length] = ch;
            length++;
        }
        command[length] = '\0';     //null terminate

        //show time left till voting polls close
        if (strncmp(command, "TIME", 4) == 0){
            chrono::steady_clock::time_point end = chrono::steady_clock::now();
            int duration = chrono::duration_cast<chrono::seconds>(end - begin).count();
            int timeLeft = MAXTIME - duration;
            if (timeLeft > 0){
                printf("\nREMAINING TIME TO VOTE: %d seconds\n", timeLeft);
                continue;
            }
            printf("\nVOTING POLLS ARE CLOSED.\n");
            continue;
        }

        //translator
        if (strncmp(command, "TRANSLATOR", 10) == 0){
            //send command to server
            commandSize = send(clientSocketDesc, command, length, 0);
            //failed to send command
            if (commandSize == -1){
                puts("Failed to send server command to server.");
            }
            //attempt to receive reply from server
            replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
            //failed to receive reply
            if (replySize == -1){
                errorMessage("Failed to receive reply.");
            }
            reply[replySize] = '\0';    //null terminate
            printf("%s\n", reply);

            length = 0;
            bzero(command,MESSAGELENGTH);
            bzero(reply, MESSAGELENGTH);
            while((ch = getchar()) != '\n'){
                command[length] = ch;
                length++;
            }
            command[length] = '\0';     //null terminate

            commandSize = send(clientSocketDesc, command, length, 0);
            //failed to send command
            if (commandSize == -1){
                puts("Failed to send English word to server.");
            }
            replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
            //failed to receive reply
            if (replySize == -1){
                errorMessage("Failed to receive reply.");
            }
            if(strncmp(reply, "MICRO OFFLINE", 13) == 0){
                printf("Translator micro server is off.\n");
                continue;
            }
            reply[replySize] = '\0';    //null terminate
            printf("French Translation: ");
            printf("%s\n", reply);
        }

        //currency converter
        else if (strncmp(command, "CURRENCY CONVERTER", 18) == 0){
            //send command to server
            commandSize = send(clientSocketDesc, command, length, 0);
            //failed to send command
            if (commandSize == -1){
                puts("Failed to send server command to server.");
            }
            //attempt to receive reply from server
            replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
            //failed to receive reply
            if (replySize == -1){
                errorMessage("Failed to receive reply.");
            }
            reply[replySize] = '\0';    //null terminate
            printf("%s\n", reply);
            
            length = 0;
            bzero(command,MESSAGELENGTH);
            bzero(reply, MESSAGELENGTH);
            while((ch = getchar()) != '\n'){
                command[length] = ch;
                length++;
            }
            command[length] = '\0';     //null terminate

            commandSize = send(clientSocketDesc, command, length, 0);
            //failed to send command
            if (commandSize == -1){
                puts("Failed to send currency converter to server.");
            }
            replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
            //failed to receive reply
            if (replySize == -1){
                errorMessage("Failed to receive reply.");
            }
            if(strncmp(reply, "MICRO OFFLINE", 13) == 0){
                printf("Currency converter micro server is off.\n");
                continue;
            }
            reply[replySize] = '\0';    //null terminate
            printf("Converted Currency: ");
            printf("%s\n", reply);
        }

        //vote
        else if (strncmp(command, "VOTE", 4) == 0){
            //send command to server
            //sends "VOTE"
            commandSize = send(clientSocketDesc, command, length, 0);
            //failed to send command
            if (commandSize == -1){
                puts("Failed to send server command to server.");
            }

            //receives welcome to voting message
            //attempt to receive reply from server
            replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
            //failed to receive reply
            if (replySize == -1){
                errorMessage("Failed to receive reply.");
            }
            //prints reply message
            reply[replySize] = '\0';    //null terminate
            printf("%s\n", reply);

            int voteFlag = 0;   //keep looping till MAIN is entered
            while (!voteFlag){
                printf("SHOW CANDIDATES || VOTE || VOTING SUMMARY || MAIN\n\n");
                length = 0;
                bzero(command,MESSAGELENGTH);
                bzero(reply, MESSAGELENGTH);
                while((ch = getchar()) != '\n'){
                    command[length] = ch;
                    length++;
                    // puts(command);
                }
                command[length] = '\0';     //null terminate
                // puts("-------------------------");
                // puts(command);
                // puts("-------------------------");

                //exit voting menu
                if (strncmp(command, "MAIN", 4) == 0){
                    commandSize = send(clientSocketDesc, command, length, 0);
                    if (commandSize == -1){
                        puts("Failed to send server command to server.");
                    }
                    voteFlag = 1;
                    continue;
                }
                //vote for candidates
                else if (strncmp(command, "VOTE", 4) == 0){

                    //end time
                    chrono::steady_clock::time_point end = chrono::steady_clock::now();

                    //check duration from start of program
                    int duration = chrono::duration_cast<chrono::seconds>(end - begin).count();

                    //voting polls are closed or client already voted, send NO VOTE to microserver
                    if (duration > MAXTIME || hasVoted){
                        commandSize = send(clientSocketDesc, "NO VOTE", strlen("NO VOTE"), 0);
                        //failed to send command
                        if (commandSize == -1){
                            puts("Failed to send vote command to server.");
                        }
                        replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
                        //failed to receive reply
                        if (replySize == -1){
                            errorMessage("Failed to receive reply.");
                        }
                        if(strncmp(reply, "MICRO OFFLINE", 13) == 0){
                            printf("Voting micro server is off.\n");
                            break;
                        }
                        reply[replySize] = '\0';    //null terminate
                        printf("%s\n", reply);
                        continue;
                    }

                    //voting polls still open
                    commandSize = send(clientSocketDesc, command, length, 0);
                    //failed to send command
                    if (commandSize == -1){
                        puts("Failed to send vote command to server.");
                    }
                    //reply contains encryption key
                    replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
                    //failed to receive reply
                    if (replySize == -1){
                        errorMessage("Failed to receive reply.");
                    }
                    if(strncmp(reply, "MICRO OFFLINE", 13) == 0){
                        printf("Voting micro server is off.\n");
                        break;
                    }

                    //ask for client input for candidate id
                    printf("\nEnter candidate ID to vote for:");
                    length = 0;
                    bzero(command,MESSAGELENGTH);
                    while((ch = getchar()) != '\n'){
                        command[length] = ch;
                        length++;
                    }

                    //encrypt using random key from microserver
                    command[length] = '\0';     //null terminate
                    int encrypt = (atoi(command) * atoi(reply));  //encode vote
                    const char* encryptedVote = to_string(encrypt).c_str();
                    //send encrypted vote
                    commandSize = send(clientSocketDesc, encryptedVote, length, 0);
                    //failed to send command
                    if (commandSize == -1){
                        puts("Failed to send vote command to server.");
                    }

                    bzero(reply, MESSAGELENGTH);
                    //receive "Thanks for Voting"
                    replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
                    //failed to receive reply
                    if (replySize == -1){
                        errorMessage("Failed to receive reply.");
                    }
                    reply[replySize] = '\0';    //null terminate
                    printf("%s\n", reply);
                    if (strlen(reply) == 20){
                        hasVoted = 1;
                    }
                }
                else {
                    // puts("----------inside----------");
                    // puts(command);
                    // puts("----------inside----------");
                    chrono::steady_clock::time_point end = chrono::steady_clock::now();

                    int duration = chrono::duration_cast<chrono::seconds>(end - begin).count();
                    
                    //if user asks for summary and has not voted OR voting polls still open, send "NO SUMMARY" to microserver
                    if ((!hasVoted) && (strncmp(command, "VOTING SUMMARY", 14) == 0) || (duration < MAXTIME) && (strncmp(command, "VOTING SUMMARY", 14) == 0)){
                        commandSize = send(clientSocketDesc, "NO SUMMARY", strlen("NO SUMMARY"), 0);
                        //failed to send command
                        if (commandSize == -1){
                            puts("Failed to send vote command to server.");
                        }
                        //receive "MICRO OFFLINE" or message that says no voting summary can be provided
                        replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
                        //failed to receive reply
                        if (replySize == -1){
                            errorMessage("Failed to receive reply.");
                        }
                        if(strncmp(reply, "MICRO OFFLINE", 13) == 0){
                            printf("Voting micro server is off.\n");
                            break;
                        }
                        reply[replySize] = '\0';    //null terminate
                        printf("%s\n", reply);
                    }

                    //any other command
                    else {
                        commandSize = send(clientSocketDesc, command, length, 0);
                        //failed to send command
                        if (commandSize == -1){
                            puts("Failed to send vote command to server.");
                        }
                        replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
                        //failed to receive reply
                        if (replySize == -1){
                            errorMessage("Failed to receive reply.");
                        }
                        if(strncmp(reply, "MICRO OFFLINE", 13) == 0){
                            printf("Voting micro server is off.\n");
                            break;
                        }
                        reply[replySize] = '\0';    //null terminate
                        printf("%s\n", reply);
                    }
                }
            }
        }

        //help
        else if (strncmp(command, "HELP", 4) == 0){
            //send command to server
            commandSize = send(clientSocketDesc, command, length, 0);
            //failed to send command
            if (commandSize == -1){
                puts("Failed to send server command to server.");
            }
            //attempt to receive reply from server
            replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
            //failed to receive reply
            if (replySize == -1){
                errorMessage("Failed to receive reply.");
            }
            reply[replySize] = '\0';    //null terminate
            printf("%s\n", reply);
        }

        //exit program
        else if (strncmp(command, "EXIT", 4) == 0){
            //get out of while loop if command is EXIT
            doneFlag = 1;
            //send command to server
            commandSize = send(clientSocketDesc, command, length, 0);
            //failed to send command
            if (commandSize == -1){
                puts("Failed to send server command to server.");
            }
            //attempt to receive reply from server
            replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
            //failed to receive reply
            if (replySize == -1){
                errorMessage("Failed to receive reply.");
            }
            reply[replySize] = '\0';    //null terminate
            printf("%s\n", reply);
        }
        //any other command
        else {
            commandSize = send(clientSocketDesc, command, length, 0);
            //failed to send command
            if (commandSize == -1){
                puts("Failed to send server command to server.");
            }
            //attempt to receive reply from server
            replySize = recv(clientSocketDesc, reply, MESSAGELENGTH, 0);
            //failed to receive reply
            if (replySize == -1){
                errorMessage("Failed to receive reply.");
            }
            reply[replySize] = '\0';    //null terminate
            printf("%s\n", reply);
        }
    }
    close(clientSocketDesc);
    return 0;
}