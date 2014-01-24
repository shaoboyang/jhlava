#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <termios.h>  /* for tcxxxattr, ECHO, etc */
#include <unistd.h>   /* for STDIN_FILENO */
#include "pwdfile.h"
#include "encrypt.h"

static FILE* f = NULL;
static struct pair account[128];
static int accPos = 0;

char *trim(char *str){
	char *end;
	while(isspace(*str) || *str=='\n' || *str=='\r') str++;
	if (*str == 0) return str;
	end = str + strlen(str) - 1;
	while (end > str && (isspace(*end) || *end=='\n' || *str=='\r')) end--;
	*(end + 1) = 0;
	return str;
}

static int openPwdFile(char* mode){
	char path[512]={0};
	char* lsfEnvDir = getenv("LSF_ENVDIR");
	if(NULL==lsfEnvDir || strcmp("",lsfEnvDir)==0){
		getcwd(path,sizeof(path)); 
	}
	strcpy(path, lsfEnvDir);
	strcat(path, "/jhspwd.dat");
	f = fopen(path, mode);
	if(NULL==f) {
//		fprintf(stderr, "Open passwd file %s failed.\n", path);
		return -1;
	}
	return 0;
}

static int readPwdFile(){
	if(NULL==f) {
		if(openPwdFile("r")<0) return -1;
	}
	char line[4096] = {0};
	while(fgets(line, 4095, f)!=NULL){
		if(0==strlen(line)) continue;
		char* p=strchr(line, ' ');
		*p='\0';
		p++;
		account[accPos].key = strdup(trim(line));
		account[accPos].value = strdup(trim(p));
		accPos++;
	}
	fclose(f);
	f=NULL;
	return 0;
}

static void clearMemAccount(){
	int i;
	for(i=0;i<accPos;i++){
		if(NULL != account[i].key) free(account[i].key);
		if(NULL != account[i].value) free(account[i].value);
	}
	accPos=0;
	if(NULL!=f) {
		fclose(f);
		f=NULL;
	}
}

/*simulate windows' getch(), it works!!*/
static int getch (void){
    int ch;
    struct termios oldt, newt;
  
    // get terminal input's attribute
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    //set termios' local mode
    newt.c_lflag &= ~(ECHO|ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    //read character from terminal input
    ch = getchar();

    //recover terminal's attribute
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  
    return ch;
}

static void inputPwd(char *p){
	int i=0;
	char c;
	c = getch();
	while(c!='\r' && c!='\n'){
//		printf("%d",c);
		if(c==127&&i>0){
			printf("\b \b");
			i--;
			p[i]='\0';
		}else{
			putchar('*');
			p[i++]=c;
		}
		c = getch();
	}
	putchar('\n');
}

int getUserPwd(char* username, char* passwd){
	if(0 == accPos) {
		if(readPwdFile()<0) return -1;
	}
	int i;
	for(i=0;i<accPos;i++){
		if(0==strcmp(username, account[i].key)){
			char* dec=jhdecrypt(account[i].value);
			if(NULL==dec) return -1;
			strcpy(passwd, jhdecrypt(account[i].value));
			return 0;
		}
	}
	return -1;
}

int setPasswd(char* username){
	printf("Input the password of %s:",username);
	char pwd[128]={0};
	inputPwd(pwd);
	return setUserAndPasswd(username, pwd);
}

int setPasswdAndEcho(char* username){
	printf("Input the password of %s:",username);
	char pwd[128]={0};
	inputPwd(pwd);
	char* lsfenvdir=getenv("LSF_ENVDIR");
	char path[1024]={0};
	sprintf(path, "%s/jhpd.tmp", lsfenvdir);
	FILE* fp=fopen(path,"wt");
	fputs(pwd, fp);
	fclose(fp);
	return setUserAndPasswd(username, pwd);
}

int setUserAndPasswd(char* username, char* pwd){
	clearMemAccount();
	readPwdFile();

	if(NULL==f) {
		if(openPwdFile("wt")<0) return -1;
	}
	
	int i;
	int insert = 1;
	for(i=0;i<accPos;i++){
		char line[4096]={0};
		strcpy(line, account[i].key);
		strcat(line, " ");
		//update
		if(strcmp(account[i].key,username)==0){
			account[i].value=jhencrypt(trim(pwd));
			insert = 0;
		}
		strcat(line, account[i].value);
		strcat(line, "\n");
		fputs(line, f);
	}
	if(insert){
		char line[4096]={0};
		account[accPos].key=strdup(trim(username));
		account[accPos].value=jhencrypt(trim(pwd));
		strcpy(line, account[i].key);
		strcat(line, " ");
		strcat(line, account[i].value);
		strcat(line, "\n");
		accPos++;
		fputs(line, f);
	}
	clearMemAccount();
	return 0;
}
/*
int main(int argc, char *argv[]){
	if(argc < 2) exit(1);
//	setUserPwd(argv[1]);
	char pwd[128]={0};
	getUserPwd(argv[1],pwd);
	printf("%s\n",pwd);
	return 0;
}
*/