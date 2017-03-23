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

map<int,string> users;

void client(){
    thread t(listenForPackets, true);
    users[0] = "Server";
    //those sweet sweet spin locks
    while(!socketfd);

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

void closeClient(int socket){
    cout << "Server Closed." << endl;
    exit(0);
}
void recvClient(int socket, const char *buffer, int packetSize){
    string temp(buffer);
    stringstream ss(temp);
    int id;
    switch(*buffer){
        case '/':
            ss >> temp;
            if (temp == "/userupdate") {
                users.clear();
                while(ss >> id >> temp){
                    users[id] = temp;
                    if(!getline(ss,temp)){
                        break;
                    }
                    users[id] += temp;
                }
            }
            break;
        default:
            ss >> id;
            getline(ss, temp);
            cout << '[' << users[id] << "]: " << temp << endl;
            break;
    }
}
