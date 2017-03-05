#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <menu.h>
#include <form.h>
#include <vector>
#include <string>



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

        Selection state;

        int rows, cols;

        FIELD *fields[1];//theres only one right now but add the username later
        FORM *msgForm;


        WINDOW *userlist;
        WINDOW *chat;
};

#endif
