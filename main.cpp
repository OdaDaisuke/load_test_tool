#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <cstdio>
#include <future>
#include <cstdint>
#include <cmath>
#include <string.h>
#include <chrono>

#include <curl/curl.h>

const std::string HELP_FLAG = "-h";
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

bool exist_flag(int argc, char *argv[], const std::string target_flag)
{
    const char* default_res = "";
    for (int i = 0; i < argc; i++) {
        if (argv[i] == target_flag) {
            return true;
        }
    }
    return false;
}

int retrieve_rps(int argc, char *argv[])
{
    return atoi(retrieve_flag(argc, argv, RPS_FLAG));
}

int retrieve_duration(int argc, char *argv[])
{
    return atoi(retrieve_flag(argc, argv, DURATION_FLAG));
}

std::string retrieve_url(int argc, char *argv[])
{
    return retrieve_flag(argc, argv, URL_FLAG);
}

std::string retrieve_help(int argc, char* argv[])
{
    return retrieve_flag(argc, argv, HELP_FLAG);
}

void print_help()
{
    std::cout << "usage: load [option] ... [-u url | -rps rps | -d | duration] ..." << std::endl;
}

int main (int argc, char *argv[])
{
    if (exist_flag(argc, argv, HELP_FLAG)) {
        print_help();
        return 0;
    }

    const int rps = retrieve_rps(argc, argv);
    if (rps <= 0) {
        std::cout << "INVALID RPS" << std::endl;
        return 1;
    }

    std::string arg_url = retrieve_url(argc, argv);
    const char url_target[] = {*arg_url.c_str()};
    if (sizeof(url_target) <= 1) {
        std::cout << "INVALID URL" << std::endl;
        return 1;
    }

    std::vector<std::thread> threads;
    std::vector<RequestRes> responses;
    double response_time_sum;
    double max_respnse_time = 0.0;
    double min_response_time = 10000.0;

    for (int i = 0;i < rps; i++) {
        struct RequestRes response;
        threads.push_back(std::thread(url_get_proc, url_target, response.body, response.response_time));
        responses.push_back(response);
    }
    for (int i = 0; i < rps; i++) {
        threads[i].join();
        char rounded_response_time_str[10];
        response_time_sum += responses[i].response_time;
        if (responses[i].response_time > max_respnse_time) {
            max_respnse_time = responses[i].response_time;
        }
        if (responses[i].response_time < min_response_time) {
            min_response_time = responses[i].response_time;
        }
    }

    double avg_response_time = response_time_sum / rps;
    std::cout << "\032[31m FINISHED \032[m" << std::endl;
    std::cout << "URL: " << url_target << std::endl;
    std::cout << "RPS: " << rps << std::endl;
    std::cout << "DURATION: " << rps << std::endl;
    std::cout << "AVG RESPONSE TIME: " << avg_response_time << "[sec]" << std::endl;
    std::cout << "MEDIAN RESPONSE TIME: " << avg_response_time << "[sec]" << std::endl;
    std::cout << "MAX RESPONSE TIME: " << max_respnse_time << "[sec]" << std::endl;
    std::cout << "MIN RESPONSE TIME: " << min_response_time << "[sec]" << std::endl;
    return 0;
}
