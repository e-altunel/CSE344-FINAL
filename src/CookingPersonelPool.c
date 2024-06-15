#include <CookingPersonelPool.h>
#include <Manager.h>
#include <poll.h>
#include <printer.h>
#include <stdlib.h>
#include <unistd.h>

int t_CookingPersonelPool_init(t_CookingPersonelPool *pool, int personel_count, t_CookOven *oven, int deque_size,
                               t_OrderDeque *startDeque, t_OrderDeque *finishedDeque, t_Manager *manager) {
  if (pool == 0 || personel_count <= 0 || oven == 0 || deque_size <= 0 || finishedDeque == 0 || startDeque == 0)
    return -1;

  pool->personels = (t_CookingPersonel *)malloc(personel_count * sizeof(t_CookingPersonel));
  if (pool->personels == 0)
    return -1;

  pool->personel_count = personel_count;
  pool->oven           = oven;
  pool->startDeque     = startDeque;
  pool->finishedDeque  = finishedDeque;
  pool->manager        = manager;

  for (int i = 0; i < personel_count; i++) {
    if (t_CookingPersonel_init(&pool->personels[i], i, oven, pool->startDeque, pool->finishedDeque, manager) == -1) {
      for (int j = 0; j < i; j++)
        t_CookingPersonel_destroy(&pool->personels[j]);
      free(pool->personels);
      return -1;
    }
  }

  return 0;
}

void t_CookingPersonelPool_destroy(t_CookingPersonelPool *pool, int *loop, int *is_exit) {
  t_CookingPersonelPool_wait(pool, loop, is_exit);
  for (int i = 0; i < pool->personel_count; i++)
    t_CookingPersonel_set_exit(&pool->personels[i]);
  int best_personel, best_cooked = 0;
  for (int i = 0; i < pool->personel_count; i++) {
    if (pool->personels[i].cooked_count > best_cooked) {
      best_personel = i;
      best_cooked   = pool->personels[i].cooked_count;
    }
  }
  for (int i = 0; i < pool->personel_count; i++)
    t_CookingPersonel_destroy(&pool->personels[i]);

  int tmp = -1;
  if (write(global_printer.fd[2], &tmp, sizeof(int)) == sizeof(int)) {
    LOG(0, "Best of Cooking: Cooking Personel %d cooked %d orders\n", best_personel, best_cooked);
    LOG(1, "Best of Cooking: Cooking Personel %d cooked %d orders\n", best_personel, best_cooked);
  }
  free(pool->personels);
}

int t_CookingPersonelPool_add_order(t_CookingPersonelPool *pool, t_Order *order, t_OrderRequestMode mode) {
  if (pool == 0 || order == 0)
    return -1;
  return t_OrderDeque_enqueue(pool->startDeque, order, mode);
}

int t_CookingPersonelPool_cancel_order(t_CookingPersonelPool *pool, int order_id) {
  if (pool == 0)
    return -1;

  int result = t_OrderDeque_cancel(pool->startDeque, order_id);
  if (result != -1 && order_id != -1)
    return result;

  result &= t_OrderDeque_cancel(pool->finishedDeque, order_id);
  if (result != -1 && order_id != -1)
    return result;

  for (int i = 0; i < pool->personel_count; i++)
    if ((result &= t_CookingPersonel_cancel(&pool->personels[i], order_id)) == 0 && order_id != -1)
      return 0;

  return result;
}

void t_CookingPersonelPool_wait(t_CookingPersonelPool *pool, int *loop, int *is_exit) {
  int remaining_orders = t_CookingPersonelPool_remaining(pool);

  struct pollfd fds = {0};
  fds.fd            = 0;
  fds.events        = POLLIN;

  while (remaining_orders > 0) {
    usleep(10000);
    remaining_orders = t_CookingPersonelPool_remaining(pool);
    if (loop != 0 && *loop == 0) {
      t_CookingPersonelPool_cancel_order(pool, -1);
      break;
    }
    if (is_exit != 0 && *is_exit == 0) {
      t_CookingPersonelPool_cancel_order(pool, -1);
      break;
    }
    if (poll(&fds, 1, 0) > 0 && (fds.revents & POLLIN)) {
      t_CookingPersonelPool_cancel_order(pool, -1);
      break;
    }
    if (check_feof()) {
      t_CookingPersonelPool_cancel_order(pool, -1);
      break;
    }
  }
}

int t_CookingPersonelPool_remaining(t_CookingPersonelPool *pool) {
  int remaining_orders = t_OrderDeque_size(pool->startDeque);
  for (int i = 0; i < pool->personel_count; i++)
    remaining_orders += t_CookingPersonel_has_active_order(&pool->personels[i]) +
                        t_CookingPersonel_has_pending_order(&pool->personels[i]);
  return remaining_orders;
}