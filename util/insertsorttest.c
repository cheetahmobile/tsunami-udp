// gcc insertsorttest.c -o insertsorttest

int insTab[8192];
int maxidx;

void dumpTab() {
  int i = 0;
  printf(" maxidx=%d  ", maxidx);
  while(i<maxidx && i<64) {
     printf(" [%d]=%d", i, insTab[i]);
     i++;
  }
  printf("\n");
}

void insert(int val) {
  int i = 0, ins = 0, up = 0;
  while ((i < maxidx) && (insTab[i] < val)) { i++; }
  if (i == maxidx) { insTab[maxidx++] = val; }
  else if (insTab[i]==val) { printf("duplicate at idx %d\n", i); }
  else {
    ins = val;
    do {
      up = insTab[i]; insTab[i++] = ins; ins = up;
    } while(i <= maxidx);
    maxidx++;
  }
  return;
}

int main(void)
{
  for(maxidx=0; maxidx<8192; maxidx++) { insTab[maxidx] = 0; }

  maxidx = 0;
  printf("insert(1); insert(2); insert(5); insert(7);\n");
  insert(1); insert(2); insert(5); insert(7);
  dumpTab();
  printf("insert(5); insert(4);\n");
  insert(5); insert(4);
  dumpTab();
  printf("insert(0); insert(6);\n");
  insert(0); insert(6);
  dumpTab();
  printf("insert(7);\n");
  insert(7);
  dumpTab();

  return 0;
}

