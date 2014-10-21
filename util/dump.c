/* 3-out-of-4 Tsunami mode dumping tool */

// Compile:
//   gcc dump.c -o dump
#include <stdio.h>

int main(void)
{
  int c;
  int i=0, j=0;
  int addr = 0;

  printf("%04X: ", addr);
  while ( EOF!=(c=getc(stdin)) ) {
     printf("%02X ", c);
     i++; addr++;
     if(i>=3) {
        j++;
        if (j>=4) { j=0; printf("\n%04X: ", addr); }
        else { printf("    "); }
        i = 0;
     }
  }

}

