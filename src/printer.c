#include <printer.h>
#include <stdio.h>

t_printer global_printer;

int printer_init() {
  for (int i = 0; i < FD_COUNT; i++) {
    global_printer.fd[i] = -1;
  }
  global_printer.feof = 0;
  return pthread_mutex_init(&global_printer.printer_mutex, 0);
}

void printer_destroy() {
  pthread_mutex_destroy(&global_printer.printer_mutex);
}

void printer_set_fd(int index, int fd) {
  pthread_mutex_lock(&global_printer.printer_mutex);
  global_printer.fd[index] = fd;
  pthread_mutex_unlock(&global_printer.printer_mutex);
}

int printer_get_fd(int index) {
  return global_printer.fd[index];
}

int check_feof() {
  pthread_mutex_lock(&global_printer.printer_mutex);
  int tmp = -1;
  if (write(global_printer.fd[2], &tmp, sizeof(int)) != sizeof(int)) {
    global_printer.feof = 1;
    pthread_mutex_unlock(&global_printer.printer_mutex);
    return 1;
  }
  pthread_mutex_unlock(&global_printer.printer_mutex);
  return 0;
}