#include "miner.h"
#include <cstring>


Miner::~Miner(){
    if(this->curl_handle != nullptr)
        curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
}
Miner& Miner::operator=(Miner&& other)
{
    std::swap(this->curl_handle,other.curl_handle);
    return *this;
}

Miner::Miner(Miner&& o) noexcept {
    std::swap(this->curl_handle,o.curl_handle);
}

static size_t
write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  std::string *mem = (std::string *)userp;
  mem->append((char*)contents,realsize);
  return realsize;
}

std::string Miner::getPage(const char* url)
{
    std::string data;
    if(this->curl_handle != nullptr){
        CURLcode res;
        curl_easy_setopt(this->curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(this->curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(this->curl_handle, CURLOPT_WRITEDATA, (void *)&data);
        curl_easy_setopt(this->curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        res = curl_easy_perform(curl_handle);
        if(res != CURLE_OK) {
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));
        }
    }
    return data;
}
