/* File Name: translator.cpp
*  Author: Jaron Baldazo
*  Date: October 21, 2021
*  Compile: g++ translator.cpp -o translator
*  Run: ./translator
*/

#include <stdio.h>         //input/ouput
#include <string.h>        //string
#include <sys/socket.h>    //socket
#include <arpa/inet.h>     //inet_addr
#include <unistd.h>        //close()
#include <stdlib.h>        //exit()

#define PORT 22333
#define MESSAGELENGTH 1000

void errorMessage(const char *msg){
    puts(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int serverSocketDesc;                   //file descriptor for server socket
    char englishWord[MESSAGELENGTH];        //English word to be translated
    int engLength;                          //length of English word
    char* frenchWord;                       //Translated word in French
    struct sockaddr_in server, client;      //structure sockeadddr_in for server
    int c;                                  //size of structure sockaddr_in

    server.sin_family = AF_INET;            //family type  
    server.sin_port = htons(PORT);          //port number of server
    server.sin_addr.s_addr = INADDR_ANY;    //bind to ip

    //create server socket
    serverSocketDesc = socket (PF_INET, SOCK_DGRAM, 0);

    //can't create server socket
    if (serverSocketDesc == -1){
        errorMessage("Cant create translator socket.");
    }

    //attempts to bind server socket to specific address and port
    if (bind(serverSocketDesc, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
        errorMessage("Can't bind translator socket to specific address and port");
    }
    else {
        puts("Successfully binded main socket.");
    }

    c = sizeof(struct sockaddr_in);     //size of sockaddr_in structure
    bzero(englishWord, MESSAGELENGTH);  //clear englishWord string

    for ( ; ; ){
        //attempts to receive English word
        engLength = recvfrom(serverSocketDesc, (char *)englishWord, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&client, (socklen_t*)&c); 
        //failed to receive English word
        if (engLength == -1){
            errorMessage("Failed to receive English word.");
        }
        englishWord[engLength] = '\0';
        puts(englishWord);
        //hello
        if (strncmp(englishWord, "Hello", 5) == 0){
            frenchWord = "Bonjour";
            sendto(serverSocketDesc, frenchWord, strlen(frenchWord), MSG_CONFIRM, (struct sockaddr *)&client, c);
            puts("French word sent.");
            puts(frenchWord);
        }
        //goodbye
        else if (strncmp(englishWord, "Goodbye", 7) == 0){
            frenchWord = "Au Revoir";
            sendto(serverSocketDesc, frenchWord, strlen(frenchWord), MSG_CONFIRM, (struct sockaddr *)&client, c);
            puts("French word sent.");
            puts(frenchWord);
        }
        //library
        else if (strncmp(englishWord, "Library", 7) == 0){
            frenchWord = "Bibliotheque";
            sendto(serverSocketDesc, frenchWord, strlen(frenchWord), MSG_CONFIRM, (struct sockaddr *)&client, c);
            puts("French word sent.");
            puts(frenchWord);
        }
        //thank you
        else if (strncmp(englishWord, "Thank You", 7) == 0){
            frenchWord = "Merci";
            sendto(serverSocketDesc, frenchWord, strlen(frenchWord), MSG_CONFIRM, (struct sockaddr *)&client, c);
            puts("French word sent.");
            puts(frenchWord);
        }
        //dog
        else if (strncmp(englishWord, "Dog", 3) == 0){
            frenchWord = "Chien";
            sendto(serverSocketDesc, frenchWord, strlen(frenchWord), MSG_CONFIRM, (struct sockaddr *)&client, c);
            puts("French word sent.");
            puts(frenchWord);
        }

        //no translation
        else {
            char* noTranslation = "Sorry! No translation can be provided.";
            sendto(serverSocketDesc, noTranslation, strlen(noTranslation), MSG_CONFIRM, (struct sockaddr *)&client, c);
        }
    }
}