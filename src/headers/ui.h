#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <menu.h>
#include <form.h>
#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <atomic>
#include <string>

#define MAXMSG 128
#define HOSTLEN 80

class UI {
    public:

        enum Selection {
            MSG,
            USER,
            NAME
        };

        UI();
        ~UI();

        //add a new user
        void addUser(const char *); 

        //update currentlu displayed users
        void updateOnlineItems();
        
        //update messages
        void updateMessages();
        void addMsgChar(const char c);
        void popMsgChar();
        void rmMsgChar(size_t i);
        void sendMsg();
        void addMsg(const char *c);
       
        void movUp();
        void movDown();

        void drawMenu();

        void loop();
        std::string loopGetHost();

        //tell the ui its time to stop
        void close() {
            running = false;
        }

        //resets the ui to blank
        //this will cause flashes so dont use it all the time
        void clear();
        //refreshes the ui
        void update();
    private:
        std::vector<std::string> onlineUsers;
        int ouTop, ouBot;
        int selected;
        std::deque<std::string> messages;
        char curMsg[MAXMSG+1];
        int curChar;

        std::atomic<bool> running;

        Selection state;

        int rows, cols;
        int rowsUser, colsUser;
        int rowsMsg, colsMsg;


        WINDOW *userlist;

        WINDOW *chatMsg;
        WINDOW *chatInput;
};

#endif
