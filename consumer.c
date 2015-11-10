#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include "pcinfo.h"

/*
The following functions were based off of the functions provided by "Beginning Linux Programming", 4th Edition, by Neil Matthews and Richard Stones.
static int set_semvalue(int i, int sem_id)
static int semaphore_p(int sem_id)
static int semaphore_v(int sem_id)
*/

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
 semctl call. We need to do this before we can use the semaphore. */

static int set_semvalue(int i, int sem_id)
{
    union semun sem_union;

    sem_union.val = i;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

/* semaphore_p changes the semaphore by -1 (waiting). */

static int semaphore_p(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);
}

/* semaphore_v is similar except for setting the sem_op part of the sembuf structure to 1,
 so that the semaphore becomes available. */

static int semaphore_v(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}

int main(){
	//declaring the necessary semaphore ids
	int sem_id_n;
	int sem_id_s;
	int sem_id_e;

	//declaring shared memory space and message
	void *shared_memory = (void *)0;
	struct buffer_msg *msg;
	int id;

	//counters and size variables
	int loop = 1;
	int n = 0;
	int count = 0;
	int size;

	//delimiting character array
	char endChar[] = "$";

	//buffer to write to file
	char buffer[BUFSIZ];

	//creating the semaphores and setting their values
	sem_id_n = semget((key_t)key_n, 1, 0666 | IPC_CREAT);
	sem_id_s = semget((key_t)key_s, 1, 0666 | IPC_CREAT);
	sem_id_e = semget((key_t)key_e, 1, 0666 | IPC_CREAT);

	if (!set_semvalue(1, sem_id_s)) {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
	
	if (!set_semvalue(0, sem_id_n)) {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
	
	if (!set_semvalue(NBUFFER, sem_id_e)) {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }

	//creating and attaching the shared memory with its address space
	id = shmget((key_t)key, MEMSIZE, 0666|IPC_CREAT);
	
	if(id==-1){
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	shared_memory = shmat(id, (void *)0, 0);
	if(shared_memory == (void *)-1){
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	//stepping through the file read in from the shared memory
	msg = (struct buffer_msg *)shared_memory;
	while (loop){
		//wait until an item is in the shared memory and mutual exclusion is held
		semaphore_p(sem_id_n);
		semaphore_p(sem_id_s); //these can be commented out to achieve the same result if there is only one producer and one consumer
		
		//copy what is in shared memory to the buffer to be written out, while keeping track of the size of each message
		strncpy(buffer+n, msg->buffer[n].msg, TEXTSIZE);
		count += msg->buffer[n].size;

		//write the message from the producer
		size = write(1, msg->buffer[n].msg, msg->buffer[n].size);

		//check to see if the size of the message written is the same as the amount read in
		if (size != msg->buffer[n].size){
			fprintf(stderr, "difference in amount of bytes read and received\n");
			exit(EXIT_FAILURE);
		}

		//check to see if there in the delimiting character
		if (strstr(buffer, endChar) != NULL) {
			loop = 0;
		}

		//signal that it is out of its critical section and a space is free in the shared memory buffer
		semaphore_v(sem_id_s); //these can be commented out to achieve the same result if there is only one producer and one consumer
		semaphore_v(sem_id_e);
		n = (n + 1) % NBUFFER;
	}
	printf("Wrote %i bytes\n", count);
	
}
