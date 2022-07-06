/* File Name: currency.cpp
*  Author: Jaron Baldazo
*  Date: October 21, 2021
*  Compile: g++ curency.cpp -o currency
*  Run: ./currency
*/

#include <stdio.h>         //input/ouput
#include <string.h>        //string
#include <string>          //converting double to string
#include <sys/socket.h>    //socket
#include <arpa/inet.h>     //inet_addr
#include <unistd.h>        //close()
#include <stdlib.h>        //exit()

using namespace std;

#define PORT 22332
#define MESSAGELENGTH 1000

//global variables treated like a map
const char *currencies [5] = {"CAD", "USD", "EUR", "GBP", "BTC"};
double rates [5] = {1, 0.81, 0.70, 0.59, 0.000016};

//error message function
void errorMessage(const char *msg){
    puts(msg);
    exit(EXIT_FAILURE);
}

//find index of currency
int findIndex (const char* cur){
    for (int i = 0; i < 5; i++){
        const char* find = strstr(cur, currencies[i]);
        if (find != 0){
            return i;
        }
    }
    return -1;
}

//convert currencies
double convert (double total, double source, double destination){
    return ((destination/source) * total);
}

//char to double
double charToDouble (const char* ch){
    return atof(ch);
}

int main() {
    int serverSocketDesc;                   //file descriptor for server socket
    char currency[MESSAGELENGTH];           //currency request
    int curLength;                          //length of currency request
    const char* converted;                  //converted currency
    struct sockaddr_in server, client;      //structure sockadddr_in for server
    int c;                                  //size of structure sockaddr_in
    char amount [MESSAGELENGTH];            //amount to be converted
    char src [MESSAGELENGTH];               //currency source
    char dest [MESSAGELENGTH];              //currency dest
    double srcRate;                         //src rate relative to 1 CAD
    double destRate;                        //dest rate relative to 1 CAD
    double amountNum;                       //amount in double to be converted
    double convertedNum;                    //converted currency in double
    string str;

    server.sin_family = AF_INET;            //family type  
    server.sin_port = htons(PORT);          //port number of server
    server.sin_addr.s_addr = INADDR_ANY;    //bind to ip

    //create server socket
    serverSocketDesc = socket (PF_INET, SOCK_DGRAM, 0);

    //can't create server socket
    if (serverSocketDesc == -1){
        errorMessage("Cant create translator socket.");
    }

    //attempts to bind server socket to speciofic address and port
    if (bind(serverSocketDesc, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
        errorMessage("Can't bind translator socket to specific address and port");
    }
    else {
        puts("Successfully binded main socket.");
    }

    c = sizeof(struct sockaddr_in);     //size of sockaddr_in structure
    bzero(currency, MESSAGELENGTH);     //clear currency request string

    //loop forever till server is closed
    for ( ; ; ){
        //attempts to receive currency conversion
        curLength = recvfrom(serverSocketDesc, (char *)currency, MESSAGELENGTH, MSG_WAITALL, (struct sockaddr*)&client, (socklen_t*)&c); 
        //failed to receive currency conversion
        if (curLength == -1){
            errorMessage("Failed to receive currency request.");
        }
        currency[curLength] = '\0';
        puts(currency);

        //if no space is found, invalid command and send an error message to client
        if (strchr(currency, ' ') == NULL){
            const char* invalidCur = "\nInvalid request. Use the following format <AMOUNT> <SOURCE CURRENCY> <DESTINATION CURRENCY>\n";
            sendto(serverSocketDesc, invalidCur, strlen(invalidCur), MSG_CONFIRM, (struct sockaddr *)&client, c);
            continue;
        }

        //parse request
        char* temp = currency;
        char* chunk1 = strchr(temp, ' ');                                   //looks for space
        strncpy(amount, currency, (strlen(currency) - strlen(chunk1)));     //copies anything before space to amount
        amount[(strlen(currency) - strlen(chunk1))] = '\0';                 //null terminate
        puts(amount);
        temp += (strlen(amount) + 1);                                       //point to starting letter of currency code
        strncpy (src, temp, 3);                                             //copy currency code into src
        src[3] = '\0';                                                      //null terminate
        puts (src);
        temp += 4;                                                          //points to starting letter of second currency code
        strncpy (dest, temp, 3);                                            //copy currency code into dest
        dest[3] = '\0';                                                     //null terminate
        puts (dest);

        //set amount to double
        amountNum = charToDouble(amount);
        printf("%lf\n", amountNum);

        //invalid currency code
        if (((findIndex(src)) == -1 ) || ((findIndex(dest)) == -1 )){
            const char* noCur = "\nInvalid currency code. Supported currencies are CAD, USD, EURO, GBP, or BTC.\n";
            sendto(serverSocketDesc, noCur, strlen(noCur), MSG_CONFIRM, (struct sockaddr *)&client, c);
            continue;
        }
        else {
            //set rates
            srcRate = rates[findIndex(src)];
            printf("%lf\n", srcRate);
            destRate = rates[findIndex(dest)];
            printf("%lf\n", destRate);

            //convert amount
            convertedNum = convert(amountNum, srcRate, destRate);
            printf("%lf\n", convertedNum);

            //double to string to char*
            str = to_string(convertedNum);
            string finalConverted = amount;
            finalConverted.append(" ");
            finalConverted.append(src);
            finalConverted.append(" is equivalent to ");
            finalConverted.append(str);
            finalConverted.append(" ");
            finalConverted.append(dest);

            converted = finalConverted.c_str();
            puts(converted);

            //send to indirection server
            sendto(serverSocketDesc, converted, strlen(converted), MSG_CONFIRM, (struct sockaddr *)&client, c);
            puts("Sent converted currency: ");
            puts(converted);
        }
    }
}