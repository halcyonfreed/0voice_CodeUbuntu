#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>


#define LIST_INSERT(item, list)do{\
    item->prev = NULL;\
    item->next = list;\
    if((list)!= NULL) (list)->prev = item;\
    (list) = item;    \
} while(0)
//����ָ�룬listҪ����(*ppeople)


    
#define LIST_REMOVE(item, list) do {	\
        if (item->prev != NULL) item->prev->next = item->next; \
        if (item->next != NULL) item->next->prev = item->prev; \
        if (list == item) list = item->next;                    \
        item->prev = item->next = NULL;                         \
} while(0)



//1.����task
struct nTask{
    void(*task_func)(struct nTask *task);
    void *user_data;

    struct nTask *prev;
    struct nTask *next;
};

//2.ִ��
struct nWorker{
    pthread_t threadid;
    int terminate;
    
    struct nManager *manager; //worker��Ҫ��manager��ϵ��ʽ


    struct nWorker *prev;
    struct nWorker *next;
};

//3������� ����worker��tas����struct nManagerͱ�������ThreadPool
typedef struct nManager{
    struct nTask *tasks;  //�������
    struct nWorker *workers; //ִ�ж���

    pthread_mutex_t mutex; //�ӻ�����
    pthread_cond_t cond; //�������������ȴ���������������
}ThreadPool;


// API
// callback!=task
static void *nThreadPoolCallback(void *arg){
    struct nWorker *worker = (struct nWorker*) arg;
   //printf("LIST_CALLBACK\n");
    while(1){
        pthread_mutex_lock(&worker->manager->mutex); //�������һ����
        
        while(worker->manager->tasks == NULL){//��������û�͵ȣ��о��ó�ִ�У�ȡ�û�����
            if(worker->terminate) break; //��1������ֹ
            pthread_cond_wait(&worker->manager->cond, &worker->manager->mutex);
            
        }

        if(worker->terminate){
            // �˳������while���Ƚ�������Ȼ����������
            pthread_mutex_unlock(&worker->manager->mutex); 
            break; 
        }
        struct nTask *task = worker->manager->tasks;
        LIST_REMOVE(task, worker->manager->tasks); //���������tasks���׽ڵ�taskִ�����ˣ��Ƴ���
        
        pthread_mutex_unlock(&worker->manager->mutex);

//        task->task_func(task->user_data);
        task->task_func(task);
    }
    free(worker);// butû���˳���break�ĵط���struct nWorker��������ֹ��ʶ    
}


// API
int nThreadPoolCreate(ThreadPool*pool, int numWorker){
    if(pool == NULL) return -1;
    if(numWorker < 1) numWorker = 1; //�����߳�û�У��Ǿ�Ĭ��1��
    memset(pool, 0, sizeof(ThreadPool));

    // 2 ��struct��4����ʼ����task����ӽ�������
    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER; //����հ�������������
    memcpy(&pool->cond, &blank_cond,sizeof(pthread_cond_t));//blank_cond �е� pthread_cond_t �ṹ������ݸ��Ƶ� pool->cond �С�

    
    //printf("LIST_CREATE\n");

    
    pthread_mutex_init(&pool->mutex,NULL);

    int i = 0;
    for(int i =0; i < numWorker; i++){
        struct nWorker *worker = (struct nWorker*)malloc(sizeof(struct nWorker));
        //����ʧ��
        if(worker ==NULL){
            perror("malloc");
            return -2;
        }
        memset(worker, 0,sizeof(struct nWorker));
        worker->manager = pool;

        //�����߳�
        int ret = pthread_create(&worker->threadid, NULL, nThreadPoolCallback, worker); //�����ɹ�����0��ʧ��1
        if(ret){
            perror("pthread_create");
            free(worker);
            return -3;
        }
        //printf("LIST_INSERT\n");
        LIST_INSERT(worker, pool->workers);
            
    }
    return 0; //�����ɹ��� callback��ҵ���ܣ�һ�������ǲ���ͬ��task���񣬵���ִ�е�����ͬ
    
}


// API
int nThreadPoolDestroy(ThreadPool*pool, int nworker){
    struct nWorker *worker = NULL;

    for(worker = pool->workers; worker != NULL; worker = worker->next){
        worker->terminate;//����1
    }

    //�ź�ʲôʱ�������ߣ��㲥ʹ���������������ź�������
    pthread_mutex_lock(&pool->mutex);
    pthread_cond_broadcast(&pool->cond);// ��������thread,�㲥�͵ȴ��õ�ͬһ������������������ʲô����û����
    pthread_mutex_unlock(&pool->mutex);

    pool->workers= NULL;
    pool->tasks = NULL;
    return 0;
}

// API
int nThreadPoolPushTask(ThreadPool*pool, struct nTask *task){
    //֪ͨthread��task����
    pthread_mutex_lock(&pool->mutex);
    LIST_INSERT(task, pool->tasks);

    //֪ͨ�������������㣬����һ��thread
    pthread_cond_signal(&pool->cond);
    
    pthread_mutex_unlock(&pool->mutex);
    
}




//
// sdk --> debug thread pool    

#if 1

#define THREADPOOL_INIT_COUNT	20
#define TASK_INIT_SIZE			1000


void task_entry(struct nTask *task) { //type 

	//struct nTask *task = (struct nTask*)task;
	int idx = *(int *)task->user_data;

	printf("idx: %d\n", idx);

	free(task->user_data);
	free(task);
}


int main(void) {

	ThreadPool pool = {0};
	
	nThreadPoolCreate(&pool, THREADPOOL_INIT_COUNT);
    
	// pool --> memset();
	
	int i = 0;
	for (i = 0;i < TASK_INIT_SIZE;i ++) {
		struct nTask *task = (struct nTask *)malloc(sizeof(struct nTask));
		if (task == NULL) {
			perror("malloc");
			exit(1);
		}
		memset(task, 0, sizeof(struct nTask));

		task->task_func = task_entry;
		task->user_data = malloc(sizeof(int));
		*(int*)task->user_data  = i;

		
		nThreadPoolPushTask(&pool, task);
	}

	getchar(); //�ȴ�
	
}


#endif

