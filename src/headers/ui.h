#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <menu.h>
#include <form.h>
#include <vector>
#include <string>
#include <memory>

#define MAXMSG 128

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
        void leftChar();
        void rightChar();
        void rmMsgChar(size_t i);
        void sendMsg();
        void addMsg(const char *c);
       
        void movUp();
        void movDown();


        void loop();
        //resets the ui to blank
        //this will cause flashes so dont use it all the time
        void clear();
        //refreshes the ui
        void update();
    private:
        std::vector<std::string> onlineUsers;
        int ouTop, ouBot;
        int selected;
        std::vector<std::string> messages;
        int mTop, mBot;
        char curMsg[MAXMSG+1];
        int curChar;

        Selection state;

        int rows, cols;
        int rowsUser, colsUser;
        int rowsMsg, colsMsg;

        FIELD *fields[2];
        FORM *msgForm;


        WINDOW *userlist;
        WINDOW *chat;
};

#endif
