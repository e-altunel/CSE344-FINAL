#include <stdio.h>

int get_int(const char *str, int min, int max) {
  if (min >= max || min < 0)
    return -1;
  int result = -1;
  sscanf(str, "%d", &result);
  if (result < min || result > max) {
    printf("Invalid input. Please enter a number between %d and %d\n", min, max);
    return -1;
  }
  return result;
}