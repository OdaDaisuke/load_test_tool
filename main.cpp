#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <cstdio>
#include <future>
#include <cstdint>

#include <curl/curl.h>

// TODO: rps & duration
const std::string RPS_FLAG = "-rps";
const std::string URL_FLAG = "-url";
const std::string DURATION_FLAG = "-d";
const std::string REQUEST_BODY_FLAG = "-b";
const std::string REQUEST_CONTENT_TYPE = "-ct";

struct RequestRes
{
    double response_time;
    std::string body;
};

size_t call_back_func(char* ptr, size_t size, size_t nmemb, std::string* stream)
{
    int realsize = size * nmemb;
    stream->append(ptr, realsize);
    return realsize;
}

void url_get_proc (const char url[], std::string chunk, double response_time)
{
    struct RequestRes response;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
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

int retrieve_rps(int argc, char *argv[])
{
    return atoi(retrieve_flag(argc, argv, RPS_FLAG));
}

int retrieve_duration(int argc, char *argv[])
{
    return atoi(retrieve_flag(argc, argv, DURATION_FLAG));
}

char retrieve_url(int argc, char *argv[])
{
    std::string res = retrieve_flag(argc, argv, URL_FLAG);
    return *res.c_str();
}

int main (int argc, char *argv[])
{
    const int rps = retrieve_rps(argc, argv);
    if (rps <= 0) {
        std::cout << "INVALID RPS" << std::endl;
        return 1;
    }

    char url_target[] = "http://httpbin.org/get";
    std::vector<std::thread> threads;
    std::vector<RequestRes> responses;
    double response_time_sum;

    for (int i = 0;i < rps; i++) {
        struct RequestRes response;
        threads.push_back(std::thread(url_get_proc, url_target, response.body, response.response_time));
        responses.push_back(response);
    }
    for (int i = 0; i < rps; i++) {
        threads[i].join();
        response_time_sum += responses[i].response_time;
    }

    double avg_response_time = response_time_sum / double(rps);
    std::cout << "URL: " << url_target << std::endl;
    std::cout << "RPS: " << rps << std::endl;
    std::cout << "DURATION: " << rps << std::endl;
    std::cout << "AVG RESPONSE TIME: " << avg_response_time << "[sec]" << std::endl;
    std::cout << "MEDIAN RESPONSE TIME: " << avg_response_time << "[sec]" << std::endl;
    std::cout << "MAX RESPONSE TIME: " << avg_response_time << "[sec]" << std::endl;
    std::cout << "MIN RESPONSE TIME: " << avg_response_time << "[sec]" << std::endl;
    return 0;
}
