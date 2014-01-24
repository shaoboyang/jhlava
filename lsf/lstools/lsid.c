/*
 * Copyright (C) 2011 David Bigagli
 * $Id: lsid.c 397 2007-11-26 19:04:00Z mblack $
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

#include <stdio.h>
#include <stdlib.h>
#include "../lsf.h"

#include "../lib/lib.table.h"
#include "../lib/lproto.h"

static void usage(char *);
extern int errLineNum_;

static void
usage(char *cmd)
{
    fprintf (stderr, "%s: %s [-h] [-V]\n", I18N_Usage, cmd);
}

int
main(int argc, char **argv)
{
    char *name;
    int cc;

    if (ls_initdebug(argv[0]) < 0) {
        ls_perror("ls_initdebug");
        return -1;
    }

    while ((cc = getopt(argc, argv, "hV")) != EOF) {
        switch (cc) {
            case 'h':
                usage(argv[0]);
                return 0;
            case 'V':
                fputs(_LS_VERSION_, stderr);
                return 0;
            default:
                usage(argv[0]);
                return -1;
        }
    }
    puts(_LS_VERSION_);

    TIMEIT(0, (name = ls_getclustername()), "ls_getclustername");
    if (name == NULL) {
        ls_perror("ls_getclustername()");
        return -1;
    }
    printf("My cluster name is %s\n", name);

    TIMEIT(0, (name = ls_getmastername()), "ls_getmastername");
    if (name == NULL) {
        ls_perror("ls_getmastername()");
        return -1;
    }
    printf("My master name is %s\n", name);

    return 0;
}

