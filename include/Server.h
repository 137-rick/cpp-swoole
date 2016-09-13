//
// Created by htf on 16-9-12.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <swoole/config.h>
#include <swoole/Server.h>

#include <vector>
#include <string>
using namespace std;

namespace swoole
{
    class ClientInfo
    {
    public:
        char address[256];
        int port;
        int server_socket;
    };

    enum
    {
        EVENT_onStart = 1u << 1,
        EVENT_onShutdown = 1u << 2,
        EVENT_onWorkerStart = 1u << 3,
        EVENT_onWorkerStop = 1u << 4,
        EVENT_onConnect = 1u << 5,
        EVENT_onReceive = 1u << 6,
        EVENT_onPacket = 1u << 7,
        EVENT_onClose = 1u << 8,
    };

    class Server
    {
    public:
        Server(string _host, int _port, int _mode = SW_MODE_PROCESS, int _type = SW_SOCK_TCP);

        virtual ~Server()
        {};

        bool start(void);
        void setEvents(int _events);
        bool listen(string host, int port, int type);
        bool send(int fd, string &data);
        bool send(int fd, const char *data, int length);
        bool close(int fd, bool reset = false);
        bool sendto(string &ip, int port, string &data, int server_socket = -1);

        virtual void onStart() = 0;
        virtual void onShutdown() = 0;
        virtual void onWorkerStart(int worker_id) = 0;
        virtual void onWorkerStop(int worker_id) = 0;
        virtual void onReceive(int fd, string &data) = 0;
        virtual void onConnect(int fd) = 0;
        virtual void onClose(int fd) = 0;
        virtual void onPacket(string &data, ClientInfo &clientInfo) = 0;

    public:
        static int _onReceive(swServer *serv, swEventData *req);
        static void _onConnect(swServer *serv, swDataHead *info);
        static void _onClose(swServer *serv, swDataHead *info);
        static int _onPacket(swServer *serv, swEventData *req);
        static void _onStart(swServer *serv);
        static void _onShutdown(swServer *serv);
        static void _onWorkerStart(swServer *serv, int worker_id);
        static void _onWorkerStop(swServer *serv, int worker_id);

    protected:
        swServer serv;
        vector<swListenPort *> ports;
        string host;
        int port;
        int mode;
        int events;
    };
}
#endif //SERVER_SERVER_H
