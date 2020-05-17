#ifndef NETWORK_H
#define NETWORK_H
#include <string>
namespace ispecid{ namespace network{
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
    std::string getPage(const char* url);
}}
#endif
