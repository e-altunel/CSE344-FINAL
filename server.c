#include <CookOven.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *job(void *arg) {
  t_CookOven *oven = (t_CookOven *)arg;
  t_CookOven_insert(oven);
  sleep(1);
  t_CookOven_remove(oven);
  return NULL;
}

int main() {
  t_CookOven oven;
  t_CookOven_init(&oven, 3, 5);

  pthread_t thread[10];

  for (int i = 0; i < 10; i++)
    pthread_create(&thread[i], 0, job, &oven);

  for (int i = 0; i < 10; i++)
    pthread_join(thread[i], 0);

  t_CookOven_destroy(&oven);
}