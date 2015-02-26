#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <stdint.h>

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

        struct stat stat_buf;

        if (fstat(fd, &stat_buf)) {
                warn("Couldn't stat %s", argv[1]);
                return 1;
        }

        char *contents = mmap(0, stat_buf.st_size, PROT_READ, MAP_SHARED, fd, 0);

        if (contents == MAP_FAILED) {
                warn("Couldn't mmap %s", argv[1]);
                return 1;
        }


        char last;
        uintmax_t total_bytes = 0, total_lines = 0;

        for (int i = 0; i < stat_buf.st_size; i++) {
                if (contents[i] == '\n') {
                        total_lines++;
                }

                total_bytes++;
                last = contents[i];
        }

        if (last == '\n') {
                total_lines--;
        } else if (total_lines == 0 && total_bytes > 0) {
                total_lines++;
        }

        printf("%ju bytes, %ju lines\n", total_bytes, total_lines);

        munmap(contents, stat_buf.st_size);

        close(fd);

        return 0;
}
