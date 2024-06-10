#define _POSIX_C_SOURCE 200112L

#include "utils.h"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void
sigint_handler (int signum)
{
  std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
  exit (signum);
}

int
main (int argc, char *argv[])
{
  if (argc != 5)
  {
    std::cerr << "Usage: " << argv[0]
              << " [portnumber] [CookthreadPoolSize] [DeliveryPoolSize] [k]"
              << std::endl;
    std::exit (1);
  }

  const int portnumber = parse_int (argv[1], 1024, 65535);
  const int CookthreadPoolSize = parse_int (argv[2], 1);
  const int DeliveryPoolSize = parse_int (argv[3], 1);
  const int k = parse_int (argv[4], 1);

  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);
  sigaction (SIGINT, &sa, NULL);

  int server_fd;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof (address);

  if ((server_fd = socket (AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror ("socket failed");
    exit (EXIT_FAILURE);
  }
  std::cout << "Socket created" << std::endl;

  if (setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                  sizeof (opt)))
  {
    perror ("setsockopt");
    exit (EXIT_FAILURE);
  }
  std::cout << "Socket set" << std::endl;

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (portnumber);

  if (bind (server_fd, (struct sockaddr *)&address, sizeof (address)) < 0)
  {
    perror ("bind failed");
    exit (EXIT_FAILURE);
  }
  std::cout << "Socket binded" << std::endl;

  if (listen (server_fd, 3) < 0)
  {
    perror ("listen");
    exit (EXIT_FAILURE);
  }
  std::cout << "Listening" << std::endl;

  int new_socket;
  if ((new_socket = accept (server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen))
      < 0)
  {
    perror ("accept");
    exit (EXIT_FAILURE);
  }

  std::cout << "Connection accepted" << std::endl;

  close (new_socket);
  close (server_fd);
  return 0;
}