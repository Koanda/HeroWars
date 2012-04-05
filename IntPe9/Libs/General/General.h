#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int   decode_base64(unsigned char *dest, const char *src);
char *encode_base64(int size, unsigned char *src);