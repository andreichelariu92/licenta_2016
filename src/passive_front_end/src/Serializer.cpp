#include "Serializer.h"
#include <iostream>

using std::string;
using std::stringstream;
using std::vector;

Serializer::Serializer(Serializer&& source)
    :ss_(source.ss_.str())
{}

Serializer& Serializer::operator=(Serializer&& source)
{
    ss_.str(source.ss_.str());
    return *this;
}
string Serializer::serialize(vector<FileEvent>& fileEvents)
{
    if (fileEvents.size() == 0)
        return string("");

    ss_.str("");
    for (FileEvent& fileEvent : fileEvents)
    {
        ss_ << "%{";
        
        ss_ << "\"absolutePath\" : \"";
        ss_ << fileEvent.absolutePath;
        
        ss_ << "\", \"fileType\" : \"";
        switch (fileEvent.fileType)
        {
            case FileType::file:
                ss_ << "file\", ";
                break;
            case FileType::directory:
                ss_ << "directory\", ";
                break;
        }
        
        ss_ << "\"eventType\" : \"";
        switch (fileEvent.eventType)
        {
            case EventType::create:
                ss_ << "create\"";
                break;
            case EventType::deleted:
                ss_ << "deleted\"";
                break;
            case EventType::movedFrom:
                ss_ << "movedFrom\"";
                break;
            case EventType::movedTo:
                ss_ << "movedTo\"";
                break;
            case EventType::open:
                ss_ << "open\"";
                break;
            case EventType::close:
                ss_ << "close\"";
                break;
            case EventType::invalid:
                return string("");
        }

        ss_ << "}%";
    }
    ss_ << "\n";
    return ss_.str();
}
