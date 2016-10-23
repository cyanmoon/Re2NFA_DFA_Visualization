//
//  Log.cpp
//  GLSL01
//
//  Created by CERN on 16/5/16.
//  Copyright © 2016年 CERN. All rights reserved.
//

#include "Log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <iostream>
#include <time.h>
using namespace std;

Log* Log::_instance = nullptr;
Log::AutoLogRelease Log::m_autoRelease = Log::AutoLogRelease();
void Log::log(const char* s, ...)
{
    //return;
    va_list arg_pointer;
    va_start(arg_pointer, s);
    const char * tmp = s;
    int i = 0;
    while (nullptr != tmp)
    {
        cout << tmp << " ";
        if (m_logFile != nullptr)
        {
            fprintf(m_logFile, tmp);
            fprintf(m_logFile, " ");
        }
        tmp = va_arg(arg_pointer, const char*);
        ++i;
    }

    if (m_logFile != nullptr)
    {
        fprintf(m_logFile, "\n\0");
    }

    cout << endl;
    va_end(arg_pointer);
}

Log::Log()
{
    m_logFile = fopen("log.txt", "w");
}

Log::~Log()
{
    if (m_logFile != nullptr)
    {
        fprintf(m_logFile, m_logStream.str().c_str());
        fclose(m_logFile);
    }
}

Log* Log::Instance()
{
    if (_instance == nullptr)
    {
        _instance = new Log();
        _instance->logs("create Log Instance\n");
    }

    return _instance;
}

void Log::ClearInstance()
{
    if (_instance != nullptr)
    {
        Log::Instance()->logs("Clear Log Instane\n");
        delete _instance;
    }
    _instance = nullptr;
}
