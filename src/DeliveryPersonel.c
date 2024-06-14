#include <DeliveryPersonel.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int t_DeliveryPersonel_init(t_DeliveryPersonel *personel, int id, t_OrderDeque *deque, t_OrderDeque *finishedDeque,
                            int order_cap) {
  if (personel == 0 || deque == 0 || finishedDeque == 0)
    return -1;

  personel->id                    = id;
  personel->deque_ref             = deque;
  personel->finishedDeque_ref     = finishedDeque;
  personel->order_cap             = order_cap;
  personel->active_order_count    = 0;
  personel->is_exit               = 0;
  personel->first_failed_time     = 0;
  personel->does_failed           = 0;
  personel->delivered_order_count = 0;

  personel->active_orders = (t_Order *)malloc(sizeof(t_Order) * order_cap);
  if (personel->active_orders == 0)
    return -1;

  if (pthread_mutex_init(&personel->variable_mutex, 0) != 0) {
    free(personel->active_orders);
    return -1;
  }

  if (pthread_create(&personel->deliver_thread, 0, t_DeliveryPersonel_deliver_thread, personel) != 0) {
    pthread_mutex_destroy(&personel->variable_mutex);
    free(personel->active_orders);
    return -1;
  }
  return 0;
}

void t_DeliveryPersonel_destroy(t_DeliveryPersonel *personel) {
  if (personel == 0)
    return;

  personel->is_exit = 1;

  pthread_join(personel->deliver_thread, 0);

  printf("Delivery personel %d delivered %d orders\n", personel->id, personel->delivered_order_count);
  free(personel->active_orders);
}

void t_DeliveryPersonel_set_exit(t_DeliveryPersonel *personel) {
  if (personel == 0)
    return;
  personel->is_exit = 1;
}

void *t_DeliveryPersonel_deliver_thread(void *arg) {
  t_DeliveryPersonel *personel = (t_DeliveryPersonel *)arg;

  while (personel->is_exit == 0 || t_DeliveryPersonel_get_active_order_count(personel)) {
    if (t_DeliveryPersonel_get_active_order_count(personel) == personel->order_cap ||
        (personel->does_failed && personel->first_failed_time + DELIVERY_PERSONEL_MAX_WAIT_TIME < time(0))) {
      t_DeliveryPersonel_deliver(personel);
      continue;
    }

    t_DeliveryPersonel_get_order(personel);
  }

  return 0;
}

void t_DeliveryPersonel_deliver(t_DeliveryPersonel *personel) {
  while (t_DeliveryPersonel_get_active_order_count(personel) != 0) {
    int delivery_time = personel->active_orders[0].delivery_location.x + personel->active_orders[0].delivery_location.y;
    for (int j = 0; j < delivery_time; j++) {
      pthread_mutex_lock(&personel->variable_mutex);
      if (personel->active_orders[0].is_cancelled) {
        pthread_mutex_unlock(&personel->variable_mutex);
        break;
      }
      pthread_mutex_unlock(&personel->variable_mutex);
      usleep(10000);
    }
    pthread_mutex_lock(&personel->variable_mutex);
    if (!personel->active_orders[0].is_cancelled) {
      pthread_mutex_unlock(&personel->variable_mutex);
      t_OrderDeque_enqueue(personel->finishedDeque_ref, &personel->active_orders[0], ORDER_REQUEST_MODE_BLOCKING);
      pthread_mutex_lock(&personel->variable_mutex);
      printf("Delivery personel %d delivered order %d\n", personel->id, personel->active_orders[0].id);
      personel->delivered_order_count++;
    }
    for (int i = 0; i < personel->active_order_count - 1; i++) {
      personel->active_orders[i] = personel->active_orders[i + 1];
    }
    personel->active_order_count--;
    pthread_mutex_unlock(&personel->variable_mutex);
  }
}

int t_DeliveryPersonel_cancel(t_DeliveryPersonel *personel, int order_id) {
  pthread_mutex_lock(&personel->variable_mutex);

  int has_cancelled = -1;
  for (int i = 0; i < personel->active_order_count; i++) {
    if (personel->active_orders[i].id == order_id || order_id == -1) {
      has_cancelled = 0;

      personel->active_orders[i].is_cancelled = 1;
      if (order_id != -1)
        break;
    }
  }

  pthread_mutex_unlock(&personel->variable_mutex);
  return has_cancelled;
}

int t_DeliveryPersonel_get_active_order_count(t_DeliveryPersonel *personel) {
  pthread_mutex_lock(&personel->variable_mutex);
  int count = personel->active_order_count;
  pthread_mutex_unlock(&personel->variable_mutex);
  return count;
}

int t_DeliveryPersonel_get_order(t_DeliveryPersonel *personel) {
  if (personel == 0)
    return -1;

  t_OrderDeque *deque = personel->deque_ref;

  pthread_mutex_lock(&personel->variable_mutex);

  if (personel->active_order_count == personel->order_cap) {
    pthread_mutex_unlock(&personel->variable_mutex);
    return -1;
  }

  t_Order *order = 0;
  if (t_OrderDeque_dequeue(deque, &order, ORDER_REQUEST_MODE_NON_BLOCKING)) {
    if (!personel->does_failed) {
      personel->does_failed       = 1;
      personel->first_failed_time = time(0);
    }
    pthread_mutex_unlock(&personel->variable_mutex);
    usleep(1000);
    return -1;
  }
  personel->first_failed_time = 0;
  personel->does_failed       = 0;

  personel->active_orders[personel->active_order_count] = *order;
  personel->active_order_count++;
  free(order);
  printf("Delivery personel %d took order\n", personel->id);

  pthread_mutex_unlock(&personel->variable_mutex);
  return 0;
}
