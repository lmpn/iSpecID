#ifndef MINER_HPP
#define MINER_HPP
#include <string>
#include <tuple>
#include <curl/curl.h>
#include <utils.hpp>
class miner{
    public:
    miner():curl_handle(curl_easy_init()){
        curl_global_init(CURL_GLOBAL_ALL);
    }
    miner& operator=( miner& other ) = delete;
    miner(miner& other) = delete;
    miner& operator=(miner&& other);
    miner(miner&& o) noexcept; 
    ~miner();
    std::string getPage(const char* url);
    private:
        CURL *curl_handle;
};
#endif