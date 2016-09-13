#include "Server.h"
#include <iostream>

using namespace std;
using namespace swoole;

class MyServer : public Server
{
public:
    MyServer(string _host, int _port, int _mode = SW_MODE_PROCESS, int _type = SW_SOCK_TCP) :
            Server(_host, _port, _mode, _type)
    {

    }

    virtual void onStart();

    virtual void onShutdown(){

    }

    virtual void onWorkerStart(int worker_id){

    }

    virtual void onWorkerStop(int worker_id){

    }

    virtual void onReceive(int fd, string &data);

    virtual void onConnect(int fd);

    virtual void onClose(int fd);

    virtual void onPacket(string &data, ClientInfo &clientInfo){

    }
};

void MyServer::onReceive(int fd, string &data)
{
    cout << "fd=" << fd << "data" << data << endl;
    int ret;
    char resp_data[SW_BUFFER_SIZE];
    int n = snprintf(resp_data, SW_BUFFER_SIZE, (char *) "Server: %*s\n", (int) data.length(), data.c_str());
    ret = this->send(fd, resp_data, (uint32_t) n);
    if (ret < 0)
    {
        printf("send to client fail. errno=%d\n", errno);
    }
    else
    {
        printf("send %d bytes to client success. data=%s\n", n, resp_data);
    }
    this->close(fd);
}

void MyServer::onConnect(int fd)
{
    printf("PID=%d\tConnect fd=%d\n", getpid(), fd);
}

void MyServer::onClose(int fd)
{
    printf("PID=%d\tClose fd=%d\n", getpid(), fd);
}

void MyServer::onStart()
{
    printf("server is start\n");
}

int main(int argc, char **argv)
{
    MyServer server("127.0.0.1", 9501);
    server.listen("127.0.0.1", 9502, SW_SOCK_UDP);
    server.listen("::1", 9503, SW_SOCK_TCP6);
    server.listen("::1", 9504, SW_SOCK_UDP6);
    server.setEvents(EVENT_onStart | EVENT_onReceive | EVENT_onClose);
    server.start();
    return 0;
}
