#define _DEFAULT_SOURCE

#include <CookingPersonel.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int t_CookingPersonel_init(t_CookingPersonel *personel, int id, t_CookOven *oven, t_OrderDeque *deque,
                           t_OrderDeque *finishedDeque) {
  if (personel == 0 || oven == 0 || deque == 0)
    return -1;

  personel->id                = id;
  personel->deque_ref         = deque;
  personel->finishedDeque_ref = finishedDeque;
  personel->oven_ref          = oven;
  personel->active_order      = 0;
  personel->pending_order     = 0;
  personel->is_exit           = 0;
  personel->is_cancelled      = 0;
  personel->cooked_count      = 0;

  if (pthread_mutex_init(&personel->job_lock, 0) != 0)
    return -1;

  if (pthread_mutex_init(&personel->variable_lock, 0) != 0) {
    pthread_mutex_destroy(&personel->job_lock);
    return -1;
  }

  if (pthread_create(&personel->prepare_thread, 0, t_CookingPersonel_prepare_thread, personel) != 0) {
    pthread_mutex_destroy(&personel->variable_lock);
    pthread_mutex_destroy(&personel->job_lock);
    return -1;
  }

  if (pthread_create(&personel->cook_thread, 0, t_CookingPersonel_cook_thread, personel) != 0) {
    pthread_cancel(personel->prepare_thread);
    pthread_mutex_destroy(&personel->variable_lock);
    pthread_mutex_destroy(&personel->job_lock);
    return -1;
  }

  return 0;
}

void t_CookingPersonel_destroy(t_CookingPersonel *personel) {
  if (personel == 0)
    return;

  personel->is_exit = 1;

  pthread_join(personel->prepare_thread, 0);
  pthread_join(personel->cook_thread, 0);

  pthread_mutex_destroy(&personel->job_lock);
  pthread_mutex_destroy(&personel->variable_lock);

  if (personel->active_order != 0)
    free(personel->active_order);

  if (personel->pending_order != 0)
    free(personel->pending_order);
}

void t_CookingPersonel_set_exit(t_CookingPersonel *personel) {
  if (personel == 0)
    return;

  personel->is_exit = 1;
}

void *t_CookingPersonel_prepare_thread(void *arg) {
  t_CookingPersonel *personel = (t_CookingPersonel *)arg;
  t_OrderDeque      *deque    = personel->deque_ref;

  while (t_CookingPersonel_is_exit(personel) == 0 || t_CookingPersonel_has_active_order(personel)) {
    if (t_CookingPersonel_has_active_order(personel) == 0) {
      if (t_OrderDeque_dequeue(deque, &personel->active_order, ORDER_REQUEST_MODE_NON_BLOCKING)) {
        usleep(10000);
        continue;
      }
    }

    t_CookingPersonel_prepare(personel);
    t_CookingPersonel_insert(personel);
  }
  printf("Personel %d exited prepare\n", personel->id);

  return 0;
}

void *t_CookingPersonel_cook_thread(void *arg) {
  t_CookingPersonel *personel = (t_CookingPersonel *)arg;

  while (t_CookingPersonel_is_exit(personel) == 0 || t_CookingPersonel_has_active_order(personel) ||
         t_CookingPersonel_has_pending_order(personel)) {
    if (t_CookingPersonel_has_pending_order(personel) == 0) {
      usleep(10000);
      continue;
    }

    t_CookingPersonel_cook(personel);
    t_CookingPersonel_remove(personel);
  }
  printf("Personel %d exited cook\n", personel->id);
  return 0;
}

int t_CookingPersonel_prepare(t_CookingPersonel *personel) {
  if (!t_CookingPersonel_check_prepare(personel, &personel->variable_lock))
    return -1;

  for (int i = 0; i < 100; i++) {
    pthread_mutex_lock(&personel->job_lock);
    sem_wait(&personel->oven_ref->available_aparatus);
    usleep(10000);
    sem_post(&personel->oven_ref->available_aparatus);
    pthread_mutex_unlock(&personel->job_lock);
  }
  pthread_mutex_lock(&personel->variable_lock);
  personel->active_order->is_prepared = 1;
  pthread_mutex_unlock(&personel->variable_lock);

  return 0;
}

int t_CookingPersonel_insert(t_CookingPersonel *personel) {
  t_CookOven *oven = personel->oven_ref;

  if (!t_CookingPersonel_check_insert(personel, &personel->variable_lock))
    return -1;

  pthread_mutex_lock(&personel->job_lock);
  pthread_mutex_lock(&personel->variable_lock);

  t_CookOven_insert(oven);

  personel->pending_order = personel->active_order;
  personel->active_order  = 0;

  pthread_mutex_unlock(&personel->variable_lock);
  pthread_mutex_unlock(&personel->job_lock);

  return 0;
}

int t_CookingPersonel_cook(t_CookingPersonel *personel) {
  if (!t_CookingPersonel_check_cook(personel, &personel->variable_lock))
    return -1;

  for (int i = 0; i < 50; i++) {
    usleep(10000);
  }

  pthread_mutex_lock(&personel->variable_lock);
  personel->pending_order->is_cooked = 1;
  pthread_mutex_unlock(&personel->variable_lock);

  return 0;
}

int t_CookingPersonel_remove(t_CookingPersonel *personel) {
  t_CookOven *oven = personel->oven_ref;

  if (!t_CookingPersonel_check_remove(personel, &personel->variable_lock))
    return -1;

  pthread_mutex_lock(&personel->job_lock);
  pthread_mutex_lock(&personel->variable_lock);

  t_CookOven_remove(oven);

  printf("Personel %d finished order %d\n", personel->id, personel->pending_order->id);

  t_OrderDeque_enqueue(personel->finishedDeque_ref, personel->pending_order, ORDER_REQUEST_MODE_BLOCKING);
  free(personel->pending_order);
  personel->pending_order = 0;
  personel->cooked_count++;

  pthread_mutex_unlock(&personel->variable_lock);
  pthread_mutex_unlock(&personel->job_lock);

  return 0;
}

int t_CookingPersonel_check_prepare(const t_CookingPersonel *personel, pthread_mutex_t *mutex) {
  if (personel == 0)
    return 0;

  pthread_mutex_lock(mutex);
  if (personel->active_order == 0 || personel->active_order->is_prepared == 1) {
    pthread_mutex_unlock(mutex);
    return 0;
  }
  pthread_mutex_unlock(mutex);
  return 1;
}

int t_CookingPersonel_check_insert(const t_CookingPersonel *personel, pthread_mutex_t *mutex) {
  if (personel == 0)
    return 0;

  pthread_mutex_lock(mutex);
  if (personel->active_order == 0 || personel->active_order->is_prepared == 0 || personel->pending_order != 0) {
    pthread_mutex_unlock(mutex);
    return 0;
  }
  pthread_mutex_unlock(mutex);
  return 1;
}

int t_CookingPersonel_check_cook(const t_CookingPersonel *personel, pthread_mutex_t *mutex) {
  if (personel == 0)
    return 0;

  pthread_mutex_lock(mutex);
  if (personel->pending_order == 0 || personel->pending_order->is_cooked == 1) {
    pthread_mutex_unlock(mutex);
    return 0;
  }
  pthread_mutex_unlock(mutex);
  return 1;
}

int t_CookingPersonel_check_remove(const t_CookingPersonel *personel, pthread_mutex_t *mutex) {
  if (personel == 0)
    return 0;

  pthread_mutex_lock(mutex);
  if (personel->pending_order == 0 || personel->pending_order->is_cooked == 0) {
    pthread_mutex_unlock(mutex);
    return 0;
  }
  pthread_mutex_unlock(mutex);
  return 1;
}

int t_CookingPersonel_is_exit(t_CookingPersonel *personel) {
  return personel->is_exit;
}

int t_CookingPersonel_has_active_order(t_CookingPersonel *personel) {
  int result;
  pthread_mutex_lock(&personel->variable_lock);
  result = personel->active_order != 0;
  pthread_mutex_unlock(&personel->variable_lock);
  return result;
}

int t_CookingPersonel_has_pending_order(t_CookingPersonel *personel) {
  int result;
  pthread_mutex_lock(&personel->variable_lock);
  result = personel->pending_order != 0;
  pthread_mutex_unlock(&personel->variable_lock);
  return result;
}
