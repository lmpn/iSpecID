#ifndef READER_H
#define READER_H
#include <string>
#include <vector>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utils.h>
#include <iostream>
#include <memory>
#include <condition_variable>
#include <fstream>
#include <boost/algorithm/string.hpp>

template<class T>
class Reader{


    private:
        using WorkItem = std::pair<std::string, size_t>;
        std::mutex feed_lock;                /**< Allow only one worker to write */
        std::condition_variable feed_cond;   /**< Wake up worker */
        std::deque<WorkItem> feed_buffer;
        std::mutex record_lock;                /**< Allow only one worker to write */
        std::condition_variable record_cond;   /**< Wake up worker */
        std::deque<T> records;
        size_t num_threads;
        std::ifstream infile;
        std::string header;

        using umap_sptr = std::shared_ptr<std::unordered_map<std::string, size_t>>;
        umap_sptr indexes;

        void read_header(){
            std::string buffer;
            if(getline(infile,buffer,'\n')){
                header = std::string(std::move(buffer));
            }
        }

        void read(){
            std::string buffer;
            size_t count = 0;
            std::vector<std::thread> pool;
            for (size_t i = 0; i < num_threads; i++)
                pool.push_back(std::thread(&Reader::worker, this));

            while (std::getline(infile,buffer)){
                std::unique_lock<std::mutex> lock{ feed_lock }; // Get lock
                feed_buffer.push_back(std::make_pair<>( std::move(buffer) , count));
                lock.unlock();
                feed_cond.notify_one();
            }

            std::unique_lock<std::mutex> lock{ feed_lock }; // Get lock
            for (size_t i = 0; i < num_threads; i++)
            {
                feed_buffer.push_back(std::make_pair<>("", 0)); // Termination signal
            }
            lock.unlock();
            feed_cond.notify_all();

            for(auto& t : pool)
                t.join();
        }

        virtual void feed_record(WorkItem&& item){
            auto data = std::move(item.first);
            T record(data,indexes);
            std::unique_lock<std::mutex> lock{ record_lock };
            records.emplace_back(std::move(record));
            lock.unlock();
            record_cond.notify_one();
        };


        void worker(){
            while (true) {
                std::unique_lock<std::mutex> lock{ feed_lock }; // Get lock
                feed_cond.wait(lock,                            // Wait
                    [this] {
                        return !(feed_buffer.empty()); });

                // Wake-up
                auto in = std::move(feed_buffer.front());
                feed_buffer.pop_front();
                // Nullptr --> Die
                if (in.first.empty()){
                    break;
                }
                lock.unlock();      // Release lock
                feed_record(std::move(in));
            }
        }

        void create_indexed_header(){
            std::unordered_map<std::string, size_t> ind;
            std::vector<std::string> results;
            boost::split(results,this->header, [](char delim){return delim == '\t';});
            size_t idx = 0;
            for(auto& r : results){
                ind.insert(std::make_pair<>(r,idx++));
            }
            if(ind.find("grade") == ind.end()){
                ind.insert({"grade", ind.size()});
            }
            this->indexes = std::make_shared<std::unordered_map<std::string, size_t>>(std::move(ind));
        };

        void close() {
            if (infile.is_open()) {
                infile.close();
            }
        }

        void fopen(std::string filename) {
            if (!infile.is_open()) {
                infile.open(filename, std::ios_base::binary);
                if (!infile.good())
                    throw std::runtime_error("Cannot open file " + std::string(filename));
            }
        }

    public:
        Reader(std::string filename, size_t threads = 1){
            fopen(filename);
            num_threads = threads;
            read_header();
            create_indexed_header();
            read();
        }
        //Copy constructor
        Reader(const Reader&) = delete;
        //Move constructor
        Reader(Reader&&) = default;
        //Copy assignment
        Reader& operator=(const Reader&) = delete;
        //Move assignment
        Reader&& operator=(Reader&& other) = delete;



        ~Reader(){
            this->close();
        }

        std::string get_header(){
            return this->header;
        }

        bool get_record(T& out){
            if(!records.empty()){
                out = std::move(records.front());
                records.pop_front();
                return true;
            }else{
                return false;
            }
        }

        std::vector<T> get_records(){
            T item;
            std::vector<T> results;
            while(get_record(item)){
                results.push_back(item);
            }
            return results;
        }

};
#endif
