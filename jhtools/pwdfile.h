
struct pair {
    char *key;
    char *value;
};

extern char *trim(char *str);
extern int getUserPwd(char* username, char* passwd);
extern int setPasswd(char* username);
extern int setUserAndPasswd(char* username, char* pwd);
