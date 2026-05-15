/*大文件传输测试，给服务器上传一个大文件，服务器将文件保存下来，观察处理结果*/
/*
    上传的文件，和服务器保存的文件一致
*/

// 发现，并修正了，之前在httpserver中获取context上下文的时候都是复制拷贝，也就是如果一次请求没有结束
// 那么context不会把接收状态和未解析完成的请求----->> 修正为传指针，果真成功，300MB大文件传输成功
#include "../source/http/Http.hpp"

int main()
{
    Socket cli_sock;
    cli_sock.BuildTcpClient("127.0.0.1", 8080);
    std::string req = "PUT /1234.txt HTTP/1.1\r\nConnection: keep-alive\r\n";
    std::string body;
    Util::ReadFromFile("./hello.txt", &body);
    req += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    assert(cli_sock.Send(req.c_str(), req.size()) != -1);
    assert(cli_sock.Send(body.c_str(), body.size()) != -1);
    char buf[1024] = {0};
    assert(cli_sock.Recv(buf, 1023));
    LOG(INFO)<< ("[%s]", buf);
    sleep(3);
    cli_sock.Close();
    return 0;
}