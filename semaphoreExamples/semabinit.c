/* semabinit.c - initialize a semaphore for use by programs sema and semb */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

/* The semaphore key is an arbitrary long integer which serves as an
   external identifier by which the semaphore is known to any program
   that wishes to use it. */

#define KEY (1492)

void main()
{
   int id; /* Number by which the semaphore is known within a program */

   /* The next thing is an argument to the semctl() function. Semctl() 
      does various things to the semaphore depending on which arguments
      are passed. We will use it to make sure that the value of the 
      semaphore is initially 0. */

	union semun {
		int val;
		struct semid_ds *buf;
		ushort * array;
	} argument;

   argument.val = 0;

   /* Create the semaphore with external key KEY if it doesn't already 
      exists. Give permissions to the world. */

   id = semget(KEY, 1, 0666 | IPC_CREAT);

   /* Always check system returns. */

   if(id < 0)
   {
      fprintf(stderr, "Unable to obtain semaphore.\n");
      exit(0);
   }

   /* What we actually get is an array of semaphores. The second 
      argument to semget() was the array dimension - in our case
      1. */

   /* Set the value of the number 0 semaphore in semaphore array
      # id to the value 0. */

   if( semctl(id, 0, SETVAL, argument) < 0)
   {
      fprintf( stderr, "Cannot set semaphore value.\n");
   }
   else
   {
      fprintf(stderr, "Semaphore %d initialized.\n", KEY);
   }
}
