#ifndef Serializer_H_INCLUDE_GUARD
#define Serializer_H_INCLUDE_GUARD

#include <sstream>

#include "DirectoryWatcher.h"

///class that serializes a vector
///of file events
///the file events are serialized in
///a JSON and are separated between %
class Serializer
{
private:
    std::stringstream ss_;
public:
    Serializer() = default;
    Serializer(const Serializer& source) = default;
    Serializer& operator=(const Serializer& source) = default;
    Serializer(Serializer&& source);
    Serializer& operator=(Serializer&& source);
    ~Serializer() = default;

    std::string serialize(std::vector<FileEvent>& fileEvents);
};
#endif
