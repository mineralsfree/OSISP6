#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>         /* exit(), malloc(), free() */
#include <sys/types.h>      /* key_t, sem_t, pid_t      */
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <errno.h>          /* errno, ECHILD            */
#include <fcntl.h>
#include <time.h>
#include <string.h>
int fd1[2];          // PIPE1
int fd2[2];
char a[4096];
char b[40];
sem_t *sema;
int *p;
void my_handler1(int signum){

    close(fd1[0]);
    int val = 0;
    sem_getvalue(sema, &val);
    printf("sem value 1handler1 %d\n", val);
    sem_wait(sema);
    snprintf(a, sizeof(b), "LINE: %d PID: %d TIME: %lu \n",*p, getpid(), clock());
    *p = *p+1;
    for (int i = 0; i < 54; ++i) {
        snprintf(b, sizeof(b), "LINE: %d PID: %d TIME: %lu \n", *p, getpid(), clock());
        strcat(a,b);
        *p = *p+1;
    }
    write(fd1[1], a, (strlen(a)+1));
    sem_post(sema);

}
void my_handler2(int signum){\
close(fd2[0]);
    int val;
    sem_getvalue(sema, &val);
    printf("sem value 2handler2 %d\n", val);
    //sem_wait(sema);
    snprintf(a, sizeof(b), "LINE: %d PID: %d TIME: %lu \n",*p, getpid(), clock());
    *p = *p+1;
    for (int i = 0; i < 54; ++i) {
        snprintf(b, sizeof(b), "LINE: %d PID: %d TIME: %lu \n", *p, getpid(), clock());
        strcat(a,b);
        *p = *p+1;
    }
    write(fd2[1], a, (strlen(a)+1));
    //sem_post(sema);
}

int main() {
    sema = mmap(NULL, sizeof *sema, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(sema, 1, 2);
    int child1PID;
    int child2PID;
    key_t shmkey;                 /*      shared memory key       */
    int shmid;
    shmkey = ftok ("/home/nikita/semester4/OS/LAB5_3", 5);       /* valid directory name and a number */
    shmid = shmget (shmkey, sizeof (int), 0644 | IPC_CREAT);
    p = (int *) shmat (shmid, NULL, 0);   /* attach p to shared memory */
    *p = 1;

    pipe(fd1);
    pipe(fd2);
    child1PID = fork();
    if (child1PID == 0) {
        //printf("child 1 self %d\n", getpid());
        signal(SIGUSR1, my_handler1);
        while (1){

        }

        // close(fd1[0]);
        // write(fd1[1], string, (strlen(string)+1));
        //exit(0);
        //child 1 action

    } else {
        child2PID = fork();
        if (child2PID == 0) {
            printf("child 2 self %d\n", getpid());
            signal(SIGUSR1, my_handler2);
            while (1){

            }

            //child 2 action

        } else {
            sleep(3);
            for (int i = 0; i < 10 ; ++i) {
                printf("child 1 %d\n", child1PID);
                printf("child 2 %d\n", child2PID);
                close(fd1[1]);
                close(fd2[1]);
                kill(child1PID, SIGUSR1);

                read(fd1[0], a, sizeof(a));
                printf("Received string: from pipe1\n%s\n", a);

                kill(child2PID, SIGUSR1);
                read(fd2[0], a, sizeof(a));
                printf("Received string: from pipe2\n%s\n", a);
            }



        }

    }

    return 0;
}