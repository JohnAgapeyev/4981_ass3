#include "headers/ui.h"
#include "headers/client.h"
#include "headers/main.h"

#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <ncurses.h>
#include <sys/socket.h>


using namespace std;

UI::UI():ouTop(0), ouBot(0), selected(0), curChar(0), running(true), state(MSG) {
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
    rowsMsg = rows - 4;
    rowsUser = rows - 1;

    colsMsg = cols - 42;
    colsUser = 40;

    memset(curMsg, 0, MAXMSG);
    curMsg[MAXMSG] = 0;

    //start at the bottom of the drawable window
    ouBot = rowsUser;

    //userlist wid 40 char
    userlist = newwin(rows, colsUser, 0, 0);
    //msg wid max -40 char
    chatMsg = newwin(rowsMsg, colsMsg, 1, 40);
    chatInput = newwin(3, colsMsg, rows-3, 40);

    //border the windows
    box(userlist, 0, 0);
    box(chatMsg, 0, 0);
    box(chatInput, 0, 0);

    drawMenu();

    refresh();
    update();
}

UI::~UI(){
    delwin(userlist);
    delwin(chatMsg);
    delwin(chatInput);
    endwin();
}

void UI::drawMenu(){
    attron(COLOR_PAIR(1));
    mvwprintw(stdscr, 0, 42, "[Exit: F1]");
    attroff(COLOR_PAIR(1));
    
    if(state == USER)
        attron(A_STANDOUT);
    attron(COLOR_PAIR(2));
    wprintw(stdscr,"[User Selection: F2]");
    attroff(COLOR_PAIR(2));
    if(state == USER)
        attroff(A_STANDOUT);
    
    if(state == MSG)
        attron(A_STANDOUT);
    attron(COLOR_PAIR(3));
    wprintw(stdscr,"[Chat Window: F3]");
    attroff(COLOR_PAIR(3));
    if(state == MSG)
        attroff(A_STANDOUT);
}

void UI::clear(){
    wclear(userlist);
    wclear(chatMsg);
    wclear(chatInput);
    box(userlist, 0, 0);
    box(chatMsg, 0, 0);
    box(chatInput, 0, 0);
}

void UI::update(){
    refresh();
    wrefresh(userlist);
    wrefresh(chatMsg);
    wrefresh(chatInput);
}

void UI::loop(){
    int c;
    update();
    while(running) {
        //every second unblock and see if the connection was terminated
        timeout(1000);
        c = getch();
        switch(c){
            case ERR:
                //no input which is fine
                //just check again
                break;
            case KEY_F(1):
                running = false;
                break;
            case KEY_F(2):
                state = USER;
                drawMenu();
                break;
            case KEY_F(3):
                state = MSG;
                drawMenu();
                break;

            case KEY_UP:
                movUp();
                break;
            case KEY_DOWN:
                movDown();
                break;
            case KEY_LEFT:
                break;
            case KEY_RIGHT:
                break;

            case KEY_BACKSPACE:
                if(state == MSG)
                    popMsgChar();
                break;
            case '\n':
            case KEY_ENTER:
                //send message | select DM
                if(state == MSG)
                    sendMsg();
                break;
            default:
                if(state == MSG) {
                    addMsgChar(c);
                }
                break;
        }
    }
}

void UI::updateMessages(){
    int j = rowsMsg - 2;
    wclear(chatMsg);
    box(chatMsg, 0, 0);
    for(const auto& m : messages){
        if(j == 2)
            break;
        mvwprintw(chatMsg, j--, 1, m.c_str());
    }
    wrefresh(chatMsg);
}

void UI::addMsgChar(const char c){
    if(curChar < MAXMSG){
        curMsg[curChar++] = c;
        mvwprintw(chatInput, 1, 1, curMsg);
        wrefresh(chatInput);
    }
}

void UI::popMsgChar(){
    if(curChar > 0){
        mvwaddch(chatInput, 1, curChar, ' ');
        wmove(chatInput, 1, curChar < colsMsg - 1 ? curChar : colsMsg);
        curMsg[curChar--] = 0;
        wrefresh(chatInput);
    }
}

void UI::sendMsg(){
    if(!curChar)
        return;
    addMsg(curMsg);

    if (send(Socket, curMsg, curChar, 0) < 0) {
        perror("Send failure");
    }
    //setMessagePending(curMsg);
    wclear(chatInput);
    box(chatInput, 0, 0);
    memset(curMsg, 0, MAXMSG);
    curChar = 0;
    updateMessages();

    wmove(chatInput, rowsMsg + 1, 0);
    wrefresh(chatInput);
}

void UI::addMsg(const char *c){
    messages.push_front(c);
    updateMessages();
}



void UI::addUser(const char *user){
    //40 - 2 from selection char
    int pad = 32 - strlen(user);
    if(pad > 0)
        onlineUsers.push_back(user + string(pad, ' '));
    else 
        onlineUsers.push_back(user);
    updateOnlineItems();
}

void UI::updateOnlineItems() {
    for(int i = ouTop, j = 1; (i+1 < ouBot) && (j < rowsUser) && (i < (int)onlineUsers.size()); ++i, ++j){
        if(i == selected) {
            attron(A_STANDOUT);
            mvwprintw(userlist, j, 1, ("> " + onlineUsers.at(i)).c_str());
            attroff(A_STANDOUT);
        } else {
            mvwprintw(userlist, j, 1, ("  " + onlineUsers.at(i)).c_str());
        }
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
    updateOnlineItems();
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
    updateOnlineItems();
}

