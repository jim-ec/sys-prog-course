#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cspinlock.h"

volatile int counter;
cspinlock_t* slock;

void* work1(void* _arg) {
	cspin_lock(slock);
	counter++;
	usleep(100);
	if (counter != 1) {
		exit(1);
	}
	cspin_unlock(slock);
	return NULL;
}

void* work2(void* _arg){
	while(counter == 0);
	cspin_lock(slock);
	counter++;
	cspin_unlock(slock);
	return NULL;
}

int main(void) {
	counter = 0;
	
	slock = cspin_alloc();
	if (!slock) {
		fprintf(stderr, "could not allocate memory\n");
		return 1;
	}

	pthread_t t1, t2;
	for (size_t i = 0; i < 10; i++) {
		counter = 0;
		pthread_create(&t1, NULL, work1, NULL);
		pthread_create(&t2, NULL, work2, NULL);
		pthread_join(t1, NULL);
		pthread_join(t2, NULL);
	}

	cspin_free(slock);

	return 0;
}
