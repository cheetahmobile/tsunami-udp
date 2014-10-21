#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define DIFX_RECEIVE_RING_LENGTH 4

int main(int argc, char *argv[])
{
    struct timeval t_start, t_isopen, t_closed;
    double tdelta, tsleeps;
    unsigned long  randsleep;
    static u_char *block;
    int    i, j;

    FILE*     file;
    u_int32_t readbytes;
    u_int64_t totalread;

    if (argc <= 1) {
       printf("Usage: ./fusereadtest filename [readsize in bytes]\n\n"); 
       return 0;
    }

    if (sizeof(off_t) != 8) {
       printf("Warning: Not compiled with 64-bit Large File Support, results can be unreliable\n");
    }

    if (argc > 2) {
       readbytes = atoi(argv[2]);
    } else {
       readbytes = 1250*512*2*sizeof(float); // DiFX blockspersend=1250, 512pt FFT, complex
    }
    block = (u_char *) malloc(readbytes);

    gettimeofday(&t_start, NULL);
    srand(*((unsigned int*)&t_start));
    file = fopen(argv[1], "r");

    totalread = 0; 
    tsleeps = 0.0;

    gettimeofday(&t_isopen, NULL);
    while (1) {
       j = rand() / (RAND_MAX/DIFX_RECEIVE_RING_LENGTH);
       for (i = 0; i < j; i++) {
          totalread += fread(block, 1, readbytes, file);
       }
       if (feof(file))
          break;
       randsleep = (10000LL * 100LL * rand())/ RAND_MAX; // 10ms * 1..100 as usecs
       tsleeps += 1e-6 * randsleep;
       usleep(randsleep);
    } 
    fclose(file);

    gettimeofday(&t_closed, NULL);
    tdelta = (t_closed.tv_sec - t_start.tv_sec) + 1e-6 * (t_closed.tv_usec - t_start.tv_usec);

	printf("Start    = %lu.%06lu\n",  (unsigned long)t_start.tv_sec, (unsigned long)t_start.tv_usec);
	printf("Opened   = %lu.%06lu\n",  (unsigned long)t_isopen.tv_sec, (unsigned long)t_isopen.tv_usec);
	printf("Finished = %lu.%06lu\n",  (unsigned long)t_closed.tv_sec, (unsigned long)t_closed.tv_usec);
	printf("Delta    = %0.3lf sec\n", tdelta);
    printf("Sleeps   = %0.3lf sec\n", tsleeps);
	printf("Speed    = %0.3lf Mbps\n",  totalread * 8.0 / (tdelta * 1024 * 1024));

    return 0;
}

