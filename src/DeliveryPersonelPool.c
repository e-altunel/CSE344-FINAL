#include <DeliveryPersonelPool.h>
#include <stdlib.h>
#include <unistd.h>

int t_DeliveryPersonelPool_init(t_DeliveryPersonelPool *pool, int personel_count, int deque_size, int order_cap,
                                t_OrderDeque *startDeque) {
  if (pool == 0 || personel_count <= 0 || deque_size <= 0 || order_cap <= 0 || startDeque == 0)
    return -1;

  pool->personel_count = personel_count;
  pool->startDeque     = startDeque;
  pool->personels      = (t_DeliveryPersonel *)malloc(sizeof(t_DeliveryPersonel) * personel_count);
  if (pool->personels == 0)
    return -1;

  if (t_OrderDeque_init(&pool->finishedDeque, deque_size) != 0) {
    free(pool->personels);
    return -1;
  }

  for (int i = 0; i < personel_count; i++) {
    if (t_DeliveryPersonel_init(&pool->personels[i], i, pool->startDeque, &pool->finishedDeque, order_cap) != 0) {
      t_OrderDeque_destroy(&pool->finishedDeque);
      for (int j = 0; j < i; j++)
        t_DeliveryPersonel_destroy(&pool->personels[j]);
      free(pool->personels);
      return -1;
    }
  }

  return 0;
}

void t_DeliveryPersonelPool_destroy(t_DeliveryPersonelPool *pool) {
  t_DeliveryPersonelPool_wait(pool);
  for (int i = 0; i < pool->personel_count; i++)
    t_DeliveryPersonel_set_exit(&pool->personels[i]);
  for (int i = 0; i < pool->personel_count; i++)
    t_DeliveryPersonel_destroy(&pool->personels[i]);
  free(pool->personels);
  t_OrderDeque_destroy(&pool->finishedDeque);
}

int t_DeliveryPersonelPool_add_order(t_DeliveryPersonelPool *pool, t_Order *order, t_OrderRequestMode mode) {
  if (pool == 0 || order == 0)
    return -1;
  return t_OrderDeque_enqueue(pool->startDeque, order, mode);
}

int t_DeliveryPersonelPool_cancel_order(t_DeliveryPersonelPool *pool, int order_id) {
  if (pool == 0)
    return -1;

  int result = t_OrderDeque_cancel(pool->startDeque, order_id);
  if (result != -1 && order_id != -1)
    return result;

  result &= t_OrderDeque_cancel(&pool->finishedDeque, order_id);
  if (result != -1 && order_id != -1)
    return result;

  for (int i = 0; i < pool->personel_count; i++) {
    if ((result &= t_DeliveryPersonel_cancel(&pool->personels[i], order_id)) == 0 && order_id != -1)
      return 0;
  }

  return result;
}

void t_DeliveryPersonelPool_wait(t_DeliveryPersonelPool *pool) {
  int remaining_orders = t_DeliveryPersonelPool_remaining(pool);
  while (remaining_orders > 0) {
    usleep(100000);
    remaining_orders = t_DeliveryPersonelPool_remaining(pool);
  }
}

int t_DeliveryPersonelPool_remaining(t_DeliveryPersonelPool *pool) {
  int remaining_orders = t_OrderDeque_size(pool->startDeque);
  for (int i = 0; i < pool->personel_count; i++)
    remaining_orders += t_DeliveryPersonel_get_active_order_count(&pool->personels[i]);
  return remaining_orders;
}