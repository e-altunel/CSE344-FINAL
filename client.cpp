#include "utils.h"
#include <iostream>

int
main (int argc, char *argv[])
{
  if (argc != 5)
  {
    std::cerr << "Usage: " << argv[0]
              << " [portnumber] [numberOfClients] [p] [q]" << std::endl;
    std::exit (1);
  }

  const int portnumber = parse_int (argv[1], 1024, 65535);
  const int numberOfClients = parse_int (argv[2], 1);
  const int p = parse_int (argv[3], 1);
  const int q = parse_int (argv[4], 1);

  std::cout << "Port number: " << portnumber << std::endl;
  std::cout << "Number of clients: " << numberOfClients << std::endl;
  std::cout << "p: " << p << std::endl;
  std::cout << "q: " << q << std::endl;

  return 0;
}