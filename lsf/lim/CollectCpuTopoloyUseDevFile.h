#ifndef _COLLECTCPUTOPOLOYUSEDEVFILE_H_
#define _COLLECTCPUTOPOLOYUSEDEVFILE_H_

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "lim.h"
#define KEY_WORLD "cpu"
#define CPU_MAXLINELEN 1024
#define MAX_PATH_LEN 1024
#define BUF_LEN      100
#define CPU_DEVICES_PATH "/sys/devices/system/cpu/"
#define PHYSICAL_PACKAGE_ID "/topology/physical_package_id"
#define THREAD_SIBLINGS_LIST "/topology/thread_siblings_list"
#define CORE_ID "/topology/core_id"
#define VIRTUAL_MACHINE 1
#define PHYSICAL_MACHINE 0
int g_machine_type = PHYSICAL_MACHINE;

static char* getNextWord2_(char **line){
    static char word[4*CPU_MAXLINELEN];
    char *wordp = word;

    while(isspace(**line)||(**line == ','))
        (*line)++;

    while (**line && !isspace(**line)&& (**line != ','))
        *wordp++ = *(*line)++;

    if (wordp == word)

        return(NULL);
    
    *wordp = '\0';
    return(word);

}

/** 
 * trim a string 
 * 
 * @param str 
 * 
 * @return 
 */
static char *trim(char *str){
    char *end;
    while(isspace(*str) || *str=='\n' || *str=='\r')
        str++;
    if(*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while(end > str && (isspace(*end) || *end == '\n' || *str == '\r'))
        end--;
    *(end + 1) = 0;
    return str;
}
/** 
 * find max cpu num at pathname
 * 
 * @param pathname [ in ]: a path example "/sys/devices/system/cpu/"
 * 
 * @return : cpuid directory num
 */
static int get_cpudir_num(char* pathname){
    DIR *dirp;
    int cpu_num = 0;
    int key_world_len = strlen(KEY_WORLD);
    int itmp;
    struct dirent *p_dirent;
    char path_buf[MAX_PATH_LEN];
    struct stat fileStat;
    sprintf(path_buf, "%scpu%d%s", pathname, 0, PHYSICAL_PACKAGE_ID);
    if(stat(path_buf, &fileStat)){
        if(errno == ENOENT)
            ls_syslog(LOG_DEBUG, "get_cpudir_num():file %s is not exist.", path_buf);
        return -1;
    }
    
    sprintf(path_buf, "%scpu%d%s", pathname, 0, CORE_ID);
    if(stat(path_buf, &fileStat)){
        if(errno == ENOENT)
            ls_syslog(LOG_DEBUG, "get_cpudir_num():file %s is not exist.", path_buf);
        return -1;
    }
        
    sprintf(path_buf, "%scpu%d%s", pathname, 0, THREAD_SIBLINGS_LIST);
    if(stat(path_buf, &fileStat)){
        if(errno == ENOENT)
            ls_syslog(LOG_DEBUG, "get_cpudir_num():file %s is not exist.", path_buf);
        return -1;
    }
        
    if((dirp = opendir(pathname)) == NULL){
        fprintf(stderr, "can't open %s directory\n", pathname);
        return -1;
    }
    while((p_dirent = readdir(dirp)) != NULL){
        if(strncasecmp(KEY_WORLD, p_dirent->d_name, key_world_len))
            continue;
        itmp = atoi(p_dirent->d_name + key_world_len);
        cpu_num = cpu_num>itmp ? cpu_num:itmp;
    }
    closedir(dirp);
    return cpu_num + 1;
}

/** 
 * open pathfile and get a line data.
 * 
 * @param pathfile [ in ]:
 * @param fd       [ in ]:
 * @param readbuf  [ out ]:
 * 
 * @return 
 */
static int readfile(char *pathfile, char *readbuf){
    char *rtrn= NULL;
    FILE *fd = NULL;
    if((fd = fopen(pathfile, "r")) == NULL){
        ls_syslog(LOG_ERR,"open %s failed\n", pathfile);
        return -1;
    }
//    rtrn = fread(readbuf, 1, 100, fd);
    rtrn = fgets(readbuf, BUF_LEN, fd);    
    if(rtrn == NULL ){
        ls_syslog(LOG_ERR,"read %s failed\n", pathfile);
        fclose(fd);
        return -1;
    }
    fclose(fd);
    fd = NULL;
    return 0;
}


/** 
 * 
 * 
 * @param array 
 * @param num 
 * @param pstr 
 * 
 * @return > 1 if find pstr string in array ,ohterwise return -1
 */
static int get_index(char *array[], int num, const char * pstr){
    int pstr_find = 0;
    int i = 0;
    int index = -1;
    for (i = 0; i < num; ++i){
        if(array[i] != NULL)
            if(!strcmp(array[i], pstr)){
                pstr_find = 1;
                index = i;
                break;
            }
    }
    return index;
}

/** 
 * get cpu socket num
 * get core num at one socket
 * get thread num at one core
 * strcat a string for  a cpuid directory, the string format  "socketid coreid threadid,threadi"
 * 
 * @param cpu_num   [ in ]: 
 * @param pathname  [ in ]: 
 * @param topo      [ out ]: get the socketnum corenum threadnum
 * @param array     [ out ]: get cpu message at cpuid directory
 * 
 * @return : sucess return zero, 
 */
static int get_cpumessage(int cpu_num, char *pathname, struct cpu_topology * topo, char *array[]){
    static char fname[] ="get_cpumessage";
    int i = 0;
    int retCode = 0;
    char * tmpsocket = NULL;
    char * tmpcore = NULL;
    int find_socket = 0;
    int find_core = 0;
    int socket_count = 0;
    int core_count = 0;
    char pathbuf[MAX_PATH_LEN];
    char tmpbuf[BUF_LEN];    
    char *tok = NULL;
    char *p = NULL;

    char **socketArray = (char**)calloc(cpu_num, sizeof(char *));
    if(socketArray == NULL){
        ls_syslog(LOG_ERR, "%s: alloc memory err, cpunum=%d", fname, cpu_num);
        return -1;
    }
    char **coreArray = (char**)calloc(cpu_num, sizeof(char *));
    if(coreArray == NULL){
        ls_syslog(LOG_ERR, "%s: alloc memory err, cpunum=%d", fname, cpu_num);
        FREEUP(socketArray);
        return -1;
    }    

    topo->socketnum = 0;
    topo->corenum = 0;
    topo->threadnum = 0;
    for(i=0; i<cpu_num; i++){
        find_socket = 0;
        find_core = 0;
            /* read physical_package_id */
        sprintf(pathbuf, "%scpu%d%s", pathname, i, PHYSICAL_PACKAGE_ID);
        if((retCode = readfile(pathbuf, tmpbuf)) != 0){
            goto READ_FILE_ERR;
        }
        trim(tmpbuf);
        if(strlen(tmpbuf) <= 0)
            goto READ_FILE_ERR;
        sprintf(array[i], "%s ", tmpbuf); //create string "socketid coreid threadid,threadid..."
        p = strdup(tmpbuf);
        if(p == NULL){
            ls_syslog(LOG_ERR, "%s: strdup failed.", fname);
            goto READ_FILE_ERR;
        }                            
        tmpsocket = p;
                    /* find socket num */
        find_socket = get_index(socketArray, socket_count, tmpsocket);
        if(find_socket < 0){
            p = strdup(tmpsocket);
            if(p == NULL){
                ls_syslog(LOG_ERR, "%s: strdup failed.", fname);
                goto READ_FILE_ERR;
            }
            socketArray[socket_count++] = p;
        }
            /* read core_id */
        sprintf(pathbuf, "%scpu%d%s", pathname, i, CORE_ID);
        if((retCode = readfile(pathbuf, tmpbuf)) != 0){
            goto READ_FILE_ERR;
        }
        trim(tmpbuf);
        if(strlen(tmpbuf) <= 0)
            goto READ_FILE_ERR;        
        sprintf(array[i]+strlen(array[i]), "%s ", tmpbuf);
        p = strdup(tmpbuf);
        if(p == NULL){
            ls_syslog(LOG_ERR, "%s: strdup failed.", fname);
            goto READ_FILE_ERR;
        }                            
        tmpcore = p;
        if(!strcmp(tmpsocket, socketArray[0])){
            find_core = get_index(coreArray, core_count, tmpcore);
            if(find_core < 0){
                p = strdup(tmpcore);
                if(p == NULL){
                    ls_syslog(LOG_ERR, "%s: strdup failed.", fname);
                    goto READ_FILE_ERR;
                }                
                coreArray[core_count++] = p;
            }            
        }
            /* read thread_siblings_list */
        sprintf(pathbuf, "%scpu%d%s", pathname, i, THREAD_SIBLINGS_LIST);
        if((retCode = readfile(pathbuf, tmpbuf)) != 0){
            goto READ_FILE_ERR;
        }
        trim(tmpbuf);
        if(strlen(tmpbuf) <= 0)
            goto READ_FILE_ERR;        
        sprintf(array[i]+strlen(array[i]), "%s", tmpbuf);
        if(!strcmp(tmpsocket, socketArray[0]) && !strcmp(tmpcore, coreArray[0])){
            tok = strtok(tmpbuf, ",");
                /* get cpu thread num when i is 0 */
            if(i == 0){
                while(tok != NULL){
                    ++topo->threadnum;
                    tok = strtok(NULL, ",");
                }
            }            
        }
        FREEUP(tmpsocket);
        FREEUP(tmpcore);
    }
    topo->socketnum = socket_count;
    topo->corenum = core_count;
    for(i=0; i<socket_count; i++)
        FREEUP(socketArray[i]);
    FREEUP(socketArray);
    for(i=0; i<core_count; i++)
        FREEUP(coreArray[i]);
    FREEUP(coreArray);
    return 0;
    
  READ_FILE_ERR:
    ls_syslog(LOG_ERR, "%s: read file failed.", fname);
    FREEUP(tmpsocket);
    FREEUP(tmpcore);
    for(i=0; i<socket_count; i++)
        FREEUP(socketArray[i]);
    FREEUP(socketArray);
    for(i=0; i<core_count; i++)
        FREEUP(coreArray[i]);
    FREEUP(coreArray);
    return -1;
    
}

/** 
 * GET cpu topology form cpumessage string array.
 * 
 * @param topo  [ in ] : input socket num, core num, thread num
 *              [ out ] : output cpu topology.
 * @param array [ in ] : 
 * 
 * @return : return zero is sucess.
 */
static int getcpu(struct cpu_topology *topo, char **array){
    static char fname[] = "getcpu";
    int i = 0, socket = 0, core = 0, thread = 0;
    int socket_index = 0;
    int count = 0;
    int index = 0;
    int num_cpu = topo->socketnum * topo->corenum * topo->threadnum;
    char *token = NULL;
    char *pstr = NULL;
    char *p = NULL;
    
    int *core_indexs = (int *)calloc(topo->socketnum, sizeof(int));
    if(core_indexs == NULL){
        ls_syslog(LOG_ERR, "%s: calloc err.", fname);
        return -1;
    }

    topo->topology = (int *)calloc(num_cpu, sizeof(int));
    if(topo->topology == NULL){
        ls_syslog(LOG_ERR, "%s: malloc error!", fname);
        FREEUP(core_indexs);
        return -1;
    }
    topo->socket_IDs = (char **)calloc(topo->socketnum, sizeof(char *));
    if(topo->socket_IDs == NULL){
        ls_syslog(LOG_ERR, "%s: calloc error!", fname);
        FREEUP(core_indexs);
        FREEUP(topo->topology);
        return -1;
    }
    topo->core_IDs = (char **)calloc(topo->corenum * topo->socketnum, sizeof(char *));
    if(topo->socket_IDs == NULL){
        ls_syslog(LOG_ERR, "%s: calloc error!", fname);
        FREEUP(core_indexs);
        FREEUP(topo->topology);
        FREEUP(topo->socket_IDs);
        return -1;        
    }
    

    for (i=0; i<num_cpu; ++i)
    {
        pstr = array[i];
        count = 0;
        ls_syslog(LOG_DEBUG, "pstr:%s", pstr);
        while((token = getNextWord2_(&pstr)) != NULL){
            ++count;
            if(1 == count){
                socket = get_index(topo->socket_IDs, topo->socketnum, token);
                if(socket < 0){
                    p = strdup(token);
                    if(p == NULL)
                        goto MALLOC_ERR;
                        topo->socket_IDs[socket_index++] = p;
                    socket = socket_index - 1;
                }
            }
            if(2 == count){
                core = get_index(&topo->core_IDs[socket * topo->corenum], topo->corenum, token);
                if(core < 0){
                    p = strdup(token);
                    if(p == NULL)
                        goto MALLOC_ERR;
                    topo->core_IDs[(socket * topo->corenum + core_indexs[socket]++)] = p;
                    core = core_indexs[socket] - 1;
                }
            }
            
            if(count >= 3){
                thread = atoi(token);
                index = socket*topo->corenum*topo->threadnum
                    + core*topo->threadnum + count -3;
                if(topo->topology != NULL)
                    topo->topology[index] = thread;
            }
        }
        if(count < 3){
            ls_syslog(LOG_ERR, "%s: analysis err.", fname);
            goto MALLOC_ERR;
        }
    }
    topo->topologyflag = 1;
    return 0;
    
  MALLOC_ERR:
    ls_syslog(LOG_ERR, "%s: strdup failed.", fname);
    FREEUP(core_indexs);
    
    for(i=0; i<topo->socketnum; i++)
        FREEUP(topo->socket_IDs[i]);
    FREEUP(topo->socket_IDs);
    
    for(i=0; i<topo->socketnum * topo->corenum; i++)
        FREEUP(topo->core_IDs[i]);
    FREEUP(topo->core_IDs);
    FREEUP(topo->topology);
    return -1;
    
}

#endif /* _COLLECTCPUTOPOLOYUSEDEVFILE_H_ */
