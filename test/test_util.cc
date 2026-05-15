#include "../source/http/Http.hpp"

static size_t SplitStr(const std::string &str, const std::string &sep, std::vector<std::string> *array)
{
    size_t offset = 0;
    while(offset < str.size())
    {
        std::string tmp;
        size_t pos = str.find(sep, offset);
        if(pos == std::string::npos)  // 最后一段
        {
            tmp = str.substr(offset);
            if(tmp.size() > 0)
                array->push_back(tmp);    
            break;
        }
        else if(pos - offset > 0)     
            tmp = str.substr(offset, pos - offset);
        
        if(tmp.size() > 0)
            array->push_back(tmp);
        offset = pos + sep.size();
    }
    return array->size();
}

static bool ReadFromFile(const std::string &filename, std::string *buf)
{
    // 以二进制模式打开文件
    std::ifstream ifs(filename, std::ios::binary);
    if(!ifs.is_open())
    {
        LOG(ERROR)<< "IFSTREAM open file failed";
        return false;
    }
    // 获取文件数据大小
    size_t fsize = 0;
    ifs.seekg(0, ifs.end); // 跳转读写位置到文件末尾
    fsize = ifs.tellg();   // 获取离起始位置的偏移量
    ifs.seekg(0, ifs.beg); // 跳转回去
    buf->resize(fsize);          // 预留文件数据大小的空间
    ifs.read(&(*buf)[0], fsize); // 读取文件数据
    if(!ifs.good())
    {
        LOG(ERROR) <<"IFSTREAM read file failed";
        ifs.close();
        return false;
    }
    ifs.close();
    return true;
}
static bool WriteToFile(const std::string &filename, const std::string &buf)
{
    // 以二进制模式打开文件  并且覆盖原先内容
    std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
    if(!ofs.is_open())
    {
        LOG(ERROR)<< "OFSTREAM open file failed: "<< filename;
        return false;
    }
    // 向文件写入buf中的数据
    ofs.write(buf.c_str(), buf.size());
    if(!ofs.good())
    {
        LOG(ERROR) <<"OFSTREAM write file failed";
        ofs.close();
        return false;
    }
    ofs.close();
    return true;
}
static std::string UrlEncode(const std::string &url, bool space_to_plus)
    {
        std::string ret;
        for(auto a : url)
        {
            if(a == '.'|| a == '-'|| a == '_' || a == '~' || isalnum(a))
            {
                ret += a;
                continue;
            }
            if(a == ' ' && space_to_plus)
            {
                ret += '+';
                continue;
            }
            // 其他都为普通字符
            char tmp[4] = {0};
            snprintf(tmp, 4, "%%%02X", a);
            ret += tmp;
        }
        return ret;
    }
    static char HEXTOI(char c)
    {
        if(c <= '9' && c >= '0')
            return c - '0';
        else if(c <= 'z' && c >= 'a')
            return c - 'a' + 10;
        else if(c <= 'Z' && c >= 'A')
            return c - 'A' + 10;
        return -1;
    }
static std::string UrlDecode(const std::string &url, bool plus_to_space)
    { //"asd%2Fsd12%2FC%2B%2B++" --> "asd/sd12/C++  "
        std::string ret;
        for(int i = 0; i < url.size(); ++i)
        {
            char a = url[i];
            if(plus_to_space && a == '+')
            {
                ret += ' ';
                continue;
            }
            if(a == '%' && i + 2 < url.size())
            {
                char a1 = HEXTOI(url[i + 1]);
                char a2 = HEXTOI(url[i + 2]);
                char v = (a1 << 4) + a2;
                ret += v;
                i += 2;
                continue;
            }
            ret += a;
        }
        return ret;
    }
     // 获取响应状态码的描述信息
    static std::string StatuDesc(int statu)
    {
        auto it = status.find(statu);
        if(it == status.end())
        {
            return "Unkown";
        }
        return it->second;
    }
    // 根据文件后缀名获取文件mine
    static std::string ExtMime(const std::string &filename)
    {
        // 获取后缀名
        std::string suffix;
        auto pos = filename.find_last_of('.');
        if(pos == std::string::npos)
            return "application/octet-stream";
        suffix = filename.substr(pos);
        // 通过后缀名找到对应mine
        auto it = mine.find(suffix);
        if(it == mine.end())
            return "application/octet-stream";
        return it->second;
    }
    static bool ValidPath(const std::string &path)
    {
        std::vector<std::string> subdir;
        SplitStr(path, "/", &subdir);
        int depth = 0;
        for(auto dir : subdir)
        {
            if(dir == "..")
                --depth;
            else if(dir == ".")
                depth;
            else
                ++depth;
            if(depth < 0)
                return false;
        }
        return true;
    }
int main()
{
    std::cout<< ValidPath("/.././dfa/sdf/")<< std::endl;
    std::cout<< ValidPath("/dfa/.././sdf/")<< std::endl;
    std::cout<< ValidPath("/dfa/../../sdf/")<< std::endl;


    // std::cout<< StatuDesc(404)<< std::endl;
    // std::cout<< StatuDesc(304)<< std::endl;
    // std::cout<< StatuDesc(0)<< std::endl;
    
    // std::cout<< ExtMime("sdfa.htm")<< std::endl;
    // std::cout<< ExtMime("sdfa.lll")<< std::endl;
    // std::cout<< ExtMime("sdfa.docx")<< std::endl;
// std::string str = "asd/sd12/C++<><>    " ;
    // std::string encode = UrlEncode(str, false);
    // // std::string encode = UrlEncode(str, true);
    // std::cout << "["<< encode<<"]"<< std::endl;
    // // std::string decode = UrlDecode(encode, true);
    // std::string decode = UrlDecode(encode, false);
    // std::cout << "["<< decode<<"]"<< std::endl;
    // std::string buff;
    // ReadFromFile("./tcp_client.cc", &buff);
    // // std::cout<< buff;
    // WriteToFile("./txt", buff);
    // std::string str = "acsd//sdf/wewe///";
    // std::string sep = "/";
    // std::vector<std::string> arr;
    // SplitStr(str, sep, &arr);
    // for(auto a : arr)
    //     std::cout << a<< std::endl;
    return 0;
}