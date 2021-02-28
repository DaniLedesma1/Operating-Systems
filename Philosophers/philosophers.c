#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define NR_PHILOSOPHERS 5

pthread_t philosophers[NR_PHILOSOPHERS];
pthread_mutex_t  forks[NR_PHILOSOPHERS];


void init(){
    int i;
    for(i=0; i<NR_PHILOSOPHERS; i++)
        pthread_mutex_init(&forks[i],NULL);
}

void think(int i) {
    printf("Philosopher %d thinking... \n" , i);
    sleep(random() % 10);
    printf("Philosopher %d stopped thinking!!! \n" , i);

}

void eat(int i) {
    printf("Philosopher %d eating... \n" , i);
    sleep(random() % 5);
    printf("Philosopher %d is not eating anymore!!! \n" , i);

}

void toSleep(int i) {
    printf("Philosopher %d sleeping... \n" , i);
    sleep(random() % 10);
    printf("Philosopher %d is awake!!! \n" , i);
    
}
void put_forks(int left, int right){
    pthread_mutex_unlock(&forks[left]);
    pthread_mutex_unlock(&forks[right]);
}

void take_fork(int left, int right){
    pthread_mutex_lock(&forks[right]);
    pthread_mutex_lock(&forks[left]);
}

void finish(){
    int i;
    for(i=0; i<NR_PHILOSOPHERS; i++)
        pthread_mutex_destroy(&forks[i]);
}


void* philosopher(void* i){
    unsigned long nPhilosopher = *((unsigned long*)i);
    int right = (int)nPhilosopher;
    int left = (nPhilosopher - 1 == -1) ? NR_PHILOSOPHERS - 1 : (nPhilosopher - 1);
    while(1){
        think(nPhilosopher);//filosofo piensa;
        take_fork(left, right);//filosofo coge tenedores si puede, si no se queda bloqueado hasta que su tenedos este libre.
        eat(nPhilosopher);//come
        put_forks(left, right);//deja los tenedores
        toSleep(nPhilosopher);//duerme
   }
}

int main(){
    init();
    unsigned long i;
    for(i=0; i<NR_PHILOSOPHERS; i++)
        pthread_create(&philosophers[i], NULL, philosopher, &i);
    
    for(i=0; i<NR_PHILOSOPHERS; i++)
        pthread_join(philosophers[i],NULL);
    return 0;
} 
