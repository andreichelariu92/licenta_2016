#ifndef LOGGER_H_
#define LOGGER_H_

#include<mutex>
#include<fstream>
#include<thread>
#include<string>
struct Information
{
    unsigned int lineNumber;
    std::thread::id threadId;
    std::string filename;
    std::string time;

    Information(unsigned int ln, std::thread::id tid, std::string file, std::string tm)
        :lineNumber(ln), threadId(tid), filename(file), time(tm)
    {}

};
#define INFO Information(__LINE__, std::this_thread::get_id(), __FILE__, __TIME__)

class Logger
{
private:
    std::ofstream m_ofs;
    std::mutex m_mutex;

    //suppress creation and copy
    //of instances for the singleton pattern
    Logger(std::string fileName);
    Logger& operator=(const Logger& source)=delete;
    Logger(const Logger& source)=delete;
public:
    enum Prio
    {
        trace=0,
        debug=1,
        error=2,
        exception=3
    };

    template<typename T>
    Logger& operator<<(T& data)
    {
        {//enter critical section
            std::lock_guard<std::mutex> lock(m_mutex);
            m_ofs<<data;
        }//exit critical section
        return *this;
    }
    Logger& operator<<(Prio data);
    Logger& operator<<(Information info);


    static Logger& getInstance(std::string filename="logfile");
};
#define LOG Logger::getInstance()
#endif
