#include "itoaX.h"
/*******************************************************************/
/* ----------Implementation of itoa() function in C----------------*/
/**************************   M. AboulSeoud ************************/
/*******************************************************************/

/*----------------------------------------------*/
/*---------------------itoa--------------------*/
/*---------------------------------------------*/
char* itoaX(int num, char* str, int base) {
  int i = 0; //index of the string
  char neg = 0;
  int rem;
  // to handle number=0 case
  if (num == 0) {
    str[i] = '0';
    str[i + 1] = '\0';
    return str;
  }
  // Standard "itoa" only handles negative numbers in base 10. Number with other bases are considered unsigned.
  if (num < 0 && base == 10) {
    neg = 1; //Flag
    //To positive for conversion to string using modulus
    num = -num;
  }

  //Conversion from number to string characters using base-x modulus followed by base division
  while (num != 0) {
    rem = num % base;
    //For base such 16 hex
    if (rem > 9) {
      str[i] = (rem - 10) + 'a';
    }
    else {
      str[i] = rem + '0';
    }
    num = num / base;
    i++; //Moving to the next string cell.
  }
  // If number with base 10 is negative, add '-' at the end of string.
  if (neg) {
    str[i++] = '-';
  }
  str[i] = '\0'; // add string null terminator
  // Reverse the string
  reverse_string(str);
  return str;
}

void reverse_string(char *str) {
  int len;
  for (len = 0; str[len] != '\0'; len++);
  char temp, i;
  for (i = 0; i < len / 2; i++) {
    temp = str[i];
    str[i] = str[len - i - 1];
    str[len - 1 - i] = temp;
  }
}
