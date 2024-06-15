#include <Manager.h>
#include <ServerClient.h>
#include <arpa/inet.h>
#include <printer.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int loop = 1;

static void sigint_handler(__attribute__((unused)) int signo) {
  loop = 0;
}

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

int server_main_loop(int cooking_personel_count, int delivery_personel_count, int port, int k, t_ipAddress ip) {
  struct sockaddr_in address;

  int server_fd, new_socket;
  int opt     = 1;
  int addrlen = sizeof(address);

  if (port == -1)
    port = DEFAULT_PORT;
  if (ip.a[0] == -1 || ip.a[1] == -1 || ip.a[2] == -1 || ip.a[3] == -1) {
    ip.a[0] = 127;
    ip.a[1] = 0;
    ip.a[2] = 0;
    ip.a[3] = 1;
  }

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    return -1;
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    close(server_fd);
    return -1;
  }

  address.sin_family      = AF_INET;
  address.sin_addr.s_addr = ip.a[0] + (ip.a[1] << 8) + (ip.a[2] << 16) + (ip.a[3] << 24);
  address.sin_port        = htons(port);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(server_fd);
    return -1;
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen");
    close(server_fd);
    return -1;
  }

  t_Manager    manager;
  t_OrderDeque startDeque, finishDeque;
  int          is_exit = 1;

  srand(time(0));

  while (loop) {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, 0);

    struct sigaction sa_pipe;
    sa_pipe.sa_handler = SIG_IGN;
    sa_pipe.sa_flags   = 0;
    sigemptyset(&sa_pipe.sa_mask);
    sigaction(SIGPIPE, &sa_pipe, 0);

    LOG(0, "Waiting for connection on %d.%d.%d.%d:%d\n", ip.a[0], ip.a[1], ip.a[2], ip.a[3], port);
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
      continue;
    }
    global_printer.feof = 0;

    int  client_pid     = 0;
    int  size           = 0;
    int  p              = 0;
    int  q              = 0;
    int  str_size       = 0;
    char ip_address[16] = {0};
    read(new_socket, &str_size, sizeof(int));
    if (str_size > 16) {
      LOG(0, "Invalid ip address size\n");
      close(new_socket);
      continue;
    }
    if (!(read(new_socket, ip_address, str_size) && read(new_socket, &client_pid, sizeof(int)) &&
          read(new_socket, &size, sizeof(int)) && read(new_socket, &p, sizeof(int)) &&
          read(new_socket, &q, sizeof(int)))) {
      LOG(0, "Client %d disconnected\n", client_pid);
      LOG(1, "Client %d disconnected\n", client_pid);
      close(new_socket);
      continue;
    }

    printer_set_fd(2, new_socket);

    LOG(0, "Connection established with client pid=%d, ip=%s\n", client_pid, ip_address);
    LOG(1, "Connection established with client pid=%d, ip=%s\n", client_pid, ip_address);
    SEND(2, "Hello client my pid is %d\n", getpid());

    LOG(0, "Client %d requested %d orders with p=%d and q=%d\n", client_pid, size, p, q);
    LOG(1, "Client %d requested %d orders with p=%d and q=%d\n", client_pid, size, p, q);

    if (feof(stdin)) {
      LOG(0, "Exiting...");
      break;
    }

    if (check_feof()) {
      LOG(0, "Client %d disconnected\n", client_pid);
      LOG(1, "Client %d disconnected\n", client_pid);
      close(new_socket);
      continue;
    }

    if (!loop || !is_exit)
      break;

    if (t_OrderDeque_init(&startDeque, size + 1) == -1) {
      LOG(0, "Start deque initialization failed!\n");
      return -1;
    }

    if (t_OrderDeque_init(&finishDeque, size + 1) == -1) {
      LOG(0, "Finish deque initialization failed!\n");
      t_OrderDeque_destroy(&startDeque);
      return -1;
    }

    t_Order order;
    for (int i = 0; i < size; i++) {
      t_Order_init(&order, i, rand() % p, rand() % q);
      t_OrderDeque_enqueue(&startDeque, &order, ORDER_REQUEST_MODE_BLOCKING);
    }

    if (t_Manager_init(&manager, cooking_personel_count, delivery_personel_count, &startDeque, &finishDeque, &is_exit,
                       0, p, q, k) == -1) {
      LOG(0, "Manager initialization failed!\n");
      t_OrderDeque_destroy(&startDeque);
      t_OrderDeque_destroy(&finishDeque);
      return -1;
    }

    if (check_feof()) {
      LOG(0, "Client %d disconnected\n", client_pid);
      LOG(1, "Client %d disconnected\n", client_pid);
      close(new_socket);
      continue;
    }

    t_Manager_destroy(&manager);
    t_OrderDeque_destroy(&startDeque);
    t_OrderDeque_destroy(&finishDeque);

    if (!check_feof()) {
      LOG(0, "Done serving client %d with pid %d, total orders: %d\n", client_pid, getpid(), size);
      LOG(1, "Done serving client %d with pid %d, total orders: %d\n", client_pid, getpid(), size);
    } else {
      LOG(0, "Client %d disconnected\n", client_pid);
      LOG(1, "Client %d disconnected\n", client_pid);
    }
    close(new_socket);
  }

  LOG(0, "Closing server...\n");

  time_t t = time(NULL);
  char   buffer[26];
  ctime_r(&t, buffer);

  LOG(1, "Closing server at %s\n", buffer);
  close(server_fd);

  return 0;
}

static int is_exit_client = 0;

static void sigint_handler_client(__attribute__((unused)) int signo) {
  is_exit_client = 1;
}

int client_main_loop(t_ipAddress ip, int number_of_orders, int p, int q, int port) {
  int                sock = 0;
  struct sockaddr_in serv_addr;
  char               buffer[BUFFER_SIZE] = {0};

  struct sigaction sa;
  sa.sa_handler = sigint_handler_client;
  sa.sa_flags   = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, 0);
  sigaction(SIGPIPE, &sa, 0);

  if (port == -1)
    port = DEFAULT_PORT;
  if (ip.a[0] == -1 || ip.a[1] == -1 || ip.a[2] == -1 || ip.a[3] == -1) {
    ip.a[0] = 127;
    ip.a[1] = 0;
    ip.a[2] = 0;
    ip.a[3] = 1;
  }

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG(0, "\n Socket creation error \n");
    return -1;
  }

  if (port == -1)
    port = DEFAULT_PORT;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port   = htons(port);

  char ip_buffer[16] = {0};
  sprintf(ip_buffer, "%d.%d.%d.%d", ip.a[0], ip.a[1], ip.a[2], ip.a[3]);
  if (inet_pton(AF_INET, ip_buffer, &serv_addr.sin_addr) <= 0) {
    LOG(0, "\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    LOG(0, "\nConnection Failed \n");
    return -1;
  }

  pid_t pid  = getpid();
  int   size = strlen(ip_buffer);
  if (size > 16) {
    LOG(0, "Invalid ip address size\n");
    close(sock);
    return -1;
  }

  write(sock, &size, sizeof(int));
  write(sock, ip_buffer, size);
  write(sock, &pid, sizeof(pid_t));
  write(sock, &number_of_orders, sizeof(int));
  write(sock, &p, sizeof(int));
  write(sock, &q, sizeof(int));

  int text_size = 0;

  while (read(sock, &text_size, sizeof(int)) > 0 && !is_exit_client) {
    if (text_size >= BUFFER_SIZE)
      break;
    if (text_size == -1)
      continue;
    read(sock, buffer, text_size);
    if (strncmp(buffer, "exit", 4) == 0) {
      LOG(0, "Connection closed\n");
      LOG(1, "Connection closed\n");
      break;
    }
    buffer[text_size] = '\0';
    LOG(0, "%s", buffer);
    LOG(1, "%s", buffer);
  }

  LOG(0, "Closing client...\n");
  LOG(1, "Closing client...\n");

  close(sock);
  return 0;
}

static int only_ip(const char *src, t_ipAddress *ip) {
  if (src == 0 || ip == 0)
    return -1;
  if (strchr(src, '.') == 0)
    return -1;

  int index = 0;
  for (int i = 0; i < 4; i++) {
    ip->a[i] = atoi(src + index);
    if (ip->a[i] < 0 || ip->a[i] > 255)
      return -1;
    index = strchr(src + index, '.') - src + 1;
  }

  return 0;
}

static int only_port(const char *src, int *port) {
  if (src == 0 || port == 0)
    return -1;
  if (strchr(src, '.') != 0)
    return -1;
  *port = atoi(src);
  if (*port < 1024 || *port > 65535)
    return -1;

  return 0;
}

int split_ip(const char *src, t_ipAddress *ip, int *port) {
  if (src == 0 || ip == 0 || port == 0)
    return -1;

  const char *ip_end = strchr(src, ':');
  if (ip_end == 0)
    ip_end = src;
  else
    ip_end++;

  return only_ip(src, ip) & only_port(ip_end, port);
}