#include <iostream>
#include <unistd.h>
#include <assert.h>

#include <vector>
#include <unordered_map>

#include <functional>
#include <memory>

using OnTimeCallback = std::function<void()>;
using ReleaseCallback = std::function<void()>;
class Timer
{
private:
    u_int64_t _timer_id;// 定时器id
    int _delay; // 延迟时间
    bool _cancled;
    OnTimeCallback _task; // 到时回调函数（同样在释放时执行）
    ReleaseCallback _release; // 释放回调函数
public:
    Timer(u_int64_t timer_id, int delay, OnTimeCallback &callback)
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
    std::unordered_map<u_int64_t, TimerWeakPtr> _timermap; // 用weakptr保存timer指针，不至于让sharedptr引用计数增大
public:
    TimeQueue()
        :_time(0), _capacity(MAXSECOND), _timewheel(_capacity)
    {}
    ~TimeQueue(){}

    //添加计时器
    void AddTimer(u_int64_t timer_id, int delay, OnTimeCallback callback)
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
    //跑起滴答指针，执行到时任务
    void RunOnTimeTask()
    {
        //向后跑一秒
        _time = (_time + 1) % _capacity;
        // task被设置在析构时执行，当sharedptr的引用计数为0，自动执行
        _timewheel[_time].clear();
    }
    //删除保存的指针
    void ReleasePtr(u_int64_t timer_id)
    {
        auto it = _timermap.find(timer_id);
        assert(it != _timermap.end());
        _timermap.erase(it);
    }
    //刷新定时任务
    void RefreshTimer(u_int64_t timer_id)
    {
        auto it = _timermap.find(timer_id);
        assert(it != _timermap.end());
        //延时到delay秒之后
        int delay = it->second.lock()->_Delay();
        _timewheel[(_time + delay) % _capacity].emplace_back(it->second);
    }
    //删除一个计时器
    void CancelTimer(u_int64_t timer_id)
    {
        auto it = _timermap.find(timer_id);
        if(it == _timermap.end()) return;
        //设置canceled     
        it->second.lock()->Cancel();
        //不能删除保存的指针，因为无法删除timewheel中的所有记录
    }
};


struct Test{
    Test()
    {
        std::cout<< "create"<< std::endl;
    }
    ~Test()
    {
        std::cout<< "destory"<< std::endl;
    }
};
void Task(Test *t)
{
    t->~Test();
}
int main()
{
    TimeQueue tq;

    Test t;

    tq.AddTimer(66, 5, std::bind(Task, &t));
    for(int i = 0; i < 3; ++i)
    {
        tq.RunOnTimeTask();
        sleep(1);
        tq.RefreshTimer(66);
        std::cout<< "refreshed!"<< std::endl;
    }
    while(true)
    {
        tq.RunOnTimeTask();
        sleep(1);
        std::cout<< "running 1 sec"<< std::endl;

    }
}