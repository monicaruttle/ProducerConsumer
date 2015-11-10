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

/* The del_semvalue function has almost the same form, except the call to semctl uses
 the command IPC_RMID to remove the semaphore's ID. */

static void del_semvalue(int sem_id)
{
    union semun sem_union;
    
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
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
	//counters and size variables
	int nread;
	int n = 0;

	//buffer to read in from the file
	char buffer[BUFSIZ];

	//declaring shared memory space and message
	void *shared_memory = (void *)0;
	struct buffer_msg *msg;
	int id;

	//declaring the necessary semaphore ids
	int sem_id_s;
	int sem_id_n;
	int sem_id_e;

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
	
	//reading in the message from the file
	nread = read(0, buffer, BUFSIZ); //BUFSIZ is from stdio.h
	if(nread==-1){
		fprintf(stderr, "nread failed\n");
		exit(EXIT_FAILURE);
	}

	//iterate through the read in message
	int i = 0;
	int count = 0;
	msg = (struct buffer_msg *)shared_memory;
	while(n<nread){
		//wait to be able to add something into the shared memory buffer (so it does not overflow) and ensure mutual exclusion is held
		semaphore_p(sem_id_e);
		semaphore_p(sem_id_s); //these can be commented out to achieve the same result if there is only one producer and one consumer
		//copy what is in the read in buffer to shared memory
		strncpy(msg->buffer[i].msg, buffer+n, TEXTSIZE);
		//put the size of the message in shared memory
		msg->buffer[i].size = strlen(msg->buffer[i].msg);
		count+=msg->buffer[i].size;
		//signal that something is in the shared memory buffer and that it has left the critical section
		semaphore_v(sem_id_s); //these can be commented out to achieve the same result if there is only one producer and one consumer
		semaphore_v(sem_id_n);
		i = (i + 1) % NBUFFER;
		n += TEXTSIZE;
	}
	printf("Read in %i bytes\n", count);
}
