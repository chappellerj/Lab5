#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

#define BUFSIZE 10
#define NP 3
#define NC 2

pthread_t tidP[NP], tidC[NC];
int buff[BUFSIZE];
sem_t *full, *empty, *mutex;
int p, c, in, out;

void *producer(void *arg);
void *consumer(void *arg);
void cleanup(int signo);

int main() {
    signal(SIGINT, cleanup);

    mutex = sem_open("mutex", O_CREAT, 0644, 1);
    empty = sem_open("empty", O_CREAT, 0644, BUFSIZE);
    full = sem_open("full", O_CREAT, 0644, 0);

    int i;
    for (i = 0; i < NP; i++)
        pthread_create(&tidP[i], NULL, producer, (void *)(size_t)i);
    for (i = 0; i < NC; i++)
        pthread_create(&tidC[i], NULL, consumer, (void *)(size_t)i);
    for (i = 0; i < NP; i++) {
        pthread_join(tidP[i], NULL);
        printf("Producer %d joined\n", i);
    }
    for (i = 0; i < NC; i++) {
        pthread_join(tidC[i], NULL);
        printf("\t\t\tConsumer %d joined\n", i);
    }

    printf("Parent quiting\n");

    sem_unlink("mutex");
    sem_unlink("empty");
    sem_unlink("full");
    raise(SIGINT);

    return 0;
}

void *producer(void *arg) {
    int item;
    int p = (int)arg;
    srand(time(0));
    do {
        item = rand() % 100;
        sem_wait(empty);
        sem_wait(mutex);

        buff[in] = item;
        in = (in + 1) % BUFSIZE;

        printf("Producer %d produced %d at buffer %d\n", p, item, in);
        sleep(1);

        sem_post(mutex);
        sem_post(full);
    } while(1);
    pthread_exit(0);
}

void *consumer(void *arg) {
    int item;
    int c = (int)arg;
    srand(time(0));
    do {
        sem_wait(full);
        sem_wait(mutex);

        item = buff[out];
        out = (out + 1) % BUFSIZE;

        printf("\t\t\tConsumer %d consumed %d at buffer %d\n", c, item, out);
        sleep(1);
        sem_post(mutex);
        sem_post(empty);
    } while(1);
    pthread_exit(0);
}

void cleanup(int signo) {
    printf("Terminating\n");
    sem_unlink("mutex");
    sem_unlink("empty");
    sem_unlink("full");
    exit(0);
}
