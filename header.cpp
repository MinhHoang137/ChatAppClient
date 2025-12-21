#include "header.h"
#include <ctime>
#include <direct.h> // _mkdir
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <mutex>

static std::ofstream g_clientLogFile;
static std::mutex g_clientLogMutex;

void initClientLog()
{
    _mkdir("logs");

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << "logs/client_log_" << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".txt";
    std::string filename = oss.str();

    g_clientLogFile.open(filename, std::ios::out | std::ios::app);
    if (g_clientLogFile.is_open()) {
        clientLogMessage("Client started.");
    } else {
        std::cerr << "Failed to create client log file: " << filename << std::endl;
    }
}

void clientLogMessage(const std::string &message)
{
    std::lock_guard<std::mutex> lock(g_clientLogMutex);
    if (g_clientLogFile.is_open()) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        g_clientLogFile << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] " << message
                        << "\n";
        g_clientLogFile.flush();
    }
}
