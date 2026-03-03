#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#else
    #include <unistd.h>
    #include <sys/sysinfo.h>
#endif

class SystemMonitor {
public:
    // Get CPU usage percentage
    static double getCPUUsage() {
        #ifdef _WIN32
            // Windows implementation (simplified)
            return calculateCPULoad(getCPUTicks());
        #else
            // Linux implementation
            static unsigned long long _prevIdleTime = 0;
            static unsigned long long _prevTotalTime = 0;

            std::ifstream statFile("/proc/stat");
            std::string line;
            std::getline(statFile, line);

            std::istringstream iss(line);
            std::string cpu;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
            iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

            unsigned long long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
            unsigned long long idleTime = idle + iowait;

            double deltaIdle = idleTime - _prevIdleTime;
            double deltaTotal = totalTime - _prevTotalTime;

            _prevIdleTime = idleTime;
            _prevTotalTime = totalTime;

            return 100.0 - (deltaIdle / deltaTotal) * 100.0;
        #endif
    }

    // Get memory usage percentage
    static double getMemoryUsage() {
        #ifdef _WIN32
            MEMORYSTATUSEX memoryStatus;
            memoryStatus.dwLength = sizeof(memoryStatus);
            if (GlobalMemoryStatusEx(&memoryStatus)) {
                return (memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys) * 100.0 / memoryStatus.ullTotalPhys;
            }
            return 0.0;
        #else
            struct sysinfo memInfo;
            sysinfo(&memInfo);

            long totalMemory = memInfo.totalram;
            long freeMemory = memInfo.freeram;

            return (totalMemory - freeMemory) * 100.0 / totalMemory;
        #endif
    }

private:
    #ifdef _WIN32
        static double getCPUTicks() {
            // Simplified CPU ticks retrieval for Windows
            FILETIME idleTime, kernelTime, userTime;
            GetSystemTimes(&idleTime, &kernelTime, &userTime);
            ULARGE_INTEGER kernel, user;
            kernel.LowPart = kernelTime.dwLowDateTime;
            kernel.HighPart = kernelTime.dwHighDateTime;
            user.LowPart = userTime.dwLowDateTime;
            user.HighPart = userTime.dwHighDateTime;
            return (kernel.QuadPart + user.QuadPart) / 10000.
