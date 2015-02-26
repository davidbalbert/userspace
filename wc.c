#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <err.h>
#include <stdint.h>

#define BUFSIZE 1024 * 8

int
main(int argc, char *argv[])
{
        if (argc != 2) {
                printf("usage: %s <file>\n", argv[0]);
                return 1;
        }

        int fd = open(argv[1], O_RDONLY);

        if (fd == -1) {
                warn("%s", argv[1]);
                return 1;
        }

        char buf[BUFSIZE], last = 0;
        ssize_t n_bytes = 0;
        uintmax_t total_bytes = 0, total_lines = 0;

        while (1) {
                n_bytes = read(fd, buf, BUFSIZE);

                if (n_bytes == -1) {
                        warn(NULL);
                        close(fd);
                        return 1;
                } else if (n_bytes == 0) { // end of file
                        break;
                }

                for (int i = 0; i < n_bytes; i++) {
                        if (buf[i] == '\n') {
                                total_lines++;
                        }

                        total_bytes++;

                        last = buf[i];
                }
        }

        if (last == '\n') {
                total_lines--;
        } else if (total_lines == 0 && total_bytes > 0) {
                total_lines++;
        }

        printf("%ju bytes, %ju lines\n", total_bytes, total_lines);

        close(fd);

        return 0;
}
