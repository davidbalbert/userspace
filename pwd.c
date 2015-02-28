#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>

int
main(int argc, char *argv[])
{
        char buf[MAXPATHLEN];

        puts(getcwd(buf, MAXPATHLEN));

        return 0;
}
