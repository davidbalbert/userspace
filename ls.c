#include <stdio.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

void
ls(char *name, int show_hidden)
{
        DIR *d = opendir(name);

        if (d == NULL) {
                warn("Couldn't open %s", name);
                exit(1);
        }

        for (;;) {
                struct dirent *ent = readdir(d);

                if (ent == NULL && errno) {
                        warn("Fail in readdir(3)");
                        exit(1);
                } else if (ent == NULL) {
                        break;
                }

                if ((ent->d_name[0] == '.' && show_hidden) || ent->d_name[0] != '.') {
                        puts(ent->d_name);
                }
        }

        if (closedir(d) == -1) {
                warn("Couldn't close %s", name);
                exit(1);
        }
}

int
main(int argc, char *argv[])
{
        int ch, show_hidden = FALSE;

        while((ch = getopt(argc, argv, "a")) != -1) {
                switch (ch) {
                case 'a':
                        show_hidden = TRUE;
                }
        }

        argc -= optind;
        argv += optind;

        if (argc <= 1) {
                ls(".", show_hidden);
        } else {
                ls(argv[1], show_hidden);
        }

        return 0;
}
