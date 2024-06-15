#include <DeliveryPersonelPool.h>
#include <poll.h>
#include <printer.h>
#include <stdlib.h>
#include <unistd.h>

int t_DeliveryPersonelPool_init(t_DeliveryPersonelPool *pool, int personel_count, int deque_size, int order_cap,
                                t_OrderDeque *startDeque, t_OrderDeque *finishedDeque, t_Manager *manager) {
  if (pool == 0 || personel_count <= 0 || deque_size <= 0 || order_cap <= 0 || startDeque == 0 || finishedDeque == 0)
    return -1;

  pool->personel_count = personel_count;
  pool->startDeque     = startDeque;
  pool->finishedDeque  = finishedDeque;
  pool->personels      = (t_DeliveryPersonel *)malloc(sizeof(t_DeliveryPersonel) * personel_count);
  pool->manager        = manager;

  if (pool->personels == 0)
    return -1;

  for (int i = 0; i < personel_count; i++) {
    if (t_DeliveryPersonel_init(&pool->personels[i], i, pool->startDeque, pool->finishedDeque, order_cap, manager) !=
        0) {
      for (int j = 0; j < i; j++)
        t_DeliveryPersonel_destroy(&pool->personels[j]);
      free(pool->personels);
      return -1;
    }
  }

  return 0;
}

void t_DeliveryPersonelPool_destroy(t_DeliveryPersonelPool *pool, int *loop, int *is_exit) {
  t_DeliveryPersonelPool_wait(pool, loop, is_exit);
  for (int i = 0; i < pool->personel_count; i++)
    t_DeliveryPersonel_set_exit(&pool->personels[i]);
  int best_personel, best_delivered = 0;
  for (int i = 0; i < pool->personel_count; i++) {
    if (pool->personels[i].delivered_order_count > best_delivered) {
      best_personel  = i;
      best_delivered = pool->personels[i].delivered_order_count;
    }
  }
  for (int i = 0; i < pool->personel_count; i++)
    t_DeliveryPersonel_destroy(&pool->personels[i]);
  int tmp = -1;
  if (write(global_printer.fd[2], &tmp, sizeof(int)) == sizeof(int)) {
    LOG(0, "Best of Delivery: Delivery Personel %d delivered %d orders\n", best_personel, best_delivered);
    LOG(1, "Best of Delivery: Delivery Personel %d delivered %d orders\n", best_personel, best_delivered);
  }
  free(pool->personels);
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

  result &= t_OrderDeque_cancel(pool->finishedDeque, order_id);
  if (result != -1 && order_id != -1)
    return result;

  for (int i = 0; i < pool->personel_count; i++) {
    if ((result &= t_DeliveryPersonel_cancel(&pool->personels[i], order_id)) == 0 && order_id != -1)
      return 0;
  }

  return result;
}

void t_DeliveryPersonelPool_wait(t_DeliveryPersonelPool *pool, int *loop, int *is_exit) {
  int remaining_orders = t_DeliveryPersonelPool_remaining(pool);

  struct pollfd fds = {0};
  fds.fd            = 0;
  fds.events        = POLLIN;

  while (remaining_orders > 0) {
    usleep(10000);
    remaining_orders = t_DeliveryPersonelPool_remaining(pool);
    if (loop != 0 && *loop == 0) {
      t_DeliveryPersonelPool_cancel_order(pool, -1);
      break;
    }
    if (is_exit != 0 && *is_exit == 0) {
      t_DeliveryPersonelPool_cancel_order(pool, -1);
      break;
    }
    if (poll(&fds, 1, 0) > 0 && (fds.revents & POLLIN)) {
      t_DeliveryPersonelPool_cancel_order(pool, -1);
      break;
    }
    if (check_feof()) {
      t_DeliveryPersonelPool_cancel_order(pool, -1);
      break;
    }
  }
}

int t_DeliveryPersonelPool_remaining(t_DeliveryPersonelPool *pool) {
  int remaining_orders = t_OrderDeque_size(pool->startDeque);
  for (int i = 0; i < pool->personel_count; i++)
    remaining_orders += t_DeliveryPersonel_get_active_order_count(&pool->personels[i]);
  return remaining_orders;
}