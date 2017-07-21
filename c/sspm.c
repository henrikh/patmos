/*
    This is a minimal C program executed on the FPGA version of Patmos.
    An embedded Hello World program: a blinking LED.

    Additional to the blinking LED we write to the UART '0' and '1' (if available).

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <machine/spm.h>
#include <stdio.h>

int main() {

  volatile _SPM int *sspm_ptr = (volatile _SPM int *) 0xF00B0000;
  volatile _SPM int *sspm_sync = (volatile _SPM int *) 0xF00BFFFF;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0080004;
  int i, j;
  int k = 0;
  sspm_ptr += 1;

  *sspm_ptr = 0x42;

  sspm_ptr += 1;

  *sspm_ptr = 0x41;

  printf("%x\n", (int) sspm_ptr);
  printf("%x\n", (int) *sspm_ptr);

  sspm_ptr -= 1;

  printf("%x\n", (int) *sspm_ptr);

  i = *sspm_ptr;
  *sspm_ptr = 0x01;
  i = *sspm_sync;
  *sspm_ptr = 0x01;
}
