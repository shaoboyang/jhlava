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
    fprintf(stderr, "lsrmhost [-h] [-V] hostname\n");
}

/* Da main()
 *
 * lsrmhost zumzum
 */
int
main(int argc, char **argv)
{
    int cc;
    struct hostent *hp;
    char hostName[MAXHOSTNAMELEN];

    if (argc != 2) {
        usage();
        return -1;
    }

    while ((cc = getopt(argc, argv, "Vh")) != EOF) {
        switch (cc) {
            case 'V':
                fputs(_LS_VERSION_, stderr);
                return 0;
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
%s: invalid hostname %s\n", __func__, argv[argc - 1]);
        return -1;
    }

    strcpy(hostName, hp->h_name);

    cc = ls_rmhost(hostName);
    if (cc < 0) {
        ls_perror("ls_rmhost");
        return -1;
    }

    printf("Host %s removed.\n", hp->h_name);

    return 0;
}
