#ifndef __SERVER_HPP__

#include <iostream>

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

#include <vector>
#include <string>
#include <any>
#include <signal.h>
#include "log.hpp"

using namespace LogModule;

#define BUFFER_DEFAULT_SIZE 1024
class Buffer
{
private:
    std::vector<char> _buffer;
    uint64_t _write_pos;
    uint64_t _read_pos;
public:
    Buffer() : _write_pos(0), _read_pos(0), _buffer(BUFFER_DEFAULT_SIZE) {}
    //buffer的起始位置
    char *GetStartPos() {return &*_buffer.begin();}
    //获取读位置
    char *GetReadPos() {return GetStartPos() + _read_pos;}
    //获取写位置
    char *GetWritePos() {return GetStartPos() + _write_pos;}
    //可读长度
    uint64_t ReadableLen() {return _write_pos - _read_pos;}
    //缓冲区末尾可写长度
    uint64_t TailBufferLen() {return _buffer.size() - _write_pos;}
    //缓冲区开头可写长度
    uint64_t HeadBufferLen() {return _read_pos;}

    //确保缓冲区足够写
    void EnsureLen(uint64_t len)
    {
        if (len <= TailBufferLen()) return;
        else if(len <= HeadBufferLen() + TailBufferLen()) //把数据移动到最前面
        {
            uint64_t read_len = ReadableLen();
            std::copy(GetReadPos(), GetWritePos(), GetStartPos());
            _read_pos = 0;
            _write_pos = read_len;
        }
        else //扩容
            _buffer.resize(_write_pos + len);
    }
    //移动读位置
    void MoveReadPos(uint64_t len) 
    {
        assert(len <= ReadableLen());
        _read_pos += len;
    }
    //移动写位置
    void MoveWritePos(uint64_t len) 
    {
        assert(len <= TailBufferLen());
        _write_pos += len;
    }

    //写入数据（void*)
    void Write(const void *data, uint64_t len)
    {
        EnsureLen(len);
        std::copy((char*)data, (char*)data + len, GetWritePos());
    }
    //写入数据（string)
    void WriteString(std::string &str)
    {
        Write((void*)str.c_str(), str.size());
    }
    //写入数据（buffer)
    void WriteBuffer(Buffer &buf)
    {
        Write(buf.GetReadPos(), buf.ReadableLen());
        // buf.MoveReadPos(buf.ReadableLen());
    }

    //写入并移动写位置
    void WriteAndPush(const void *data, uint64_t len)
    {
        if(len == 0) return;
        Write(data, len);
        MoveWritePos(len);
    }
    void WriteAndPush(std::string &str)
    {
        WriteString(str);
        MoveWritePos(str.size());
    }
    void WriteAndPush(Buffer &buf)
    {
        WriteBuffer(buf);
        MoveWritePos(buf.ReadableLen());
        // LOG(DEBUG)<< buf.ReadableLen() << ": "<< ReadableLen();
    }

    //读取数据
    void Read(void *data, uint64_t len)
    {
        assert(len <= ReadableLen());
        std::copy(GetReadPos(), GetReadPos() + len, (char*)data);
    }
    //读取数据(string)
    std::string ReadasString(uint64_t len)
    {
        assert(len <= ReadableLen());
        std::string str;
        str.resize(len);
        Read(&str[0], len);
        return str;
    }
    //读取数据(one line)
    std::string ReadOneLine()
    {
        char* pos = (char*)memchr(GetReadPos(), '\n', ReadableLen()) ;
        if(pos == NULL) return "";
        uint64_t len = (uint64_t)(pos - GetReadPos()) + 1;
        return ReadasString(len);        
    }
    //读取并移动读位置
    void ReadAndPop(void *data, uint64_t len)
    {
        Read(data, len);
        MoveReadPos(len);
    }
    std::string ReadAndPop(uint64_t len)
    {
        std::string ret = ReadasString(len);
        MoveReadPos(len);
        return ret;
    }
    std::string ReadAndPop()
    {
        std::string ret = ReadOneLine();
        MoveReadPos(ret.size());
        return ret;
    }
};

#define defaultsockfd -1
class Socket
{
private:
    int _sockfd;
public:
    Socket() : _sockfd(defaultsockfd) {}
    Socket(int fd) : _sockfd(fd) {}
    ~Socket(){LOG(DEBUG)<< "close socket "<< _sockfd; }
    int _Fd(){return _sockfd;}
    //创建套接字
    bool Create()
    {
        //创建tcp套接字
        _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(_sockfd < 0)
        {
            LOG(ERROR) << "create socket error";
            return false;
        }
        LOG(DEBUG) << "create socket success";
        return true;    
    }
    //绑定ip和端口号
    bool Bind(const std::string &ip, uint16_t port)
    { 
        sockaddr_in addr;
        memset(&addr, 0, sizeof addr);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
        int n = bind(_sockfd, (sockaddr*)&addr, sizeof addr);
        if(n < 0)
        {
            LOG(ERROR) << "bind error";
            return false;
        }
        LOG(DEBUG) << "bind success";
        return true;
    }
#define MAXLISTEN 128  //backlog表示了accept队列的最大长度，与后续高并发场景有关，128 是一个经验值
    //开启监听状态
    bool Listen(int backlog = MAXLISTEN)
    {
        int n = listen(_sockfd, backlog);
        if(n < 0)
        {
            LOG(ERROR) << "listen error";
            return false;
        }
        LOG(DEBUG) << "listen success";
        return true;
    }
    //向服务器发起连接
    bool Connect(const std::string &ip, uint16_t port)
    { 
        sockaddr_in addr;
        memset(&addr, 0, sizeof addr);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
        int n = connect(_sockfd, (sockaddr*)&addr, sizeof addr);
        if(n < 0)
        {
            LOG(ERROR) << "connect error";
            return false;
        }
        LOG(DEBUG) << "connect success";
        return true;
    }
    //接受连接
    int Accept(sockaddr_in *client)
    {
        socklen_t len = sizeof(sockaddr_in);
        int fd = accept(_sockfd, (sockaddr*)client, &len);
        if(fd < 0)
        {
            LOG(ERROR) << "accept error";
            return -1;
        }
        LOG(DEBUG) << "accept success";

        return fd;
    }
    int Accept()
    {
        int fd = accept(_sockfd, nullptr, nullptr);
        if(fd < 0)
        {
            LOG(ERROR) << "accept error: "<< fd;
            return -1;
        }
        LOG(DEBUG) << "accept success";

        return fd;
    }
    //接受数据
    ssize_t Recv(void* buff, size_t len, int flags = 0)
    {
        ssize_t n = recv(_sockfd, buff, len, flags);
        // recv 返回0, 表示对端关闭, 放入判错逻辑, 最终返回0    here n<0 to n<=0 
        if(n <= 0)
        {
            // LOG(DEBUG)<< "recv errno: " << errno << " (" << strerror(errno) << ")";
            if(errno == EAGAIN || errno == EINTR)
                return 0;
            // LOG(ERROR) << "recv error";
            return -1;
        }

        // 还差一个对方断连的处理

        LOG(DEBUG) << "recv success:" << n;
        return n; 
    }
    //发送数据
    ssize_t Send(const void* buff, size_t len, int flags = 0)
    {
        ssize_t n = send(_sockfd, buff, len, flags);
        if(n < 0)
        {
            if(errno == EAGAIN || errno == EINTR)
                return 0;
            LOG(ERROR) << "send error";
            return -1;
        }
        LOG(DEBUG) << "send success";
        return n;
    }
    //关闭套接字
    void Close()
    {
        if(_sockfd != defaultsockfd)
            close(_sockfd);
    }

    //创建一个tcp服务端连接
    bool BuildTcpServer(uint16_t port, bool is_block = true, const std::string &ip = "0.0.0.0", int backlog = MAXLISTEN)
    {
        if(!Create()) return false;
        if(!is_block) SetNonBlock();
        ReUseAddr();
        if(!Bind(ip, port)) return false;
        if(!Listen(backlog)) return false;
        // ReUseAddr(); 怎么能比bind更晚调用
        return true;
    }
    //创建一个tcp客户端连接
    bool BuildTcpClient(const std::string &ip, uint16_t port, bool is_block = true)
    {
        if(!Create()) return false;
        if(!is_block) SetNonBlock();
        ReUseAddr();
        if(!Connect(ip, port)) return false;
        return true;
    }

    void ReUseAddr()
    {
        int opt = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    }
    void SetNonBlock()
    {
        int fl = fcntl(_sockfd, F_GETFL);
        if(fl < 0)
            return; 
        fcntl(_sockfd, F_SETFL, fl | O_NONBLOCK);
    }
};

class Poller;
class EventLoop;
class Channel
{
public:
    using EventCallback = std::function<void()>;
private:
    int _fd;
    // Poller *_poller;  //事件监控器
    EventLoop *loop; //real的事件监控器

    uint32_t _events;  //需要监控的事件
    uint32_t _revents; //触发的事件
    EventCallback _write_callback; //写事件回调
    EventCallback _read_callback;  //读事件回调
    EventCallback _error_callback; //错误事件回调
    EventCallback _close_callback; //关闭事件回调
    EventCallback _event_callback; //默认事件回调
public:
    // Channel(Poller *poller, int fd) : _poller(poller), _fd(fd), _events(0), _revents(0)
    Channel(EventLoop *eventloop, int fd) : loop(eventloop), _fd(fd), _events(0), _revents(0)
    {
        // LOG(DEBUG)<< "create channel: " << _fd;
    }
    ~Channel()
    {
        // LOG(DEBUG)<< "del channel: " << _fd; 
        // Remove(); // 关闭监听
        // close(_fd); // channel里关fd,外面不用管了
    }
    int _Fd() {return _fd;}
    uint32_t _Events() {return _events;}
    void SetRevents(uint32_t revents) {_revents = revents;}
    void SetWriteCall(const EventCallback &cb){_write_callback = cb;}
    void SetReadCall(const EventCallback &cb){_read_callback = cb;}
    void SetErrorCall(const EventCallback &cb){_error_callback = cb;}
    void SetCloseCall(const EventCallback &cb){_close_callback = cb;}
    void SetEventCall(const EventCallback &cb){_event_callback = cb;}

    //是否监控可读
    bool IsRead()
    {return (_events & EPOLLIN);}
    //是否监控可写
    bool IsWrite()
    {return (_events & EPOLLOUT);}
    //设置监控可读
    void SetRead()
    {_events |= EPOLLIN; Update();}
    //设置监控可写
    void SetWrite()
    {_events |= EPOLLOUT; Update();}
    // 取消监控可读
    void UnsetRead()
    {_events &= ~EPOLLIN; Update();}
    // 取消监控可写
    void UnsetWrite()
    {_events &= ~EPOLLOUT; Update();}
    // 取消所有事件监控
    void UnsetAll()
    {_events = 0; Update();}
    // 更新poller监控的事件
    void Update();
    // 取消对fd的监控
    void Remove();
    //处理触发事件
    void HandleEvents()
    {   // 处理读事件       //对端主动发FIN断联,读取最后数据       //???            
        if((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI))
        {
            // LOG(DEBUG)<< _fd<< " :" << &_read_callback;
            if(_read_callback) _read_callback();
            // 处理默认事件
            // if(_event_callback) _event_callback();
        }
        //处理写事件
        if(_revents & EPOLLOUT)
        {
            if(_write_callback) _write_callback();
            // 处理默认事件
            // if(_event_callback) _event_callback(); // here refresh
        }
        //处理错误事件 实际上都是关闭连接,一次就好       这里的处理顺序很有问题,待修改
        if(_revents & EPOLLERR)
        {
            if(_error_callback) _error_callback();
        }
        //处理关闭事件
        else if(_revents & EPOLLHUP)
        {
            if(_close_callback) _close_callback();
        }
        // 处理完前面的任务,再刷新时间片,不然执行任务时间过长,导致误以为连接不活跃
        if(_event_callback) _event_callback(); // here refresh
    }
};


#define MAXPOLLEVENTS 1024 // 一次最多获取的活跃事件数
class Poller
{
private:
    int _epfd;
    struct epoll_event _revents[MAXPOLLEVENTS]; // 保存活跃事件
    std::unordered_map<int, Channel*> _channels;// Poller监听的Channel

// 对epoll接口的封装
public:
    Poller()
    {   // 一次监听的最大事件数已经,不受限制了这个参数没啥意义
        _epfd = epoll_create(MAXPOLLEVENTS);
    }
    ~Poller()
    {
        close(_epfd);
    }
    // 对于epoll监控的文件描述符进行更新 -- 
    void UpdateEpoll(Channel *channel, int op)
    {
        auto it = _channels.find(channel->_Fd());
        assert(it != _channels.end());
        epoll_event event;
        event.events = channel->_Events();
        event.data.fd = channel->_Fd();
        int ret = epoll_ctl(_epfd, op, channel->_Fd(), &event);
        if(ret < 0)
        {
            LOG(ERROR) << "epoll_ctl error";
            abort();
        }
        // LOG(DEBUG)<< "epoll_ctl success: "<< channel->_Fd();
    }
    // 监听就绪事件
    void Poll(std::vector<Channel *> *active_channels)
    {
        // epoll_wait(epfd, events, maxevents, timeout)
        int nfds = epoll_wait(_epfd, _revents, MAXPOLLEVENTS, -1);
        if(nfds < 0)
        {   
            if(errno == EINTR)// 被信号中断
                return;
            LOG(ERROR) << "epoll_wait error";
            abort();
        }
        // LOG(DEBUG)<< "epoll_wait got: "<< nfds;
        for(int i = 0; i < nfds; i++)
        {
            // LOG(DEBUG)<< "fd: "<< _revents[i].data.fd;
            auto it = _channels.find(_revents[i].data.fd);
            assert(it != _channels.end());
            it->second->SetRevents(_revents[i].events); // 设置触发事件到channel中
            active_channels->push_back(it->second);
            // LOG(DEBUG)<< it->first<< ":"<< it->second;
        }
    }

    bool HasChannel(Channel *channel)
    {
        auto it = _channels.find(channel->_Fd());
        if(it == _channels.end()) return false;
        return true;
    }
// 交给下层调用的接口
public:
    void Update(Channel *channel)
    {
        if(!HasChannel(channel))
        {
            _channels.emplace(channel->_Fd(), channel);
            UpdateEpoll(channel, EPOLL_CTL_ADD);
        }
        else
            UpdateEpoll(channel, EPOLL_CTL_MOD);
    }
    void Remove(Channel *channel)
    {
        if(HasChannel(channel))
        {
            UpdateEpoll(channel, EPOLL_CTL_DEL);
            _channels.erase(channel->_Fd());
        }
        // delete channel; 
    }
};

using OnTimeCallback = std::function<void()>;
using ReleaseCallback = std::function<void()>;
class Timer
{
private:
    uint64_t _timer_id;// 定时器id
    int _delay; // 延迟时间
    bool _cancled;
    OnTimeCallback _task; // 到时回调函数（同样在释放时执行）
    ReleaseCallback _release; // 释放回调函数
public:
    Timer(uint64_t timer_id, int delay, OnTimeCallback &callback)
        : _timer_id(timer_id), _delay(delay), _task(callback), _cancled(false)
    {}
    ~Timer()
    {
        if (_release) _release();
        // 如果定时器被取消，则无需执行回调函数
        if(_task && !_cancled) _task();
    }

    void Cancel() {_cancled = true;}
    void SetRelease(const ReleaseCallback &relscall) {_release = relscall;}
    
    int _Delay() {return _delay;}
};
#define MAXSECOND 60
class TimeQueue
{ 
    using TimerPtr = std::shared_ptr<Timer>;
    using TimerWeakPtr = std::weak_ptr<Timer>;
private:
    int _time; // 滴答指针 秒
    int _capacity; // 时间轮节点数量 以秒为单位，60个节点
    std::vector<std::vector<TimerPtr>> _timewheel; // 时间轮 本轮
    std::unordered_map<uint64_t, TimerWeakPtr> _timermap; // 用weakptr保存timer指针，不至于让sharedptr引用计数增大

    EventLoop *_loop;
    int _timerfd; // 自动时间托管
    std::unique_ptr<Channel> _timer_channel;
private:
    int CreateTimerfd()
    {
        int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
        if(timerfd < 0)
        {
            LOG(ERROR)<< "timerfd_create failed...";
            abort();
        }
        struct itimerspec itime;
        itime.it_value.tv_sec = 1; 
        itime.it_value.tv_nsec = 0;// 第一次1s超时
        itime.it_interval.tv_sec = 1;
        itime.it_interval.tv_nsec = 0;// 第一次之后每隔一秒超时
        int ret = timerfd_settime(timerfd, 0, &itime, NULL);
        if(ret < 0)
        {
            LOG(ERROR)<< "timerfd_settime failed...";
            abort();
        }
        return timerfd;
    }
    int ReadTimefd()
    {
        uint64_t res; //返回超时多少次
        int ret = read(_timerfd, &res, sizeof res);
        if(ret < 0)
        {
            LOG(ERROR)<< "read timerfd failed...";
            abort();
        }
        return res;
    }
    //跑起滴答指针，执行到时任务
    void RunOnTimeTask()
    {
        //向后跑一秒
        _time = (_time + 1) % _capacity;
        // task被设置在析构时执行，当sharedptr的引用计数为0，自动执行
        _timewheel[_time].clear();
    }
    void OnTime()
    {
        // 超时多少秒,滴答指针跑多少次
        int times = ReadTimefd();
        for(int i = 0; i < times; ++i)
        {
            RunOnTimeTask();
        }
    }

    //删除保存的指针
    void ReleasePtr(uint64_t timer_id)
    {
        // LOG(INFO)<< "timer-"<< timer_id<< " released";
        auto it = _timermap.find(timer_id);
        assert(it != _timermap.end());
        _timermap.erase(it);
    }
    //添加计时器
    void AddTimerInLoop(uint64_t timer_id, int delay, OnTimeCallback callback)
    {
        assert(delay < _capacity && delay > 0);
        // 检查timer_id的使用情况
        auto it = _timermap.find(timer_id);
        if(it != _timermap.end())
            return;

        // 构造一个计时器
        TimerPtr pt = std::make_shared<Timer>(timer_id, delay, callback);
        pt->SetRelease(std::bind(&TimeQueue::ReleasePtr, this, timer_id));
        // 添加timer到当前时间之后delay秒的位置
        int pos = (_time + delay) % _capacity;
        _timewheel[pos].push_back(pt);
        // 保存timer指针
        _timermap[timer_id] = TimerWeakPtr(pt);
    }
    //刷新定时任务
    void RefreshTimerInLoop(uint64_t timer_id)
    {
        auto it = _timermap.find(timer_id);
        assert(it != _timermap.end());
        //延时到delay秒之后
        int delay = it->second.lock()->_Delay();
        _timewheel[(_time + delay) % _capacity].emplace_back(it->second);
    }
    //删除一个计时器
    void CancelTimerInLoop(uint64_t timer_id)
    {
        auto it = _timermap.find(timer_id);
        if(it == _timermap.end()) return;
        //设置canceled     
        it->second.lock()->Cancel();
        //不能删除保存的指针，因为无法删除timewheel中的所有记录
    }
public:
    TimeQueue(EventLoop *loop)
        :_time(0), _capacity(MAXSECOND), _timewheel(_capacity)
        , _loop(loop), _timerfd(CreateTimerfd()), _timer_channel(std::make_unique<Channel>(_loop, _timerfd))
    {
        // 将timerfd添加到读监听, 自动托管跑起滴答指针
        _timer_channel->SetReadCall(std::bind(&TimeQueue::OnTime, this));
        _timer_channel->SetRead();
    }
    ~TimeQueue(){}

    void AddTimer(uint64_t timer_id, int delay, OnTimeCallback callback);
    void RefreshTimer(uint64_t timer_id);
    void CancelTimer(uint64_t timer_id);
    // 都是在函数内部被使用(注意线程安全), 
    bool HasTimer(uint64_t timer_id)
    {
        auto it = _timermap.find(timer_id);
        return (it != _timermap.end());
    }
};

// 整合 事件监控 和 事件处理
// 定时器模块
class EventLoop
{
private:
    using task_t = std::function<void()>;
    std::thread::id _thread_id;
    Poller _poller; //事件监控 ----- 细节:必须在eventchannel和timewheel前声明,析构时就是最后一个, 他们都用shared_ptr封装了channel,其生命周期不和poller绑定
    int _event_fd;  //唤醒事件监控阻塞的情况                        // Channel的析构 需要调用poller的方法
    std::shared_ptr<Channel> _event_channel; //封装使用eventfd
    TimeQueue _time_wheel; // 时间轮,整合定时器模块  

    std::vector<task_t> _tasks; // 任务池--被其他线程获取的任务,压入任务池
    std::mutex _mutex; //保护任务池的线程安全
public:
    static int CreateEventfd()
    {
        int ret = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if(ret < 0)
        {
            LOG(FATAL)<< "create eventfd failed";
            abort();
        }
        return ret;
    }
    void ReadEventfd()
    {
        uint64_t res = 0;
        int ret = read(_event_fd, &res, sizeof res);
        if(ret < 0)
        {
            if(errno == EINTR || errno == EAGAIN)
                return;
            LOG(FATAL)<< "read eventfd failed";
            abort();
        }
        return;
    }
    void WakeUpEventfd()
    {
        uint64_t res = 1;
        int ret = write(_event_fd, &res, sizeof res);
        if(ret < 0)
        {
            if(errno == EINTR)
                return;
            LOG(FATAL)<< "write eventfd failed";
            abort();
        }
        return;
    }
    EventLoop()
        :_thread_id(std::this_thread::get_id())
        ,_poller(Poller())
        ,_event_fd(CreateEventfd())
        ,_event_channel(std::make_shared<Channel>(this, _event_fd))
        ,_time_wheel(this)
    {
        // 开启对eventfd的读事件监听,通过写入唤醒读事件,从而唤醒事件监听的阻塞
        _event_channel->SetReadCall(std::bind(&EventLoop::ReadEventfd, this));
        _event_channel->SetRead();
    }
    ~EventLoop() {LOG(INFO)<< "eventloop released"; }
    void RunAllTasks()
    {
        std::vector<task_t> working; //上锁,直接一次拿出来,后面慢慢执行,大大降低互斥冲突
        {
            std::unique_lock<std::mutex> lock(_mutex);
            working.swap(_tasks);
        }
        for(auto f : working) //依次执行
            f();
    }
//提供给别的模块调用的方法
public:
    void Start()
    {
        while(true)
        {
            //1.监听事件
            std::vector<Channel *> actives;
            _poller.Poll(&actives);
            //2.处理事件
            for(auto c : actives)
                c->HandleEvents(); //不是real的事件处理,而是把其他线程的任务压入对应线程的任务池.....
            //3.执行任务
            RunAllTasks();
        }
    }
// 运行任务 和操作任务池
    bool IsInLoop()
    {return _thread_id == std::this_thread::get_id(); }
    // 压入对应Loop任务队列
    void QueueInLoop(task_t task)
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _tasks.push_back(task); 
        }
        WakeUpEventfd(); // 防止事件监听被阻塞,延误任务执行
    }
    void RunInLoop(task_t task)
    {
        if(IsInLoop()) //是当前线程的任务, 直接执行
            return task();
        return QueueInLoop(task);
    }
// poller 相关接口
    void UpdateEvent(Channel *chan)
    {_poller.Update(chan); }
    void RemoveEvent(Channel *chan)
    {_poller.Remove(chan); }
// timewheel 相关接口
    void AddTimer(uint64_t timer_id, int delay, OnTimeCallback callback)
    {_time_wheel.AddTimer(timer_id, delay, callback); }
    void RefreshTimer(uint64_t timer_id)
    {_time_wheel.RefreshTimer(timer_id); }
    void CancelTimer(uint64_t timer_id)
    {_time_wheel.CancelTimer(timer_id); }
    bool HasTimer(uint64_t timer_id)
    {return _time_wheel.HasTimer(timer_id);}
};

void Channel::Update(){loop->UpdateEvent(this);}
void Channel::Remove(){loop->RemoveEvent(this);}
void TimeQueue::AddTimer(uint64_t timer_id, int delay, OnTimeCallback callback)
{_loop->RunInLoop(std::bind(&TimeQueue::AddTimerInLoop, this, timer_id, delay, callback)); }
void TimeQueue::RefreshTimer(uint64_t timer_id)
{_loop->RunInLoop(std::bind(&TimeQueue::RefreshTimerInLoop, this, timer_id)); }
void TimeQueue::CancelTimer(uint64_t timer_id)
{_loop->RunInLoop(std::bind(&TimeQueue::CancelTimerInLoop, this, timer_id)); }

// 管理一个EventLoop对应一个thread 
class LoopThread
{
private:
    std::mutex _mutex;             // 互斥锁
    std::condition_variable _cond; // 条件变量
    // std::atomic<EventLoop*> _loop;    // 在线程内部构造, 先给NULL
    EventLoop* _loop;    
    std::thread _thread;
private:
    // 线程入口函数 : 1.创建一个EventLoop  2.跑起来EventLoop的逻辑  
    // void ThreadEntry()
    // {
    //     EventLoop loop; // 真正创建一个loop
    //     {   // 加锁保护给_loop赋值的过程
    //         std::unique_lock<std::mutex> lock(_mutex); 
    //         LOG(DEBUG) << "lock : "<< std::this_thread::get_id();
    //         _loop.store(&loop, std::memory_order_release); 
    //         LOG(DEBUG)<< _loop.load(std::memory_order_acquire);  // BUG!!!! 这边赋值了,外面竟然没有更新 ---- 暂时解决方案: 先创建thread再依次获取_loop(没用依旧不更新)
    //         // _loop已经构造出了, 唤醒要获取_loop的线程
    //         _cond.notify_all();
    //         LOG(DEBUG) << "unlock : "<< std::this_thread::get_id();
    //     }
    //     // 运行起来eventloop
    //     loop.Start();
    //     LOG(INFO)<< "thread: "<< std::this_thread::get_id()<<" released"; 
    // }
    void ThreadEntry()
    {
        LOG(DEBUG)<< "New Thread : "<< std::this_thread::get_id();
        EventLoop loop; // 真正创建一个loop
        {   // 加锁保护给_loop赋值的过程
            std::unique_lock<std::mutex> lock(_mutex); 
            _loop = &loop;
            _cond.notify_all();
        }
        // 运行起来eventloop
        loop.Start();
        LOG(INFO)<< "thread: "<< std::this_thread::get_id()<<" released"; 
    }
public:
    LoopThread() :_thread(&LoopThread::ThreadEntry, this), _loop(nullptr)
    {}
    ~LoopThread() {LOG(INFO)<< "threadloop released"; }
    // 获取EventLoop对象的指针
    EventLoop* GetLoop()
    {
        EventLoop* loop = nullptr;
        {   // 加锁保护获取_loop的过程
            std::unique_lock<std::mutex> lock(_mutex);
            // 等待另一线程把_loop构造了
            _cond.wait(lock, [this](){return (_loop != nullptr); }); //LOG(DEBUG)<< _loop;
            loop = _loop;
        }
        return loop;
    }
    //  LoopThread() :_thread(&LoopThread::ThreadEntry, this)//, _loop(nullptr)
    // {_loop.store(nullptr, std::memory_order_release);  }
    // ~LoopThread() {LOG(INFO)<< "threadloop released"; }
    // // 获取EventLoop对象的指针
    // EventLoop* GetLoop()
    // {
    //     EventLoop* loop = nullptr;
    //     {   // 加锁保护获取_loop的过程
    //         std::unique_lock<std::mutex> lock(_mutex);
    //         // 等待另一线程把_loop构造了
    //         LOG(DEBUG) << "thread wait: "<< std::this_thread::get_id();
    //         _cond.wait(lock, [this](){LOG(DEBUG)<< _loop.load(std::memory_order_acquire); return (_loop.load(std::memory_order_acquire) != nullptr); }); //LOG(DEBUG)<< _loop;
    //         LOG(DEBUG) << "thread awake: "<< std::this_thread::get_id();
    //         loop = _loop.load(std::memory_order_acquire);
    //     }
    //     return loop;
    // }
};

class LoopThreadPool
{
private:
    EventLoop *_base_loop;  // 主Reactor---只负责建立新连接
    int _IO_thread_count;   // IO线程的数量
    int _index;             // 轮转分配新连接的下标 --- 分配策略:轮转依次分配
    std::vector<LoopThread *> _loop_thread_pool;  // 从Reactor的线程池 从Reactor---负责连接的事件监听,和事件的处理
    std::vector<EventLoop *> _event_loop_pool;    // 线程池对应的eventloop池,一次性拿出
public:
    LoopThreadPool(EventLoop *baseloop) :_base_loop(baseloop), _IO_thread_count(0), _index(0)
    {}
    // 设置IO线程的数量
    void SetCount(int count) {_IO_thread_count = count;/*LOG(DEBUG)<< _IO_thread_count;*/ }
    // 构建loopthread
    void Create()
    {
        if(_IO_thread_count == 0) return;
        _loop_thread_pool.resize(_IO_thread_count);
        _event_loop_pool.resize(_IO_thread_count);
        for(int i = 0; i < _IO_thread_count; ++i)
        {
            _loop_thread_pool[i] = new LoopThread();
            _event_loop_pool[i] = _loop_thread_pool[i]->GetLoop();
        }
        // sleep(2);
        // for(int i = 0; i < _IO_thread_count; ++i)
        //     _event_loop_pool[i] = _loop_thread_pool[i]->GetLoop();

    }
    // 获取下一个loop
    EventLoop *GetNextLoop()
    {
        // _IO_thread_count为0, 单Reactor模式
        if(_IO_thread_count == 0) return _base_loop;
        // 轮转依次分配
        _index = (_index + 1) % _IO_thread_count;
        return _event_loop_pool[_index];
    }
};

// 处理连接模块 
class Connection;
typedef enum {DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING} CONNSTATU;
using ConnectionPtr = std::shared_ptr<Connection>;
                    //模版类: 支持 获取封装this指针的shared_ptr
class Connection : public std::enable_shared_from_this<Connection>
{
private:
    uint64_t _con_id;  // 连接的唯一标识id --- 也可以作为timer_id简化操作
    int _sockfd;       // 用于连接的文件描述符
    CONNSTATU _statu;      // 连接状态
    bool _silence_connect; // 是否支持静默连接 --- 是否超时断连
    std::any _context;     // 请求的接收处理上下文

    Socket _socket;    // 套接字操作管理
    EventLoop *_loop;  // 关联的eventloop
    Channel _channel;  // 事件管理模块

    Buffer _buff_out;  // 用户级输出缓冲区
    Buffer _buff_in;   // 用户级输入缓冲区

    // 回调函数 --- 为上层使用设计,由服务器模块设置
    using ConnectedCallback = std::function<void(const ConnectionPtr &)>;
    using MessageCallback = std::function<void(const ConnectionPtr &, Buffer *)>;
    using CloseCallback = std::function<void(const ConnectionPtr &)>;
    using AnyEventCallback = std::function<void(const ConnectionPtr &)>;
    ConnectedCallback _connected_callback;
    MessageCallback _message_callback;
    CloseCallback _close_callback;
    AnyEventCallback _event_callback;
    // 上层删除管理连接的方法
    CloseCallback _server_close_callback;
private:
    // Channel的5个回调函数
    void ReadHandler()
    {
        // LOG(DEBUG)<< "--- Read ---";
        //1.读取数据到缓冲区
        char buff[65536]; //64kb
        ssize_t ret = _socket.Recv(buff, 65535);
        if(ret < 0) // 出错了
            return ShutdownInLoop(); // 检查缓冲区后关闭连接
        // 读到的数据写入buff_in
        _buff_in.WriteAndPush(buff, ret); 
        //2.主动调用回调,处理业务
        // 如果输入缓冲区有数据,调用message回调
        if(_buff_in.ReadableLen())
            _message_callback(shared_from_this(), &_buff_in);
    }
    void WriteHandler()
    {           // 将buffout中数据手动发送
        ssize_t ret = _socket.Send(_buff_out.GetReadPos(), _buff_out.ReadableLen());
        // LOG(DEBUG)<< "Send: to client "<< ret << ": "<< _buff_out.ReadableLen()<< _buff_out.GetReadPos();
        if(ret < 0) // send出错了
        { // 手动检测buff_in, 有数据则将数据先处理
            if(_buff_in.ReadableLen())
                _message_callback(shared_from_this(), &_buff_in);
            // 真的关闭连接 -- 不能调shutdown套娃
            return Release();   
            // return ReleaseInLoop();
        }
        // 手动修正buffout的readpos --- 手动操作不能忘
        _buff_out.MoveReadPos(ret);
        // 读完了,设置无需监控读,防止老被调,浪费
        if(_buff_out.ReadableLen() == 0) 
            _channel.UnsetWrite();
        // ?? 不懂为啥这里, 先写好吧
        // 当执行过Shutdown(半连接状态),还要最后把发送缓冲区的数据发出去, 直到: 1.发送出错 2.发送完了(这里)
        if(_statu == DISCONNECTING)
            return Release();   
            // ReleaseInLoop(); 调用栈帧太深了; 还要考虑调用的顺序性,如果前面有读写操作正常压入任务池->先调用release关闭了fd.....
    }
    void CloseHandler()
    {
        // 如果有数据待处理,那先处理 ----- ?: 为什么不直接调用shutdownInLoop????
        if(_buff_in.ReadableLen() > 0) _message_callback(shared_from_this(), &_buff_in);
        return Release();
    }
    void ErrorHandler() {CloseHandler(); }
    void EventHandler()
    {
        if(!_silence_connect) _loop->RefreshTimer(_con_id);   //yeah, refresh here
        if(_event_callback) _event_callback(shared_from_this()); 
    }

    void SilenceConnectInLoop()
    {
        _silence_connect = true;
        if(_loop->HasTimer(_con_id)) _loop->CancelTimer(_con_id);
    }
    void RefuseSilenceConnectInLoop(int delay)
    {
        _silence_connect = false;
        if(_loop->HasTimer(_con_id)) return _loop->RefreshTimer(_con_id); //若添加过, 刷新
        return _loop->AddTimer(_con_id, delay, std::bind(&Connection::Release, this)); //若原来没有添加, 新添一个
    }

    // 连接建立之后,设置状态, 真正的开始工作
    void EstablishedInLoop()
    {
        LOG(DEBUG)<< "--- Established ---";
        //1.设置已连接状态
        if(_statu == CONNECTING) _statu = CONNECTED;
        //2.经典开局设置读监听
        _channel.SetRead();
        //3.调用连接回调
        if(_connected_callback) _connected_callback(shared_from_this());
    }
    // 发送数据到客户端
    void SendInLoop(Buffer &data)
    {
        // 写入缓冲区先
        _buff_out.WriteAndPush(data);
        // 打开写监听
        if(!_channel.IsWrite()) _channel.SetWrite(); 
    }
     //真正的关闭连接
    void ReleaseInLoop()
    {
        LOG(DEBUG)<< "---- released ----"<< _sockfd;
        LOG(INFO)<< "client-"<< _Id()<< " left......";
        // //设置关闭连接状态
        // _statu = DISCONNECTED;   在Release里设置

        _channel.Remove(); // 取消poller的监控
        _socket.Close();   // 关闭文件描述符      
        // 打开静默状态, 就是关掉定时器,防止后面再调用什么相关接口
        if(!_silence_connect) SilenceConnectInLoop();
        // 调用关闭回调
        if(_close_callback) _close_callback(shared_from_this());
        // 调用管理端关闭回调
        if(_server_close_callback) _server_close_callback(shared_from_this());
        // while(1) sleep(1);
    }
    // 关闭连接前,处理剩下的数据
    void ShutdownInLoop()
    {
        // LOG(DEBUG)<< "--- Shutdown ---";
        // 0.设置半连接状态*--- 让后续write回调时通过状态去调用release
        _statu = DISCONNECTING;
        // 1.先看看有没有数据需要处理
        if(_buff_in.ReadableLen()) _message_callback(shared_from_this(), &_buff_in);
        // 2.有没有数据需要最后发送, 设置写事件监控,自然会调用channel的写事件回调
        if(_buff_out.ReadableLen() && !_channel.IsWrite()) _channel.SetWrite();
        // 3.如果没有数据可以发了, release
        if(_buff_out.ReadableLen() == 0) Release();
        // 这里没有成功调用Release也不用担心, 已经设置好了半连接状态, 后续发送完毕就会调用release
    }
    // 切换协议
    void UpgradeInLoop(const std::any &context,
        const ConnectedCallback &connect,
        const MessageCallback &message,
        const CloseCallback &close,
        const AnyEventCallback &event)
    {
        _context = context;
        _connected_callback = connect;
        _message_callback = message;
        _close_callback = close;
        _event_callback = event;
    }
// 给上层提供的接口
public:
    Connection(uint64_t id, int fd, EventLoop *loop)
        : _con_id(id), _sockfd(fd), _statu(CONNECTING), _silence_connect(true)
        , _socket(_sockfd), _loop(loop), _channel(_loop, _sockfd)
    {
         // 设置非阻塞
        _socket.SetNonBlock();
        // //设置channel回调            
        _channel.SetReadCall(std::bind(&Connection::ReadHandler, this));
        _channel.SetWriteCall(std::bind(&Connection::WriteHandler, this));
        _channel.SetCloseCall(std::bind(&Connection::CloseHandler, this));
        _channel.SetErrorCall(std::bind(&Connection::ErrorHandler, this));
        _channel.SetEventCall(std::bind(&Connection::EventHandler, this));
    }
    // void SetChannelCallback()
    // {
    //     //设置channel回调            维护connection的生命周期
    //     _channel.SetReadCall(std::bind(&Connection::ReadHandler, shared_from_this()));
    //     _channel.SetWriteCall(std::bind(&Connection::WriteHandler, shared_from_this()));
    //     _channel.SetCloseCall(std::bind(&Connection::CloseHandler, shared_from_this()));
    //     _channel.SetErrorCall(std::bind(&Connection::ErrorHandler, shared_from_this()));
    //     _channel.SetEventCall(std::bind(&Connection::EventHandler, shared_from_this()));
    // }
    ~Connection() {LOG(DEBUG)<< "connection closed: "<< ("%p", this) ;}
    int _Fd() {return _sockfd; }
    int _Id() {return _con_id; }
    void SetContext(const std::any context) {_context = context; } 
    std::any *GetContext() {return &_context; }
    void SetConnectedCall(const ConnectedCallback &cb) {_connected_callback = cb; }
    void SetMessageCall(const MessageCallback &cb) {_message_callback = cb; }
    void SetCloseCall(const CloseCallback &cb) {_close_callback = cb; }
    void SetEventCall(const AnyEventCallback &cb) {_event_callback = cb; }
    void SetSrvCloseCall(const CloseCallback &cb) {_server_close_callback = cb; }

   void SilenceConnect()
    {
        if(_statu == DISCONNECTED) return; // 已关闭连接状态,套接字被释放, 甚至连接管理也已经失效
        _loop->RunInLoop(std::bind(&Connection::SilenceConnectInLoop, this)); 
    }
    void RefuseSilenceConnect(int delay = 8)
    {
        if(_statu == DISCONNECTED) return; // 已关闭连接状态,套接字被释放, 甚至连接管理也已经失效
        _loop->RunInLoop(std::bind(&Connection::RefuseSilenceConnectInLoop, this, delay)); 
    }

    void Established()
    {
        if(_statu == DISCONNECTED) return; // 已关闭连接状态,套接字被释放, 甚至连接管理也已经失效
        _loop->RunInLoop(std::bind(&Connection::EstablishedInLoop, this)); 
    }
    void Send(const char *data, size_t len)
    {
        if(_statu == DISCONNECTED) return; // 已关闭连接状态,套接字被释放, 甚至连接管理也已经失效
        //data可能是临时对象, 转成buffer
        Buffer buff;
        buff.WriteAndPush(data, len);           // 在bind这样的打包器里用move转换普通对象非常好使---bind里会保存一个左值副本,刚好对应move后的右值引用,移动构造
        // LOG(DEBUG)<< buff.GetReadPos()<< ":"<< data;
        _loop->RunInLoop(std::bind(&Connection::SendInLoop, this, std::move(buff))); 
    }
    void Release() //真正的关闭连接
    {
        if(_statu == DISCONNECTED) return; // 防止重入release
        // 只要调用了Release并把ReleaseInLoop加入任务队列, 就设置关闭连接状态, 不允许其他回调加入任务队列 (在执行完ReleaseInLoop后,socket已关闭,连接对象已经被销毁)
        _statu == DISCONNECTED;
        // _loop->RunInLoop(std::bind(&Connection::ReleaseInLoop, this)); 
        //  !!!!!  兄弟,有的回调不是因为在别的线程才压入任务队列的
        //   而是考虑到顺序性, 还有栈帧深度..... 这种只能让其在本线程执行,
        //   无论如何都要入任务队列
        // _loop->QueueInLoop(std::bind(&Connection::ReleaseInLoop, this)); 
        // connection在任务队列的最后一道屏障, tcpserver管理的_cons会提前销毁? ???
        _loop->QueueInLoop(std::bind(&Connection::ReleaseInLoop, shared_from_this())); 
    }
    void Shutdown()
    {
        if(_statu == DISCONNECTED) return; // 已关闭连接状态,连接管理也已经失效
        _loop->RunInLoop(std::bind(&Connection::ShutdownInLoop, this)); 
    }
    // 切换协议
    void Upgrade(const std::any &context,
        const ConnectedCallback &connect,
        const MessageCallback &message,
        const CloseCallback &close,
        const AnyEventCallback &event)
    {
        if(_statu == DISCONNECTED) return; // 已关闭连接状态,套接字被释放, 甚至连接管理也已经失效
        _loop->QueueInLoop(std::bind(&Connection::UpgradeInLoop, this, context, connect, message, close, event)); 
    }
    // 考虑到协议切换的顺序性, 强制插入任务队列
    // {_loop->RunInLoop(std::bind(&Connection::UpgradeInLoop, this, context, connect, message, close, event)); }
};

// listenfd管理模块
class Acceptor
{
    using AcceptCallback = std::function<void(int)>;
private:
    // int _listen_fd; 声明顺序就是初始化顺序, 先有socket才给的fd.....
    Socket _listen_socket;
    int _listen_fd;
    EventLoop *_loop;
    Channel _listen_channel;
    AcceptCallback _callback;
private:
    int CreateSocket(uint16_t port)
    {
        bool ret = _listen_socket.BuildTcpServer(port);
        assert(ret == true);
        return _listen_socket._Fd();
    }
    void GetNewfd()
    {
        int newfd = _listen_socket.Accept();
        // sleep(10);
        if(newfd < 0) return;
        if(_callback) _callback(newfd);
    }
public:
    Acceptor(EventLoop *loop, uint16_t port) :_listen_fd(CreateSocket(port)), _loop(loop), _listen_channel(_loop, _listen_fd) 
    {
        _listen_channel.SetReadCall(std::bind(&Acceptor::GetNewfd, this));
        // 不可在此处设置监听, 此时callback还未设置, 获得的newfd就丢失了
        // _listen_channel.SetRead();
    }
    // 设置回调
    void SetAcceptCallback(const AcceptCallback &cb) {_callback = cb; }
    // 开启监听
    void Listen() {_listen_channel.SetRead(); }
};

class TcpServer
{
private:
    uint64_t _unique_id;  // 连接的唯一标识(包含timer_id)
    uint16_t _port;       //
    int _timeout;            // 多久算是非活跃连接 ---刷新延迟定时器的时长
    bool _silence_connect;   // 是否支持静默连接(非活跃不断连)

    EventLoop _baseloop;    // 主Reactor -- 只负责建立新连接
    Acceptor _acceptor;     // 监听管理模块
    LoopThreadPool _pool;   // 管理从Reactor的线程池 从Reactor---负责连接的事件监听,和事件的处理
    std::unordered_map<uint64_t, ConnectionPtr> _conn_map;  // 管理所有连接

    // 协议层给提供的回调
    using ConnectedCallback = std::function<void(const ConnectionPtr &)>;
    using MessageCallback = std::function<void(const ConnectionPtr &, Buffer *)>;
    using CloseCallback = std::function<void(const ConnectionPtr &)>;
    using AnyEventCallback = std::function<void(const ConnectionPtr &)>;
    using TimerTask = std::function<void()>;
    ConnectedCallback _connected_callback;
    MessageCallback _message_callback;
    CloseCallback _close_callback;
    AnyEventCallback _event_callback;

private:
    void RunAfterInloop(const TimerTask &task, int delay)
    {
        _baseloop.AddTimer(_unique_id, delay, task);
        ++_unique_id;
    }
    // connection的SrvClose回调 -- 删除map中的对应连接
    void RemoveConnectionInLoop(const ConnectionPtr & conn)
    {
        int id = conn->_Id();
        // LOG(DEBUG)<< "Remove connection: "<< id<<" use_count: "<<conn.use_count();
        auto it = _conn_map.find(id);

        //assert(it != _conn_map.end());  高并发的场景下, 同个connection被删了两次也是有可能
        if(it != _conn_map.end())
            _conn_map.erase(it);
    }
    // 必须在本线程内操作, 不然可能多个IO线程同时修改conn_map ---- 反正一定遵守"自己线程管理的资源,只能自己操作"
    void RemoveConnection(const ConnectionPtr & conn)
    {_baseloop.RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));}
    // acceptor的回调, 创建(分配给从Reactor),设置新连接, 并加入连接管理map
    void NewConnection(int newfd)
    {
        ConnectionPtr new_conn = std::make_shared<Connection>(_unique_id, newfd, _pool.GetNextLoop());
        // new_conn->SetChannelCallback();
        if(_connected_callback)
            new_conn->SetConnectedCall([this](const ConnectionPtr & conn){_connected_callback(conn) ;});
        if(_message_callback)
            new_conn->SetMessageCall([this](const ConnectionPtr & conn, Buffer *data){_message_callback(conn, data); });
        if(_close_callback)
            new_conn->SetCloseCall([this](const ConnectionPtr & conn){_close_callback(conn); });
        if(_event_callback)
            new_conn->SetEventCall([this](const ConnectionPtr & conn){_event_callback(conn); });
        new_conn->SetSrvCloseCall([this](const ConnectionPtr & conn){RemoveConnection(conn); });

        if(!_silence_connect) new_conn->RefuseSilenceConnect(_timeout);
        else new_conn->SilenceConnect();
        new_conn->Established();
        LOG(INFO)<< "new connection:"<< new_conn->_Id();
        _conn_map.emplace(_unique_id, new_conn);
        ++_unique_id;
    }
public:
    TcpServer(uint16_t port) :_unique_id(0), _port(port), _timeout(0), _silence_connect(true)
            , _acceptor(&_baseloop, _port), _pool(&_baseloop) 
    {
        _acceptor.SetAcceptCallback([this](int fd){return NewConnection(fd); });
        _acceptor.Listen();
    }
    // 设置从Reactor,线程数量
    void SetPoolCount(int count) {_pool.SetCount(count);}
    // 设置拒绝静默连接 --- 开启超时断连机制
    void RefuseSilenceConnect(int timeout) {_silence_connect = false; _timeout = timeout; }
    // 开启静默连接
    void SetSilenceConnect() {_silence_connect = true; }
    // 设置回调
    void SetConnectedCall(const ConnectedCallback &cb) {_connected_callback = cb; }
    void SetMessageCall(const MessageCallback &cb) {_message_callback = cb; }
    void SetCloseCall(const CloseCallback &cb) {_close_callback = cb; }
    void SetEventCall(const AnyEventCallback &cb) {_event_callback = cb; }

    // 添加一个定时任务
    void RunAfter(const TimerTask &task, int delay) 
    {_baseloop.RunInLoop(std::bind(&TcpServer::RunAfterInloop, this, task, delay)); }
    // 开始运行Server
    void Start()
    {    // 先运行起来从Reactor, 主Reactor的逻辑里要分配新连接给从Reactor
        _pool.Create();
        _baseloop.Start();
    }
};


// 对已关闭的 socket 进行 write 会触发 SIGPIPE 信号，进程将直接终止
// 这是绝对必要的安全屏障
class NetWork {
    public:
        NetWork() {
           // DBG_LOG("SIGPIPE INIT");
            signal(SIGPIPE, SIG_IGN);
        }
};
static NetWork nw;



#endif