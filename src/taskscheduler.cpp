#include "taskscheduler.h"

static HRESULT get_root_folder(ITaskService *service, ITaskFolder **result)
{
    HRESULT hr = service->GetFolder(_bstr_t("\\"), result);
    if (FAILED(hr))
    {
        ShowError("[ERROR::COM] Cannot get root folder pointer: %x\n", hr);
        return hr;
    }

    return hr;
}

TaskScheduler::TaskScheduler()
{
    this->result = initialize();
}

TaskScheduler::~TaskScheduler()
{
    if (this->service)
    {
        this->service->Release();
        this->service = nullptr;
    }

    if (this->godnattFolder)
    {
        this->godnattFolder->Release();
        this->godnattFolder = nullptr;
    }

    CoUninitialize();
}

bool TaskScheduler::did_succeed() const
{
    return !FAILED(this->result);
}

HRESULT TaskScheduler::initialize()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        ShowError("[ERROR::COM] CoInitializeEx failed: %x\n", hr);
        return hr;
    }

    hr = CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        0,
        nullptr);

    if (FAILED(hr))
    {
        ShowError("[ERROR::COM] CoInitializeSecurity failed: %x\n", hr);
        return hr;
    }

    this->service = nullptr;
    hr = CoCreateInstance(CLSID_TaskScheduler,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void **)&this->service);

    if (FAILED(hr))
    {
        ShowError("[ERROR::COM] Failed to CoCreate an instance of the TaskService class: %x\n", hr);
        return hr;
    }

    hr = service->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        ShowError("[ERROR::COM] ITaskService::Connect failed: %x\n", hr);
        return hr;
    }

    this->godnattFolder = nullptr;
#ifdef DEBUG
    hr = service->GetFolder(_bstr_t("\\godnatt_debug"), &this->godnattFolder);
#else
    hr = service->GetFolder(_bstr_t("\\godnatt"), &this->godnattFolder);
#endif
    
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            ITaskFolder *rootFolder = nullptr;
            hr = get_root_folder(service, &rootFolder);
            if (FAILED(hr))
            {
                return hr;
            }

#ifdef DEBUG
            hr = rootFolder->CreateFolder(_bstr_t("godnatt_debug"), _variant_t(), &this->godnattFolder);
#else
            hr = rootFolder->CreateFolder(_bstr_t("godnatt"), _variant_t(), &this->godnattFolder);
#endif
            rootFolder->Release();
            if (FAILED(hr))
            {
                ShowError("[ERROR::COM] Cannot create godnatt folder: %x\n", hr);
                return hr;
            }
        }
        else
        {
            ShowError("[ERROR::COM] Cannot get Godnatt folder pointer: %x\n", hr);
            return hr;
        }
    }

    return S_OK;
}

// Map from 0-6 to 1,2,4...64
// Week starts with sunday which means
// 6 maps to 1
// 0 maps to 2
// 1 maps to 4
// 2 maps to 8
// 3 maps to 16
// 4 maps to 32
// 5 maps to 64
// Can this be done in a concise way with shifts instead?
// If not then this is probably the best
short weekdayToFlag[7] = { 2, 4, 8, 16, 32, 64, 1 };
const char *weekdayToString[7] = { "Monday ", "Tuesday ", "Wednesday ", "Thursday ", "Friday ", "Saturday ", "Sunday " };

void TaskScheduler::add_weekly_trigger(const char *start, short day, const char *arguments)
{
    // HAS to have been initialized properly first
    assert(this->did_succeed());
    assert(day >= 0 && day <= 6);

    const char *executablePath = paths.executable;

    char taskName[256] = {};
    if (strcpy_s(taskName, 256, weekdayToString[day]) != 0)
    {
        ShowError("[ERROR::COM] strcpy_s failed when copying day into buffer");
        return;
    }
        
    if (strcat_s(taskName, 256, arguments) != 0)
    {
        ShowError("[ERROR::COM] strcat_s failed when copying arguments into buffer");
        return;
    }

    day = weekdayToFlag[day];

    // Delete if already exists
    this->godnattFolder->DeleteTask(_bstr_t(taskName), 0);

    ITaskDefinition *task = nullptr;
    this->result = this->service->NewTask(0,  &task);

    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Failed to create a task definition: %x\n", this->result);
        return;
    }

    ITaskSettings *settings = nullptr;
    this->result = task->get_Settings(&settings);
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot get settings pointer: %x\n", this->result);
        task->Release();
        return;
    }

    this->result = settings->put_DisallowStartIfOnBatteries(_variant_t(false));
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put power settings: %x\n", this->result);
        task->Release();
        settings->Release();
        return;
    }

    this->result = settings->put_StopIfGoingOnBatteries(_variant_t(false));
    settings->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put power settings: %x\n", this->result);
        task->Release();
        return;
    }

    IRegistrationInfo *regInfo = nullptr;
    this->result = task->get_RegistrationInfo(&regInfo);
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot get identification pointer: %x\n", this->result);
        task->Release();
        return;
    }

    this->result = regInfo->put_Author(_bstr_t("godnatt"));
    regInfo->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put identification info: %x\n", this->result);
        task->Release();
        return;
    }

    ITriggerCollection *triggerCollection = nullptr;
    this->result = task->get_Triggers(&triggerCollection);
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot get trigger collection: %x\n", this->result);
        task->Release();
        return;
    }

    ITrigger *trigger = nullptr;
    this->result = triggerCollection->Create(TASK_TRIGGER_WEEKLY, &trigger);
    triggerCollection->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot create the trigger: %x\n", this->result);
        task->Release();
        return;
    }

    IWeeklyTrigger *weeklyTrigger = nullptr;
    this->result = trigger->QueryInterface(IID_IWeeklyTrigger, (void **)&weeklyTrigger);
    trigger->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] QueryInterface call for IWeeklyTrigger failed: %x\n", this->result);
        task->Release();
        return;
    }

    this->result = weeklyTrigger->put_Id(_bstr_t("Trigger"));
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put trigger ID: %x\n", this->result);
    }

    this->result = weeklyTrigger->put_StartBoundary(_bstr_t(start));
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put the start boundary: %x\n", this->result);
    }

    this->result = weeklyTrigger->put_WeeksInterval((short)1);
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put weeks interval: %x\n", this->result);
        task->Release();
        weeklyTrigger->Release();
        return;
    }

    // This takes a flag so we multiply by 2 to get 
    this->result = weeklyTrigger->put_DaysOfWeek(day);
    weeklyTrigger->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put days of week: %x\n", this->result);
        task->Release();
        return;
    }

    IActionCollection *actionCollection = nullptr;
    this->result = task->get_Actions(&actionCollection);
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot get task collection pointer: %x\n", this->result);
        task->Release();
        return;
    }

    IAction *action = nullptr;
    this->result = actionCollection->Create(TASK_ACTION_EXEC, &action);
    actionCollection->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot create the action: %x\n", this->result);
        task->Release();
        return;
    }

    IExecAction *execAction = nullptr;
    this->result = action->QueryInterface(IID_IExecAction, (void **)&execAction);
    action->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] QueryInterface call failed on IExecAction: %x\n", this->result);
        task->Release();
        return;
    }

    this->result = execAction->put_Path(_bstr_t(executablePath));
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put executable path: %x\n", this->result);
        task->Release();
        execAction->Release();
        return;
    }

    this->result = execAction->put_Arguments(_bstr_t(arguments));
    execAction->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Cannot put executable arguments: %x\n", this->result);
        task->Release();
        return;
    }

    IRegisteredTask *registeredTask = nullptr;
    this->result = this->godnattFolder->RegisterTaskDefinition(
        _bstr_t(taskName),
        task,
        TASK_CREATE_OR_UPDATE,
        _variant_t(),
        _variant_t(),
        TASK_LOGON_INTERACTIVE_TOKEN,
        _variant_t(""),
        &registeredTask);
    registeredTask->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Error saving the task: %x\n", this->result);
        task->Release();
        return;
    }

    task->Release();
}

void TaskScheduler::clear()
{
    IRegisteredTaskCollection *tasks = nullptr;
    this->result = this->godnattFolder->GetTasks(0, &tasks);
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Error getting tasks to delete: %x\n", this->result);
        return;
    }

    long numTasks = 0;
    this->result = tasks->get_Count(&numTasks);
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Error getting tasks count: %x\n", this->result);
        tasks->Release();
        return;
    }

    for (long i = 1; i <= numTasks; i++)
    {
        IRegisteredTask *task = nullptr;
        this->result = tasks->get_Item(_variant_t(i), &task);
        if (FAILED(this->result))
        {
            ShowError("[ERROR::COM] Error getting task %ld: %x\n", i, this->result);
            tasks->Release();
            return;
        }

        BSTR name;
        this->result = task->get_Name(&name);
        if (FAILED(this->result))
        {
            ShowError("[ERROR::COM] Error getting task %ld name: %x\n", i, this->result);
            tasks->Release();
            task->Release();
            return;
        }

        task->Release();

        this->godnattFolder->DeleteTask(name, 0);
        if (FAILED(this->result))
        {
            ShowError("[ERROR::COM] Error deleting task %ld: %x\n", i, this->result);
            tasks->Release();
            return;
        }
    }

    tasks->Release();

    ITaskFolder *rootFolder = nullptr;
    this->result = get_root_folder(this->service, &rootFolder);
    if (FAILED(this->result))
    {
        return;
    }

#ifdef DEBUG
    this->result = rootFolder->DeleteFolder(_bstr_t("godnatt_debug"), 0);
#else
    this->result = rootFolder->DeleteFolder(_bstr_t("godnatt"), 0);
#endif
    rootFolder->Release();
    if (FAILED(this->result))
    {
        ShowError("[ERROR::COM] Error deleting folder: %x\n", this->result);
        return;
    }
}