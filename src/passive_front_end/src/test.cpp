//standard library headers
#include <cstdio>
#include <memory>
#include <exception>
//OS headers
#include <errno.h>
//my headers
#include "../../util/src/Logger.h"
#include "FileEventServer.h"

int main()
{
    //test results
    //63.7 MB consumed -> 590.5 MB file hierarchy
    //215.1 MB consumed -> 13.263 GB file hierarchy
    try
    {
        boost::asio::io_service ioService;
        std::remove("./file.txt");
        FileEventSessionFactory factory("/home/andrei/test");
        FileEventServer server(ioService,
                               2001,
                               factory);
        ioService.run();
    }
    catch (std::exception& e)
    {
        LOG << INFO << Logger::error
            << "exception "
            << e.what()
            << " errno = " << errno
            <<"\n";
    }
    return 0;
}
