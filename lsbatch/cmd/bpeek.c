/*
 * Copyright (C) 2007 Platform Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include "cmd.h"

#include <netdb.h>
#define NL_SETN 8 	

extern int errno;
static void displayOutput(char *, struct jobInfoEnt *, char, char **);
static void oneOf (char *);
static void output(char *, char *, int, char *, char **);
static void usage (char *cmd);
static void remoteOutput(int fidx, char **disOut, char *exHost, char *fname,
			 char *execUsername, char **);
static int useTmp(char *exHost, char *fname);
static void stripClusterName(char *);

static void 
usage (char *cmd)
{
    fprintf(stderr, "%s: %s [-h] [-V] [-f] [-m host_name | -q queue_name |\n        -J job_name | jobId",I18N_Usage, cmd); 
    if (lsbMode_ & LSB_MODE_BATCH)
	fprintf(stderr, " | \"jobId[index]\"");
    fprintf(stderr, "]\n");
    exit(-1);
}

static void 
oneOf (char *cmd)
{
    fprintf(stderr, "%s.\n", 
	_i18n_msg_get(ls_catd,NL_SETN,2453, 
        "Command syntax error: more than one of -m , -q, -J or jobId are specified")); /* catgets  2453  */
    usage(cmd);
}

#define MAX_PEEK_ARGS  5 

int 
main (int argc, char **argv, char **environ)
{
    char   *queue = NULL, *host = NULL, *jobName = NULL, *user = NULL;
    LS_LONG_INT  jobId;
    int options;
    struct jobInfoEnt *jInfo;
    char   *outFile;
    char   fflag = FALSE;
    int    cc;
    int    rc;

    rc = _i18n_init ( I18N_CAT_MIN );	


    if (lsb_init(argv[0]) < 0) {
	lsb_perror("lsb_init");
	exit(-1);
    }

    while ((cc = getopt(argc, argv, "Vhfq:m:J:")) != EOF) {
        switch (cc) {
        case 'q':
            if (queue || host || jobName)
                oneOf (argv[0]);
            queue = optarg;
            break;
        case 'm':
            if (queue || host || jobName)
                oneOf (argv[0]);
            host = optarg;
            break;
        case 'J':
            if (queue || host || jobName)
                oneOf (argv[0]);
            jobName = optarg;
            break;
	case 'V':
	    fputs(_LS_VERSION_, stderr);
	    exit(0);
	case 'f':
	    fflag = TRUE;   
	    break;
        case 'h':
        default:
            usage(argv[0]);
        }
    }            
    set_replace_user_map_env(1);
    jobId = 0;
    options = LAST_JOB;
    if ( argc >= optind + 1) {
        if (queue || host || jobName) {
	    oneOf (argv[0]);
        } else if ((argc > 2 && !fflag) || (argc > 3 && fflag)) 
            usage (argv[0]);             
     
	if (getOneJobId (argv[optind], &jobId, 0)) {
	    usage(argv[0]);
	}

        options = 0;
    }
    
   

    if (lsb_openjobinfo (jobId, jobName, NULL, queue, host, options) < 0
        || (jInfo = lsb_readjobinfo (NULL)) == NULL) {

        if (jobId != 0 || jobName != NULL) {
           user = ALL_USERS;
           if (lsb_openjobinfo (jobId, jobName, user, queue, host, options) < 0
               || (jInfo = lsb_readjobinfo (NULL)) == NULL) {
               jobInfoErr (jobId, jobName, NULL, queue, host, options);
               exit(-1);
           }
        } else {
           jobInfoErr (jobId, jobName, NULL, queue, host, options);
           exit(-1);
        }
    }
    lsb_closejobinfo();

    
    if (jobId && jInfo->jobId != jobId) {
        lsberrno = LSBE_JOB_ARRAY;
        lsb_perror("bpeek");
        exit(-1);
    }

    
    if ((jInfo->submit.options & SUB_INTERACTIVE) &&
        !(jInfo->submit.options & (SUB_OUT_FILE | SUB_ERR_FILE) ) ) {
        fprintf(stderr, _i18n_msg_get(ls_catd,NL_SETN,2456,
                "Job <%s> : Cannot bpeek an interactive job.\n"), /* catgets  2456 */
             lsb_jobid2str(jInfo->jobId));
        exit(-1);
    }
    
    if (IS_PEND(jInfo->status) || jInfo->execUsername[0] == '\0') {
        fprintf(stderr,  _i18n_msg_get(ls_catd,NL_SETN,2454,
		"Job <%s> : Not yet started.\n"), /* catgets  2454 */
	    lsb_jobid2str(jInfo->jobId));

        exit(-1);
    }
    if (IS_FINISH(jInfo->status)) {
        fprintf(stderr, _i18n_msg_get(ls_catd,NL_SETN,2455,
		"Job <%s> : Already finished.\n"), /* catgets  2455  */ 
	    lsb_jobid2str(jInfo->jobId));
        exit(-1);
    }
 
    if ((outFile = lsb_peekjob (jInfo->jobId)) == NULL) {
        char msg[50];
	sprintf(msg,  "%s <%s>", I18N_Job, lsb_jobid2str(jInfo->jobId)); 
        lsb_perror(msg);
        exit(-1);
    }
    displayOutput (outFile, jInfo, fflag, environ);
    _i18n_end ( ls_catd );
    set_replace_user_map_env(0);			
    exit(0);

} 

static void 
displayOutput (char *jobFile, struct jobInfoEnt *jInfo, char fflag, char **envp)
{
    char fileOut[MAXFILENAMELEN];
    char fileErr[MAXFILENAMELEN];
    char buf[1024];
    char **envptmp;
    char *sp = NULL;
    int i=0, k=0;

    sprintf(fileOut, "%s.out", jobFile);
    sprintf(fileErr, "%s.err", jobFile);

    
    stripClusterName(jInfo->exHosts[0]);
    for (k = 0; envp[k]; k++)
        ;
    envptmp = (char **)calloc(k + 2, sizeof(char *));
	  
    for (k = 0; envp[k]; k ++) {
        if ((sp = strstr(envp[k], "WIN_USER_MAPPING")) != NULL)
	    continue;
		
        envptmp[i] = strdup(envp[k]);
	i++;
    }
    if (strcmp(jInfo->user, jInfo->execUsername) != 0) {
        sprintf(buf, "WIN_USER_MAPPING=%s", jInfo->execUsername);
	envptmp[i] = strdup(buf);
	set_user_map_value(buf);
	i++;
    }
    envptmp[i] = NULL;

    if (! ((jInfo->submit.options & SUB_OUT_FILE) 
           && strcmp(jInfo->submit.outFile,  LSDEVNULL) == 0)) {
	printf("<< %s >>\n",(_i18n_msg_get(ls_catd,NL_SETN,2457, "output from stdout"))); /* catgets  2457  */
	output(fileOut, jInfo->exHosts[0], fflag, jInfo->execUsername, envptmp);
    }
    
    
    if ((jInfo->submit.options & SUB_ERR_FILE) 
          && strcmp(jInfo->submit.errFile,  LSDEVNULL) != 0) {
	printf("\n<< %s >>\n",(_i18n_msg_get(ls_catd,NL_SETN,2458, "output from stderr"))); /* catgets  2458  */
	output(fileErr, jInfo->exHosts[0], fflag, jInfo->execUsername, envptmp);
    }
    
    exit(0);

}  

 
static void
output(char *fname, char *exHost, int fflag, char *execUsername, char **envp)
{
    char *disOut[MAX_PEEK_ARGS];
    int fidx;
    int pid=0;
    struct stat buf;

    if (!fflag ) {
	
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit (-1);
         }
    } 

    if ( pid > 0) {
	
        wait(NULL); 
        return;
     }

    
    if (fflag) {                          
	disOut[0] = "tail";
	disOut[1] = "-f";
	fidx = 2;
    } else {
	disOut[0] = "cat";
	fidx = 1;
    }
    disOut[fidx+1] = NULL;

    
    if ( fname[0] != '/' || stat(fname,&buf) < 0) {
	remoteOutput(fidx, disOut, exHost, fname, execUsername, envp);
    } else {
	setuid(getuid()); 

	disOut[fidx] = fname;	    
	lsfExecvp(disOut[0], disOut);
	fprintf(stderr, I18N_FUNC_S_S_FAIL_S, "execvp", disOut[0], strerror(errno));
    }
    exit(-1);

} 


 
static void
remoteOutput(int fidx, char **disOut, char *exHost, char *fname,
	     char *execUsername, char **envp )
{
    char buf[MAXFILENAMELEN];
    char *args[MAX_PEEK_ARGS+4];
    char lsfUserName[MAXLINELEN];

#   define RSHCMD "rsh"
#if 0
    if ((getLSFUser_(lsfUserName, MAXLINELEN) == 0)
			&& strcmp(lsfUserName, execUsername)) { 
	

    if (useTmp(exHost, fname)) {
        sprintf(buf, "/tmp/.lsbtmp%d/%s", (int) getuid(), fname);
        disOut[fidx] = buf;
    } else {
        disOut[fidx] = fname;
    }

	args[0] = RSHCMD;
	args[1] = exHost;
	args[2] = "-l";
	args[3] = execUsername;
	if (fidx == 2) {
	    args[4] = disOut[0];
	    args[5] = disOut[1];
	    args[6] = disOut[2];
	    args[7] = NULL;
	} else  {
	    args[4] = disOut[0];
	    args[5] = disOut[1];
	    args[6] = NULL;
	}
	
	
	lsfExecvp(RSHCMD, args);
	fprintf(stderr, I18N_FUNC_S_S_FAIL_S, "execvp", args[0], strerror(errno)); 
	return;
    } 
#endif
    if (useTmp(exHost, fname)) {
	sprintf(buf, "/tmp/.lsbtmp%d/%s", (int) getuid(), fname);
	disOut[fidx] = buf;
    } else
	disOut[fidx] = fname;

    if (ls_initrex(1, 0) < 0) {
	ls_perror("ls_initrex");
	return;
    }

    ls_rfcontrol(RF_CMD_RXFLAGS, REXF_CLNTDIR);
    ls_rexecve(exHost, disOut, REXF_CLNTDIR, envp);

    fprintf(stderr, I18N_FUNC_S_S_FAIL_S, "ls_rexecv", disOut[0], ls_sysmsg());

} 

static int
useTmp(char *exHost, char *fname)
{
    int pid;
    LS_WAIT_T status;
    struct stat st;
    char *fName;
   
    fName = "useTmp";
    if ((pid = fork()) == 0) {
	if (ls_initrex(1, 0) < 0) {
	    ls_perror("ls_initrex");
	    exit(FALSE);
	}
	
	ls_rfcontrol(RF_CMD_RXFLAGS, REXF_CLNTDIR);

	if (ls_rstat(exHost, fname, &st) < 0) {
	    if (lserrno == LSE_FILE_SYS && 
	       (errno == ENOENT || errno == EACCES)) {
		exit(TRUE);
	    }

	    ls_perror("ls_rstat");
	}

	exit(FALSE);
    }
		
    if (pid == -1) {
	perror ("fork");
	return (FALSE);
    }

    if (waitpid(pid, &status, 0) == -1) {
	perror("waitpid");
	return (FALSE);
    }

    return (WEXITSTATUS(status));
} 
 
 
static void
stripClusterName(char *str)
{
    char *p;

    if ((p = strchr(str, '@')) == NULL) {
	return;
    }
    str[p-str] = '\0';
    return;
}
