#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define P_NUM 3
#define C_NUM 4


#define P_CIRCLE 4
#define C_CIRCLE 3

#define BLUEBUF_SIZE (sizeof(struct BlueBuf))


#define BUF_SIZE 4

#define LETTER_NUM  3

#define SHM_FLAG 0666

#define SEM_ALL_KEY   1234                   //	a key value for recgnize

#define EMPTY_FIRST 0					//semaphore
#define FULL_SECOND 1
#define MUTEX       2             //is the list number instead of top number

//the buffer
struct BlueBuf
{
    char letter[BUF_SIZE];
    int head;
    int tail;
    int is_empty;
    int num;
};

//the random sleep time
int get_random()
{
    int t;
    srand((unsigned)(getpid() + time(NULL)));
    t = rand() % 3;
    return t;
}

//get a random letter of my name
char get_letter()
{
    char a;
    char NAME[]={'C','H','M'};   // if change  the num of characters,remember to change the LETTER_NUM

    srand((unsigned)(getpid() + time(NULL)));
    a=NAME[rand() % LETTER_NUM];
    return a;
}

//P
void P(int sem_id, int sem_num)
{
    struct sembuf sem_buf;
    sem_buf.sem_num = sem_num;
    sem_buf.sem_op = -1;
    sem_buf.sem_flg = 0;
    semop(sem_id, &sem_buf, 1);  //id  pointer to struct sembuf    the number of option  for each time
}

//V
void V(int sem_id, int sem_num)
{
    struct sembuf sem_buf;
    sem_buf.sem_num = sem_num;
    sem_buf.sem_op = 1;
    sem_buf.sem_flg = 0;
    semop(sem_id, &sem_buf, 1);
}


int main(int argc, char * argv[])
{
    int i, j;
    int shm_id, sem_id;
    int num_p = 0, num_c = 0;
    struct BlueBuf * buf_PTR;
    char lt;
    time_t now;
    pid_t pid_p, pid_c;
    
    sem_id = semget(SEM_ALL_KEY, 3, IPC_CREAT | 0660);  //retuen -1 if failed ;else >=0   0660//  ppt  secrity leval

    if (sem_id >= 0)
    {
        printf("Main process starts. Semaphore created.\n");
    }

    semctl(sem_id, EMPTY_FIRST, SETVAL, BUF_SIZE);   //semaphore control        id,0,set value of semaphore to LEtternUM     
    semctl(sem_id, FULL_SECOND, SETVAL, 0);            //set second
    semctl(sem_id, MUTEX, SETVAL, 1);     //control the write
    
    if ((shm_id = shmget(IPC_PRIVATE, BLUEBUF_SIZE, SHM_FLAG)) < 0)
    {
        printf("Error on shmget.\n");
        exit(1);
    }
    buf_PTR = shmat(shm_id, 0, 0);//ID    system did->path   ,0
    
    if ( buf_PTR == (void *)-1)    // return -1 when failed
    {
        printf("Error on shmat.\n");
        exit(1);
    }

    //initialization
    buf_PTR->head = 0;
    buf_PTR->tail = 0;
    buf_PTR->is_empty = 1;
    buf_PTR->num =0;
    strcpy(buf_PTR->letter,"----");
    
    for(num_p=0;num_p<P_NUM;num_p++)               //create the producer used by fork()
    {
        if ((pid_p = fork()) < 0)
        {
            printf("Error on fork.\n");
            exit(1);
        }
        

        if (pid_p == 0)
        {
            if ((buf_PTR = shmat(shm_id, 0, 0)) == (void *)-1)
            {
                printf("Error on shmat.\n");
                exit(1);
            }
            for (i = 0; i < P_CIRCLE; i++)
            {
            	sleep(1);
                P(sem_id, EMPTY_FIRST);   //request for buf
                P(sem_id,MUTEX);//write
                sleep(get_random());
                buf_PTR->letter[buf_PTR->tail] = lt = get_letter();
                buf_PTR->tail = (buf_PTR->tail + 1) % BUF_SIZE;
                buf_PTR->is_empty = 0;
                buf_PTR->num ++;
                now = time(NULL);
                printf("[%02d]\t%02d:%02d:%02d\t", buf_PTR->num,localtime(&now)->tm_hour, localtime(&now)->tm_min, localtime(&now)->tm_sec);


                for(int k=0;k<BUF_SIZE;k++)
                    printf("%c",buf_PTR->letter[k] );



                printf("\tP%d puts '%c'.--%d", num_p+1, lt,i+1);
                 if((i+1)==P_CIRCLE)
                    printf("-------P%dEnd!\n",num_p+1);
                else
                    printf("\n");
                fflush(stdout);
                V(sem_id,MUTEX);
                V(sem_id, FULL_SECOND);
            }
            shmdt(buf_PTR);
            exit(0);
        }
    }

    for(num_c=0; num_c < C_NUM; num_c++)
    {
        if ((pid_c = fork()) < 0)
        {
            printf("Error on fork.\n");
            exit(1);
        }
        
        if (pid_c == 0)
        {
            if ((buf_PTR = shmat(shm_id, 0, 0)) == (void *)-1)
            {
                printf("Error on shmat.\n");
                exit(1);
            }
            for (i = 0; i < C_CIRCLE; i++)
            {
                P(sem_id, FULL_SECOND);
                P(sem_id, MUTEX);
                sleep(get_random());
                lt = buf_PTR->letter[buf_PTR->head];
                buf_PTR->letter[buf_PTR->head]='-'; 
                buf_PTR->head = (buf_PTR->head + 1) % BUF_SIZE;
                buf_PTR->is_empty = (buf_PTR->head == buf_PTR->tail);
                now = time(NULL);
                buf_PTR->num++;
                printf("[%02d]\t%02d:%02d:%02d\t",
                        buf_PTR->num, localtime(&now)->tm_hour, 
                        localtime(&now)->tm_min, localtime(&now)->tm_sec
                    );

                for( int k=0;k<BUF_SIZE;k++)
                    printf("%c",buf_PTR->letter [k]);

                printf("\tC%d gets '%c'.--%d", num_c+1, lt,i+1);
                if((i+1)==C_CIRCLE)
                    printf("-------C%dEnd!\n",num_c+1);
                else
                    printf("\n");


               // fflush(stdout);
                V(sem_id,MUTEX);
                V(sem_id, EMPTY_FIRST);
            }
            shmdt(buf_PTR);
            exit(0);
        }
    }
    
    //主控程序最后退出
    while(wait(0) != -1);
    shmdt(buf_PTR);
    shmctl(shm_id, IPC_RMID, 0);
    semctl(sem_id, IPC_RMID, 0);
    printf("EVERYTHING HAS BEEN DONE SUCCESSFUALLY!\n");
    //fflush(stdout);
    exit(0);
}
