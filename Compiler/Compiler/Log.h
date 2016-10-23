//
//  Log.h
//  GLSL01
//
//  Created by CERN on 16/5/16.
//  Copyright © 2016年 CERN. All rights reserved.
//
#ifndef MM_LOG
#define MM_LOG
#include <string>
#include <stdarg.h>
#include <iostream>
#include <initializer_list>
#include <time.h>
#include <sstream>
class Log
{
    class AutoLogRelease
    {
    public:
        ~AutoLogRelease()
        {
            Log* logptr = Log::Instance();
            if (logptr != nullptr)
            {
                logptr->ClearInstance();
            }
        }
    };

    public:
        static Log* Instance();
        void log(const char* s, ...);

        template<typename T>
        void logs(T t);

        template<typename T, typename... Args>
        void logs(T t, Args... args);

        void ClearInstance();
    private:
        Log();
        ~Log();

        FILE *m_logFile;
        std::stringstream m_logStream;
        static Log* _instance;
        static AutoLogRelease m_autoRelease;
};


template<typename T>
void Log::logs(T t)
{
    //time_t timeStamp;
    //timeStamp = time(nullptr);
    std::cout << t << " ";
    m_logStream << t << " ";
}

template<typename T, typename... Args>
void Log::logs(T t, Args... args)
{
    //std::cout << std::endl;
    //time_t timeStamp;
    //timeStamp = time(nullptr);
    std::cout << t << " ";
    m_logStream << t << " ";
    logs(args...);
    std::cout << std::endl;
}

#endif
