#ifndef INC_SERVERCLIENT
#define INC_SERVERCLIENT

typedef struct t_ipAddress {
  int a[4];
} t_ipAddress;

int server_main_loop(int cooking_personel_count, int delivery_personel_count, int port, int k, t_ipAddress ip);
int client_main_loop(t_ipAddress ip, int number_of_orders, int p, int q, int port);
int split_ip(const char *src, t_ipAddress *ip, int *port);

#endif /* INC_SERVERCLIENT */
