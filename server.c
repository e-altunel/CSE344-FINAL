#include <ServerClient.h>
#include <fcntl.h>
#include <input.h>
#include <printer.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc != 5) {
    printf("Usage: %s <ip:port> <cooking_personel_count> <delivery_personel_count> <k>\n", argv[0]);
    return -1;
  }
  int         port = -1;
  t_ipAddress ip   = {0};
  ip.a[0]          = -1;
  if (split_ip(argv[1], &ip, &port) == -1) {
    printf("Invalid ip:port\n");
    return -1;
  }

  int cooking_personel_count  = get_int(argv[2], 1, 100);
  int delivery_personel_count = get_int(argv[3], 1, 100);
  int k                       = get_int(argv[4], 1, 100);

  if (cooking_personel_count == -1 || delivery_personel_count == -1) {
    return -1;
  }

  printer_init();
  printer_set_fd(0, 1);
  printer_set_fd(1, open("server_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644));

  time_t t = time(NULL);
  char   buffer[26];
  ctime_r(&t, buffer);

  LOG(1, "Server started at %s", buffer);

  server_main_loop(cooking_personel_count, delivery_personel_count, port, k, ip);
  close(printer_get_fd(1));
  return 0;
}