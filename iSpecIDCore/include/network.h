#ifndef NETWORK_H
#define NETWORK_H
#include <curl/curl.h>
#include <string>
namespace ispecid{ namespace network{
    void prepareNetwork();
    std::string getPage(const char* url);
}}
#endif
