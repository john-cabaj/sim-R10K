#include "cacti_misc.h"

unsigned int 
cacti_floor_log_base2(unsigned int n)
{
  int power = 0;
  while (n >>= 1) power++;
  return power;
}

unsigned int 
cacti_ceil_log_base2(unsigned int n)
{
  int power = 0;
  if (n & (n-1)) power++;
  while (n >>= 1) power++;
  return power;
}
