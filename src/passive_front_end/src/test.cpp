//standard library headers
#include <cstdio>
#include <memory>
#include <exception>
#include <algorithm>
//OS headers
#include <errno.h>
//my headers
#include "../../util/src/Logger.h"
#include "FileEventServer.h"

struct Params
{
    std::string path;
    int port;

    Params(std::string argPath, int argPort)
        :path(argPath), port(argPort)
    {}
};

void showHelp()
{
    std::cout <<"passive_front_end path port\n"
              << "watches the file events happening on the given "
              << "directory and send them to the specifed port\n"
              << "WHERE:\n"
              << "    path = the path to the directory observed "
              << "DEFAULT_VALUE: /home/andrei/test\n"
              << "    port = the port number on which the process "
              << "will send the file events DEFAULT_VALUE: 2001\n";
}

Params processCommandLine(int argc, char* argv[])
{
    if (argc != 3) {
        showHelp();
        std::cout << "The DEFAULT_VALUES will be used\n";
        return Params("/home/andrei/test", 2001);
    } else {
        std::string path = argv[1];
        if (!isDirectory(path)) {
            std::cout << path << " is not a valid directory\n";
            showHelp();
            std::cout << "The default directory will be used\n";
            path = "/home/andrei/test";
        }

        int port = atoi(argv[2]);
        if (port < 0 || port > 65535) {
            std::cout << argv[2] << " is not a valid port\n";
            showHelp();
            std::cout << "The default port will be used\n";
            port = 2001;
        }

        return Params(path, port);
    }

}

int main(int argc, char* argv[])
{
    
    //test results
    //63.7 MB consumed -> 590.5 MB file hierarchy
    //215.1 MB consumed -> 13.263 GB file hierarchy
    try
    {
        Params p = processCommandLine(argc, argv);
        boost::asio::io_service ioService;
        FileEventSessionFactory factory(p.path);
        FileEventServer server(ioService,
                               p.port,
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
