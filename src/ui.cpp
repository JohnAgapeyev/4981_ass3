/*
 *  SOURCE FILE:
 *  ui.cpp
 *  --
 *  PROGRAM: 4981_ass3
 *  --
 *  FUNCTIONS:
 *  void addUser(const char *); 
 *  void updateOnlineItems();
 *  void updateMessages();
 *  void addMsgChar(const char c);
 *  void popMsgChar();
 *  void rmMsgChar(size_t i);
 *  void sendMsg();
 *  void addMsg(const char *c);
 *  void movUp();
 *  void movDown();
 *  void drawMenu();
 *  void loop();
 *  std::string loopGetHost();
 *  void loopGetName();
 *  std::string getName();
 *  void clear();
 *  void update();
 *  UI();
 *  ~UI();
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  NOTES:
 *  This method handles all the UI-related calls
 */
#include "headers/ui.h"
#include "headers/client.h"
#include "headers/main.h"
#include "headers/packet.h"

#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <ncurses.h>
#include <sys/socket.h>

using namespace std;

/*
 *  FUNCTION:
 *  UI constructor
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  UI();
 *  --
 *  NOTES:
 *  Creates, initializes and renders the initial window for use in the program
 */
UI::UI():ouTop(0), ouBot(0), selected(0), curChar(0), state(MSG) {
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

    memset(curMsg,0,MAXMSG);

    //start at the bottom of the drawable window
    ouBot = rowsUser;

    //userlist wid 40 char
    userlist = newwin(rows, colsUser, 0, 0);
    //msg wid max -40 char
    chatMsg = newwin(rowsMsg, colsMsg, 1, 40);
    chatInput = newwin(3, colsMsg, rows-3, 40);

    clear();
    drawMenu();
    refresh();
    update();
}

/*
 *  FUNCTION:
 *  UI destructor
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  ~UI();
 *  --
 *  NOTES:
 *  Deletes the window and frees their resources
 */
UI::~UI(){
    delwin(userlist);
    delwin(chatMsg);
    delwin(chatInput);
    endwin();
}

/*
 *  FUNCTION:
 *  drawMenu
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void drawMenu();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Draws the top menu bar onto the ncurses menu
 */
void UI::drawMenu(){
    attron(COLOR_PAIR(1));
    mvwprintw(stdscr, 0, 42, "[Exit: F1]");
    attroff(COLOR_PAIR(1));
    
    if(state == USER)
        attron(A_STANDOUT);
    attron(COLOR_PAIR(2));
    wprintw(stdscr, "[User Selection: F2]");
    attroff(COLOR_PAIR(2));
    if(state == USER)
        attroff(A_STANDOUT);
    
    if(state == MSG)
        attron(A_STANDOUT);
    attron(COLOR_PAIR(3));
    wprintw(stdscr, "[Chat Window: F3]");
    attroff(COLOR_PAIR(3));
    if(state == MSG)
        attroff(A_STANDOUT);
}

/*
 *  FUNCTION:
 *  clear
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void clear();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Clears the screen of any printed text
 */
void UI::clear(){
    memset(curMsg, 0, MAXMSG);
    curChar = 0;
    wclear(userlist);
    wclear(chatMsg);
    wclear(chatInput);
    box(userlist, 0, 0);
    box(chatMsg, 0, 0);
    box(chatInput, 0, 0);
}

/*
 *  FUNCTION:
 *  update
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void update();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Updates the UI screen. This forces a rerender of the screen. This is called whenever an 
 *  and update to the UI is performed, and the screen needs to render it.
 */
void UI::update(){
    wmove(chatInput, 1, 1 + curChar);
    refresh();
    wrefresh(userlist);
    wrefresh(chatMsg);
    wrefresh(chatInput);
}

/*
 *  FUNCTION:
 *  loopGetHost();
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  std::string loopGetHost();
 *  --
 *  RETURNS:
 *  std::string - The hostname the user entered
 *  --
 *  NOTES:
 *  Waits for the user to enter a hostname and returns it as a string
 */
std::string UI::loopGetHost(){
    int c;
    bool run = true;
    char host[HOSTLEN];
    memset(host, 0, HOSTLEN);

    int cur = 0;

    WINDOW *temp = newwin(4, HOSTLEN + 2, rows / 2, cols / 2 - HOSTLEN / 2);
    box(temp,0,0);

    const char *msg = "Enter Hostname Of Server:";
    mvwprintw(temp, 1, strlen(msg)/2, msg);
    attron(A_STANDOUT);
    while(run) {
        mvwprintw(temp, 2, 1, host);
        mvwaddch(temp, 2, 1 + cur, ' ');
        wmove(temp,2,1 + cur);
        wrefresh(temp);
        switch(c = getch()){
            case KEY_F(1):
                exit(1);
                break;
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
                break;
            case KEY_BACKSPACE:
                if(cur > 0)
                    host[cur--] = 0;
                break;
            case '\n':
            case KEY_ENTER:
                run = false;
                break;
            default:
                if(cur < HOSTLEN)
                    host[cur++] = c;
                break;
        }
    }
    attroff(A_STANDOUT);
    delwin(temp);
    return host;
}

/*
 *  FUNCTION:
 *  loopGetName
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void loopGetName();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Prompts the user to enter their username and sets it locally for the client
 */
void UI::loopGetName(){
    int c;
    bool run = true;
    char name[NAMELEN+1];
    memset(name, 0, HOSTLEN);

    int cur = 0;

    WINDOW *temp = newwin(4, NAMELEN + 2, rows / 2, cols / 2 - NAMELEN / 2);
    box(temp,0,0);

    const char *msg = "Enter Username:";
    mvwprintw(temp, 1, strlen(msg)/2, msg);
    attron(A_STANDOUT);
    while(run) {
        mvwprintw(temp, 2, 1, name);
        mvwaddch(temp, 2, 1 + cur, ' ');
        wmove(temp,2,1 + cur);
        wrefresh(temp);
        switch(c = getch()){
            case KEY_F(1):
                exit(1);
                break;
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
                break;
            case KEY_BACKSPACE:
                if(cur > 0)
                    name[cur--] = 0;
                break;
            case '\n':
            case KEY_ENTER:
                run = false;
                break;
            default:
                if(cur < HOSTLEN)
                    name[cur++] = c;
                break;
        }
    }
    attroff(A_STANDOUT);
    delwin(temp);
    username = name;
}

/*
 *  FUNCTION:
 *  loop
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void loop();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  The main UI update loop that handles key press events.
 */
void UI::loop(){
    int c;
    clear();
    update();
    for(;;) {
        switch(c = getch()){
            case KEY_LEFT:
            case KEY_RIGHT:
                break;

            case KEY_F(1):
                exit(1);
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

/*
 *  FUNCTION:
 *  updateMessages
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void updateMessages();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Updates the main message window with any newly received messages
 */
void UI::updateMessages(){
    int j = rowsMsg - 2;
    wclear(chatMsg);
    box(chatMsg, 0, 0);
    for(const auto& m : messages){
        if(j == 2)
            break;
        mvwprintw(chatMsg, j--, 1, m.c_str());
    }
    wmove(chatInput, 1, 1 + curChar);
    wrefresh(chatMsg);
    wrefresh(chatInput);
}

/*
 *  FUNCTION:
 *  addMsgChar
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void addMsgChar(const char c);
 *  --
 *  ARGS:
 *  const char c - The character to add to the current message
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Adds a character to the pending message
 */
void UI::addMsgChar(const char c){
    if(curChar < MAXMSG){
        curMsg[curChar++] = c;
        mvwprintw(chatInput, 1, 1, curMsg);
        wrefresh(chatInput);
    }
}

/*
 *  FUNCTION:
 *  popMsgChar
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void popMsgChar();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Removes the last char from the pending message
 */
void UI::popMsgChar(){
    if(curChar > 0)
        curMsg[curChar--] = 0;
    mvwaddch(chatInput, 1, 1 + curChar, ' ');
    wmove(chatInput, 1,1 + curChar);
    wrefresh(chatInput);
}

/*
 *  FUNCTION:
 *  sendMsg
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void sendMsg();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Sends the pending message to the server after generating the required header
 */
void UI::sendMsg(){
    if(!curChar)
        return;
    messages.push_front(username + ": " + std::string(curMsg));
    int buffSize = MAXMSG;
    char buff[MAXMSG];
    genMsgPacket(buff, &buffSize, curMsg, username.c_str());
    if (send(Socket, buff, buffSize, 0) < 0) {
        perror("Send failure");
    }
    wclear(chatInput);
    box(chatInput, 0, 0);
    memset(curMsg, 0, MAXMSG);
    curChar = 0;
    updateMessages();

    wmove(chatInput, rowsMsg + 1, 0);
    wrefresh(chatInput);
}

/*
 *  FUNCTION:
 *  addMsg
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void addMsg(const char *c);
 *  --
 *  ARGS:
 *  cost char *c - The message to add to the message history
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Adds a message to the message history log
 */
void UI::addMsg(const char *c){
    if(strlen(c)){
        messages.push_front(c);
        updateMessages();
    }
}

/*
 *  FUNCTION:
 *  addUser
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void addUser(const char *user);
 *  --
 *  ARGS:
 *  const char *user - The username to add to the list
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Adds a username to the list of connected clients
 */
void UI::addUser(const char *user){
    //40 - 2 from selection char
    int pad = 32 - strlen(user);
    if(pad > 0)
        onlineUsers.push_back(user + string(pad, ' '));
    else 
        onlineUsers.push_back(user);
    updateOnlineItems();
}

/*
 *  FUNCTION:
 *  updateOnlineItems
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void updateOnlineItems();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Updates the client list based on client connections or disconnections
 */
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

/*
 *  FUNCTION:
 *  movUp
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void movUp();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Moves the cursor up one level in the client list
 */
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

/*
 *  FUNCTION:
 *  movDown
 *  --
 *  DATE:
 *  March 20, 2017
 *  --
 *  DESIGNER:
 *  Isaac Morneau
 *  --
 *  PROGRAMMER:
 *  Isaac Morneau
 *  --
 *  INTERFACE:
 *  void movDown();
 *  --
 *  RETURNS:
 *  void
 *  --
 *  NOTES:
 *  Moves the cursor down one level in the client list
 */
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

