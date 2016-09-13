#include "Server.h"

#include <iostream>

using namespace std;

namespace swoole
{
    Server::Server(string _host, int _port, int _mode, int _type)
    {
        host = _host;
        port = _port;
        mode = _mode;

        swServer_init(&serv);

        serv.reactor_num = 4;
        serv.worker_num = 2;
        serv.factory_mode = (uint8_t) mode;
        //serv.factory_mode = SW_MODE_SINGLE; //SW_MODE_PROCESS/SW_MODE_THREAD/SW_MODE_BASE/SW_MODE_SINGLE
        serv.max_connection = 10000;
        //serv.open_cpu_affinity = 1;
        //serv.open_tcp_nodelay = 1;
        //serv.daemonize = 1;
//	memcpy(serv.log_file, SW_STRL("/tmp/swoole.log")); //日志

        serv.dispatch_mode = 2;
//	serv.open_tcp_keepalive = 1;

#ifdef HAVE_OPENSSL
        //serv.ssl_cert_file = "tests/ssl/ssl.crt";
        //serv.ssl_key_file = "tests/ssl/ssl.key";
        //serv.open_ssl = 1;
#endif
        //create Server
        int ret = swServer_create(&serv);
        if (ret < 0)
        {
            swTrace("create server fail[error=%d].\n", ret);
            exit(0);
        }
        this->listen(host, port, _type);
    }

    void Server::setEvents(int _events)
    {
        events = _events;
    }

    bool Server::listen(string host, int port, int type)
    {
        auto ls = swServer_add_port(&serv, type, (char *) host.c_str(), port);
        if (ls == NULL)
        {
            return false;
        } else
        {
            ports.push_back(ls);
            return true;
        }
    }

    bool Server::send(int fd, string &data)
    {
        if (SwooleGS->start == 0)
        {
            return false;
        }
        if (data.length() <= 0)
        {
            return false;
        }
        return serv.send(&serv, fd, (char *) data.c_str(), data.length()) == SW_OK ? true : false;
    }

    bool Server::send(int fd, const char *data, int length)
    {
        if (SwooleGS->start == 0)
        {
            return false;
        }
        if (length <= 0)
        {
            return false;
        }
        return serv.send(&serv, fd, (char *) data, length) == SW_OK ? true : false;
    }

    bool Server::close(int fd, bool reset)
    {
        if (SwooleGS->start == 0)
        {
            return false;
        }
        if (swIsMaster())
        {
            return false;
        }

        swConnection *conn = swServer_connection_verify_no_ssl(&serv, fd);
        if (!conn)
        {
            return false;
        }

        //Reset send buffer, Immediately close the connection.
        if (reset)
        {
            conn->close_reset = 1;
        }

        int ret;
        if (!swIsWorker())
        {
            swWorker *worker = swServer_get_worker(&serv, conn->fd % serv.worker_num);
            swDataHead ev;
            ev.type = SW_EVENT_CLOSE;
            ev.fd = fd;
            ev.from_id = conn->from_id;
            ret = swWorker_send2worker(worker, &ev, sizeof(ev), SW_PIPE_MASTER);
        }
        else
        {
            ret = serv.factory.end(&serv.factory, fd);
        }
        return ret == SW_OK ? true : false;
    }

    bool Server::sendto(string &ip, int port, string &data, int server_socket)
    {
        if (SwooleGS->start == 0)
        {
            return false;
        }
        if (data.length() <= 0)
        {
            return false;
        }
        bool ipv6 = false;
        if (strchr(ip.c_str(), ':'))
        {
            ipv6 = true;
        }

        if (ipv6 && serv.udp_socket_ipv6 <= 0)
        {
            return false;
        }
        else if (serv.udp_socket_ipv4 <= 0)
        {
            swWarn("You must add an UDP listener to server before using sendto.");
            return false;
        }

        if (server_socket < 0)
        {
            server_socket = ipv6 ? serv.udp_socket_ipv6 : serv.udp_socket_ipv4;
        }

        int ret;
        if (ipv6)
        {
            ret = swSocket_udp_sendto6(server_socket, (char *) ip.c_str(), port, (char *) data.c_str(), data.length());
        }
        else
        {
            ret = swSocket_udp_sendto(server_socket, (char *) ip.c_str(), port, (char *) data.c_str(), data.length());
        }
        return ret == SW_OK ? true : false;
    }

    bool Server::start(void)
    {
        serv.ptr2 = this;
        if (this->events & EVENT_onStart)
        {
            serv.onStart = Server::_onStart;
        }
        if (this->events & EVENT_onShutdown)
        {
            serv.onShutdown = Server::_onShutdown;
        }
        if (this->events & EVENT_onConnect)
        {
            serv.onConnect = Server::_onConnect;
        }
        if (this->events & EVENT_onReceive)
        {
            serv.onReceive = Server::_onReceive;
        }
        if (this->events & EVENT_onPacket)
        {
            serv.onPacket = Server::_onPacket;
        }
        if (this->events & EVENT_onClose)
        {
            serv.onClose = Server::_onClose;
        }
        if (this->events & EVENT_onWorkerStart)
        {
            serv.onWorkerStart = Server::_onWorkerStart;
        }
        if (this->events & EVENT_onWorkerStop)
        {
            serv.onWorkerStop = Server::_onWorkerStop;
        }
        int ret = swServer_start(&serv);
        if (ret < 0)
        {
            swTrace("start server fail[error=%d].\n", ret);
            return false;
        }
        return true;
    }

    int Server::_onReceive(swServer *serv, swEventData *req)
    {
        swConnection *conn = swWorker_get_connection(serv, req->info.fd);
        swoole_rtrim(req->data, req->info.len);
        printf("onReceive: ip=%s|port=%d Data=%s|Len=%d\n", swConnection_get_ip(conn),
               swConnection_get_port(conn), req->data, req->info.len);

        cout << req->data << endl;
        string str(req->data, req->info.len);
        Server *_this = (Server *) serv->ptr2;
        _this->onReceive(req->info.fd, str);
        return SW_OK;
    }

    void Server::_onWorkerStart(swServer *serv, int worker_id)
    {
        printf("WorkerStart[%d]PID=%d\n", worker_id, getpid());
        Server *_this = (Server *) serv->ptr2;
        _this->onWorkerStart(worker_id);
    }

    void Server::_onWorkerStop(swServer *serv, int worker_id)
    {
        printf("WorkerStop[%d]PID=%d\n", worker_id, getpid());
        Server *_this = (Server *) serv->ptr2;
        _this->onWorkerStop(worker_id);
    }

    int Server::_onPacket(swServer *serv, swEventData *req)
    {
        swDgramPacket *packet;

        swString *buffer = swWorker_get_buffer(serv, req->info.from_id);
        packet = (swDgramPacket *) buffer->str;

        char *data;
        int length;
        int ret;
        ClientInfo clientInfo;
        clientInfo.server_socket  = req->info.from_fd;

        //udp ipv4
        if (req->info.type == SW_EVENT_UDP)
        {
            struct in_addr sin_addr;
            sin_addr.s_addr = packet->addr.v4.s_addr;
            char *tmp = inet_ntoa(sin_addr);
            memcpy(clientInfo.address, tmp, strlen(tmp));
            data = packet->data;
            length = packet->length;
            clientInfo.port = packet->port;
        }
            //udp ipv6
        else if (req->info.type == SW_EVENT_UDP6)
        {
            inet_ntop(AF_INET6, &packet->addr.v6, clientInfo.address, sizeof( clientInfo.address));
            data = packet->data;
            length = packet->length;
            clientInfo.port = packet->port;
        }
            //unix dgram
        else if (req->info.type == SW_EVENT_UNIX_DGRAM)
        {
            memcpy(clientInfo.address, packet->data, packet->addr.un.path_length);
            data = packet->data + packet->addr.un.path_length;
            length = packet->length - packet->addr.un.path_length;
        }

        printf("Packet[client=%s:%d, %d bytes]: data=%*s\n", clientInfo.address, clientInfo.port, length, length, data);

        string str(data, length);
        Server *_this = (Server *) serv->ptr2;
        _this->onPacket(str, clientInfo);

        return SW_OK;
    }

    void Server::_onStart(swServer *serv)
    {
        Server *_this = (Server *) serv->ptr2;
        _this->onStart();
    }

    void Server::_onShutdown(swServer *serv)
    {
        Server *_this = (Server *) serv->ptr2;
        _this->onShutdown();
    }

    void Server::_onConnect(swServer *serv, swDataHead *info)
    {
        Server *_this = (Server *) serv->ptr2;
        _this->onConnect(info->fd);
    }

    void Server::_onClose(swServer *serv, swDataHead *info)
    {
        Server *_this = (Server *) serv->ptr2;
        _this->onClose(info->fd);
    }
}