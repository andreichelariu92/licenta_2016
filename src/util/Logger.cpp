#include"../util/Logger.h"
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
        switch(data)
        {
        case trace:
            m_ofs<<"TRC  ";
            break;
        case debug:
            m_ofs<<"DBG  ";
            break;
        case error:
            m_ofs<<"ERR  ";
            break;
        case exception:
            m_ofs<<"EXC  ";
            break;
        default:
            break;
        }
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

    }//exit critical section

    return *this;
}
