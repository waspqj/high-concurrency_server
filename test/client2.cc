// 发送的请求出错:
// Content-Length设置的更长 导致后续请求出现问题
// 观察是否会正确错误处理,并关闭连接

#include "../source/Server.hpp"

int main()
{
    Socket connect_server;
    connect_server.BuildTcpClient("127.0.0.1", 8080);

    std::string buff = "GET /png.html HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 100\r\n\r\n232314";
    for(int i = 0; i < 3; ++i)
    {
        int write_ret = connect_server.Send(buff.c_str(), buff.size());
    }
    for(int i = 0; i < 3; ++i)
    {
        char in[1024 << 8] = {0};
        int read_ret = connect_server.Recv((void*)in, 1024<<8 -1);
        if(read_ret < 0)
        {
            LOG(ERROR)<< "read error" ;
            break;
        }
        std::cout<< "server says: "<< in<< std::endl;
        // sleep(1);
    }
    // connect_server.Close();
    while(1) sleep(1);
    // std::cout<< ret << std::endl;
    return 0;
}