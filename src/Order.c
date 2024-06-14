#include <Order.h>

int t_Order_init(t_Order *order, int id, int x, int y) {
  if (order == 0)
    return -1;

  order->id           = id;
  order->is_prepared  = 0;
  order->is_cooked    = 0;
  order->is_delivered = 0;
  order->is_cancelled = 0;

  order->delivery_location.x = x;
  order->delivery_location.y = y;
  return 0;
}