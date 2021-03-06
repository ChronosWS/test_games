
#include "macro.h"

EXTERN(int platform_optind = 1);
EXTERN(const char* platform_optarg);

int
platform_getopt(int argc, char* const argv[], const char* optstring)
{
  for (; platform_optind < argc; ++platform_optind) {
    const char* arg = argv[platform_optind];
    platform_optarg = argv[platform_optind + 1];

    const char* optiter = optstring;
    for (; *optiter; ++optiter) {
      int val_param = *(optiter + 1) == ':';
      if (arg[0] == '-' && *optiter == arg[1]) {
        if (!val_param) platform_optarg = nullptr;
        ++platform_optind;
        return *optiter;
      }
    }
  }

  return -1;
}
