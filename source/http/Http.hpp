#ifndef __HTTP_HPP__

#include "../Server.hpp"
#include <regex>
#include <sys/stat.h>

const std::unordered_map<uint16_t, std::string> status = {
    {100, "Continue"},
    {101, "Switching Protocol"},
    {102, "Processing"},
    {103, "Early Hints"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {207, "Multi-Status"},
    {208, "Already Reported"},
    {226, "IM Used"},
    {300, "Multiple Choice"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {306, "unused"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Payload Too Large"},
    {414, "URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {418, "I'm a teapot"},
    {421, "Misdirected Request"},
    {422, "Unprocessable Entity"},
    {423, "Locked"},
    {424, "Failed Dependency"},
    {425, "Too Early"},
    {426, "Upgrade Required"},
    {428, "Precondition Required"},
    {429, "Too Many Requests"},
    {431, "Request Header Fields Too Large"},
    {451, "Unavailable For Legal Reasons"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {506, "Variant Also Negotiates"},
    {507, "Insufficient Storage"},
    {508, "Loop Detected"},
    {510, "Not Extended"},
    {511, "Network Authentication Required"}};
const std::unordered_map<std::string, std::string> mine = {
    {".aac", "audio/aac"},
    {".abw", "application/x-abiword"},
    {".arc", "application/x-freearc"},
    {".avi", "video/x-msvideo"},
    {".azw", "application/vnd.amazon.ebook"},
    {".bin", "application/octet-stream"},
    {".bmp", "image/bmp"},
    {".bz", "application/x-bzip"},
    {".bz2", "application/x-bzip2"},
    {".csh", "application/x-csh"},
    {".css", "text/css"},
    {".csv", "text/csv"},
    {".doc", "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".eot", "application/vnd.ms-fontobject"},
    {".epub", "application/epub+zip"},
    {".gif", "image/gif"},
    {".htm", "text/html"},
    {".html", "text/html"},
    {".ico", "image/vnd.microsoft.icon"},
    {".ics", "text/calendar"},
    {".jar", "application/java-archive"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".js", "text/javascript"},
    {".json", "application/json"},
    {".jsonld", "application/ld+json"},
    {".mid", "audio/midi"},
    {".midi", "audio/x-midi"},
    {".mjs", "text/javascript"},
    {".mp3", "audio/mpeg"},
    {".mpeg", "video/mpeg"},
    {".mpkg", "application/vnd.apple.installer+xml"},
    {".odp", "application/vnd.oasis.opendocument.presentation"},
    {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt", "application/vnd.oasis.opendocument.text"},
    {".oga", "audio/ogg"},
    {".ogv", "video/ogg"},
    {".ogx", "application/ogg"},
    {".otf", "font/otf"},
    {".png", "image/png"},
    {".pdf", "application/pdf"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar", "application/x-rar-compressed"},
    {".rtf", "application/rtf"},
    {".sh", "application/x-sh"},
    {".svg", "image/svg+xml"},
    {".swf", "application/x-shockwave-flash"},
    {".tar", "application/x-tar"},
    {".tif", "image/tiff"},
    {".tiff", "image/tiff"},
    {".ttf", "font/ttf"},
    {".txt", "text/plain"},
    {".vsd", "application/vnd.visio"},
    {".wav", "audio/wav"},
    {".weba", "audio/webm"},
    {".webm", "video/webm"},
    {".webp", "image/webp"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".xhtml", "application/xhtml+xml"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml", "application/xml"},
    {".xul", "application/vnd.mozilla.xul+xml"},
    {".zip", "application/zip"},
    {".3gp", "video/3gpp"},
    {".3g2", "video/3gpp2"},
    {".7z", "application/x-7z-compressed"}};

// 工具类
class Util
{
public:
    // 分割字符串
    static size_t SplitStr(const std::string &str, const std::string &sep, std::vector<std::string> *array)
    {
        size_t offset = 0;
        while (offset < str.size())
        {
            std::string tmp;
            size_t pos = str.find(sep, offset);
            if (pos == std::string::npos) // 最后一段
            {
                tmp = str.substr(offset);
                if (tmp.size() > 0)
                    array->push_back(tmp);
                break;
            }
            else if (pos - offset > 0)
                tmp = str.substr(offset, pos - offset);

            if (tmp.size() > 0)
                array->push_back(tmp);
            offset = pos + sep.size();
        }
        return array->size();
    }
    // 读取文件数据
    static bool ReadFromFile(const std::string &filename, std::string *buf)
    {
        // 以二进制模式打开文件
        std::ifstream ifs(filename, std::ios::binary);
        if(!ifs.is_open())
        {
            LOG(ERROR)<< "IFSTREAM open file failed: "<< filename;
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
            LOG(ERROR) <<"IFSTREAM read file failed: "<< filename;
            ifs.close();
            return false;
        }
        ifs.close();
        return true;
    }
    // 向文件写入数据
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
            LOG(ERROR) <<"OFSTREAM write file failed: "<< filename;
            ofs.close();
            return false;
        }
        ofs.close();
        return true;
    }

    //URL编码，避免URL中资源路径与查询字符串中的特殊字符与HTTP请求中特殊字符产⽣歧义
    //编码格式：将特殊字符的ascii值，转换为两个16进制字符，前缀% C++ -> C%2B%2B
    // 不编码的特殊字符： RFC3986⽂档规定 . - _ ~ 字⺟，数字属于绝对不编码字符
    //RFC3986⽂档规定，编码格式 %HH 
    //W3C标准中规定，查询字符串中的空格，需要编码为+， 解码则是+转空格
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
    // URL解码
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
    // 判断是否是目录
    static bool IsDirectory(const std::string &filename)
    {
        struct stat st;
        int ret = stat(filename.c_str(), &st);
        if(ret < 0) return false;
        return S_ISDIR(st.st_mode);
    }
    // 判断是否是普通文件
    static bool IsRegular(const std::string &filename)
    {
        struct stat st;
        int ret = stat(filename.c_str(), &st);
        if(ret < 0) return false;
        return S_ISREG(st.st_mode);
    }
    // 请求路径资源有效性
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
    // 构建一个错误页面
    static std::string HttpErrorPage(int code, const std::string &reason)
    {
        char buf[1024];
        int n = std::snprintf(buf, sizeof(buf),
        "<!DOCTYPE html>"
        "<html lang=\"zh-CN\">"
        "<head><meta charset=\"UTF-8\"><title>%d %s</title></head>"
        "<body style=\"margin:0;background:#f8f9fa;font-family:system-ui,sans-serif;display:flex;align-items:center;justify-content:center;height:100vh;\">"
        "<div style=\"text-align:center;\">"
        "<h1 style=\"font-size:6rem;margin:0;color:#dc3545;font-weight:700;\">%d</h1>"
        "<p style=\"font-size:1.5rem;margin:0.5rem 0;color:#6c757d;\">%s</p>"
        "<hr style=\"width:80px;border:1px solid #dee2e6;margin:1.5rem auto;\">"
        "<p style=\"font-size:0.875rem;color:#adb5bd;\">High‑Concurrency Server</p>"
        "</div></body></html>",
        code, reason.c_str(),
        code, reason.c_str());
        return std::string(buf, n);
    }
};
// http请求报文信息
class HttpRequest
{
// private:
public:
    // 请求行信息
    std::string _method;   // 请求方法
    std::string _path;     // 请求资源路径
    std::smatch _matches;   // 资源路径中解析出的参数
    std::unordered_map<std::string, std::string> _params;  // uri的参数部分(查询字符串)
    std::string _version;  // http版本
    // 请求报头
    std::unordered_map<std::string, std::string> _heads;
    // 请求正文
    std::string _body;
public:
    HttpRequest() :_version("HTTP/1.1"){}
    // 请求报文信息重置
    void Reset()
    {
        _method.clear();
        _path.clear();
        _params.clear();
        _version = "HTTP/1.1";
        _heads.clear();
        _body.clear();
    }
    // uri参数的操作
    bool HasParam(const std::string &name)
    {
        auto it = _params.find(name);
        if(it == _params.end()) return false;
        return true;
    }
    void SetParam(const std::string &name, const std::string &param)
    {_params.emplace(name, param); }
    std::string GetParam(const std::string &name)
    {
        auto it = _params.find(name);
        if(it == _params.end()) return "";
        return it->second;
    }
     // 请求报头的操作
    bool HasHead(const std::string &name)
    {
        auto it = _heads.find(name);
        if(it == _heads.end()) return false;
        return true;
    }
    void SetHead(const std::string &name, const std::string &head)
    {_heads.emplace(name, head); }
    std::string GetHead(const std::string &name)
    {
        auto it = _heads.find(name);
        if(it == _heads.end()) return "";
        return it->second;
    }
    // 获取正文长度
    size_t ContentLength()
    {
        if(!HasHead("Content-Length"))
            return 0;
        auto len = GetHead("Content-Length");
        return std::stol(len);
    }
    // 是否短连接
    bool Close()
    {
        if(HasHead("Connection") && GetHead("Connection") == "keep-alive")
            return false;
        return true;
    }
};
#define DEFAULT_STATU 200
// http响应报文信息
class HttpResponse
{
public:
    // 状态行信息
    int _statu;  // 状态码
    // 版本信息在request里  状态码描述直接查询
    // 重定向信息
    bool _is_redirect;          // 是否重定向
    std::string _redirect_url;  // 重定向资源路径 (url完整路径)
    // 响应报头
    std::unordered_map<std::string, std::string> _heads;
     // 响应正文
    std::string _body;
public:
    HttpResponse(int statu = DEFAULT_STATU) :_statu(statu), _is_redirect(false) {}
    // 响应报文信息重置
    void Reset()
    {
        _statu = DEFAULT_STATU;
        _is_redirect = false;
        _redirect_url.clear();
        _heads.clear();
        _body.clear();
    }
     // 响应报头的操作
    bool HasHead(const std::string &name)
    {
        auto it = _heads.find(name);
        if(it == _heads.end()) return false;
        return true;
    }
    void SetHead(const std::string &name, const std::string &head)
    {_heads.emplace(name, head); }
    std::string GetHead(const std::string &name)
    {
        auto it = _heads.find(name);
        if(it == _heads.end()) return "";
        return it->second;
    }
    // 设置正文部分 和 正文类型
    void SetContent(const std::string &body, const std::string type)
    {
        SetHead("Content-Type", type);
        _body = body;
    }
    // 设置重定向
    void SetRedirect(const std::string &url, const int statu = 302)
    {
        _statu = statu;
        _redirect_url = url;
        _is_redirect = true;
    }
    // 是否是短链接
     bool Close()
    {
        if(HasHead("Connection") && GetHead("Connection") == "keep-alive")
            return false;
        return true;
    }
};

typedef enum{RECV_ERROR, RECV_LINE, RECV_HEAD, RECV_BODY, RECV_OVER} RecvStatu;
#define MAX_LINE_SIZE 8192
// http上下文类实现
class HttpContext
{
private:
    int _resp_statu;        // 相应状态码
    RecvStatu _recv_statu;  // 接收请求报文的状态
    HttpRequest _req;       // 接收请求报文的请求信息管理
private:
    // 解析请求行
    void ParseReqLine(const std::string &line)
    {
        // 用正则表达式获取请求行信息
        std::smatch matchs;
        // std::regex reg("(GET|POST|PUT|DELETE|HEAD) ([^?]+)(?:\\?(.*?))? (HTTP/1\\.[01])(?:\r\n|\n)");
        std::regex reg("(GET|HEAD|POST|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?", std::regex::icase);
        if (!std::regex_match(line, matchs, reg)) 
        {
            _resp_statu = 400; // Bad Request
            _recv_statu = RECV_ERROR;
            return;
        }
        //matchs中的数据
        // 0. GET /index.html/abc/?user=waspqj&password=12345 HTTP/1.1
        // 1. GET
        // 2. /index.html/abc/
        // 3. user=waspqj&password=12345
        // 4. HTTP/1.1
        // 1. 设置请求方法
        _req._method = matchs[1];
        // 注意对资源路径 和 查询字符串URL解码
        // 2. 设置路径资源
        _req._path = Util::UrlDecode(matchs[2], false);
        // 3. 设置查询字符串
        std::string param_str = Util::UrlDecode(matchs[3], true);
        std::vector<std::string> param_array;
        Util::SplitStr(param_str, "&", &param_array); // 拆分查询字符串
        for(auto p : param_array)  //将拆分好的查询字符串依次插入params
        {
            auto pos = p.find("=");// 都是:key=val形式
            if(pos == std::string::npos)  
            {
                _resp_statu = 400; // Bad Request
                _recv_statu = RECV_ERROR;
                return;
            }
            std::string param_key = p.substr(0, pos);
            std::string param_val = p.substr(pos + 1);
            _req.SetParam(param_key, param_val);
        }
        // 4.设置HTTP版本信息
        _req._version = matchs[4];
        // 请求行设置完成, 更新recv_statu 
        _recv_statu = RECV_HEAD;
    }
    // 接收并解析请求行
    void RecvReqLine(Buffer &buf)
    {
        if(_recv_statu != RECV_LINE) return;
        // 获取请求行报文
        std::string line = buf.ReadAndPop();
        // 没有找到一行完整请求行, 等待下次接收到完整的
        if(line.size() == 0 && buf.ReadableLen() < MAX_LINE_SIZE) return;
        // 没有找到一行完整请求行, 但是缓冲区数据已经超了                 || 一行太多了,不正常
        if((line.size() == 0 && buf.ReadableLen() >= MAX_LINE_SIZE) || line.size() > MAX_LINE_SIZE) 
        {
            LOG(ERROR)<< "line";
            _resp_statu = 414;        // URI Too Long
            _recv_statu = RECV_ERROR;
            return;
        }
        // LOG(INFO)<< line;
        return ParseReqLine(line);
    }
    // 解析一行报头
    void ParseReqHead(std::string &line)
    {   
        if(line.back() == '\n') line.pop_back(); 
        if(line.back() == '\r') line.pop_back(); 
        //key: val
        auto pos = line.find(": ");
        if(pos == std::string::npos)
        {
            _resp_statu = 400; // Bad Request
            _recv_statu = RECV_ERROR;
            return;
        }
        std::string head_key = line.substr(0, pos);
        std::string head_val = line.substr(pos + 2); //分隔符": "两位
        if(head_key == "User-Agent")
            LOG(INFO)<< line;
        _req.SetHead(head_key, head_val);
    }
    // 接收并解析请求报头
    void RecvReqHeads(Buffer &buf)
    {
        if(_recv_statu != RECV_HEAD) return;
        // 依次获取报头并设置报头 直到遇到空行(\r\n) key: val\r\nkey: val\r\n.....\r\n 
        while(true)
        {
            std::string line = buf.ReadAndPop();
            // 报头每行也需要限制行数据大小
            // 没有找到一行完整报头行, 等待下次接收到完整的
            if(line.size() == 0 && buf.ReadableLen() < MAX_LINE_SIZE) return;
            // 没有找到一行完整报头行, 但是缓冲区数据已经超了                 || 一行太多了,不正常
            if((line.size() == 0 && buf.ReadableLen() >= MAX_LINE_SIZE) || line.size() > MAX_LINE_SIZE) 
            {
                // LOG(ERROR)<< "head";
                _resp_statu = 414;        // URI Too Long
                _recv_statu = RECV_ERROR;
                return;
            }
            if(line == "\r" || line == "\n" || line == "\r\n") // 遇到空行 报头解析完毕
                break;
            
            // 解析一行报头
            ParseReqHead(line);
            if(_recv_statu == RECV_ERROR)
                return;
        }
        // 更新 _recv_statu
        _recv_statu = RECV_BODY;
    }
    // 接收并解析请求正文
    void RecvReqBody(Buffer &buf)
    {
        if(_recv_statu != RECV_BODY) return;
        // 报头中获取正文长度
        size_t content_len = _req.ContentLength();
        if(content_len == 0)  //没有正文,直接解析成功
        {
            _recv_statu = RECV_OVER;
            return;
        }
        // 获取正文
        size_t remain_len = content_len - _req._body.size();
        LOG(INFO)<< _req._body.size()<< " : "<< remain_len<< " : "<< buf.ReadableLen();
        // if(buf.ReadableLen() < content_len) // 还没传完,等下次一起拿--- 正文的数据可能很多,所以...
        if(buf.ReadableLen() < remain_len) // 没传完,先拿出来
        {       //  全部拿出来, 追加到body中
            // _req._body.append(buf.ReadAndPop(buf.ReadableLen()));
            _req._body.append(buf.GetReadPos(), buf.ReadableLen());
            LOG(INFO)<< "waiting for next";
            return buf.MoveReadPos(buf.ReadableLen());
        }
        else{  //   有了完整正文
            _req._body.append(buf.GetReadPos(), remain_len);
            buf.MoveReadPos(remain_len);
            // 设置解析成功
            _recv_statu = RECV_OVER;
        }
    }
public:
    HttpContext() :_resp_statu(200), _recv_statu(RECV_LINE) 
    {}// LOG(INFO)<< "context create!!"; 
    // 重置上下文
    void Reset()
    {
        // LOG(INFO)<< "context reset!!";
        _resp_statu = 200;
        _recv_statu = RECV_LINE;
        _req.Reset();
    }
    int GetRespStatu() {return _resp_statu; }
    RecvStatu GetRecvStatu() {return _recv_statu; }
    HttpRequest &GetHttpReq() {return _req; }
    void RecvHttpReq(Buffer &buf)
    {
        // LOG(INFO) << buf.GetReadPos();
        // 不加break 可以依次执行, 一次获取解析报文信息
        switch(_recv_statu)
        {
            case RECV_LINE: RecvReqLine(buf);
            case RECV_HEAD: RecvReqHeads(buf);
            case RECV_BODY: RecvReqBody(buf);
        }
    }
};
#define DEFAULT_TIMEOUT 15
class HttpServer
{
using Handler = std::function<void(HttpServer &, HttpRequest &, HttpResponse &)>;
using Handlers = std::vector<std::pair<std::regex, Handler>>;
private:
    // 高并发服务器
    TcpServer _tcp_server;  
    // 资源管理
    std::string _basedir;   // 静态资源路径
    Handlers _get_route;      // GET方法的动态资源路由   
    Handlers _post_route;     // POST方法的动态资源路由   
    Handlers _put_route;      // PUT方法的动态资源路由   
    Handlers _delete_route;   // DELETE方法的动态资源路由   
public:
    // 错误处理
    void ErrorHandler(HttpResponse &resp)
    {
        // 构建一个错误HTML页面
        int statu = resp._statu;
        std::string body = Util::HttpErrorPage(statu, Util::StatuDesc(statu));
        // 设置http响应正文(错误页面), 里面设置了content-type
        resp.SetContent(body, Util::ExtMime(".html"));
    }
private:
    // 判断是否是请求静态资源
    bool IsFileHandler(const HttpRequest &reqs)
    {
        // 没设根目录
        if(_basedir.empty()) return false;
        // 请求方法不是GET || HEAD
        if(reqs._method != "GET" && reqs._method != "HEAD") return false;
        // 请求资源不合法
        if(!Util::ValidPath(reqs._path)) return false;
        // 请求资源不存在,或者请求资源不是普通文件
        std::string req_path = _basedir + reqs._path;
        if(reqs._path.back() == '/')
            req_path += "index.html";
        if(!Util::IsRegular(req_path)) return false;

        return true;
    }
    // 请求静态资源处理
    void FileHandler(HttpRequest &reqs, HttpResponse &resp)
    {
        // 获取静态资源真正的路径
        std::string req_path = _basedir + reqs._path;
        if(reqs._path.back() == '/')
            req_path += "index.html";
        // 读取文件内容
        auto ret = Util::ReadFromFile(req_path, &resp._body);
        if(ret == false)
            return;
        // 设置报头 -- 文件类型
        std::string mine = Util::ExtMime(req_path);
        resp.SetHead("Content-Type", mine);
    }
    // 分派动态资源
    void Dispacher(HttpRequest &reqs, HttpResponse &resp, Handlers &handlers)
    {
        // 遍历对应方法的动态资源, 正则匹配出
        for(auto [pattern, handler] : handlers)
        {
            if(std::regex_match(reqs._path, reqs._matches, pattern))
            {
                return handler(*this, reqs, resp);
            }
        }
        // 没有匹配的方法, 返回404
        resp._statu = 404;
        return ErrorHandler(resp);
    }
    // 组织http响应,发送给客户端
    void WriteResponse(const ConnectionPtr &conn, HttpRequest &reqs, HttpResponse &resp)
    {
        LOG(INFO)<< "client-"<< conn->_Id()<< " visit "<< reqs._path;
        // 1.设置响应报头
        if(reqs.Close()) {resp.SetHead("Connection", "close"); }
        else {resp.SetHead("Connection", "keep-alive"); }
        if(!resp.HasHead("Content-Length"))
            resp.SetHead("Content-Length", std::to_string(resp._body.size())); 
        // resp->SetHead("Cache-Control", "max-age=3600");  // 缓存设置: 1hour
        if(resp._is_redirect)
            resp.SetHead("Location", resp._redirect_url);        
        // 2.组织http响应
        int statu = resp._statu;
        std::stringstream resp_str;
        resp_str<< reqs._version<< " "<< std::to_string(statu)<< " "<< Util::StatuDesc(statu)<< "\r\n";
        for(auto h : resp._heads)
        {
            resp_str<< h.first<< ": "<< h.second<< "\r\n";
        }
        resp_str<< "\r\n";
        resp_str<< resp._body;
        // 3.发送响应报文到客户端
        conn->Send(resp_str.str().c_str(), resp_str.str().size());
    }
    // 处理请求 -- 路由请求资源
    void Route(HttpRequest &reqs, HttpResponse &resp)
    {
        //1. 静态资源
        if(IsFileHandler(reqs))
            return FileHandler(reqs, resp);
        //2. 动态资源
        if(reqs._method == "GET" || reqs._method == "HEAD")
            return Dispacher(reqs, resp, _get_route);
        if(reqs._method == "POST")
            return Dispacher(reqs, resp, _post_route);
        if(reqs._method == "PUT")
            return Dispacher(reqs, resp, _put_route);
        if(reqs._method == "DELETE")
            return Dispacher(reqs, resp, _delete_route);
        
        resp._statu = 405;  // Method Not Allowed
        return ErrorHandler(resp);  // 直接错误处理
    }
    // tcp服务器的connected回调 -->> 设置tcp服务器的上下文
    void OnConnected(const ConnectionPtr &conn)
    {
        conn->SetContext(HttpContext());  // 正是context模块, 保存未完成的request请求, 提供响应状态码
        // LOG(DEBUG)<< conn->_Id()<< " : context is ready";
    }
    // tcp服务器的connected回调 :  1.根据缓冲区数据提取http请求 2.如果有完整请求,处理请求构建应答 3.将构建的应答发送到缓冲区
    void OnMessage(const ConnectionPtr &conn, Buffer *buf)
    {
        // LOG(DEBUG)<< "--- Message Call ---";

        while(buf->ReadableLen())
        {
            // 获取上下文
            // HttpContext context = std::any_cast<HttpContext>(conn->GetContext());
            HttpContext *context = std::any_cast<HttpContext>(conn->GetContext());
            // 提取http请求
            context->RecvHttpReq(*buf);
            HttpRequest reqs = context->GetHttpReq();
            // 根据状态码,初始化一个http响应
            HttpResponse resp(context->GetRespStatu());
            if(context->GetRecvStatu() == RECV_ERROR)  // 提取请求时遇到了错误
            {
                // 错误处理
                ErrorHandler(resp);
                // 组织http响应,发送给客户端
                WriteResponse(conn, reqs, resp);
                // 关闭连接之前,必须要初始化连接的上下文,Shutdown调用的接口,最后又回到这里
                context->Reset();
                // buff 也清空, 这样看到buffer_in为空,也不会再调用这个了
                buf->MoveReadPos(buf->ReadableLen());
                // 关闭连接 并退出
                return conn->Shutdown();
            }
            if(context->GetRecvStatu() != RECV_OVER)   // 没有提取到完整的请求报文, 退出等下次
                break; 
            // 得到完整的请求, 正常处理请求
            Route(reqs, resp);
            // 组织http响应,发送给客户端
            WriteResponse(conn, reqs, resp);
            // 重置上下文, 防止剩余数据影响下次连接
            context->Reset();
            // 短连接,连接完成后关闭连接
            if(resp.Close())
                return conn->Shutdown();
        }
    }
public:
    HttpServer(uint16_t port, int timeout = DEFAULT_TIMEOUT) : _tcp_server(port)
    {
        // 设置tcp服务器 超时断联机制
        _tcp_server.RefuseSilenceConnect(timeout);
        // 设置tcp服务器回调
        _tcp_server.SetConnectedCall([this](const ConnectionPtr &conn) {return OnConnected(conn); });
        _tcp_server.SetMessageCall([this](const ConnectionPtr &conn, Buffer *buf) {return OnMessage(conn, buf); });
    }
    // 设置tcp服务器,从reactor数量
    void SetCount(int count) 
    {_tcp_server.SetPoolCount(count); }
    // 设置动静态资源
    void SetBaseDir(const std::string &basedir) 
    {
        assert(Util::IsDirectory(basedir));    
        _basedir = basedir; 
    }
    void Get(const std::string &pattern, const Handler &handler)
    {_get_route.emplace_back(std::regex(pattern), handler);}
    void Post(const std::string &pattern, const Handler &handler)
    {_post_route.emplace_back(std::regex(pattern), handler);}
    void Put(const std::string &pattern, const Handler &handler)
    {_put_route.emplace_back(std::regex(pattern), handler);}
    void Delete(const std::string &pattern, const Handler &handler)
    {_delete_route.emplace_back(std::regex(pattern), handler);}
    // 运行服务器
    void Start()
    {_tcp_server.Start(); }
};

#endif