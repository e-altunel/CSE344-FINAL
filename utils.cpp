#include "utils.h"

const int
parse_int (const char *str, int min, int max)
{
  const int n = std::atoi (str);
  if (n == 0 && str[0] != '0')
  {
    std::cerr << "Invalid number: " << str << std::endl;
    std::exit (1);
  }
  if (n < min || n > max)
  {
    std::cerr << "Number out of range: " << str << " [";
    std::cerr << "Min: " << min;
    if (max != INT32_MAX)
      std::cerr << ", Max: " << max;
    std::cerr << "]" << std::endl;
    std::exit (1);
  }

  return n;
}