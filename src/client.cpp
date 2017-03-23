#include "headers/networkhelpers.h"
#include "headers/server.h"
#include "headers/client.h"


#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <thread>
#include <sstream>
#include <map>

using namespace std;

//id, username
map<int,string> users;

/*
 *  FUNCTION:
 *  client
 *  --
 *  DATE:
 *  march 20, 2017
 *  --
 *  DESIGNER:
 *  isaac morneau
 *  --
 *  PROGRAMMER:
 *  isaac morneau
 *  --
 *  INTERFACE:
 *  void main();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  starts the network thread, prompts for connection details then loops
 *  on user input. it handles the buffering of waiting for user requests.
 *  
 */
void client(){
    thread t(listenForPackets, true);
    //those sweet sweet spin locks
    while(!socketfd.load());

    string name;

    cout << "Please enter your username: " << endl;
    if (!getline(cin, name)) {
        //they closed stdin, just quit silently
        exit(1);
    }
    string text { "/set name " };
    text += name;
    if (send(socketfd, text.c_str(), text.size()+1, 0) == -1) {
        perror("send failure");
        exit(2);
    }

    while (getline(cin, text)) {
        if (text == "/exit")
            break;
        if (send(socketfd, text.c_str(), text.size()+1, 0) == -1) {
            perror("send failure");
            exit(3);
        }
    }

    exit(0);
    t.join();
}


/*
 *  FUNCTION:
 *  connectSock
 *  --
 *  DATE:
 *  march 20, 2017
 *  --
 *  DESIGNER:
 *  isaac morneau
 *  --
 *  PROGRAMMER:
 *  isaac morneau
 *  --
 *  INTERFACE:
 *  void connectSock(int socket);
 *  int socket - the socket to connect to
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  prompts for hostname and then connects the specified socket to it
 *  
 */
void connectSock(int socket) {
    hostent *server;
    string host;

    while(1){
        cout << "Please enter hostname" << endl;
        getline(cin,host);
        if ((server = gethostbyname(host.c_str())) != nullptr)
            break;
        cout << "Invalid Host." << endl;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    memmove(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
    addr.sin_port = htons(LISTEN_PORT);

    if (connect(socket, (sockaddr *) &addr, sizeof(addr)) < 0) {
        if (errno != EINPROGRESS && errno != EALREADY) {
            perror("Failed to connect");
            exit(4);
        }
    }
    cout << "Connected to server [" << host << ']' << endl;
}


/*
 *  FUNCTION:
 *  closeClient
 *  --
 *  DATE:
 *  march 20, 2017
 *  --
 *  DESIGNER:
 *  isaac morneau
 *  --
 *  PROGRAMMER:
 *  isaac morneau
 *  --
 *  INTERFACE:
 *  void closeClient();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  this means that the server was shutdown so just exit the program
 *  
 */
void closeClient(){
    cout << "Server Closed." << endl;
    exit(0);
}


/*
 *  FUNCTION:
 *  recvClient
 *  --
 *  DATE:
 *  march 20, 2017
 *  --
 *  DESIGNER:
 *  isaac morneau
 *  --
 *  PROGRAMMER:
 *  isaac morneau
 *  --
 *  INTERFACE:
 *  void recvClient(int socket, const char *buffer, int packetSize);
 *  int socket - the socket the message is from
 *  const char *buffer - the raw data that was read from the buffer
 *  int packetSize - the size of the packet that was read in
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  This is the function that implements the client logic
 *  the most basic generalization is command vs raw message
 *  commands are all prefixed with /
 *      these commands have all their logic implemented server side
 *  the format for messages that are not commands is `id message...`
 *  
 */
void recvClient(int socket, const char *buffer, int packetSize){
    string temp(buffer);
    stringstream ss(temp);
    int id;
    switch(*buffer){
        //command
        case '/':
            ss >> temp;
            if (temp == "/userupdate") {
                users.clear();
                users[0] = "PM";
                while(ss >> id >> temp){
                    users[id] = temp;
                    if(!getline(ss,temp)){
                        break;
                    }
                    users[id] += temp;
                }
            }
            break;
            //normal message
        default:
            ss >> id;
            getline(ss, temp);
            cout << '[' << users[id] << "]: " << temp << endl;
            break;
    }
}
