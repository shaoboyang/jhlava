/*
 * Copyright (C) 2011 David Bigagli
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

/* Note. lsrun is missing from the openlava distribution. We have
 * developed our own version which merges lsrun and lsgrun functionalites
 * in one command. The API documentation is available on the internet
 * and examples are provided in the openlava Programming Guide, the one
 * we have is for version 3.2 Fourth Edition, august 1998.
 * It works great.
 */

#include <stdlib.h>
#include <string.h>
#include "../lsf.h"
#include "../lib/lproto.h"

struct lsrunParams {
    char **hlist;
    char **cmd;
    char *resReq;
    char *hosts;
    char *cwd;
    int verbose;
    int parallel;
    int pty;
};
extern char **environ;
static struct lsrunParams *P;

static void
usage(void)
{
    fprintf(stderr, "\
lsrun: [-h] [-V] [-P] [-v] [-p] [-R] [-m hosts...][-d path]\n");
}
static int gethostsbyname(const char *, char **);
static char **gethostsbylist(int *);
static char **gethostsbyresreq(int *);

int
main(int argc, char **argv)
{
    int cc;
    int num;
    int tid;
    char **hlist;
    int k;
    char **envp;
    char **unique_host_list = NULL;
    int unique_host_num = 0;
    int i = 0, find = FALSE;

    putenv("POSIXLY_CORRECT=y");

    P = calloc(1, sizeof(struct lsrunParams));

    while ((cc = getopt(argc, argv, "hVPvpR:m:d:")) != EOF) {

        switch (cc) {
            char *err;
            case 'V':
                fprintf(stderr, "%s\n", _JHLAVA_PROJECT_);
                return 0;
            case 'p':
                P->parallel = 1;
                break;
            case 'v':
                P->verbose = 1;
                break;
            case 'R':
                P->resReq = optarg;
                break;
            case 'P':
                P->pty = 1;
                break;
            case 'm':
                P->hosts = optarg;
                if (gethostsbyname(P->hosts, &err) < 0) {
                    fprintf(stderr, "\
lsrun: cannot resolve %s hostname", err);
                    free(err);
                    return -1;
                }
                break;
            case 'd':               
                    P->cwd = optarg;
                break;
            case 'h':
            case '?':
                usage();
                return -1;
        }
    }

    P->cmd = &argv[optind];
    if (P->cmd[0] == NULL) {
        usage();
        free(P);
        return -1;
    }
    /* Ignore SIGUSR1 and use ls_rwait()
     * to poll for done tasks.
     */
    signal(SIGUSR1, SIG_IGN);
    if (P->hosts)
        hlist = gethostsbylist(&num);
    else
        hlist = gethostsbyresreq(&num);

    if (hlist == NULL) {
        free(P);
        return -1;
    }

    if(P->cwd){
        putEnv("LSRUN_CWD", P->cwd);
    }
    /* initialize the remote execution
     * library
     */
    cc = ls_initrex(1, 0);
    if (cc < 0) {
        ls_perror("ls_initrex()");
        return -1;
    }
/*bug 73 problem one:when environ in ls_rtaske function will be unrecognizable code.
  so copy another before ls_rtaske function execute*/
    for (k = 0; environ[k]; k++)
        ;
    envp = (char **)calloc(k + 1, sizeof(char *));
    for (k = 0; environ[k]; k ++)
        envp[k] = strdup(environ[k]);
    envp[k] = NULL;

    unique_host_list = (char **)calloc(num, sizeof(char*));
    if(unique_host_list == NULL){
        ls_perror("calloc err,\n");
        return -1;
    }
    
    for (cc = 0; cc < num; cc++) {
        find = FALSE;
        for(i=0; i<num; i++){
            if(unique_host_list[i] != NULL){
                if(!strcmp(unique_host_list[i], hlist[cc])){
                    find = TRUE;
                    break;
                }
            }
        }
        if(TRUE == find)
            continue;
        unique_host_list[unique_host_num++] = strdup(hlist[cc]);
        
        if (P->verbose) {
            printf("Running task on host %s\n", hlist[cc]);
        }

        tid = ls_rtaske(hlist[cc],
                        P->cmd,
                        P->pty ? REXF_USEPTY : 0,
                        envp);
        if (tid < 0) {
            fprintf(stderr, "\
lsrun: ls_rtaske() failed on host %s: %s\n",
                    hlist[cc], ls_sysmsg());
        }

        if (P->verbose)
            printf("Task %d on host %s started\n", tid, hlist[cc]);

        /* the host was manually selected
         * so let's tell li to jackup the
         * load so this host won't be
         * selected again
         */
        if (P->hosts) {
            struct placeInfo place;

            strcpy(place.hostName, hlist[cc]);
            place.numtask = 1;

            ls_loadadj(P->resReq, &place, 1);
        }
    }

    if (P->verbose)
        printf("Going to ls_rwait() for %d tasks\n", num);

    while (unique_host_num > 0) {
        LS_WAIT_T stat;
        struct rusage ru;

        tid = ls_rwait(&stat, 0, &ru);
        if (tid < 0) {
            ls_perror("ls_rwait()");
            break;
        }

        if (P->verbose)
            printf("Task %d done\n", tid);

        --unique_host_num;
    }

    /* the openlava library keeps memory to
     * this array inside, but since we are not
     * calling it again this should be safe.
     */
    free(P);
    for(i=0; i<num; i++){
        if(unique_host_list[i] != NULL){
            free(unique_host_list[i]);
            unique_host_list[i] = NULL;
        }
    }
    free(unique_host_list);
    unique_host_list = NULL;
    return 0;
}

/* gethostbylist()
 * If user specified the list of host where to
 * run we ignore resource requirement an pty.
 */
static char **
gethostsbylist(int *num)
{
    int cc;
    char *p;
    char *p0;
    char *word;
    char **hlist;

    p0 = p = strdup(P->hosts);

    cc = 0;
    while ((word = getNextWord_(&p)))
        ++cc;
    free(p0);
    *num = cc;

    if (cc == 0) {
        fprintf(stderr, "\
%s: Not enough host(s) currently eligible\n", __FUNCTION__);
        return NULL;
    }

    hlist = calloc(cc, sizeof(char *));
    cc = 0;
    p0 = p = strdup(P->hosts);
    while ((word = getNextWord_(&p))) {
        hlist[cc] = strdup(word);
        ++cc;
    }
    free(p0);

    return hlist;

}

/* gethostbyresreq()
 */
static char **
gethostsbyresreq(int *num)
{
    char **hlist;

    /* ask lim for only one host if not
     * parallel request, in this case lim
     * will schedule different host all
     * the time avoid to overload on
     * host.
     */
    *num = 0;
    if (P->parallel == 0)
        *num = 1;

    hlist = ls_placereq(P->resReq, num, 0, NULL);
    if (hlist == NULL) {
        ls_perror("ls_placereq()");
        return NULL;
    }

    return hlist;
}

/* gethostsbyname()
 * Valide a list of given hosts, each host must be resolvable
 * with gethostbyname().
 */
static int
gethostsbyname(const char *list, char **err)
{
    return 0;
}
