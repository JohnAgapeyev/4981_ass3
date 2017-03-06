#include "headers/ui.h"

#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <ncurses.h>


using namespace std;

UI::UI():ouTop(0), ouBot(0), selected(0), mTop(0), mBot(0), curChar(0), state(USER) {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, 1);

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);

    getmaxyx(stdscr , rows, cols);

    //drawable size
    rowsMsg = rows - 3;
    rowsUser = rows - 1;

    colsMsg = cols - 42;
    colsUser = 38;

    memset(curMsg, 0, MAXMSG);
    curMsg[MAXMSG] = 0;

    //start at the bottom of the drawable window
    ouBot = rowsUser;
    mBot = rowsMsg;

    //userlist wid 40 char
    userlist = newwin(rows, 40, 0, 0);
    //msg wid max -40 char
    chat = newwin(rows, cols - 40, 0, 40);
    chatMsg = subwin(chat, rows-3, cols - 40, 0, 40);
    chatInput = subwin(chat, 3, cols - 40, rows-3, 40);

    //border the windows
    box(userlist, 0, 0);
    box(chatMsg, 0, 0);
    box(chatInput, 0, 0);

    refresh();
    update();
}

UI::~UI(){
    delwin(userlist);
    delwin(chat);
    endwin();
}

void UI::clear(){
    wclear(userlist);
    wclear(chat);
    box(userlist, 0, 0);
    box(chatMsg, 0, 0);
    box(chatInput, 0, 0);
}

void UI::update(){
    box(userlist, 0, 0);
    box(chatMsg, 0, 0);
    box(chatInput, 0, 0);
    refresh();
    wrefresh(userlist);
    wrefresh(chat);
    wrefresh(chatMsg);
    wrefresh(chatInput);
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
//            case KEY_LEFT:
//                leftChar();
//                break;
//            case KEY_RIGHT:
//                rightChar();
//                break;
            case KEY_F(2):
                state = USER;
                break;
            case KEY_F(3):
                state = MSG;
                break;
            case KEY_BACKSPACE:
                if(state == MSG)
                    popMsgChar();
                break;
            case '\n':
            case KEY_ENTER:
                //send message | select DB
                if(state == MSG)
                    sendMsg();
                break;
            default:
                if(state == MSG) {
                    addMsgChar(c);
                    updateMessages();
                }
                break;
        }
    }
}

/*
//while theses work they end up acting in replace mode so i decided to leave them out
void UI::leftChar(){
    if(curChar > 0) {
        --curChar;
        wmove(chat, rowsMsg+1, curChar < colsMsg - 1 ? curChar : colsMsg);
        wrefresh(chat);
    }
}
void UI::rightChar(){
    if(curChar <= (int)strlen(curMsg)) {
        ++curChar;
        wmove(chat, rowsMsg+1, curChar < colsMsg - 1? curChar : colsMsg);
        wrefresh(chat);
    }
}*/

void UI::updateMessages(){
    for(int i = mTop, j = 1; (i < mBot) && (j < rowsMsg - 1) && (i < (int)messages.size()); ++i, ++j){
        mvwprintw(chat, j, 1, messages.at(i).c_str());
    }
    mvwprintw(chat, rowsMsg + 1, 1, curMsg);
    wrefresh(chat);
}

void UI::addMsgChar(const char c){
    if(curChar < MAXMSG){
        curMsg[curChar++] = c;
        wrefresh(chat);
    }
}

void UI::popMsgChar(){
    if(curChar > 0){
        mvwaddch(chat, rowsMsg+1, curChar, ' ');
        wmove(chat, rowsMsg+1, curChar < colsMsg - 1 ? curChar : colsMsg);
        curMsg[curChar--] = 0;
        wrefresh(chat);
    }
}

void UI::sendMsg(){
    messages.push_back(curMsg);
    memset(curMsg, ' ', MAXMSG);
    mvwprintw(chat, rowsMsg + 1, 1, curMsg);
    memset(curMsg, 0, MAXMSG);
    curChar = 0;
    wmove(chat, rowsMsg+1, 0);
    updateMessages();
    //send it to the server
}

void UI::addMsg(const char *c){
    messages.push_back(c);
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
    for(int i = ouTop, j = 1; (i+1 < ouBot) && (j < rowsUser) && (i < (int)onlineUsers.size()); ++i, ++j){
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
    if (selected > 0)
        --selected;
    if(selected-1 <= ouTop && ouTop > 0){
        int diff = ouBot - ouTop -1;
        ouTop -= diff;
        ouBot -= diff;
        wclear(userlist);
        box(userlist,0,0);
    }
}

void UI::movDown(){
    if(state != USER)
        return;
    if (selected < (int)onlineUsers.size())
        ++selected;
    if(selected+1 >= ouBot){
        int diff = ouBot - ouTop - 1;
        ouTop += diff;
        ouBot += diff;
        wclear(userlist);
        box(userlist,0,0);
    }
}

