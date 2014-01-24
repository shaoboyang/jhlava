#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encrypt.h"

static unsigned char key[16]={'J','H','R','C','-','S','C','I','N','D','A','P','S','U','S','\0'};        // AES_BLOCK_SIZE = 16
static unsigned char iv[16]={'S','C','I','N','D','A','P','S','U','S','-','J','H','R','C','\0'};        // init vector
static unsigned char rkey[AES_BLOCK_SIZE]={1};
static unsigned char riv[AES_BLOCK_SIZE]={1};

static int initkey(){
	if(AES_BLOCK_SIZE < 16) {
		fprintf(stderr, "error: AES_BLOCK_SIZE is too small\n");
		return(-1);
	}
	memset(rkey, 1, AES_BLOCK_SIZE); 
	memset(riv, 1, AES_BLOCK_SIZE); 
	memcpy(rkey, key, 16);
	memcpy(riv, iv, 16);
	return 0;
}

char* jhencrypt(const char* oristr){
	AES_KEY aes;
	unsigned char* input_string = NULL;
	unsigned char* encrypt_string = NULL;
	char* out_string = NULL;
	int i;
    // set the encryption length
    int len = strlen(oristr)+1;
	int t = len/AES_BLOCK_SIZE;
	int m = len%AES_BLOCK_SIZE;
	if(0!=m) len = (t+1)*AES_BLOCK_SIZE;

	if(initkey()<0) return NULL;

	    // set the input string
    input_string = (unsigned char*)calloc(len, sizeof(unsigned char));
	encrypt_string = (unsigned char*)calloc(len, sizeof(unsigned char)); 
	out_string = (char*)calloc(len*2+1, sizeof(unsigned char)); 

	strncpy((char*)input_string, oristr, strlen(oristr));

	if (AES_set_encrypt_key(rkey, 128, &aes) < 0) {
        fprintf(stderr, "error: Unable to set encryption key in AES\n");
		if(NULL != input_string) free(input_string); 
		if(NULL != encrypt_string) free(encrypt_string); 
		if(NULL != out_string) free(out_string); 
        return NULL;
    }

    // encrypt (iv will change)
    AES_cbc_encrypt(input_string, encrypt_string, len, &aes, riv, AES_ENCRYPT);
	
	// conver encrypted string to hexadecimal number string 
	for (i=0; i<len; ++i) {
		char xchar[3]={0};
		sprintf(xchar,"%02x",encrypt_string[i]);
		strcat(out_string, xchar);
	}

	if(NULL != input_string) free(input_string); 
	if(NULL != encrypt_string) free(encrypt_string); 
	
	return out_string;
}

char* jhdecrypt(const char* encryptedStr){
	AES_KEY aes;
	unsigned char* decrypt_string;
	unsigned char* rencstr;
	int len=strlen(encryptedStr);
	int i;
	if(len%2 !=0 ) {
		fprintf(stderr, "error: Invalid encrypted string.\n");
		return NULL;
	}

	if(initkey()<0) return NULL;

	rencstr=(unsigned char*)calloc(len/2, sizeof(unsigned char));

	// conver hexadecimal number string to encrypted string
	for(i=0;i<len;i+=2){
		char xchar[3]={0};
		xchar[0]=encryptedStr[i];
		xchar[1]=encryptedStr[i+1];
		int nValude = 0;       
		sscanf(xchar,"%x",&nValude);  
		rencstr[i/2]=nValude;
	}

	decrypt_string = (unsigned char*)calloc(len/2, sizeof(unsigned char));

	if (AES_set_decrypt_key(rkey, 128, &aes) < 0) {
        fprintf(stderr, "Unable to set decryption key in AES\n");
		if(NULL != rencstr) free(rencstr); 
		if(NULL != decrypt_string) free(decrypt_string); 
        return NULL;
    }

    // decrypt notice rencstr'length is len/2
    AES_cbc_encrypt(rencstr, decrypt_string, len/2, &aes, riv, AES_DECRYPT);
	return (char*)decrypt_string;
}