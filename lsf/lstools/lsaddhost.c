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

#include "../lsf.h"
#include "../lib/lib.h"

static void
usage(void)
{
    fprintf(stderr, "\
lsaddhost: [-h] [-V] -m model -t type -f cpuFactor \
-D numDisks -R \"resource list\" -w \"windows\" \
-b \"busy onlist \" [-v] hostname\n");
}
static int getResList(struct hostEntry *, const char *);
static int getBusyThr(struct hostEntry *, const char *);
static void freeInfo(struct hostEntry **);
static void printInfo(struct hostEntry *);
static char buf[BUFSIZ];

/* Da main()
 *
 * lsadd -m Inteli5 -t linux -f 100 -D 5 -R "" -w "" -b "" sofia
 *
 */
int
main(int argc, char **argv)
{
    int v;
    int cc;
    struct hostEntry *hPtr;
    struct hostent *hp;

    v = 0;
    hPtr = calloc(1, sizeof(struct hostEntry));
    hPtr->rcv = 1;
    hPtr->window = strdup("");
    hPtr->numIndx = 11;
    hPtr->busyThreshold = calloc(11, sizeof(float));
    for (cc = 0; cc < 11; cc++)
        hPtr->busyThreshold[cc] = INFINIT_LOAD;

    while ((cc = getopt(argc, argv, "Vhvm:t:f:D:R:w:b:")) != EOF) {
        switch (cc) {
            case 'V':
                fputs(_LS_VERSION_, stderr);
                return 0;
            case 'm':
                strcpy(hPtr->hostModel, optarg);
                break;
            case 't':
                strcpy(hPtr->hostType, optarg);
                break;
            case 'f':
                hPtr->cpuFactor = atof(optarg);
                break;
            case 'D':
                hPtr->nDisks = atoi(optarg);
                break;
            case 'R':
                getResList(hPtr, optarg);
                break;
            case 'w':
                free(hPtr->window);
                hPtr->window = strdup(optarg);
                break;
            case 'b':
                getBusyThr(hPtr, optarg);
                break;
            case 'v':
                v = 1; /* verbose */
                break;
            case 'h':
            case '?':
            default:
                usage();
                return -1;
        }
    }

    hp = Gethostbyname_(argv[argc - 1]);
    if (hp == NULL) {
        fprintf(stderr, "\
%s: invalid hostname %s\n", __func__, argv[optind]);
        return -1;
    }

    strcpy(hPtr->hostName, argv[optind]);

    if (v)
        printInfo(hPtr);

    cc = ls_addhost(hPtr);
    if (cc < 0) {
        ls_perror("ls_addhost");
        freeInfo(&hPtr);
        return -1;
    }

    printf("Host %s added.\n", hPtr->hostName);
    freeInfo(&hPtr);

    return 0;
}

static int
getResList(struct hostEntry *hPtr, const char *str)
{
    /* these are the static resources defined in the
     * lsf.cluster file.
     */
    char *p;
    char *word;
    int cc;

    cc = 0;
    p = buf;
    strcpy(buf, str);
    while (getNextWord_(&p))
        ++cc;

    hPtr->nRes = cc;
    hPtr->resList = calloc(cc, sizeof(char *));
    p = buf;
    cc = 0;
    while ((word = getNextWord_(&p))) {
        hPtr->resList[cc] = strdup(word);
        ++cc;
    }

    return 0;
}
static int
getBusyThr(struct hostEntry *hPtr, const char *str)
{
    return 0;
}

static void
freeInfo(struct hostEntry **hPtr)
{
    int cc;

    for (cc = 0; cc < (*hPtr)->nRes; cc++)
        free((*hPtr)->resList[cc]);
    free((*hPtr)->resList);
    free((*hPtr)->busyThreshold);
    free((*hPtr)->window);
    free(*hPtr);
}

static void
printInfo(struct hostEntry *hPtr)
{
    int cc;

    printf("hostName: %s\n", hPtr->hostName);
    printf("hostType: %s\n", hPtr->hostType);
    printf("hostModel: %s\n", hPtr->hostModel);
    printf("cpuFactor: %4.2f\n", hPtr->cpuFactor);
    printf("nDisks: %d\n", hPtr->nDisks);
    printf("Resources:\n");
    for (cc = 0; cc < hPtr->nRes; cc++)
        printf("  %s\n", hPtr->resList[cc]);
    printf("Windows: %s\n", hPtr->window);
    printf("busyThresholds:\n");
    for (cc = 0; cc < hPtr->numIndx; cc++)
        printf("   %4.2f\n", hPtr->busyThreshold[cc]);
}
