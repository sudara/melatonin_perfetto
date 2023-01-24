#include <melatonin_perfetto/melatonin_perfetto.h>

#ifndef PERFETTO
  #error The PERFETTO symbol is not defined!
#endif

#if ! PERFETTO
  #error The PERFETTO symbol should be defined to 1, but it's not!
#endif
