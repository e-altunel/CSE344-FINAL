#include <unistd.h>

#ifndef SRC_MANAGER
#define SRC_MANAGER
#endif /* SRC_MANAGER */

#include <Manager.h>
#include <printer.h>
#include <signal.h>

int t_Manager_init(t_Manager *manager, int cooking_personel_count, int delivery_personel_count,
                   t_OrderDeque *startDeque, t_OrderDeque *finishDeque, int *loop, int *is_cancel, int p, int q,
                   int k) {
  if (manager == 0 || startDeque == 0 || finishDeque == 0 || cooking_personel_count <= 0 ||
      delivery_personel_count <= 0)
    return -1;

  manager->is_exit     = 0;
  manager->startDeque  = startDeque;
  manager->finishDeque = finishDeque;
  manager->loop        = loop;
  manager->is_cancel   = is_cancel;
  manager->p           = p;
  manager->q           = q;
  manager->k           = k;

  if (t_OrderDeque_init(&manager->sharedDeque, 10) == -1)
    return -1;

  if (t_CookOven_init(&manager->oven, 3, 6) == -1) {
    t_OrderDeque_destroy(&manager->sharedDeque);
    return -1;
  }

  if (t_CookingPersonelPool_init(&manager->cookingPersonelPool, cooking_personel_count, &manager->oven, 100,
                                 manager->startDeque, &manager->sharedDeque, manager) == -1) {
    t_CookOven_destroy(&manager->oven);
    t_OrderDeque_destroy(&manager->sharedDeque);
    return -1;
  }

  if (t_DeliveryPersonelPool_init(&manager->deliveryPersonelPool, delivery_personel_count, 100, 3,
                                  &manager->sharedDeque, manager->finishDeque, manager) == -1) {
    t_CookOven_destroy(&manager->oven);
    t_CookingPersonelPool_destroy(&manager->cookingPersonelPool, 0, 0);
    t_OrderDeque_destroy(&manager->sharedDeque);
    return -1;
  }

  if (pthread_create(&manager->managerThread, 0, t_Manager_thread, manager) != 0) {
    t_CookOven_destroy(&manager->oven);
    t_CookingPersonelPool_destroy(&manager->cookingPersonelPool, 0, 0);
    t_DeliveryPersonelPool_destroy(&manager->deliveryPersonelPool, 0, 0);
    t_OrderDeque_destroy(&manager->sharedDeque);
    return -1;
  }

  return 0;
}

void t_Manager_destroy(t_Manager *manager) {
  if (manager == 0)
    return;

  manager->is_exit = 1;

  pthread_join(manager->managerThread, 0);

  t_CookingPersonelPool_wait(&manager->cookingPersonelPool, manager->loop, manager->is_cancel);
  t_DeliveryPersonelPool_wait(&manager->deliveryPersonelPool, manager->loop, manager->is_cancel);

  t_CookingPersonelPool_destroy(&manager->cookingPersonelPool, manager->loop, manager->is_cancel);
  t_DeliveryPersonelPool_destroy(&manager->deliveryPersonelPool, manager->loop, manager->is_cancel);

  t_CookOven_destroy(&manager->oven);
  t_OrderDeque_destroy(&manager->sharedDeque);
}

static t_Manager *manager;

static void t_Manager_signal_exit(int signum) {
  (void)signum;
  LOG(0, "\033[31mExiting...\033[0m\n");
  t_Manager_cancel_all(manager);
  loop = 0;
}

void *t_Manager_thread(void *arg) {
  manager = (t_Manager *)arg;

  struct sigaction sa;
  sa.sa_handler = t_Manager_signal_exit;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, 0);

  struct sigaction sa_pipe;
  sa_pipe.sa_handler = SIG_IGN;
  sigemptyset(&sa_pipe.sa_mask);
  sa_pipe.sa_flags = 0;
  sigaction(SIGPIPE, &sa_pipe, 0);

  while (manager->is_exit == 0) {
    usleep(10000);
    if (feof(stdin)) {
      t_Manager_cancel_all(manager);
      LOG(0, "Exiting due to stdin EOF\n");
      LOG(1, "Exiting due to stdin EOF\n");
      break;
    }
    int tmp = -1;
    if (write(global_printer.fd[2], &tmp, sizeof(int)) == 0) {
      t_Manager_cancel_all(manager);
      LOG(0, "Client disconnected\n");
      LOG(1, "Client disconnected\n");
      break;
    }
  }
  return 0;
}

void t_Manager_cancel_all(t_Manager *manager) {
  if (manager == 0)
    return;
  LOG(0, "\033[31mCancelling all orders\033[0m\n");
  *(manager->loop) = 0;
  manager->is_exit = 1;
  t_CookingPersonelPool_cancel_order(&manager->cookingPersonelPool, -1);
  t_DeliveryPersonelPool_cancel_order(&manager->deliveryPersonelPool, -1);
}
