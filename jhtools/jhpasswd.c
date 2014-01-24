#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encrypt.h"
#include "pwdfile.h"

void usage(){
	printf("jhpasswd username [password]\n");
	exit(1);
}

int main(int argc, char** argv) {
	if(argc < 2){
		usage();
	}else if(2 == argc){
		setPasswd(argv[1]);
	}else if(3 == argc){
		if(strcmp(argv[1], "-e")==0){
			setPasswdAndEcho(argv[2]);
		}else{
			setUserAndPasswd(argv[1], argv[2]);
		}
	}else{
		usage();
	}
    return 0;
}
