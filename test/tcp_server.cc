#include "../source/Server.hpp"

// void CloseHandler(Channel *chan)
// {
//     LOG(INFO) << "close: "<< chan->_Fd();
//     // LOG(DEBUG) << "^^^^^^^____^^^^^^";
//     delete chan;
//     // sleep(10);
// }
// void ReadHandler(Channel *chan)
// {
//     int fd = chan->_Fd();
//     // LOG(DEBUG)<< "fd: "<< fd;
//     Socket socket(fd);
//     char buff[1025] = {0};
//     int ret = socket.Recv((void*)buff, 1024);
//     if(ret < 0)
//     {
//         LOG(ERROR)<< "read error" ;
//         CloseHandler(chan);
//     }
//     else if(ret == 0)
//     {
//         chan -> UnsetRead();
//         return;
//     }
//     chan->SetWrite();
//     std::cout<< "client says: "<< buff<< std::endl;
// }
// void WriteHandler(Channel *chan)
// {
//     int fd = chan->_Fd();
//     Socket socket(fd);
//     char buff[1024] = "今天天气真不错";
//     int ret = socket.Send(buff, strlen(buff));
//     if(ret < 0)
//     {
//         LOG(ERROR)<< "send error" ;
//         CloseHandler(chan);
//     }
//     chan->UnsetWrite();
// }
// void ErrorHandler(Channel *chan){CloseHandler(chan);}
// void EventHandler(EventLoop *loop, uint64_t timeid, Channel *chan){loop->RefreshTimer(timeid);}

EventLoop baseloop;
LoopThreadPool pool(&baseloop);

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
    // LOG(DEBUG)<< recvdata.c_str()<< " "<< recvdata.size();
    conn->Send(recvdata.c_str(), recvdata.size());
}
void CloseHandler(const ConnectionPtr & conn)
{}
void EventHandler(const ConnectionPtr & conn)
{}

uint64_t conn_id = 1;
std::unordered_map<uint64_t, ConnectionPtr> _conn_map;
void SrvConnectedHandler(const ConnectionPtr &conn)
{
    // LOG(DEBUG)<< "Erase"<< conn.get() << ": "<< conn.use_count();
    // LOG(DEBUG) << _conn_map.erase(conn->_Id());
    _conn_map.erase(conn->_Id());
    // LOG(DEBUG) << conn.use_count();
}

void NewConnection(int newfd)
{
    ConnectionPtr new_conn = std::make_shared<Connection>(conn_id, newfd, pool.GetNextLoop());
    new_conn->SetConnectedCall([](const ConnectionPtr & conn){
        ConnectedHandler(conn);
    });
    new_conn->SetMessageCall([](const ConnectionPtr & conn, Buffer *data){
        MessageHandler(conn, data);
    });
    new_conn->SetCloseCall([](const ConnectionPtr & conn){
        CloseHandler(conn);
    });
    new_conn->SetEventCall([](const ConnectionPtr & conn){
        EventHandler(conn);
    });
    new_conn->SetSrvCloseCall([](const ConnectionPtr & conn){
        SrvConnectedHandler(conn);
    });
    // 取消静默访问
    new_conn->RefuseSilenceConnect(10);
    
    new_conn->Established();
    
    _conn_map.emplace(conn_id, new_conn);
    ++conn_id;
}
int main()
{
    pool.SetCount(3);
    pool.Create();

    Acceptor accept(&baseloop, 8081);
    accept.SetAcceptCallback([](int fd){
        NewConnection(fd);
    });

    accept.Listen();
    
    baseloop.Start();
    return 0;
}