/* File Name: voting.cpp
*  Author: Jaron Baldazo
*  Date: October 21, 2021
*  Compile: g++ voting.cpp -o voting
*  Run: ./voting
*/

#include <stdio.h>         //input/ouput
#include <string.h>        //string
#include <string>          //string
#include <sys/socket.h>    //socket
#include <arpa/inet.h>     //inet_addr
#include <unistd.h>        //close()
#include <stdlib.h>        //exit() rand()

using namespace std;

#define PORT 22331
#define MESSAGELENGTH 1000

//candidates, id, votes treated like a map
string candidates [5] = {"STROLL", "ALONSO", "VETTEL", "NORRIS", "BOTTAS"};
int id [5] = {1001, 1002, 1003, 1004, 1005};
int votes [5] = {26, 58, 35, 145, 177};

//error message
void errorMessage(const char *msg){
    puts(msg);
    exit(EXIT_FAILURE);
}

//add vote
void addVote(int index){
    votes[index] = (votes[index] + 1);
}

//find index using decrypted ID
int findIndex(int findID){
    for (int i = 0; i < 5; i++){
        if(findID == id[i]){
            return i;
        }
    }
    return -1;
}

int main(){
    int serverSocketDesc;                   //file descriptor for socket socket
    char voteReq[MESSAGELENGTH];            //vote request
    int voteReqLength;                      //length of vote request
    const char* voteReply;                  //reply to indirection
    struct sockaddr_in server, client;      //structure sockadddr_in for server
    int c;                                  //size of structure sockaddr_in
    int encryptKey;                         //encryption key

    server.sin_family = AF_INET;            //family type  
    server.sin_port = htons(PORT);          //port number of server
    server.sin_addr.s_addr = INADDR_ANY;    //bind to ip

    //create server socket
    serverSocketDesc = socket (PF_INET, SOCK_DGRAM, 0);

    //can't create server socket
    if (serverSocketDesc == -1){
        errorMessage("Cant create vote socket.");
    }

    //attempts to bind server socket to speciofic address and port
    if (bind(serverSocketDesc, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
        errorMessage("Can't bind vote socket to specific address and port");
    }
    else {
        puts("Successfully binded vote socket.");
    }

    c = sizeof(struct sockaddr_in);     //size of sockaddr_in structure
    bzero(voteReq, MESSAGELENGTH);      //clear currency request string

    for ( ; ; ){
        //attempts to receive vote request
        voteReqLength = recvfrom(serverSocketDesc, (char *)voteReq, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&client, (socklen_t*)&c); 
        //failed to receive vote request
        if (voteReqLength == -1){
            errorMessage("Failed to receive vote request.");
        }
        voteReq[voteReqLength] = '\0';
        puts(voteReq);

        //show candidates
        if (strncmp(voteReq, "SHOW CANDIDATES", 15) == 0){
            string temp;
            temp.append("\n");
            temp.append("ID:    CANDIDATE NAME:\n");
            temp.append("----------------------\n");
            for (int i = 0; i < 5; i++){
                temp.append(to_string(id[i]));
                temp.append("       ");
                temp.append(candidates[i]);
                temp.append("\n");
            }
            voteReply = temp.c_str();
            // puts(temp.c_str());
            puts(voteReply);
            sendto(serverSocketDesc, voteReply, strlen(voteReply), MSG_CONFIRM, (struct sockaddr *)&client, c);
            voteReply = NULL;
        }
        //vote
        else if (strncmp(voteReq, "VOTE", 13) == 0){
            
            encryptKey = rand() % 10;   //range 0 to 9
            voteReply = to_string(encryptKey).c_str();
            //send encyption key
            sendto(serverSocketDesc, voteReply, strlen(voteReply), MSG_CONFIRM, (struct sockaddr *)&client, c);

            bzero(voteReq, MESSAGELENGTH);
            //attempts to receive encrypted vote
            voteReqLength = recvfrom(serverSocketDesc, (char *)voteReq, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&client, (socklen_t*)&c); 
            //failed to receive encrypted vote
            if (voteReqLength == -1){
                errorMessage("Failed to receive vote request.");
            }
            voteReq[voteReqLength] = '\0';
            puts(voteReq);
            //decrypt to get id
            int decrypt = (atoi(voteReq) / encryptKey);
            printf("%d\n", decrypt);
            int index = (findIndex(decrypt));

            //invalid id
            if(index == -1){
                const char* noID= "\nInvalid ID. Try again.\n";
                sendto(serverSocketDesc, noID, strlen(noID), MSG_CONFIRM, (struct sockaddr *)&client, c);
            }
            //add vote and send message
            else {
                addVote(findIndex(decrypt));
                const char* thxMessage= "\nThanks for voting!\n";
                sendto(serverSocketDesc, thxMessage, strlen(thxMessage), MSG_CONFIRM, (struct sockaddr *)&client, c);
            }
            voteReply = NULL;
        }
        //voting summary
        else if (strncmp(voteReq, "VOTING SUMMARY", 14) == 0){
            string temp;
            temp.append("\n");
            temp.append("ID:    CANDIDATE NAME:    VOTES:\n");
            temp.append("--------------------------------\n");
            for (int i = 0; i < 5; i++){
                temp.append(to_string(id[i]));
                temp.append("       ");
                temp.append(candidates[i]);
                temp.append("          ");
                temp.append(to_string(votes[i]));
                temp.append("\n");
            }
            voteReply = temp.c_str();
            // puts(temp.c_str());
            puts(voteReply);
            sendto(serverSocketDesc, voteReply, strlen(voteReply), MSG_CONFIRM, (struct sockaddr *)&client, c);
            voteReply = NULL;
        }
        //no summary to handle voting timer and client not voting yet
        else if (strncmp(voteReq, "NO SUMMARY", 10) == 0){
            char* noVote = "\nCan't show voting results. Must vote first or wait for voting polls to be closed.\n";
            sendto(serverSocketDesc, noVote, strlen(noVote), MSG_CONFIRM, (struct sockaddr *)&client, c);
        }
        //no vote to handle voting timer 
        else if (strncmp(voteReq, "NO VOTE", 7) == 0){
            char* noTime = "\nCan't vote anymore since polls are closed or client has already voted.\n";
            sendto(serverSocketDesc, noTime, strlen(noTime), MSG_CONFIRM, (struct sockaddr *)&client, c);
        }
        //other command
        else {
            char* noTranslation = "\nInvalid vote command.\n";
            sendto(serverSocketDesc, noTranslation, strlen(noTranslation), MSG_CONFIRM, (struct sockaddr *)&client, c);
        }
    }
}