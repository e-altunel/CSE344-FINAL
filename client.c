#include <ServerClient.h>
#include <fcntl.h>
#include <input.h>
#include <printer.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc != 5) {
    printf("Usage: %s <ip:port> <number_of_orders> <p> <q>\n", argv[0]);
    return -1;
  }
  int         port = -1;
  t_ipAddress ip   = {0};
  ip.a[0]          = -1;
  if (split_ip(argv[1], &ip, &port) == -1) {
    printf("Invalid ip:port\n");
    return -1;
  }

  int number_of_orders = get_int(argv[2], 1, 1000);
  int p                = get_int(argv[3], 1, 100);
  int q                = get_int(argv[4], 1, 100);

  if (number_of_orders == -1 || p == -1 || q == -1) {
    return -1;
  }

  printer_init();
  printer_set_fd(0, 1);
  printer_set_fd(1, open("client_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644));

  LOG(0, "Client started with pid %d\n", getpid());

  time_t t = time(NULL);
  char   buffer[26];
  ctime_r(&t, buffer);

  LOG(1, "Client started with pid %d at %s\n", getpid(), buffer);

  client_main_loop(ip, number_of_orders, p, q, port);

  close(printer_get_fd(1));

  return 0;
}