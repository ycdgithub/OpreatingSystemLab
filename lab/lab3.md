# 实验3   OpenHarmony任务管理

（建议2学时）

## 一、实验目的

1.掌握OpenHarmony的任务创建；

2.掌握OpenHarmony的任务入口函数构建；

3.掌握OpenHarmony的任务运行。

## 二、实验环境

1.安装OpenHarmony开发环境的PC一台。

## 三、实验内容

1.学习OpenHarmony的任务创建；

2.学习OpenHarmony的任务入口函数构建；

3.学习OpenHarmony的任务运行。

## 四、实验原理

### 1. OpenHarmony任务

每一个任务都含有一个任务控制块（TCB）。
TCB包含了任务上下文栈指针（stack pointer）、任务状态、任务优先级、任务ID、任务名、任务栈大小等信息。

TCB可以反映出每个任务运行情况。

任务是抢占式调度机制，同时支持时间片轮转调度方式。LiteOS-m内核的任务一共有32个优先级(0-31)，最高优先级为0，最低优先级为31。

任务入口函数如代码示例3.1所示。

代码示例3.1  任务入口函数HelloWorldEntry()

```c
VOID HelloWorldEntry(VOID)
{
    while (1) {
        printf("Hello World!\n");
        LOS_TaskDelay(2000); /* 2 Seconds */
    }
}
```

### 2. OpenHarmony任务定义创建

首先定义TSK_INIT_PARAM_S类型的结构体stTask来完成任务的初始化，设置任务入口函数、堆栈大小、任务名称以及优先级。

任务初始化时需要通过结构体TSK_INIT_PARAM_S提供一些任务所需的信息。

其中，成员pfnTaskEntry是任务入口函数，这样在任务第一次启动进入运行态时，将会执行任务入口函数。

因此本实验中可定义一个函数HelloWorldEntry执行输出"Hello World！"并把该函数作为任务的入口函数。

函数LOS_TaskCreate()创建任务，其原型如下：

```c
UINT32 LOS_TaskCreate(UINT32 *taskID, TSK_INIT_PARAM_S *taskInitParam);
```

参数*taskID带回任务的ID，参数*taskInitParam传入任务的初始化信息，函数返回值代表创建是否成功。

创建过程如代码示例3.2。

代码示例3.2  任务创建

```c
VOID TaskHelloWorld(VOID)
{
    UINT32 uwRet;
    UINT32 taskID1;
    TSK_INIT_PARAM_S stTask = {0};

    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)HelloWorldEntry;
    stTask.uwStackSize = 0x0800;
    stTask.pcName = "HelloWorld";
    stTask.usTaskPrio = 6; /* Os task priority is 6 */
    uwRet = LOS_TaskCreate(&taskID1, &stTask);
    if (uwRet != LOS_OK) {
        printf("Task HelloWorld creatation failed\n");
    }
}
```

### 3. OpenHarmony任务运行

任务创建完成后，需要将任务TaskHelloWorld放到用户代码的main()函数中，

然后调用LOS_Start()启动任务的调度，

但是在此之前需要先调用 LOS_KernelInit()初始化用户代码的内核空间。

main()函数如代码示例3.3。

代码示例3.3  main()函数中运行任务

```c
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN Init */
    UINT32 ret;
    ret = LOS_KernelInit();
    if (ret == LOS_OK) {
        TaskHelloWorld();
        LOS_Start();
    }
    while (1) {
    }
}
```

# 五、实验练习与思考题

### 1. API编程

根据实验原理，创建一个任务，完成完整的HelloWorld程序，输出"Hello World!"。

### 2. 源码分析


分析OpenHarmony的LiteOS-m内核的TCB数据结构、任务状态、全局变量及其导致状态转换的函数的源码，源码主要涉及los_task.h和los_task.c。

#### 2.1 数据结构

分析任务初始化参数结构体TSK_INIT_PARAM_S;

分析任务控制块LosTaskCB结构体的作用及其每个成员的意义。

#### 2.2 任务状态

分析LiteOS-m每个任务状态的意义，给出每个状态转换的条件及其转换后的状态。

LiteOS-m的任务状态定义如代码引用3.3所示。

代码引用3.3  任务状态（los_task.h）

```c
/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task control block is unused.
 */
#define OS_TASK_STATUS_UNUSED                       0x0001

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is suspended.
 */
#define OS_TASK_STATUS_SUSPEND                      0x0002

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is ready.
 */
#define OS_TASK_STATUS_READY                        0x0004

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is blocked.
 */
#define OS_TASK_STATUS_PEND                         0x0008

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is running.
 */
#define OS_TASK_STATUS_RUNNING                      0x0010

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is delayed.
 */
#define OS_TASK_STATUS_DELAY                        0x0020

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The time for waiting for an event to occur expires.
 */
#define OS_TASK_STATUS_TIMEOUT                      0x0040

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is blocked on a time.
 */
#define OS_TASK_STATUS_PEND_TIME                    0x0080

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task exits and waits for the parent thread to reclaim the resource.
 */
#define OS_TASK_STATUS_EXIT                         0x0100
```

#### 2.3  全局变量

LiteOS-m的任务全局变量定义如代码引用3.4所示。说明全局变量的作用及引用这些变量的主要相关函数。

代码引用3.4  任务相关全局变量（los_task.c）

```c
LITE_OS_SEC_BSS  LosTaskCB                           *g_taskCBArray = NULL;
LITE_OS_SEC_BSS  LosTask                             g_losTask;
LITE_OS_SEC_BSS  UINT16                              g_losTaskLock;
LITE_OS_SEC_BSS  UINT32                              g_taskMaxNum;
LITE_OS_SEC_BSS  UINT32                              g_idleTaskID;
LITE_OS_SEC_BSS  UINT32                              g_swtmrTaskID;
LITE_OS_SEC_DATA_INIT LOS_DL_LIST                    g_losFreeTask;
LITE_OS_SEC_DATA_INIT LOS_DL_LIST                    g_taskRecyleList;
LITE_OS_SEC_BSS  BOOL                                 g_taskScheduled = FALSE;
```

#### 2.4 函数

分析以下函数功能，对于加*号的函数，详细分析函数具体功能、主要过程及其对任务状态的改变。注释标有①、②、③等编号的行。

| 序号 | 函数名 | 功能 |
| ----------- | ----------- |----------- |
|*1|OsTaskInit |  |
|2 | OsIdleTaskCreate | |
|*3| OsNewTaskInit||
|*4| LOS_TaskCreateOnly||
|5| LOS_TaskCreate||
|*6| LOS_TaskResume ||
|*7| LOS_TaskSuspend ||
|8| LOS_TaskDelay ||
|*9| LOS_TaskDelete ||
|10| LOS_TaskPriSet ||
|*11| LOS_TaskYield ||
|12| LOS_TaskLock ||
|13| LOS_TaskUnlock ||

##### 2.4.1 任务初始化函数OsTaskInit()

函数OsTaskInit()的实现如代码引用3.5所示。
代码引用3.5  函数OsTaskInit()的实现（los_task.c）

```c
/*****************************************************************************
 Function    : OsTaskInit
 Description : Task init function.
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsTaskInit(VOID)
{
    UINT32 size;
    UINT32 index;

    size = (g_taskMaxNum + 1) * sizeof(LosTaskCB);
    g_taskCBArray = (LosTaskCB *)LOS_MemAlloc(m_aucSysMem0, size);
    if (g_taskCBArray == NULL) {
        return LOS_ERRNO_TSK_NO_MEMORY;
    }

// Ignore the return code when matching CSEC rule 6.6(1).

①　(VOID)memset_s(g_taskCBArray, size, 0, size);
②　LOS_ListInit(&g_losFreeTask); 
③　LOS_ListInit(&g_taskRecyleList); 

④　for (index = 0; index <= LOSCFG_BASE_CORE_TSK_LIMIT; index++) {
        g_taskCBArray[index].taskStatus = OS_TASK_STATUS_UNUSED;
        g_taskCBArray[index].taskID = index;
        LOS_ListTailInsert(&g_losFreeTask, &g_taskCBArray[index].pendList);
    }

// Ignore the return code when matching CSEC rule 6.6(4).

⑤　(VOID)memset_s((VOID *)(&g_losTask), sizeof(g_losTask), 0, sizeof(g_losTask));

⑥　g_losTask.runTask = &g_taskCBArray[g_taskMaxNum];
    g_losTask.runTask->taskID = index;
    g_losTask.runTask->taskStatus = (OS_TASK_STATUS_UNUSED | OS_TASK_STATUS_RUNNING);
    g_losTask.runTask->priority = OS_TASK_PRIORITY_LOWEST + 1;


⑦　g_idleTaskID = OS_INVALID;
    return OsSchedInit();
}
```

##### 2.4.2 函数OsNewTaskInit()

函数OsNewTaskInit()的实现如代码引用3.6所示。
代码引用3.6  函数OsNewTaskInit()的实现（los_task.c）

```c
LITE_OS_SEC_TEXT_INIT UINT32 OsNewTaskInit(LosTaskCB *taskCB, TSK_INIT_PARAM_S *taskInitParam, VOID *topOfStack)
{
①　taskCB->arg             = taskInitParam->uwArg;   
②　taskCB->topOfStack      = (UINT32)(UINTPTR)topOfStack; 
③　taskCB->stackSize       = taskInitParam->uwStackSize; 
④　taskCB->taskSem         = NULL;      
⑤　taskCB->taskMux         = NULL;      
⑥　taskCB->taskStatus      = OS_TASK_STATUS_SUSPEND;  
⑦　taskCB->priority        = taskInitParam->usTaskPrio; 
⑧　taskCB->timeSlice       = 0;       
⑨　taskCB->waitTimes       = 0;       
⑩　taskCB->taskEntry       = taskInitParam->pfnTaskEntry; 
⑪　taskCB->event.uwEventID = OS_NULL_INT;     
⑫　taskCB->eventMask       = 0;       
⑬　taskCB->taskName        = taskInitParam->pcName;  
⑭　taskCB->msg             = NULL;      
⑮　taskCB->stackPointer    = HalTskStackInit(taskCB->taskID, taskInitParam->uwStackSize, topOfStack);     

⑯　SET_SORTLIST_VALUE(&taskCB->sortList, OS_SORT_LINK_INVALID_TIME);
⑰　LOS_EventInit(&(taskCB->event));      

    if (taskInitParam->uwResved & LOS_TASK_ATTR_JOINABLE) {
        taskCB->taskStatus |= OS_TASK_FLAG_JOINABLE;
⑱　    LOS_ListInit(&taskCB->joinList);     
    }
    return LOS_OK;
}
```

##### 2.4.3 函数LOS_TaskCreate Only()

函数LOS_TaskCreate Only()的实现如代码引用3.7所示。
代码引用3.7  函数LOS_TaskCreate Only()的实现（los_task.c）

```c
/*****************************************************************************
 Function    : LOS_TaskCreateOnly
 Description : Create a task and suspend
 Input       : taskInitParam --- Task init parameters
 Output      : taskID        --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreateOnly(UINT32 *taskID, TSK_INIT_PARAM_S *taskInitParam)
{
    UINT32 intSave;
    VOID  *topOfStack = NULL;
    LosTaskCB *taskCB = NULL;
    UINT32 retVal;

    if (taskID == NULL) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }


①　retVal = OsTaskInitParamCheck(taskInitParam);
    if (retVal != LOS_OK) {
        return retVal;
    }


②　OsRecyleFinishedTask();

    intSave = LOS_IntLock();
    if (LOS_ListEmpty(&g_losFreeTask)) {
        retVal = LOS_ERRNO_TSK_TCB_UNAVAILABLE;
        OS_GOTO_ERREND();
    }


③　taskCB = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&g_losFreeTask));

④　LOS_ListDelete(LOS_DL_LIST_FIRST(&g_losFreeTask));

    LOS_IntRestore(intSave);

#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 1)
    UINTPTR stackPtr = (UINTPTR)LOS_MemAllocAlign(OS_TASK_STACK_ADDR, taskInitParam->uwStackSize +
        OS_TASK_STACK_PROTECT_SIZE, OS_TASK_STACK_PROTECT_SIZE);
    topOfStack = (VOID *)(stackPtr + OS_TASK_STACK_PROTECT_SIZE);
#else
    topOfStack = (VOID *)LOS_MemAllocAlign(OS_TASK_STACK_ADDR, taskInitParam->uwStackSize,
        LOSCFG_STACK_POINT_ALIGN_SIZE);
#endif
    if (topOfStack == NULL) {
        intSave = LOS_IntLock();
        LOS_ListAdd(&g_losFreeTask, &taskCB->pendList);
        LOS_IntRestore(intSave);
        return LOS_ERRNO_TSK_NO_MEMORY;
    }


⑤　retVal = OsNewTaskInit(taskCB, taskInitParam, topOfStack);
    if (retVal != LOS_OK) {
        return retVal;
    }
#if (LOSCFG_BASE_CORE_CPUP == 1)
    intSave = LOS_IntLock();
    g_cpup[taskCB->taskID].cpupID = taskCB->taskID;
    g_cpup[taskCB->taskID].status = taskCB->taskStatus;
    LOS_IntRestore(intSave);
#endif
    *taskID = taskCB->taskID;
    OsHookCall(LOS_HOOK_TYPE_TASK_CREATE, taskCB);
    return retVal;

LOS_ERREND:
    LOS_IntRestore(intSave);
    return retVal;
}
```

##### 2.4.4 LOS_TaskResume ()

函数LOS_TaskResume ()的实现如代码引用3.8所示。

代码引用3.8  函数LOS_TaskResume ()的实现（los_task.c）

```c
/*****************************************************************************
 Function    : LOS_TaskResume
 Description : Resume suspend task
 Input       : taskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskResume(UINT32 taskID)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 tempStatus;
    UINT32 retErr = OS_ERROR;

    if (taskID > LOSCFG_BASE_CORE_TSK_LIMIT) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    intSave = LOS_IntLock();
    tempStatus = taskCB->taskStatus;

    if (tempStatus & OS_TASK_STATUS_UNUSED) {
        retErr = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    } else if (!(tempStatus & OS_TASK_STATUS_SUSPEND)) {
        retErr = LOS_ERRNO_TSK_NOT_SUSPENDED;
        OS_GOTO_ERREND();
    }

#if (LOSCFG_KERNEL_PM == 1)
    if (tempStatus & OS_TASK_FALG_FREEZE) {
        OsPmUnfreezeTaskUnsafe(taskID);
    }
#endif

    
①　taskCB->taskStatus &= (~OS_TASK_STATUS_SUSPEND);
if (!(taskCB->taskStatus & OS_CHECK_TASK_BLOCK)) {
②　   OsSchedTaskEnQueue(taskCB); 
        if (g_taskScheduled) {
            LOS_IntRestore(intSave);
③　        LOS_Schedule(); 
            return LOS_OK;
        }
    }

    LOS_IntRestore(intSave);
    return LOS_OK;

LOS_ERREND:
    LOS_IntRestore(intSave);
    return retErr;
}
```

##### 2.4.5 函数LOS_TaskSuspend ()

函数LOS_TaskSuspend ()的实现如代码引用3.9所示。

代码引用3.9  函数LOS_TaskSuspend ()的实现（los_task.c）

```c
/*****************************************************************************
 Function    : LOS_TaskSuspend
 Description : Suspend task
 Input       : taskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskSuspend(UINT32 taskID)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 tempStatus;
    UINT32 retErr;

    retErr = OsCheckTaskIDValid(taskID);
    if (retErr != LOS_OK) {
        return retErr;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    intSave = LOS_IntLock();
    tempStatus = taskCB->taskStatus;
    if (tempStatus & OS_TASK_STATUS_UNUSED) {
        retErr = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    }

    if (tempStatus & OS_TASK_STATUS_SUSPEND) {
        retErr = LOS_ERRNO_TSK_ALREADY_SUSPENDED;
        OS_GOTO_ERREND();
    }

    if ((tempStatus & OS_TASK_STATUS_RUNNING) && (g_losTaskLock != 0)) {
        retErr = LOS_ERRNO_TSK_SUSPEND_LOCKED;
        OS_GOTO_ERREND();
    }

    if (tempStatus & OS_TASK_STATUS_READY) {
①　    OsSchedTaskDeQueue(taskCB); 
    }

#if (LOSCFG_KERNEL_PM == 1)
    if ((tempStatus & (OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY)) && OsIsPmMode()) {
        OsPmFreezeTaskUnsafe(taskID);
    }
#endif

②　taskCB->taskStatus |= OS_TASK_STATUS_SUSPEND; 
    OsHookCall(LOS_HOOK_TYPE_MOVEDTASKTOSUSPENDEDLIST, taskCB);
    if (taskID == g_losTask.runTask->taskID) {
        LOS_IntRestore(intSave);
③　    LOS_Schedule(); 
        return LOS_OK;
    }

    LOS_IntRestore(intSave);
    return LOS_OK;

LOS_ERREND:
    LOS_IntRestore(intSave);
    return retErr;
}
```

##### 2.4.6 函数LOS_TaskDelete ()

函数LOS_TaskDelete ()的实现如代码引用3.10所示。

代码引用3.10  函数LOS_TaskDelete ()的实现（los_task.c）

```c
/*****************************************************************************
 Function    : LOS_TaskDelete
 Description : Delete a task
 Input       : taskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskDelete(UINT32 taskID)
{
    UINT32 intSave;
    UINTPTR stackPtr = 0;
    LosTaskCB *taskCB = NULL;

    UINT32 ret = OsCheckTaskIDValid(taskID);
    if (ret != LOS_OK) {
        return ret;
    }

    taskCB = OS_TCB_FROM_TID(taskID);
    intSave = LOS_IntLock();

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_TSK_ALREADY_EXIT;
    }

    /* If the task is running and scheduler is locked then you can not delete it */
    if (((taskCB->taskStatus) & OS_TASK_STATUS_RUNNING) && (g_losTaskLock != 0)) {
        PRINT_INFO("In case of task lock, task deletion is not recommended\n");
        g_losTaskLock = 0;
    }

    OsHookCall(LOS_HOOK_TYPE_TASK_DELETE, taskCB);
    OsSchedTaskExit(taskCB);
    OsTaskJoinPostUnsafe(taskCB);

    LOS_EventDestroy(&(taskCB->event));
    taskCB->event.uwEventID = OS_NULL_INT;
    taskCB->eventMask = 0;
#if (LOSCFG_BASE_CORE_CPUP == 1)
    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)memset_s((VOID *)&g_cpup[taskCB->taskID], sizeof(OsCpupCB), 0, sizeof(OsCpupCB));
#endif
①　if (taskCB->taskStatus & OS_TASK_STATUS_RUNNING) { 
②　    if (!(taskCB->taskStatus & OS_TASK_STATUS_EXIT)) { 
            taskCB->taskStatus = OS_TASK_STATUS_UNUSED;
③　        OsRunningTaskDelete(taskID, taskCB);
        }
        LOS_IntRestore(intSave);
④　    LOS_Schedule(); 
        return LOS_OK;
    }

⑤　taskCB->joinRetval = LOS_CurTaskIDGet(); 
⑥　OsRecycleTaskResources(taskCB, &stackPtr); 
    LOS_IntRestore(intSave);
⑦　(VOID)LOS_MemFree(OS_TASK_STACK_ADDR, (VOID *)stackPtr); /** 释放任务栈 */
    return LOS_OK;
}
```

##### 2.4.7 函数LOS_TaskYield ()

函数LOS_TaskYield ()的实现如代码引用3.11所示。

代码引用3.11  函数LOS_TaskYield ()的实现（los_task.c）

```c
/*****************************************************************************
 Function    : LOS_TaskYield
 Description : Adjust the procedure order of specified task
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskYield(VOID)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
①　  OsSchedYield();

    LOS_IntRestore(intSave);
②　LOS_Schedule(); 
    return LOS_OK;
}
```
