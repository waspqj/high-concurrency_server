#include "Server.hpp"

int main()
{
    Use_Screen_Log_Strategy;
    Buffer buf;
    for(int i = 0; i < 105; ++i)
    {
        std::string str = "hello world" + std::to_string(i) + "\n";
        buf.WriteAndPush(str);
    }
    std::string out = buf.ReadAndPop();
    while(out != "")
    {
        std::cout<< out<< std::endl;
        out = buf.ReadAndPop();
    }
    LOG(DEBUG)<< "success";
    // std::string str = "hello world";
    // // buf.WriteAndPush(str);
    // Buffer bufw;
    // str += "buffer";
    // bufw.WriteAndPush(str);

    // buf.WriteAndPush(bufw);
    // std::cout<< buf.ReadableLen()<< std::endl;
    // std::string recv = buf.ReadAndPop(str.size());
    // std::cout<< recv<< std::endl;
    // std::cout<< buf.ReadableLen()<< std::endl;

    return 0;
}