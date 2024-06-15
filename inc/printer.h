#ifndef INC_PRINTER
#define INC_PRINTER

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef FD_COUNT
#define FD_COUNT 3
#endif

typedef struct s_printer {
  int             fd[FD_COUNT];
  int             feof;
  pthread_mutex_t printer_mutex;
} t_printer;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
extern t_printer       global_printer;
#pragma GCC diagnostic pop

int  printer_init();
void printer_destroy();
void printer_set_fd(int index, int fd);
int  printer_get_fd(int index);
int  check_feof();

#define LOG(fd_index, ...)                                                                                             \
  {                                                                                                                    \
    pthread_mutex_lock(&global_printer.printer_mutex);                                                                 \
    time_t t = time(NULL);                                                                                             \
    char   time_str[26];                                                                                               \
    ctime_r(&t, time_str);                                                                                             \
    dprintf(global_printer.fd[fd_index], "[%.*s] ", (int)strlen(time_str) - 1, time_str);                              \
    dprintf(global_printer.fd[fd_index], ##__VA_ARGS__);                                                               \
    fsync(global_printer.fd[fd_index]);                                                                                \
    pthread_mutex_unlock(&global_printer.printer_mutex);                                                               \
  }
#define SEND(fd_index, ...)                                                                                            \
  {                                                                                                                    \
    pthread_mutex_lock(&global_printer.printer_mutex);                                                                 \
    char buffer[1024] = {0};                                                                                           \
    int  i            = snprintf(buffer, 1024, ##__VA_ARGS__);                                                         \
    if (write(global_printer.fd[fd_index], &i, sizeof(int)) == 0) {                                                    \
      global_printer.feof = 1;                                                                                         \
    } else if (write(global_printer.fd[fd_index], buffer, i) == 0) {                                                   \
      global_printer.feof = 1;                                                                                         \
    }                                                                                                                  \
    pthread_mutex_unlock(&global_printer.printer_mutex);                                                               \
  }

#endif /* INC_PRINTER */
