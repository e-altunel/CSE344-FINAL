#ifndef INC_ORDER
#define INC_ORDER

#include <semaphore.h>

typedef struct s_Order {
  unsigned int id;

  int is_prepared;
  int is_cooked;
  int is_delivered;
} t_Order;

int t_Order_init(t_Order *order, unsigned int id);

#endif /* INC_ORDER */
