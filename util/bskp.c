/*
 * BSKP - byte skip : hard coded byte skipping, reads stdin, writes to stdout
 *                    outputs first three bytes from every 4 byte pair
 */

#include <stdio.h>

// Compile:
//   gcc bskp.c -o bskp

int main(void) {
  int c = 0, i;

  while (c!=EOF) {
     for (i=0; i<3 && c!=EOF; i++) { // read+write 3
        c = getc(stdin);
        putc(c, stdout);
     }
     c = getc(stdin); // read+skip 1
  }

  return 0;
}
