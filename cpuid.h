#ifndef _CPUID_H_
#define _CPUID_H_

#include <stdlib.h>
#include <string.h>
#include <inttypes.h> 

/** Define some other flags for later use **/
#define _AC_FLAG_ (1 << 19)
#define WORD_BUFFER 128


#if defined(__i386__)
  #define __pop_edx__  " pop edx;  "       
  #define __push_edx__ " push edx; "
#elif defined(__x86_64__)
  #define __pop_edx__  " popq rdx;  "
  #define __push_edx__ " pushq rdx; "
#else 
  #error "Unsupported architecture"
#endif

/** Small utility function used for diagnostics */
void print_bits(int64_t type, int pad) {
 char bits[WORD_BUFFER];  
 char pos;
 memset(bits, '0', WORD_BUFFER);
 
 /* inner printing function */
 void inner_print(int64_t l) { 
  pos++;
  if(l < 2) { 
   bits[pad - pos] = l + 48;
   return;
  } else {
   innerPrint(l >> 1);
   pos--;
   bits[pad - pos] = (l % 2) + 48;
  }
 }
 
 inner_print(type);

 bits[pad] = '\0';
 pos = 0;

 for(char* c = bits; *c != '\0'; ++c) {
  printf("%c",*c); 
  ++pos;
  if(pos > 3) {
   printf(" "); 
   pos = 0;
  }
 }

 printf("\n");
}

/** 
 * Returns an array of sizeof(int) characters 
 * given the word in proper little endianness 
*/
char* to_carray(int word) {
 int mask = 0xFF;
 char* carr = (char*) malloc(sizeof(char) * sizeof(int));
 for(int i = 0; i < 4; ++i) {
  carr[i] = ((word & mask) >> (8*i));
  mask <<= 8;
 }
 return carr;
} 

/**
 * Ako ti ne ovo ne radi kojim slucajem, onda zameni sa asm(...) 
 * ali obavezno dodaj ".intel_syntax noprefix" na pocetku. 
*/
void set_eflags(unsigned int flag) {
 unsigned int eflags = 1;

 __asm__ __volatile__ ( 
          ".intel_syntax noprefix; "
          " pushf; "         //push flags onto stack
	  __pop_edx__        //pop flags from stack onto edx
          __push_edx__       
          " or edx, %1; "    //or the bits
          " mov %0, edx; "   //store the bits
          : "=r"(eflags)
          : "r"(flag)
          : "edx"
        ); 

 printf("flags = "); print_bits(eflags, 32);

 __asm__ __volatile__ (
          __pop_edx__        //recovers dirty edx from stack
          __push_edx__       //pushes clean edx onot stack
          " popf; "          //pop flags from stack [edx]
          : : : "edx"
	);          
} 

char* vendor_name(void) {
 char* vendor = (char*) malloc(sizeof(char) * 12); //occupies 3 registers
 int bword,cword,dword;
 
 //There's no other way than this to obtain vendor name
 __asm__ __volatile__ ( 
          " .intel_syntax noprefix; " 
          " mov eax, 0; "           //we'll inspect vendor's name
          " cpuid; "                //get the id
          " mov %0, ebx; " 
          " mov %1, edx; "
          " mov %2, ecx; "       
	  : "=r"(bword), "=r"(cword), "=r"(dword) 
          : //no input
          : "ebx","ecx","edx"       //clobbered registers: 
                                    //eax is under constraint here 
                                    //as it is used for output
 	);

 vendor = to_carray(bword);
 strcat(vendor, to_carray(cword));
 strcat(vendor, to_carray(dword));
 return vendor;      
}

#endif
