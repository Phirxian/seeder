#include "SeederClient.h"
#include <ncurses.h>
#include <iostream>

using namespace network;

WINDOW *full, *chatbox, *inputwin;
int i = 0;

class ChatClient : public SeederListener
{
    public:
        ChatClient() { }
        
        void setSeederSocket(SeederClient *socket) { se = socket; }

        virtual void recvData(const ReceivedMessage &msg)
        {
            wprintw(chatbox, msg.buffer, i);
            wprintw(chatbox, "\n", i);
            wrefresh(chatbox);
            i++;
            //std::cout << msg.to_string() << std::endl;
        }

        virtual void notifySendError(const SendingMessage &msg, int error)
        {
        }
        
        virtual void notifySendError(const UnconnectedMessage&, int error) { }
        virtual void notifyPeerLost(ConnectedClient *client, int index) { }
        virtual void notifyPeerCrashed(ConnectedClient *client, int index) { }
        virtual void notifyPeerConnected(ConnectedClient *client, int index) { }
        virtual void notifyPeerDisconnected(ConnectedClient *client, int index) { }

        virtual void notifyConnected(bool) {}
        virtual void notifyConnectionLost() {}
    
    protected:
        SeederClient *se;
};

void* networkThread(void *user)
{
    SeederClient *se = (SeederClient*)user;
    while(se->isConnected())
        se->run();
    return nullptr;
}

int main(int argc, const char **argv)
{
    ChatClient *srv = new ChatClient();
    SeederClient *se = new SeederClient(srv);
    srv->setSeederSocket(se);
    
    SocketInfo info(option::STM_TCP, 4096, 300);
    se->init(info);
    se->connectLocalHost();
    
    if(!se->isConnected())
        return 1;
    
    pthread_t thread;
    pthread_create(&thread, nullptr, networkThread, se);
    
    initscr();
    nocbreak();
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    full = newwin(height-3, width, 0, 0);
    box(full, 0, 0);
    chatbox = derwin(full, height-5, width - 2, 1, 1);
    scrollok(chatbox,true);
    wrefresh(full);
    
    inputwin = newwin(3, width, height-3, 0);
    box(inputwin, 0, 0);
    wmove(inputwin, 1, 1);
    wrefresh(inputwin);
    
    std::string input;
    input.reserve(80);
    
    while(true)
    {
        int c = wgetch(inputwin);
        
        box(inputwin, 0, 0);
        box(full, 0, 0);
        wrefresh(full);
        wrefresh(chatbox);
        wrefresh(inputwin);
        refresh();
            
        if(c == '\n')
        {
            SendingMessage send(input.c_str(), input.size()+1);
            se->sendData(send);
            input.clear();
        }
        else
        {
            input.push_back(c);
            wclear(inputwin);
        }
        
        wmove(inputwin, 1, 1);
    }
        
    endwin();
    
    void *retval;
    se->shutdown();
    pthread_join(thread, &retval);
    
    delete se;
}
