#pragma once

#include <comdef.h>
#include <taskschd.h>

class TaskScheduler
{
    HRESULT result;
    ITaskService *service;
    ITaskFolder *godnattFolder;

    HRESULT initialize();

public:
    TaskScheduler();
    ~TaskScheduler();

    bool did_succeed() const;
    void add_weekly_trigger(const char *start, short day, const char *arguments);
    void clear();
};