#include <iostream>
#include <regex>

int main()
{
    std::string str = "GET /index.html/abc/?user=waspqj&password=12345 HTTP/1.1\r\n";
    // std::string str = "GET /index.html/abc/ HTTP/1.1\n";
//             1.   ():表示捕捉符合其中条件的内容  xx|xxx|xxx 表示或
//             2.   [^?]:表示匹配除了?之外的单个字符 +：表示前面的表达式匹配一次或多次 ([^?]+):表示匹配除了？的字符直到遇到？
//             3.   (?:……)：表示设置括号内为非捕获对象 \\?(.*?):表示一个由？开始的一串尽量短的匹配字符串，并且捕获？后的字符串不包括？
//             4.   (.*?): .表示任意字符 ，*？表示贪婪匹配，匹配尽量少的字符（可为空）
//             5.   ()? :表示匹配0次或1次,就是可以没有
//             6.   (HTTP/1\\.[01]): 表示HTTP/1.0或HTTP/1.1  \\.:表示匹配. [01]: 0 or 1
    std::regex reg("(GET|POST|PUT|DELETE|HEAD) ([^?]+)(?:\\?(.*?))? (HTTP/1\\.[01])(?:\r\n|\n)");
    
    std::smatch matchs;
    if (std::regex_match(str, matchs, reg))
    {
       for(auto m : matchs)
        std::cout<< m<< std::endl;  
    }
    else{
        std::cout<< "no match"<< std::endl;
    }
}