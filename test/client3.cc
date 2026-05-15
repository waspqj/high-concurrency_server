// 唤醒的channel依次处理事件,如果有一个处理的时间过长甚至超过了超时重传设置的时间
// 那么有着一种情况,当这个channel后面是timerfd对应的channel,
// 更后面的channel的定时器一直没得到刷新,timerfd channel就必然会执行后面channel的超时任务(关闭连接)
// 那后面的任务还能执行吗? ---- 能,因为前面我们解决了这个问题
// 我们把关闭连接的操作放到了任务队列的末尾, 前面已经存在的对这个连接的操作,执行时还未退出
// 并且执行过Release之后连接的状态已经标记为"已断开",不允许这个连接的任务进行入队
// 所以真正退出之后, 不再有访问这个连接的情况, 保证安全


#include "../source/Server.hpp"

int main()
{
    Socket connect_server;
    connect_server.BuildTcpClient("127.0.0.1", 8080);

    std::string buff = "GET /register?nickname=www HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
        int write_ret = connect_server.Send(buff.c_str(), buff.size());
        
        char in[1024 << 8] = {0};
        int read_ret = connect_server.Recv((void*)in, 1024<<8 -1);
        if(read_ret < 0)
        {
            LOG(ERROR)<< "read error" ;
        }
        std::cout<< "server says: "<< in<< std::endl;
    while(1) sleep(1);
    // std::cout<< ret << std::endl;
    return 0;
}