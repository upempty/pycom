// layer_messaging.c
/*
gcc -c -fPIC layer_messaging.c
gcc -shared -o layer_messaging.so layer_messaging.o
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>

#define CONTAINER_HEADROOM_LEN 4
#define MAX_BUFFERS 100
#define CONTAINER_KEY 1000
#define QUEUE_KEY 2000
#define CPID_POOL_KEY 3000
#define MAX_MSG_SIZE 256
#define MAX_PROCESS_COUNT 64
#define MAX_MSG_COUNT_IN_PROCESS 64

//msg pool for all messages among processes
typedef struct Containers
{
    
    char buf[MAX_BUFFERS][MAX_MSG_SIZE];
    int buffer_index; //not used
    int reserved[MAX_BUFFERS];
    sem_t lock;
} Containers;

//one queue for each process
typedef struct MsgQueue
{
    int msgRB[MAX_MSG_COUNT_IN_PROCESS]; //ring buffer
    int in; // size==max FULL; msgPos[in] = data; in = (in+1)%MAX_MSG_COUNT_IN_PROCESS
    int out; // size==0 empty; msgPos[out] return; out = (out+1)%MAX_MSG_COUNT_IN_PROCESS
    int length; // 0-empty; MAX_MSG_COUNT_IN_PROCESS-full; length = (in - out + MAX_MSG_COUNT_IN_PROCESS)%MAX_MSG_COUNT_IN_PROCESS.
    sem_t lock;
} MsgQueue;

typedef struct MsgQueues
{
    MsgQueue msgQueue[MAX_PROCESS_COUNT];
} MsgQueues;

#define MAX_SRV_NAME_LEN 32
typedef struct Cpid
{
    char sname[MAX_SRV_NAME_LEN];
    int pid;
} Cpid;

typedef struct CpidPool 
{
    Cpid cpids[MAX_PROCESS_COUNT]; //array index indicates CPID
    sem_t lock;
} CpidPool;

int initCpidPool(void) 
{

    CpidPool *cpidPool;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)CPID_POOL_KEY, sizeof(CpidPool), 0666 | IPC_CREAT);
    printf("Key of shared memory is %d\n", shmid);
    shm_addr = shmat(shmid, NULL, 0);
    printf("Process attached at %p\n", shm_addr);
    cpidPool = (struct CpidPool*)((void *)shm_addr);
    memset(cpidPool, 0, sizeof(CpidPool));
    printf("write to shared memory=%d\n", 1);

    int r = sem_init(&cpidPool->lock, 1 /*shared*/, 1 /*value*/);
    printf("sem init ret=%d, sem addr = %p, semvalue = %d\n", r, &(cpidPool->lock), cpidPool->lock);
    return 0;
}

int is_pid_alive(pid_t pid) 
{
    int ret = kill(pid, 0);
    return ret == 0;
}

int layer_register_serv_to_cpid(char *str)
{
    printf("layer_register_serv_to_cpid:=%s \n", str);

    CpidPool *cpidPool;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)CPID_POOL_KEY, sizeof(CpidPool), 0666 | IPC_CREAT);
    shm_addr = shmat(shmid, NULL, 0);
    cpidPool = (struct CpidPool*)((void *)shm_addr);
    
    sem_wait(&cpidPool->lock);
    //to find same sname slot and service is dead.
    int cpid = 0;
    while (cpid < MAX_PROCESS_COUNT)
    {
        int len = strlen(cpidPool->cpids[cpid].sname) + 1;
        int lenstr = strlen(str)+1;
        len = len < lenstr ? len : lenstr;
        if ((len > 1) && (memcmp(cpidPool->cpids[cpid].sname, str, len) == 0) && !is_pid_alive(cpidPool->cpids[cpid].pid))
        {
            printf("CPID to overwrite as it was dead: %d\n", cpid);
            break;
        }
        cpid++;
    }
    if (cpid < MAX_PROCESS_COUNT)
    {
        cpidPool->cpids[cpid].pid = getpid();
        //memcpy(cpidPool->cpids[cpid].sname, str, len);
        sem_post(&cpidPool->lock);
        printf("CPID to reused: %d\n", cpid);
        return cpid;
    }

    //to find free slots index
    cpid = 0;
    while (cpid < MAX_PROCESS_COUNT)
    {
        if (cpidPool->cpids[cpid].pid == 0)
            break;
        cpid++;
    }
    if (cpid >= MAX_PROCESS_COUNT)
    {
        sem_post(&cpidPool->lock);
        printf("CPID overflows: %d\n", cpid);
        return -1;
    }
    cpidPool->cpids[cpid].pid = getpid();
    int len = strlen(str)+1;
    len = len < MAX_SRV_NAME_LEN ? len : MAX_SRV_NAME_LEN;
    memcpy(cpidPool->cpids[cpid].sname, str, len);
    //printf("buf index=%d \n", containers->buffer_index);
    sem_post(&cpidPool->lock);

    return cpid;
}

int layer_retrieve_cpid_from_serv(char *str)
{
    printf("layer_register_serv_to_cpid:=%s \n", str);

    CpidPool *cpidPool;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)CPID_POOL_KEY, sizeof(CpidPool), 0666 | IPC_CREAT);
    shm_addr = shmat(shmid, NULL, 0);
    cpidPool = (struct CpidPool*)((void *)shm_addr);
    
    sem_wait(&cpidPool->lock);
    //to find free slots index
    int cpid = 0;
    while (cpid < MAX_PROCESS_COUNT)
    {
        int len = strlen(cpidPool->cpids[cpid].sname) + 1;
        int lenstr = strlen(str)+1;
        len = len < lenstr ? len : lenstr;
        if (memcmp(cpidPool->cpids[cpid].sname, str, len) == 0)
            break;
        cpid++;
    }
    if (cpid >= MAX_PROCESS_COUNT)
    {
        sem_post(&cpidPool->lock);
        printf("CPID not found: %d\n", cpid);
        return -1;
    }
    //printf("buf index=%d \n", containers->buffer_index);
    sem_post(&cpidPool->lock);
    return cpid;
}

int layer_get_own_cpid(void)
{
    printf("layer_get_own_cpid \n");

    CpidPool *cpidPool;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)CPID_POOL_KEY, sizeof(CpidPool), 0666 | IPC_CREAT);
    shm_addr = shmat(shmid, NULL, 0);
    cpidPool = (struct CpidPool*)((void *)shm_addr);
    
    sem_wait(&cpidPool->lock);
    //to find free slots index
    int cpid = 0;
    while (cpid < MAX_PROCESS_COUNT)
    {
        if (cpidPool->cpids[cpid].pid == getpid())
            break;
        cpid++;
    }
    if (cpid >= MAX_PROCESS_COUNT)
    {
        sem_post(&cpidPool->lock);
        printf("CPID not found: %d\n", cpid);
        return -1;
    }
    //printf("buf index=%d \n", containers->buffer_index);
    sem_post(&cpidPool->lock);
    return cpid;
}

int initContainers(void)
{
    Containers *containers;
    int key = CONTAINER_KEY;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)key, sizeof(Containers) + CONTAINER_HEADROOM_LEN, 0666 | IPC_CREAT);
    printf("Key of shared memory is %d\n", shmid);
    shm_addr = shmat(shmid, NULL, 0); //SHM_RDONLY - read only, 0 - read and write.
    printf("Process attached at %p\n", shm_addr);
    containers = (struct Containers*)((void *)shm_addr + CONTAINER_HEADROOM_LEN);
    int *headroom = (int *)shm_addr;
    *headroom = 0;// not used currently so to set 0.
    memset(containers, 0, sizeof(containers));
    printf("write to shared memory=%d\n", key);

    int r = sem_init(&containers->lock, 1 /*shared*/, 1 /*value*/);
    printf("sem init ret=%d, sem addr = %p, semvalue = %d\n", r, &(containers->lock), containers->lock);
    return 0;
}

// return msg buffer index
int layer_mem_reserve(char *str)
{
    printf("layer_mem_reserve:=%s \n", str);

    Containers *containers;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)CONTAINER_KEY, sizeof(Containers) + CONTAINER_HEADROOM_LEN, 0666 | IPC_CREAT);
    shm_addr = shmat(shmid, NULL, 0);
    containers = (struct Containers*)((void *)shm_addr + CONTAINER_HEADROOM_LEN);
    
    //char *str = "first msg to sent";
    int msg_len = strlen(str) + 1;
    containers->buffer_index = 1; // not used.

    sem_wait(&containers->lock);
    //to find free slots index
    int buf_id = 0;
    while (buf_id < MAX_BUFFERS)
    {
        if (containers->reserved[buf_id] == 0)
            break;
        buf_id++;
    }
    if (buf_id >= MAX_BUFFERS)
    {
        sem_post(&containers->lock);
        return -1;
    }
  
    containers->reserved[buf_id] = msg_len;
    strcpy(containers->buf[buf_id], str);
    //printf("buf index=%d \n", containers->buffer_index);
    printf("buf content=%s \n", containers->buf[buf_id]);
    sem_post(&containers->lock);

    //to use sem_t for lock/unlok, sem_open, sem_wait, sem_post
    return buf_id;
}

//receive from buf index
char* layer_mem_receive(int buf_id)
{
    printf("layer_mem_receive \n");
    Containers *containers;
    void *shm_addr;
    char tmp[20];
    int shmid;
    shmid = shmget((key_t)CONTAINER_KEY, sizeof(Containers) + CONTAINER_HEADROOM_LEN, 0666 | IPC_CREAT);
    shm_addr = shmat(shmid, NULL, 0);
    containers = (struct Containers*)((void *)shm_addr + CONTAINER_HEADROOM_LEN);
    printf("buf content received=%s \n", containers->buf[buf_id]);
    return containers->buf[buf_id];
}

int layer_mem_release(int buf_id)
{
    printf("layer_mem_release \n");
    Containers *containers;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)CONTAINER_KEY, sizeof(Containers) + CONTAINER_HEADROOM_LEN, 0666 | IPC_CREAT);
    shm_addr = shmat(shmid, NULL, 0);
    containers = (struct Containers*)((void *)shm_addr + CONTAINER_HEADROOM_LEN);
    
    sem_wait(&containers->lock);
    containers->reserved[buf_id] = 0; //!!! important
    memset(containers->buf[buf_id], 0, MAX_MSG_SIZE);
    printf("buf content release=%s \n", containers->buf[buf_id]);
    sem_post(&containers->lock);

    return 0;
}

int initQueues(void) 
{
    //SmsgLocation localqueue[128];

    MsgQueues *msgQs;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)QUEUE_KEY, sizeof(MsgQueues), 0666 | IPC_CREAT);
    printf("Key of shared memory is %d\n", shmid);
    shm_addr = shmat(shmid, NULL, 0);
    printf("Process attached at %p\n", shm_addr);
    msgQs = (struct MsgQueues*)((void *)shm_addr);
    memset(msgQs, 0, sizeof(MsgQueues));
    printf("write to shared memory=%d\n", 1);

    for (int i=0; i < MAX_PROCESS_COUNT; i++)
    {
        int r = sem_init(&(msgQs->msgQueue[i].lock), 1 /*shared*/, 1 /*value*/);
        printf("sem init ret=%d, sem addr = %p, semvalue = %d\n", r, &(msgQs->msgQueue[i].lock), msgQs->msgQueue[i].lock);
    }
    return 0;
}

// cpid communication point id/queue id for one process
int layer_push_queue(int cpid, int buf_id)
{
    printf("layer_push_queue");
    MsgQueues *msgQs;
    void *shm_addr;

    int shmid;
    shmid = shmget((key_t)QUEUE_KEY, sizeof(MsgQueues), 0666 | IPC_CREAT);
    shm_addr = shmat(shmid, NULL, 0);
    msgQs = (struct MsgQueues*)((void *)shm_addr);

    sem_wait(&msgQs->msgQueue[cpid].lock);

    //the process's queue is full
    if (msgQs->msgQueue[cpid].length == MAX_MSG_COUNT_IN_PROCESS)
    {
        sem_post(&msgQs->msgQueue[cpid].lock);
        return -1;
    }
    int in = msgQs->msgQueue[cpid].in;
    msgQs->msgQueue[cpid].msgRB[in] = buf_id;
    msgQs->msgQueue[cpid].in = (in + 1) % MAX_MSG_COUNT_IN_PROCESS;
    msgQs->msgQueue[cpid].length++;

    printf("shared memory updated in-1=%d, len=%d\n", in, msgQs->msgQueue[cpid].length);
    sem_post(&msgQs->msgQueue[cpid].lock);

    return 0;
}

int layer_pop_queue(int cpid)
{
    printf("layer_pop_queue");
    MsgQueues *msgQs;
    void *shm_addr;
    int shmid;
    shmid = shmget((key_t)QUEUE_KEY, sizeof(MsgQueues), 0666 | IPC_CREAT);
    shm_addr = shmat(shmid, NULL, 0);
    msgQs = (struct MsgQueues*)((void *)shm_addr);
    
    int r = sem_wait(&msgQs->msgQueue[cpid].lock);
    //the process's queue is emoty
    if (msgQs->msgQueue[cpid].length == 0)
    {
        sem_post(&msgQs->msgQueue[cpid].lock);
        return -1;
    }
    int out = msgQs->msgQueue[cpid].out;
    int buf_id = msgQs->msgQueue[cpid].msgRB[out];
    msgQs->msgQueue[cpid].out = (out + 1) % MAX_MSG_COUNT_IN_PROCESS;
    msgQs->msgQueue[cpid].length--;

    printf("shared memory updated out=%d, len=%d\n", out, msgQs->msgQueue[cpid].length);
    sem_post(&msgQs->msgQueue[cpid].lock);

    return buf_id;
}