#include <Order.h>

int t_Order_init(t_Order *order, unsigned int id) {
  if (order == 0)
    return -1;

  order->id           = id;
  order->is_prepared  = 0;
  order->is_cooked    = 0;
  order->is_delivered = 0;
  return 0;
}