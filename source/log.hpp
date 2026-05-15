#ifndef __LOG_HPP__
#define __LOG_HPP__

#include "Mutex.hpp"
#include <filesystem>
#include <fstream>
#include <time.h>

#define RN "\r\n"

namespace LogModule
{
    using namespace LockModule;

    //策略模式
    class LogStrategy
    {
    public:
        virtual ~LogStrategy() = default;
        virtual void RefreshLog(std::string &message) = 0; 
    };
    //显示屏模式
    class ScreenLogStrategy : public LogStrategy
    {
    public:
        void RefreshLog(std::string &message) override
        {
            LockGuard lg(_mutex);
            std::cout<< message<< RN;
        } 
    private:
        Mutex _mutex;
    };
    //写入文件策略
    #define PATH "./log"
    #define FILE "log.log"
    class FileLogStrategy : public LogStrategy
    {
    public:
        FileLogStrategy(std::string path = PATH, std::string file = FILE)
            :_path(path)
            ,_file(file)
            ,_mutex()
        {
            LockGuard lg(_mutex);
            if(std::filesystem::exists(_path))
                return;
            try{
                std::filesystem::create_directories(_path);
            }
            catch(const std::filesystem::filesystem_error &e)
            {
                std::cerr << e.what() << '\n';
            }
        }

        void RefreshLog(std::string &message) override
        {
            LockGuard lg(_mutex);
            std::string log = _path + (_path.back() == '/' ? "" : "/") + _file;
            std::ofstream in(log.c_str(), std::ios::app);
            if(!in.is_open())
                return;
            in<< message<< RN;
        }

    private:
        std::string _path;
        std::string _file;
        Mutex _mutex;
    };

    //生成日志

    std::string GetCurTime()
    {
        time_t tm = time(nullptr);
        struct tm result;
        localtime_r(&tm, &result);
        
        std::stringstream ss;
        ss<< result.tm_year<< "-"<< result.tm_mon<< "-"<< result.tm_mday<< " "<< result.tm_hour<< ":"<< result.tm_min<<":"<<result.tm_sec;
        return ss.str();
    }

    enum class Level
    {
        INFO,
        WARNING,
        ERROR,
        DEBUG,
        FATAL
    };

    std::string LevToStr(Level &level)
    {
        switch(level)
        {
            case Level::DEBUG:
                return "DEBUG";
            case Level::INFO:
                return "INFO";
            case Level::WARNING:
                return "WARNING";
            case Level::FATAL:
                return "FATAL";
            case Level::ERROR:
                return "ERROR";
            default:
                return "!!!!";
        }
        return "OOOOO";
    }

    //外部类（报社）—— 指导策略，提供刷新方案
    class Logger
    {
    public:
        Logger()
        {
            UseScreenLogStrategy();
        }
        void UseScreenLogStrategy()
        {
            _strat = std::make_unique<ScreenLogStrategy>();
        }
        void UseFileLogStrategy()
        {
            _strat = std::make_unique<FileLogStrategy>();
        }
        ~Logger()
        {}

        //内部类（编辑）—— 形成具体日志，根据刷新方案刷新
        class Message
        {
        public:
            Message(Level &level, std::string name, int line, Logger &logger)
                :_time(GetCurTime())
                ,_level(level)
                ,_name(name)
                ,_line(line)
                ,_logger(logger)
            {
                std::stringstream buffer;
                buffer/*<< std::this_thread::get_id()*/
                    << "["<< _time<<"]"
                    << "["<< LevToStr(_level)<<"]"
                    << "["<< _name
                    << ":"<< _line<<"] - ";
                _loginfo = buffer.str();
            }

            template<class T>
            Message &operator<<(T info)
            {
                std::stringstream buffer;
                buffer<< info;
                _loginfo += buffer.str();
                return *this;
            }
    
    #define DEBUG_OPEN 1

            ~Message()
            {
            #ifdef DEBUG_OPEN
                if(_level != Level::DEBUG)
            #endif
                    _logger._strat->RefreshLog(_loginfo);
            }

        private:
            std::string _time;
            Level _level;
            std::string _name;
            int _line;
            std::string _loginfo;
            Logger &_logger;
        };
    
        Message operator()(Level level, std::string name, int line)
        {
            return Message(level, name, line, *this);
        }

    private:
        std::unique_ptr<LogStrategy> _strat;
    };

    Logger logger;

    #define LOG(level) logger(Level::level, __FILE__, __LINE__)
    #define Use_Screen_Log_Strategy logger.UseScreenLogStrategy()
    #define Use_File_Log_Strategy logger.UseFileLogStrategy()
}

#endif