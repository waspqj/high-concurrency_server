#include "../source/Server.hpp"

int main()
{
    Socket connect_server;
    connect_server.BuildTcpClient("127.0.0.1", 8081);

    std::string buff = "hello world";
    for(int i = 0; i < 10; ++i)
    {
        int write_ret = connect_server.Send(buff.c_str(), buff.size());
        
        char in[1025] = {0};
        int read_ret = connect_server.Recv((void*)in, 1024);
        if(read_ret < 0)
        {
            LOG(ERROR)<< "read error" ;
            break;
        }
        std::cout<< "server says: "<< in<< std::endl;
        sleep(1);
    }
    // connect_server.Close();
    while(1) sleep(1);
    // std::cout<< ret << std::endl;
    return 0;
}