#ifndef INC_ORDER
#define INC_ORDER

#include <semaphore.h>

typedef struct s_Order {
  int id;

  int is_prepared;
  int is_cooked;
  int is_delivered;
  int is_cancelled;
} t_Order;

int t_Order_init(t_Order *order, int id);

#endif /* INC_ORDER */
