#include "../source/Server.hpp"

class Echo
{
private:
    TcpServer _server;
private:
    void ConnectedHandler(const ConnectionPtr & conn)
    {
        LOG(INFO)<< "connected: "<< ("%p", conn.get()) ;
    }
    void MessageHandler(const ConnectionPtr & conn, Buffer *data)
    {
        // 获取发来数据
        std::string recvdata = data->ReadAndPop(data->ReadableLen()); 
        std::cout<< "client says: "<< recvdata<< std::endl;
        // 向客户端发送回应
        recvdata = "I got: " + recvdata + "\n hello I'm server";
        conn->Send(recvdata.c_str(), recvdata.size());
    }
    void CloseHandler(const ConnectionPtr & conn)
    {/*LOG(INFO)<< "Close: "<< ("%p", conn.get()) ;*/}
    void EventHandler(const ConnectionPtr & conn)
    {/*LOG(INFO)<< "refresh timer ";*/}
public:
    Echo(uint16_t port, int count) :_server(port)
    {
        _server.SetPoolCount(count);
        _server.RefuseSilenceConnect(6);
        _server.SetConnectedCall([this](const ConnectionPtr &conn){
            ConnectedHandler(conn);
        });
        _server.SetMessageCall([this](const ConnectionPtr & conn, Buffer *data){
            MessageHandler(conn, data);
        });
        _server.SetCloseCall([this](const ConnectionPtr & conn){
            CloseHandler(conn);
        });
        _server.SetEventCall([this](const ConnectionPtr &conn){
            EventHandler(conn);
        });
    }
    void Start() {_server.Start(); }
};