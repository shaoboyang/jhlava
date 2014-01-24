/*
 * Copyright (C) 2013 jhinno Inc
 */

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <assert.h>


#define MAX_LINE 4096
#define MAX_RESOURCE_NUM 256
#define DEFAULT_INTVL 5

typedef void (*SIGFUNCTYPE)(int);

struct elimdata{
                char name[MAX_LINE];  //the full pathname of elim
                pid_t pid;            //the elim pid
                int pipe_fd[2]; 	  //the elim pipe 
                FILE *fp;
                int rd;    			  //the elim Pipe descriptor for select
                int intvl;
                char valStr[MAX_LINE];
}elim[MAX_RESOURCE_NUM];

//the number of all elim start
int count = 0;


// wrapper for signal
SIGFUNCTYPE Signal_Wrapper(int sig, void (*handler)(int))
{
	struct sigaction act, oact;

	act.sa_handler = handler;
	act.sa_flags = 0;	
	sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, sig);
	if(sigaction(sig, &act, &oact) == -1){
            oact.sa_handler = (void (*)())SIG_ERR;
	}
	return(oact.sa_handler);
} 


void elim_clean(int i)
{
    close(elim[i].pipe_fd[0]);
    fclose(elim[i].fp);
    elim[i].fp = NULL;
    elim[i].pid = -1;
    elim[i].rd = -1;
    elim[i].intvl = -1;
    elim[i].valStr[0] = '\0';
}

//Handling of child processes exit function
void child_handler_(int sig)
{
	int pid;
	int status;
	int i;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
		for(i = 0;i<count;i++){
			if(pid == elim[i].pid){
				//fprintf(stderr,"elim (pid=%d) died (exit_code=%d,exit_sig=%d)\n", elim[i].pid,
				//	WEXITSTATUS (status),WIFSIGNALED (status) ? WTERMSIG (status) : 0);
					elim_clean(i);
			}
		} 
	}
}

void term_handler_(int signum)
{

	int i;
	
	for(i = 0;i<count;i++){
		if(elim[i].pid != -1)
		kill(elim[i].pid, SIGTERM);
	}

	sleep (2);
	exit(0);

}
//the function for start a elim,first create a pipe ,then open the pipe,fork process to execute the elim.
void startElim(int num)
{
	int i = num;
	int Elimexist = 0;
	char elimpath[MAX_LINE] = {0};
	char *argv[] = {elim[i].name,(char *)0};
	int pid;

	if(access(elim[i].name,F_OK) != -1){
		Elimexist = 1;
		strcpy(elimpath,elim[i].name);
	}
		
	if(Elimexist == 1){
		
		if(pipe(elim[i].pipe_fd) == -1){
			perror("pipe_pfd");
            elim[i].pid = -1;
			return;
        }
	
		if((pid = fork()) == 0){
			close(elim[i].pipe_fd[0]);
			dup2(elim[i].pipe_fd[1], STDOUT_FILENO);
			execvp(elimpath,argv);
			perror("execvp err");
			close(elim[i].pipe_fd[1]);
			exit(EXIT_FAILURE);
		}

		if(pid == -1){                  
			close(elim[i].pipe_fd[0]);
			close(elim[i].pipe_fd[1]);
			perror("fork-pid1");
			elim[i].pid = -1;
			return;
		}

		elim[i].pid = pid;
		close(elim[i].pipe_fd[1]);

		if((elim[i].fp = fdopen(elim[i].pipe_fd[0], "r")) == NULL){
			perror("fp");
			elim[i].pid = -1;
			elim[i].fp = NULL;
			return;
		}

		elim[i].rd = fileno(elim[i].fp);
		elim[i].intvl = DEFAULT_INTVL;
		sleep(1);

	}
	else{ 
		elim[i].pid = -1;
		elim[i].fp = NULL;
		elim[i].rd = -1;
		elim[i].intvl = -1;
	}
}

static void
melim_output()
{
    static time_t prev = 0;
    time_t now;
    char outputStr[MAX_LINE];
    char lineStr[MAX_LINE];
    char namValStr[MAX_LINE];
    int index = 0;
    int totalIndex = 0;
    int n;
    int i;

    now  = time(0);
    if (now - prev < DEFAULT_INTVL)
	return;
    prev = now;

    lineStr[0] = '\0';
    for (i = 0; i < count; i++) {
	if (elim[i].pid <= 0)
	    continue;

	if (elim[i].valStr[0] == '\0')
	    continue;

	n = sscanf(elim[i].valStr, "%d %[^\n]", 
		    &index,
		    namValStr);
	if (n != 2)
	    continue;
	
	if (MAX_LINE - strlen(lineStr) < strlen(namValStr) + 1 + 32)
	    continue;

	totalIndex += index;
    strcat(lineStr, " ");
	strcat(lineStr, namValStr);
    }

    sprintf(outputStr, "%d %s\n", totalIndex, lineStr);
    
    write(1, outputStr, strlen(outputStr));
}

int main(int argc, char *argv[])
{
	char line[MAX_LINE] = {0};
	char tmp[MAX_LINE] = {0};
	fd_set rfds;
	struct timeval tv;
	int retval;
	char *p = NULL;
	char *delim = " ";
	int i = 0,max_rd = 0;
	char *lsf_resources = NULL;
	char *lsf_serverdir = NULL;

	//Set for SIGCHLD handling behavior child_handler
	Signal_Wrapper(SIGCHLD, (SIGFUNCTYPE) child_handler_);
	Signal_Wrapper(SIGTERM, (SIGFUNCTYPE) term_handler_);

	// get resource information form LSF_RESOURCES
	if((lsf_resources = getenv("LSF_RESOURCES")) == NULL){
		perror("Can not get resource in LSF_RESOURCES.");
		exit(EXIT_FAILURE);
		}

	// get elim PATH	
	if((lsf_serverdir=getenv("LSF_SERVERDIR")) == NULL){
        	perror("Can not get elim PATH in LSF_SERVERDIR.");
        	exit(EXIT_FAILURE);
        }
	
	strcpy(tmp,lsf_resources);

	sprintf(elim[i].name,"%s/elim",lsf_serverdir);
	startElim(i);
	i++;

	//Traversal resource in "LSF_RESOURCES".As the split with spaces.	
	p = strtok(tmp,delim);
	while(p != NULL){

		sprintf(elim[i].name,"%s/elim.%s",lsf_serverdir,p);

		//start a elim
		startElim(i); 

		i++;
			
		p = strtok(NULL,delim);
	}

	
	count = i;
	
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	
	while(1){
		FD_ZERO(&rfds);

		max_rd = 0;

		for(i = 0;i<count;i++){

			if(elim[i].pid == -1)
				startElim(i);

			//get max elim Pipe descriptor for select funcation
			if(elim[i].rd > max_rd)
				max_rd = elim[i].rd;
			
			if(elim[i].pid != -1 && elim[i].rd != -1 &&elim[i].fp != NULL)
				FD_SET(elim[i].rd, &rfds);
		}

		retval = select(max_rd + 1, &rfds, NULL, NULL, NULL);
			
		if(retval == -1){
			perror("select()");
		}
		else if(retval){
			for(i = 0;i<count;i++){
				if(elim[i].pid != -1 && elim[i].rd != -1 && elim[i].fp != NULL){
					if(FD_ISSET(elim[i].rd, &rfds)){
						if(fgets(line, MAX_LINE, elim[i].fp) == NULL){
							perror("fgets");
							elim_clean(i);
						} else
							sprintf(elim[i].valStr, "%s", line);										
					}
				}
			}
		}

		melim_output();
	}
}
