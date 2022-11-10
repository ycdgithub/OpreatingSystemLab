# 实验9   OpenHarmony进程间通信

（建议2学时）

## 一、实验目的

1. 理解操作系统的IPC机制；
2. 掌握OpenHarmony的消息队列和事件机制；
3. 实现两个进程的同步。

## 二、实验环境

1. 安装OpenHarmony开发环境的PC一台。

## 三、实验内容

1. 分析OpenHarmony的消息队列机制；
2. 分析OpenHarmony的消息事件机制；
3. 实现管道。

## 四、实验原理

### 1. 进程间通信

进程间通信（IPC，InterProcess Communication）是指在不同进程之间传播或交换信息。 IPC的方式通常有管道、消息队列、事件、共享存储和套接字（Socket）等。

### 2. OpenHarmony进程间通信

LiteOS-M内核为任务间通信提供了多种机制，包括队列和事件、信号量和互斥锁。
队列又称消息队列，消息队列是一种常用于任务间通信的数据结构，可以在任务间传递消息内容或消息的地址。内核用队列控制块来管理消息队列，同时又使用双向环形链表来管理控制块。
事件用于任务间的同步，一个或多个任务可以通过写一个或多个不同的事件来触发内核调度，让另一个等待读取事件的任务进入运行状态，但事件通信是无数据传输，事件控制块由任务申请，内核负责维护。

### 3. 管道

管道是一个小的内核缓冲区，作为一对文件描述符提供给进程，一个用于读取，一个用于写入。从管道的一端写的数据可以从管道的另一端读取。管道为进程提供了一种通信方式。

## 五、实验练习与思考题

### 1.API编程

创建了两个任务，一个是写消息任务，另一个是读消息任务，两个任务独立运行，写消息任务是通过检测按键的按下情况来写入消息；而读消息任务则一直等待消息的到来，当读取消息成功就通过串口输出消息。

### 2. 源码分析

#### 2.1 数据结构

分析OpenHarmony消息队列相关数据结构struct LosQueueCB及其成员的功能。
消息队列数据结构struct LosQueueCB如代码引用9.1所示。

```c
代码引用9.1  消息队列数据结构（los_queue.h）
typedef struct {
    UINT8 *queue;      /**< Pointer to a queue handle */
    UINT16 queueState; /**< Queue state */
    UINT16 queueLen;   /**< Queue length */
    UINT16 queueSize;  /**< Node size */
    UINT16 queueID;    /**< queueID */
    UINT16 queueHead;  /**< Node head */
    UINT16 queueTail;  /**< Node tail */
    UINT16 readWriteableCnt[OS_READWRITE_LEN]; /**< Count of readable or writable resources, 0:readable, 1:writable */
    LOS_DL_LIST readWriteList[OS_READWRITE_LEN]; /**< Pointer to the linked list to be read or written,
                                                      0:readlist, 1:writelist */
    LOS_DL_LIST memList; /**< Pointer to the memory linked list */
} LosQueueCB;
```

#### 2.2 宏

分析宏的作用。
消息队列相关宏如代码引用9.2所示。
代码引用9.2  消息队列相关宏（los_queue.h）

```c
#define OS_QUEUE_OPERATE_TYPE(ReadOrWrite, HeadOrTail, PointOrNot)  \
                (((UINT32)(PointOrNot) << 2) | ((UINT32)(HeadOrTail) << 1) | (ReadOrWrite))
#define OS_QUEUE_READ_WRITE_GET(type) ((type) & (0x01))
#define OS_QUEUE_READ_HEAD     (OS_QUEUE_READ | (OS_QUEUE_HEAD << 1))
#define OS_QUEUE_READ_TAIL     (OS_QUEUE_READ | (OS_QUEUE_TAIL << 1))
#define OS_QUEUE_WRITE_HEAD    (OS_QUEUE_WRITE | (OS_QUEUE_HEAD << 1))
#define OS_QUEUE_WRITE_TAIL    (OS_QUEUE_WRITE | (OS_QUEUE_TAIL << 1))
#define OS_QUEUE_OPERATE_GET(type) ((type) & (0x03))
#define OS_QUEUE_IS_POINT(type)    ((type) & (0x04))
#define OS_QUEUE_IS_READ(type)     (OS_QUEUE_READ_WRITE_GET(type) == OS_QUEUE_READ)
#define OS_QUEUE_IS_WRITE(type)    (OS_QUEUE_READ_WRITE_GET(type) == OS_QUEUE_WRITE)
```

#### 2.3 全局变量

分析每个全局变量的作用。
消息队列相关全局变量如代码引用9.3所示。
代码引用9.3  消息队列数相关全局变量（los_queue.c）

```c
LITE_OS_SEC_BSS LosQueueCB *g_allQueue = NULL ;  /** 所有消息队列控制块指针 */
LITE_OS_SEC_BSS LOS_DL_LIST g_freeQueueList;  /** 记录空闲消息队列的链表 */
```

#### 2.4 函数

分析OpenHarmony消息队列的初始化、创建、删除、操作、读和写函数功能。功能。注释标有①、②、③等编号的行。

##### 2.4.1 消息队列初始化函数OsQueueInit()

消息队列初始化函数OsQueueInit()如代码引用9.4所示。
代码引用9.4  消息队列初始化函数（los_queue.c）

```c
LITE_OS_SEC_TEXT_INIT UINT32 OsQueueInit(VOID)
{
    LosQueueCB *queueNode = NULL;
    UINT16 index;

    if (LOSCFG_BASE_IPC_QUEUE_LIMIT == 0) {
        return LOS_ERRNO_QUEUE_MAXNUM_ZERO;
    }

①　g_allQueue = (LosQueueCB *)LOS_MemAlloc(m_aucSysMem0, LOSCFG_BASE_IPC_QUEUE_LIMIT * sizeof(LosQueueCB));
    if (g_allQueue == NULL) {
        return LOS_ERRNO_QUEUE_NO_MEMORY;
    }

②　(VOID)memset_s(g_allQueue, LOSCFG_BASE_IPC_QUEUE_LIMIT * sizeof(LosQueueCB),
                   0, LOSCFG_BASE_IPC_QUEUE_LIMIT * sizeof(LosQueueCB));

③　LOS_ListInit(&g_freeQueueList);
④　for (index = 0; index < LOSCFG_BASE_IPC_QUEUE_LIMIT; index++) {
        queueNode = ((LosQueueCB *)g_allQueue) + index;
        queueNode->queueID = index;

        LOS_ListTailInsert(&g_freeQueueList, &queueNode->readWriteList[OS_QUEUE_WRITE]);
    }

    return LOS_OK;
}
```

##### 2.4.2 消息队列创建函数OsQueueCreate()

消息队列创建函数OsQueueCreate()如代码引用9.5所示。
代码引用9.5  消息队列创建函数（los_queue.c）

```c
/*****************************************************************************
 Function    : LOS_QueueCreate
 Description : Create a queue
 Input       : queueName  --- Queue name, less than 4 characters
             : len        --- Queue length
             : flags      --- Queue type, FIFO or PRIO
             : maxMsgSize --- Maximum message size in byte
 Output      : queueID    --- Queue ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_QueueCreate(CHAR *queueName,
                                             UINT16 len,
                                             UINT32 *queueID,
                                             UINT32 flags,
                                             UINT16 maxMsgSize)
{
    LosQueueCB *queueCB = NULL;
    UINT32 intSave;
    LOS_DL_LIST *unusedQueue = NULL;
    UINT8 *queue = NULL;
    UINT16 msgSize;

    (VOID)queueName;
    (VOID)flags;

    if (queueID == NULL) {
        return LOS_ERRNO_QUEUE_CREAT_PTR_NULL;
    }

    if (maxMsgSize > (OS_NULL_SHORT - sizeof(UINT32))) {
        return LOS_ERRNO_QUEUE_SIZE_TOO_BIG;
    }

    if ((len == 0) || (maxMsgSize == 0)) {
        return LOS_ERRNO_QUEUE_PARA_ISZERO;
}

①　msgSize = maxMsgSize + sizeof(UINT32);

    /** Memory allocation is time-consuming, to shorten the time of disable interrupt,
       move the memory allocation to here. */
②　queue = (UINT8 *)LOS_MemAlloc(m_aucSysMem0, len * msgSize);
    if (queue == NULL) {
        return LOS_ERRNO_QUEUE_CREATE_NO_MEMORY;
    }

    intSave = LOS_IntLock();
    if (LOS_ListEmpty(&g_freeQueueList)) {
        LOS_IntRestore(intSave);
        (VOID)LOS_MemFree(m_aucSysMem0, queue);
        return LOS_ERRNO_QUEUE_CB_UNAVAILABLE;
}
③　unusedQueue = LOS_DL_LIST_FIRST(&(g_freeQueueList));
④　LOS_ListDelete(unusedQueue);
⑤　queueCB = (GET_QUEUE_LIST(unusedQueue)); 
⑥　queueCB->queueLen = len;
    queueCB->queueSize = msgSize;
    queueCB->queue = queue;
    queueCB->queueState = OS_QUEUE_INUSED;
    queueCB->readWriteableCnt[OS_QUEUE_READ] = 0;
    queueCB->readWriteableCnt[OS_QUEUE_WRITE] = len;
    queueCB->queueHead = 0;
queueCB->queueTail = 0;
⑦　LOS_ListInit(&queueCB->readWriteList[OS_QUEUE_READ]);
⑧　LOS_ListInit(&queueCB->readWriteList[OS_QUEUE_WRITE]);
⑨　LOS_ListInit(&queueCB->memList);
    LOS_IntRestore(intSave);

    *queueID = queueCB->queueID;

    OsHookCall(LOS_HOOK_TYPE_QUEUE_CREATE, queueCB);

    return LOS_OK;
}
```

##### 2.4.3 消息队列读函数OsQueueRead()

消息队列读函数OsQueueRead()如代码引用9.6所示。
代码引用9.6  消息队列读函数（los_queue.c）

```c
LITE_OS_SEC_TEXT UINT32 LOS_QueueRead(UINT32 queueID, VOID *bufferAddr, UINT32 bufferSize, UINT32 timeOut)
{
    UINT32 ret;
UINT32 operateType;
①　ret = OsQueueReadParameterCheck(queueID, bufferAddr, &bufferSize, timeOut);
    if (ret != LOS_OK) {
        return ret;
    }
②　operateType = OS_QUEUE_OPERATE_TYPE(OS_QUEUE_READ, OS_QUEUE_HEAD, OS_QUEUE_POINT);

OsHookCall(LOS_HOOK_TYPE_QUEUE_READ, (LosQueueCB *)GET_QUEUE_HANDLE(queueID), operateType, bufferSize, timeOut);

③　return OsQueueOperate(queueID, operateType, bufferAddr, &bufferSize, timeOut);
}
```

##### 2.4.4 消息队列写OsQueueWrite()

消息队列写函数OsQueueWrite()如代码引用9.7所示。
代码引用9.7  消息队列读函数（los_queue.c）

```c
LITE_OS_SEC_TEXT UINT32 LOS_QueueWrite(UINT32 queueID, VOID *bufferAddr, UINT32 bufferSize, UINT32 timeOut)
{
    UINT32 ret;
    UINT32 operateType;
    UINT32 size = sizeof(UINT32 *);
    (VOID)bufferSize;

①　ret = OsQueueWriteParameterCheck(queueID, bufferAddr, &size, timeOut);
    if (ret != LOS_OK) {
        return ret;
    }

②　operateType = OS_QUEUE_OPERATE_TYPE(OS_QUEUE_WRITE, OS_QUEUE_TAIL, OS_QUEUE_POINT);

    OsHookCall(LOS_HOOK_TYPE_QUEUE_WRITE, (LosQueueCB *)GET_QUEUE_HANDLE(queueID), operateType, size, timeOut);

    return OsQueueOperate(queueID, operateType, &bufferAddr, &size, timeOut);
}
```

##### 2.4.5 消息队列删除函数OsQueuesDelete()

消息队列删除函数OsQueuesDelete()如代码引用9.8所示。
代码引用9.8  消息队列删除函数（los_queue.c）

```c
/*****************************************************************************
 Function    : LOS_QueueDelete
 Description : Delete a queue
 Input       : queueID   --- QueueID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_QueueDelete(UINT32 queueID)
{
    LosQueueCB *queueCB = NULL;
    UINT8 *queue = NULL;
    UINT32 intSave;
    UINT32 ret;

    if (queueID >= LOSCFG_BASE_IPC_QUEUE_LIMIT) {
        return LOS_ERRNO_QUEUE_NOT_FOUND;
    }

    intSave = LOS_IntLock();
queueCB = (LosQueueCB *)GET_QUEUE_HANDLE(queueID);
①　if (queueCB->queueState == OS_QUEUE_UNUSED) {
        ret = LOS_ERRNO_QUEUE_NOT_CREATE;
        goto QUEUE_END;
}
②　if (!LOS_ListEmpty(&queueCB->readWriteList[OS_QUEUE_READ])) {
        ret = LOS_ERRNO_QUEUE_IN_TSKUSE;
        goto QUEUE_END;
    }

③　if (!LOS_ListEmpty(&queueCB->readWriteList[OS_QUEUE_WRITE])) {
        ret = LOS_ERRNO_QUEUE_IN_TSKUSE;
        goto QUEUE_END;
    }

④　if (!LOS_ListEmpty(&queueCB->memList)) {
        ret = LOS_ERRNO_QUEUE_IN_TSKUSE;
        goto QUEUE_END;
    }


⑤　if ((queueCB->readWriteableCnt[OS_QUEUE_WRITE] + queueCB->readWriteableCnt[OS_QUEUE_READ]) !=
        queueCB->queueLen) {
        ret = LOS_ERRNO_QUEUE_IN_TSKWRITE;
        goto QUEUE_END;
    }

⑥　queue = queueCB->queue;
    queueCB->queue = (UINT8 *)NULL;
queueCB->queueState = OS_QUEUE_UNUSED;
⑦　LOS_ListAdd(&g_freeQueueList, &queueCB->readWriteList[OS_QUEUE_WRITE]);
    LOS_IntRestore(intSave);

    OsHookCall(LOS_HOOK_TYPE_QUEUE_DELETE, queueCB);

    ret = LOS_MemFree(m_aucSysMem0, (VOID *)queue);
    return ret;

QUEUE_END:
    LOS_IntRestore(intSave);
    return ret;
}
```

##### 2.4.6)消息队列操作函数OsQueuesOperate()

消息队列操作函数OsQueuesOperate()如代码引用9.9所示。
代码引用9.9  消息队列操作函数（los_queue.c）

```c
UINT32 OsQueueOperate(UINT32 queueID, UINT32 operateType, VOID *bufferAddr, UINT32 *bufferSize, UINT32 timeOut)
{
    LosQueueCB *queueCB = NULL;
    LosTaskCB *resumedTask = NULL;
    UINT32 ret;
    UINT32 readWrite = OS_QUEUE_READ_WRITE_GET(operateType);
    UINT32 readWriteTmp = !readWrite;

    UINT32 intSave = LOS_IntLock();

queueCB = (LosQueueCB *)GET_QUEUE_HANDLE(queueID);
①　ret = OsQueueOperateParamCheck(queueCB, operateType, bufferSize);
    if (ret != LOS_OK) {
        goto QUEUE_END;
    }
②　if (queueCB->readWriteableCnt[readWrite] == 0) {
③　    if (timeOut == LOS_NO_WAIT) { 
            ret = OS_QUEUE_IS_READ(operateType) ? LOS_ERRNO_QUEUE_ISEMPTY : LOS_ERRNO_QUEUE_ISFULL;
            goto QUEUE_END;
        }
④　if (g_losTaskLock) {
            ret = LOS_ERRNO_QUEUE_PEND_IN_LOCK;
            goto QUEUE_END;
        }

LosTaskCB *runTsk = (LosTaskCB *)g_losTask.runTask;
⑤　    OsSchedTaskWait(&queueCB->readWriteList[readWrite], timeOut);
        LOS_IntRestore(intSave);
        LOS_Schedule();

        intSave = LOS_IntLock();
        if (runTsk->taskStatus & OS_TASK_STATUS_TIMEOUT) {
            runTsk->taskStatus &= ~OS_TASK_STATUS_TIMEOUT;
            ret = LOS_ERRNO_QUEUE_TIMEOUT;
            goto QUEUE_END;
        }
    } else {
⑥　    queueCB->readWriteableCnt[readWrite]--; 

    }

⑦　OsQueueBufferOperate(queueCB, operateType, bufferAddr, bufferSize);

⑧　if (!LOS_ListEmpty(&queueCB->readWriteList[readWriteTmp])) {
        resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&queueCB->readWriteList[readWriteTmp]));
        OsSchedTaskWake(resumedTask);
        LOS_IntRestore(intSave);
        LOS_Schedule();
        return LOS_OK;
    } else {
        queueCB->readWriteableCnt[readWriteTmp]++;
    }

QUEUE_END:
    LOS_IntRestore(intSave);
    return ret;
}
```
