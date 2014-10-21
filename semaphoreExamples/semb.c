/* Semaphore example program b (semb.c) */
/* We have two programs, sema and semb. Semb may be initiated at any 
  time, but will be forced to wait until sema is executed. Sema and
  semb do not have to be executed by the same user! */

/* HOW TO TEST:
   Execute semb &
   The & is important - otherwise you would have have to move to
   a different terminal to execute sema.

   Then execute sema.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define KEY (1492)
/* This is the external name by which the semaphore is known to any
   program that wishes to access it. */

void main()
{
   int id;  /* Internal identifier of the semaphore. */
   struct sembuf operations[1];
   /* An "array" of one operation to perform on the semaphore. */

   int retval; /* Return value from semop() */

   /* Get the index for the semaphore with external name KEY. */
   id = semget(KEY, 1, 0666);
   if(id < 0)
   /* Semaphore does not exist. */
   {
      fprintf(stderr, "Program semb cannot find semaphore, exiting.\n");
      exit(0);
   }

   /* Do a semaphore P-operation. */
   printf("Program semb about to do a P-operation. \n");
   printf("Process id is %d\n", getpid());

   /* Set up the sembuf structure. */
   /* Which semaphore in the semaphore array : */
    operations[0].sem_num = 0;
    /* Which operation? Subtract 1 from semaphore value : */
    operations[0].sem_op = -1;
    /* Set the flag so we will wait : */   
    operations[0].sem_flg = 0;

    /* So do the operation! */
    retval = semop(id, operations, 1);

    if(retval == 0)
    {
       printf("Successful P-operation by program semb.\n");
       printf("Process id is %d\n", getpid());
    }
    else
    {
       printf("semb: P-operation did not succeed.\n");
    }
}

/* Think carefully about what the V-operation does. If sema is executed 
   twice, then semb can execute twice. */
