#include <stdio.h>

#include "cpuid.h"

int main(int argc, char** argv) {
  printf("Vendor: %s\n", vendor_name());
  set_eflags(_AC_FLAG_);
  return 0;
}
