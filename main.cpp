#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <cstdio>
#include <cstdint>

#include <curl/curl.h>

int response_time;
const std::string SIZE_FLAG = "-s";
const std::string URL_FLAG = "-url";
const std::string CALL_DURATION_FLAG = "-d";
const std::string REQUEST_BODY_FLAG = "-b";

size_t call_back_func(char* ptr, size_t size, size_t nmemb, std::string* stream)
{
    int realsize = size * nmemb;
    stream->append(ptr, realsize);
    return realsize;
}

std::string url_get_proc (const char url[])
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    std::string chunk;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_back_func);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (std::string*)&chunk);
        curl_easy_setopt(curl, CURLOPT_PROXY, "");
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &response_time);
        curl_easy_cleanup(curl);
    }
    if (res != CURLE_OK) {
        std::cout << "curl error" << std::endl;
        exit(1);
    }
    return chunk;
}

const char* retrieve_flag(int argc, char *argv[], const std::string target_flag)
{
    const char* default_res = "";
    for (int i = 0; i < argc; i++) {
        if (argv[i] == target_flag) {
            if (i + 1 < argc) {
                return argv[i+1];
            }
            break;
        }
    }
    return default_res;
}

int retrieve_times(int argc, char *argv[])
{
    return atoi(retrieve_flag(argc, argv, SIZE_FLAG));
}

char retrieve_url(int argc, char *argv[])
{
    std::string res = retrieve_flag(argc, argv, URL_FLAG);
    return *res.c_str();
}

int main (int argc, char *argv[])
{
    const int times = retrieve_times(argc, argv);
    if (times <= 0) {
        std::cout << "INVALID TIMES" << std::endl;
        return 1;
    }

    std::cout << "REQUEST " << times << "TIMES" << std::endl;
    std::vector<std::thread> threads;

    char url_target[] = "http://httpbin.org/get";
    for (int i = 0;i < times; i++) {
        threads.push_back(std::thread(url_get_proc, url_target));
        // TODO: calc statistic
    }
    for (int i = 0; i < times; i++) {
        threads[i].join();
    }

    // TODO: generate report
    return 0;
}
