#include "headers/ui.h"

#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <ncurses.h>
#include <menu.h>
#include <form.h>


using namespace std;

UI::UI():ouTop(0), ouBot(0), selected(0),state(USER) {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, 1);

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);

    getmaxyx(stdscr , rows, cols);
    ouBot = rows - 2;

    //userlist wid 40 char
    userlist = newwin(rows, 40, 0, 0);
    //msg wid max -40 char
    chat = newwin(rows, cols - 40, 0, 40);

    box(userlist, 0, 0);
    box(chat, 0, 0);

    //on in from either side of max msg
    //height of 3 leave bottom row alone
    fields[0] = new_field(3, cols - 42, rows - 4, 42, 0, 0);
    field_opts_off(fields[0], O_AUTOSKIP);
    msgForm = new_form(fields);

    refresh();
    update();
}

void UI::loop(){
    int c;
    update();
    while((c = getch()) != KEY_F(1)){
        switch(c){
            case KEY_UP:
                movUp();
                updateOnlineItems();
                break;
            case KEY_DOWN:
                movDown();
                updateOnlineItems();
                break;
            default:
                if(state == UI::MSG) {
                    form_driver(msgForm, c);
                }
        }
    }
}

UI::~UI(){
    delwin(userlist);
    delwin(chat);
    endwin();
}


void UI::addUser(const char *user){
    //40 - 2 from selection char
    int pad = 32 - strlen(user);
    if(pad > 0)
        onlineUsers.push_back(user + string(pad, ' '));
    else 
        onlineUsers.push_back(user);
}

void UI::updateOnlineItems() {
    for(int i = ouTop, j = 1; (i < ouBot) && (j < rows - 1) && (i < (int)onlineUsers.size()); ++i, ++j){
        if(i == selected)
            mvwprintw(userlist, j, 1, ("> " + onlineUsers.at(i)).c_str());
        else
            mvwprintw(userlist, j, 1, ("  " + onlineUsers.at(i)).c_str());
    }
    wrefresh(userlist);
}

void UI::movUp(){
    if(state != USER)
        return;
    if(selected == ouTop && ouTop > 0){
        --ouTop;
        --ouBot;
        --selected;
    } else if (selected > ouTop) {
        --selected;
    }
}

void UI::movDown(){
    if(state != USER)
        return;
    //without +1 it will scoll one line too late
    if(selected + 1 == ouBot && ouBot < (int)onlineUsers.size()){
        ++ouTop;
        ++ouBot;
        ++selected;
    } else if (selected + 1 < ouBot) {
        ++selected;
    }
}


void UI::clear(){
    wclear(userlist);
    wclear(chat);
    box(userlist, 0, 0);
    box(chat, 0, 0);
}

void UI::update(){
    box(userlist, 0, 0);
    box(chat, 0, 0);
    form_driver(msgForm, REQ_NEXT_FIELD);
    form_driver(msgForm, REQ_PREV_FIELD);
    refresh();
    wrefresh(userlist);
    wrefresh(chat);
}
