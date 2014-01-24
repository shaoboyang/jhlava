#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "mbd.h"
#include "ugtree.h"

#define START_LINE "Begin UserGroup"
#define END_LINE "End UserGroup"

extern char** getAllUsersFromMBD();

// store the group defined all member type, because we need update this group memeber in tree.
static GHashTable* pGroupAllMap=NULL;
static int SHARE_DEFINED=1;

GHashTable* getGroupAllTypeDefine(){
	return pGroupAllMap;
}

void destroyGroupAllMap(){
	if(NULL==pGroupAllMap) return;
	g_hash_table_destroy(pGroupAllMap);
}

int isShareDefined(){
	return SHARE_DEFINED;
}

/*
   split a string by a set of delimiters
   return a array of string
   remove string with 0 length and NULL from array.
   trim every string in array
*/

char** mysplit_ex(const char* string, const char* delimiters, int* num){
	char** its = g_strsplit_set(string, delimiters, -1);
	char** p=its;
	int n=0;
	while(NULL!=*p){
		g_strstrip(*p);
		if(*p!=NULL && (*p)[0]!='\0') n++;
		p++;
	}
	if(NULL!=num) *num=n;
	char** res=(char**)calloc(n+1,sizeof(char*));
	char** q=res;
	p=its;
	while(NULL!=*p){
		if(*p!=NULL && (*p)[0]!='\0') *(q++)=g_strdup(*p);
		p++;
	}
	g_strfreev(its);
	return res;
}


char** mysplit(const char* string, const char* delimiters){
	return mysplit_ex(string, delimiters, NULL);
}


/*
void printMap(gpointer key,gpointer value,gpointer user_data){
	ls_syslog(LOG_ERR,"map:%s===%s\n",key,value);
}
*/

//parse a group define as a group object
Group* parseLine(char* line,  struct keymap *keylist){
	Group* g = g_new0(Group, 1);
	char** ginfo=mysplit(line,"()");

	//get group name
	strcpy(g->name,ginfo[0]);

	//get members share
	GHashTable* shareMap = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	char** p;
    if(keylist[2].position == -1){
        SHARE_DEFINED=0;
    }else{
        if(NULL==ginfo[keylist[2].position] || '#'==ginfo[keylist[2].position][0]){
            fprintf(stderr, "Can not find share define of %s.\n", ginfo[0]);
            SHARE_DEFINED=0;
        }else{
                /*bug 9:increase support for '\'*/
                /*******************************************************/
            char *sp=ginfo[keylist[2].position];
            int num=0;		
            int len = strlen(sp)+1;
            char *tmp = (char *) malloc (len*sizeof(char));
            if (tmp==NULL)
                return (NULL);
            char *cp=tmp;
            while (sp!= &(ginfo[keylist[2].position][len-1])) {
                if (*sp!='\\') {				    
                    *tmp=*sp;		    
                    sp++;
                    tmp++;
                }
                else 
                {
                    sp++;
                    num++;
                }
            }
            *(cp+len-num-1)='\0';		
                /*******************************************************/
            char** shares=mysplit(cp,"[]");
            p = shares;
            while(NULL!=*p){
                char** kv=mysplit(*p,", ");
                g_hash_table_insert(shareMap,strdup(kv[0]),strdup(kv[1]));
                g_strfreev(kv);
                p++;
            }
            free(cp);
            g_strfreev(shares);
        }
    }

	//g_hash_table_foreach(shareMap,printMap,NULL);
	
	//get all members name
	g->members = g_array_new(TRUE, TRUE, sizeof(Node*));
	char** ms;
	if(g_ascii_strcasecmp(ginfo[1],"all")==0){
		pGroupAllMap=shareMap;
		char* val=g_hash_table_lookup(shareMap, "all");
		int shareInt;
		if(NULL==val) {
			shareInt=0;
			//exit(1);
		}else{
			shareInt=atoi(val);
		}
		
		g->share=shareInt;
		g->type=1;
		ms=getAllUsersFromMBD();
	}else{
		ms=mysplit(ginfo[1]," ");
	}
	p=ms;
	while(NULL!=*p){
		char* val=g_hash_table_lookup(shareMap, *p);
		if(NULL==val) val=g_hash_table_lookup(shareMap, "default");
		int shareInt;
		if(NULL==val) {
			ls_syslog(LOG_WARNING, "Can not find the share of %s in lsb.users.",*p);
			shareInt=0;
			//exit(1);
		}else{
			shareInt=atoi(val);
		}
		Node* n=node_new_user(*p, shareInt);
		g_array_append_val(g->members,n);
		p++;
	}
	g_strfreev(ms);
	
	if(g_ascii_strcasecmp(ginfo[1],"all")!=0){
		g_hash_table_destroy(shareMap);
	}
	
	g_strfreev(ginfo);
	return g;
}


int readConf(char* filename, GArray** groups){
	GError *error = NULL;
	char* fileContent;
	gsize fileLen;
	GArray* pgroups;
    struct keymap keylist[] = {
    	{"GROUP_NAME", NULL, -1},
    	{"GROUP_MEMBER", NULL, -1},
        {"USER_SHARES",  NULL, -1},
        {"GROUP_ADMIN",  NULL, -1},
    	{NULL, NULL, 0}
    };
	if(!g_file_get_contents(filename, &fileContent, &fileLen, &error)){
		ls_syslog(LOG_ERR,"ERR:Load conf file error:%s\n",error->message);
		return -1;
	}
	pgroups=g_array_new(TRUE, TRUE, sizeof(Group*));
	*groups = pgroups;
	int num=0;
	char** userconf=mysplit_ex(fileContent,"\n",&num);
	char** p = userconf;
	char** userconfNoComments=(char**)calloc(num+1,sizeof(char*));
	char** q = userconfNoComments;
	while(NULL!=*p){
		if('#'==(*p)[0]) {
			p++;
			continue;
		}
		*q=*p;
		p++;q++;
	}

	p = userconfNoComments;
	int parseFlag = 0;
	while(NULL!=*p){
		if(strstr(*p, START_LINE)==*p) {
			parseFlag = 1;
                     p++;
                     if(strstr(*p, END_LINE)==*p){
                         ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5101,
                                                 "in File %s  Keyword line format error for section UserGroup."), filename); 
                         parseFlag = -1;
                         break;
             }
            if(parseFlag > 0)
                if (!keyMatch(keylist, *p, FALSE)) {
                    ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5101,
                                                 "in File %s  Keyword line format error for section UserGroup."), filename); 
                }
		   p+=1;
                 if (p == NULL)
                    break;
                
		}
		if(strstr(*p, END_LINE)==*p) parseFlag = -1;
		if(parseFlag > 0){ 
			Group* g = parseLine(*p, keylist);
			//printf("Name:%s\n",g->name);
			//int n=g->members->len;
			//int i=0;
			/* print all group members
			for(;i<n;i++){
				node* m=&g_array_index(g->members,node,i);
				printf("member:%s=%d\n",m->name,m->oriShare);
			}
			*/
			g_array_append_val(pgroups,g);
		}
		
		p++;
	}

	g_strfreev(userconf);
	g_free(fileContent);
	g_free(userconfNoComments);
	return 0;
}

