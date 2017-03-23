#include "headers/server.h"
#include "headers/networkhelpers.h"

#include <cstring>
#include <cctype>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <cstdio>
#include <map>
#include <sstream>
#include <iostream>

using namespace std;

map<int, string> sockets;
map<int, string> channels;



/*
 *  FUNCTION:
 *  server
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
 *  void server();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  for consistancy in main, it only calls listenforPackets
 *  
 */
void server() {
    listenForPackets(false);
}


/*
 *  FUNCTION:
 *  listenSock
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
 *  void listenSock(int socket);
 *  int socket - the socket to bind and listen to
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  a client has left, remove its channel and socket settings
 *  
 */
void listenSock(int socket){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(LISTEN_PORT); 

    if ((bind(socket, (struct sockaddr *) &addr, sizeof(addr))) == -1) {
        perror("bind");
        exit(5);
    }

    if (listen(socket, LISTENQ) == -1) {
        perror("listen");
        exit(6);
    }
}

/*
 *  FUNCTION:
 *  closeServer
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
 *  void closeServer(int socket);
 *  int socket -  the socket that was just closed
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  a client has left, remove its channel and socket settings
 *  
 */
void closeServer(int sock){
    sockets.erase(sock);
    channels.erase(sock);
}


/*
 *  FUNCTION:
 *  recvServer
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
 *  void recvServer(int socket, const char *buffer, int packetSize);
 *  int socket - the socket the message is from
 *  const char *buffer - the raw data that was read from the buffer
 *  int packetSize - the size of the packet that was read in
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  This is the function that implements the server logic
 *  the most basic generalization is command vs raw message
 *  commands are all prefixed with /
 *  the implemented commands are:
 *      /ips - list all usernames and their ips
 *      /channels list - list all usernames and their current channel
 *      /channels [channel name] - switch to [channel name]
 *      /pm list - list all usernames wih private message ids
 *      /pm [id] message ... - send [id] the message
 *  the format for messages that are not commands is `id message...`
 *  
 */
void recvServer(int sock, const char *buffer, int packetSize) {
    string temp(buffer);
    stringstream ss(temp);
    switch (*buffer) {
        case '/'://its a command
            {
                ss >> temp;
                //list the ips
                if (temp == "/ips") {
                    ss.str("");
                    ss.clear();
                    ss << "Connect users:";

                    struct sockaddr_in addr;
                    socklen_t len = sizeof(addr);

                    for(const auto& us : sockets) {
                        getpeername(us.first, (struct sockaddr*)(&addr), &len);
                        ss << '\n' << us.second << ':' << string(inet_ntoa(addr.sin_addr));
                    }

                    temp = ss.str();
                    if (send(sock, temp.c_str(), temp.size()+1, 0) == -1) {
                        perror("send");
                        exit(7);
                    }
                    //set the name property
                } else if (temp == "/set") {
                    ss >> temp;
                    if(temp == "name") {
                        //set channel name if the user is new
                        if(!channels.count(sock))
                            channels[sock] = "main";

                        //get trimmed username
                        ss >> temp;
                        sockets[sock] = temp;
                        if(getline(ss, temp))
                            sockets[sock] += temp;

                        ss.str("");
                        ss.clear();

                        //construct packet of all other connected users
                        ss << "/userupdate ";
                        for(const auto& fd : sockets)
                            ss << fd.first << ' ' << fd.second << '\n';
                        temp = ss.str();
                        for(const auto& fd : sockets) {
                            if (send(fd.first, temp.c_str(), temp.size()+1, 0) == -1) {
                                perror("send");
                                exit(7);
                            }
                        }
                    }
                    //channel functions
                } else if (temp == "/channels") {
                    if (ss >> temp) {
                        //force all channels to a new channel
                        if (temp == "force") {
                            ss >> temp;
                            for(auto& ch : channels)
                                ch.second = temp;
                        } else if (temp == "list") {
                            ss.str("");
                            ss.clear();
                            ss << "Current Channels:";
                            for (const auto& us : channels)
                                ss << '\n' << sockets[us.first] << ':' << us.second;
                            temp = ss.str();
                            if (send(sock, temp.c_str(), temp.size()+1, 0) == -1) {
                                perror("send");
                                exit(7);
                            }
                        } else {
                            channels[sock] = temp;
                        }
                    } else {
                        channels[sock] = "main";
                    }
                } else if(temp == "/pm") {
                    int id;
                    ss >> id;
                    if(ss) {
                        if (sockets.count(id)){
                            getline(ss, temp);
                            ss.str("");
                            ss.clear();
                            //blank name with pm title
                            ss << "0 " << sockets[sock] << ">:" << temp;
                            temp = ss.str();
                            if (send(id, temp.c_str(), temp.size()+1, 0) == -1) {
                                perror("send");
                                exit(7);
                            }
                        } else {
                            ss.str("");
                            ss.clear();
                            ss << "ID not found " << id << "\nPrivate Message IDs:";
                            for (const auto& us : sockets)
                                ss << '\n' << us.first << ':' << us.second;
                            temp = ss.str();
                            if (send(sock, temp.c_str(), temp.size()+1, 0) == -1) {
                                perror("send");
                                exit(7);
                            }
                        }
                        break;
                        //show pm ids
                    } else {
                        ss.clear();
                    }
                    if (ss >> temp) {
                        if (temp == "list"){
                            ss.str("");
                            ss.clear();
                            ss << "Private Message IDs:";
                            for (const auto& us : sockets)
                                ss << '\n' << us.first << ':' << us.second;
                            temp = ss.str();
                            if (send(sock, temp.c_str(), temp.size()+1, 0) == -1) {
                                perror("send");
                                exit(7);
                            }
                        }
                    }
                }
            }
            break;
            //normal message
        default:
            {
                ss << sock << ' ' << temp;
                temp = ss.str();
                for (const auto& fd : sockets) {
                    if (channels[fd.first] == channels[sock] && fd.first != sock) {
                        if (send(fd.first, temp.c_str(), temp.size()+1, 0) == -1) {
                            perror("send");
                            exit(9);
                        }
                    }
                }
                break;
            }
    }
}
