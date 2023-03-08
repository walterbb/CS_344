#include "server.h"
#include "cipher.h"

int main(int argc, char *argv[]){
    struct sockaddr_in serverAddress;

    // Check usage & args
    if (argc < 2) { 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1);
    } 

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("enc_server: ERROR opening socket", 1);
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        error("enc_server: ERROR on binding", 1);
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5); 

    // Accept a connection, blocking if one is not available until one connects
    while(1) {
        int connectionSocket;
        struct sockaddr_in clientAddress;
        socklen_t sizeOfClientInfo = sizeof(clientAddress);

        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket, (struct sockaddr *) &clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0) {
            perror("enc_server: ERROR on accept");
        }

        printf("enc_server: Connected to client running at host %d port %d\n", 
                                ntohs(clientAddress.sin_addr.s_addr),
                                ntohs(clientAddress.sin_port));

        // service client in a childe process
        pid_t pid = fork();

        // error handling for fork()
        if (pid < 0) {
            perror("enc_server: ERROR on fork");
        }
        // child process
        else if (pid == 0) {
            int charsRead, charsSent;
            char plainText[200000];
            char keyText[200000];
            char cipherText[200000];

            // send authentication
            char authCode[] = "enc_server";
            sendAll(authCode, connectionSocket);
            sendAll("@@", connectionSocket);

            // receive plaintext
            receive(plainText, connectionSocket);
            printf("server received plaintext: %s\n", plainText);

            // receive key
            receive(keyText, connectionSocket);
            printf("server received key: %s\n", keyText);

            // encode plaintext
            encode(plainText, keyText, cipherText);

            // send ciphertext
            sendAll(cipherText, connectionSocket);
            sendAll("@@", connectionSocket);
        }
        // parent process
        else {

        }

        // Close the connection socket for this client
        close(connectionSocket);
    }

    // Close the listening socket
    close(listenSocket); 
    return 0;
}
