#ifndef LOGGER_H_
#define LOGGER_H_

#include<mutex>
#include<ostream>
#include<fstream>

template<typename OutputStream>
class Logger
{
private:
    OutputStream m_os;
    std::mutex m_mutex;
    unsigned int getLineNumber();
    unsigned int getThreadId();
protected:
    Logger(OutputStream argOs);
public:
    template<typename T>
    Logger& operator<<(T& data);

};

#endif
