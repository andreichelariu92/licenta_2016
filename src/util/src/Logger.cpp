#include"Logger.h"
Logger::Logger(std::string fileName)
   :m_ofs(fileName, std::ios_base::out),
    m_mutex()
{
}

Logger& Logger::getInstance(std::string filename)
{
    static Logger instance(filename);
    return instance;
}
Logger& Logger::operator <<(Logger::Prio data)
{
    std::string priority;
    switch(data)
    {
    case trace:
        priority = "TRC  ";
        break;
    case debug:
        priority = "DBG  ";
        break;
    case error:
        priority = "ERR  ";
        break;
    case exception:
        priority = "EXC  ";
        break;
    default:
        break;
    }

    {//enter critical section
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ofs.flush();
    }//exit critical section

    return *this;
 }
Logger& Logger::operator <<(Information info)
{
    info.threadId=std::this_thread::get_id();
    std::time_t t= std::time(NULL);
    //get hh:mm:ss from t
    info.time=std::string( std::ctime( &t ), 11, 8);

    {//enter critical section
    std::lock_guard<std::mutex> lock(m_mutex);

    m_ofs<<info.filename<<"  ";
    m_ofs<<info.lineNumber<<"  ";
    m_ofs<<info.threadId<<"  ";
    m_ofs<<info.time<<"  ";

    m_ofs.flush();

    }//exit critical section

    return *this;
}

Logger::~Logger()
{
    m_ofs.flush();
    m_ofs.close();
}
