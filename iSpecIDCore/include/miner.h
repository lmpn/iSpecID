#ifndef MINER_H
#define MINER_H
#include <string>
#include <tuple>
#include <curl/curl.h>
#include "utils.h"



class Miner{
    public:
    Miner():curl_handle(curl_easy_init()){
        curl_global_init(CURL_GLOBAL_ALL);
    }
    Miner& operator=( Miner& other ) = delete;
    Miner(Miner& other) = delete;
    Miner& operator=(Miner&& other);
    Miner(Miner&& o) noexcept;
    ~Miner();
    std::string getPage(const char* url);
    private:
        CURL *curl_handle;
};
#endif
