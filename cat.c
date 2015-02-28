#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>

#define BUF_SIZE 1024 * 8

void
cat(char *name) {
        int fd = open(name, O_RDONLY);

        if (fd == -1) {
                warn("%s", name);
                exit(1);
        }

        char buf[BUF_SIZE];
        ssize_t nbytes;

        for (;;) {
                nbytes = read(fd, buf, BUF_SIZE);

                if (nbytes == -1) {
                        warn(NULL);
                        exit(1);
                } else if (nbytes == 0) {
                        break;
                }

                char *p = buf;
                ssize_t b;

                while (nbytes > 0) {
                        b = write(STDOUT_FILENO, p, nbytes);

                        if (b == -1) {
                                warn(NULL);
                                exit(1);
                        }

                        p += b;
                        nbytes -= b;
                }

        }

        close(fd);
}

int
main(int argc, char *argv[])
{
        if (argc == 1) {
                return 1;
        }

        for (int i = 1; i < argc; i++) {
                cat(argv[i]);
        }

        return 0;
}
