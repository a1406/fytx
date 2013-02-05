#ifndef __TEA__
#define __TEA__

#include <stdint.h>
void sg_encrypt(uint32_t *v);
void sg_decrypt(uint32_t *v);

int sg_base64_encode(const unsigned char *value, int vlen, char *result);
int sg_base64_decode(const char *value, int vlen, char *result);
#endif
