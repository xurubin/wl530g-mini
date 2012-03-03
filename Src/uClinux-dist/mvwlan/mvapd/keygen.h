#ifndef _KEYGEN_H_
#define _KEYGEN_H_

void KEYGEN_gen40bitkeys(char *genstr, UINT8 key[4][5]);
void KEYGEN_gen128bitkeys(char *genstr, UINT8 *key);
void KEYGEN_gen128bitdefaultkey(unsigned char *key, unsigned char *macaddr1, unsigned char *macaddr2);

#endif
