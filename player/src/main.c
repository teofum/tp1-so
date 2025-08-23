#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  printf("[player] Hello world! My pid is %d\n", getpid());

  return 0;
}
