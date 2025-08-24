#include <stdio.h>
#include <unistd.h>

void logpid() { printf("[player: %d] ", getpid()); }

int main(int argc, char **argv) {
  fprintf(stderr, "[player: %d] Waiting for master input...\n", getpid());
  char buf[256];
  int read_bytes = read(0, buf, 256);
  buf[read_bytes] = 0;

  logpid();
  printf("Master says \"%s\"", buf);

  return 0;
}
