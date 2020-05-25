#ifndef NETWORK_H
#define NETWORK_H
#include <string>
namespace ispecid{ namespace network{
    void prepareNetwork();
    std::string getPage(const char* url);
}}
#endif
