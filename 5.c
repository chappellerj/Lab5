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
pthread_mutex_t mutex;
pthread_cond_t full, empty;
int p, c, in, out, count;

void *producer(void *arg);
void *consumer(void *arg);

int main() {

    pthread_mutex_init(&mutex, NULL);
    count = 0;

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

    return 0;
}

void *producer(void *arg) {
    int item;
    int p = (int)arg;
    srand(time(0));
    do {
        item = rand() % 100;
        pthread_mutex_lock(&mutex);
        while(count == BUFSIZE) {
            pthread_cond_wait(&empty, &mutex);
        }

        buff[in] = item;
        in = (in + 1) % BUFSIZE;

        printf("Producer %d produced %d at buffer %d\n", p, item, in);
        count++;
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    } while(1);
    return (NULL);
}

void *consumer(void *arg) {
    int item;
    int c = (int)arg;
    srand(time(0));
    do {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&full, &mutex);
        }

        item = buff[out];
        out = (out + 1) % BUFSIZE;

        printf("\t\t\tConsumer %d consumed %d at buffer %d\n", c, item, out);
        count--;
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    } while(1);
    return (NULL);
}
