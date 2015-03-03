#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFSIZE 1024 * 8

// Grammar
//
// line <- pipeline
//
// pipeline <- command ['|' command]*
//
// command <- program arg*
//
// program <- token
// arg <- token
//
// token <- ["] [^"]* ["]
//        | \S+

typedef struct Vector {
        size_t size;
        size_t capa;
        char   **a;
} Vector;

Vector *
vector_new(size_t capacity)
{
        Vector *v = calloc(1, sizeof(Vector));

        if (v == NULL) {
                err(1, "Couldn't allocate vector with capacity %zu", capacity);
        }

        v->size = 0;
        v->capa = capacity;
        v->a = calloc(capacity, sizeof(char *));

        if (v->a == NULL) {
                err(1, "Couldn't allocate vector with capacity %zu", capacity);
        }

        return v;
}

size_t
vector_size(Vector *self)
{
        return self->size;
}

char *
vector_get(Vector *self, size_t index)
{
        assert(index < self->size);
        return self->a[index];
}

static void
vector_resize(Vector *self)
{
        char **new_a = calloc(self->capa * 2, sizeof(char *));

        if (new_a == NULL) {
                err(1, "Couldn't resize vector to capacity %lu", self->capa * 2);
        }

        for (int i = 0; i < self->capa; i++) {
                new_a[i] = self->a[i];
        }

        free(self->a);
        self->a = new_a;
        self->capa *= 2;
}

void
vector_append(Vector *self, char *s)
{
        if (self->size == self->capa) {
                vector_resize(self);
        }

        self->a[self->size] = s;
        self->size++;
}

void
vector_free(Vector *self)
{
        free(self->a);
        free(self);
}

char **
vector_to_argv(Vector *self)
{
        char **argv = calloc(vector_size(self) + 1, sizeof(char *));

        if (argv == NULL) {
                err(1, "Couldn't allocate argv");
        }

        for (int i = 0; i < vector_size(self); i++) {
                argv[i] = vector_get(self, i);
        }

        argv[self->size] = NULL;

        return argv;
}


typedef struct Command {
        char **argv;
} Command;

void
get_line(char *buf, size_t size)
{
        printf("$ ");
        fgets(buf, size, stdin);
}

Command *
parse_command(char *s)
{
        Command *c = calloc(1, sizeof(Command));

        if (c == NULL) {
                warn("Couldn't allocate Command");
                exit(1);
        }

        char *token;
        Vector *v = vector_new(10);

        while ((token = strsep(&s, " \t\r\n")) != NULL) {
                if (*token == '\0') {
                        continue;
                }

                token = strdup(token);

                if (token == NULL) {
                        err(1, "Couldn't strdup token");
                }

                vector_append(v, token);
        }

        c->argv = vector_to_argv(v);
        vector_free(v);

        return c;
}

char *
find_program(Command *c)
{
        char *program = c->argv[0];

        if (program[0] == '/' && access(program, F_OK) == 0) {
                return program;
        }

        char *path = getenv("PATH");

        if (path == NULL && strstr(program, "/") != NULL && access(program, F_OK) == 0) {
                return program;
        } else if (path == NULL) {
                return NULL;
        }

        path = strdup(path);

        if (path == NULL) {
                err(1, "Couldn't strdup path");
        }

        char *dirname, *s = path;

        while ((dirname = strsep(&s, ":")) != NULL) {
                // dirname + '/' + command + '\0'
                size_t len = strlen(dirname) + 1 + strlen(program) + 1;
                char *fname = calloc(len, sizeof(char));

                snprintf(fname, len, "%s/%s", dirname, program);

                if (access(fname, F_OK) == 0) {
                        return fname;
                }

                free(fname);
        }

        free(path);

        if (strstr(program, "/") != NULL && access(program, F_OK) == 0) {
                return program;
        } else {
                return NULL;
        }
}

void
run(Command *c)
{
        if (c->argv[0] == NULL) {
                // The user typed nothing
                return;
        }

        if (strcmp(c->argv[0], "exit") == 0) {
                if (c->argv[1]) {
                        char *end;
                        int status = strtol(c->argv[1], &end, 10);

                        if (*end != '\0') {
                                fprintf(stderr, "%s is not a number\n", c->argv[1]);
                                exit(255);
                        }

                        exit(status);
                } else {
                        exit(0);
                }
        } else if (strcmp(c->argv[0], "cd") == 0) {
                char *path;

                if (c->argv[1]) {
                        path = c->argv[1];
                } else {
                        path = getenv("HOME");

                        if (path == NULL) {
                                fprintf(stderr, "cd: HOME not set");
                                return;
                        }
                }

                if (chdir(path) == -1) {
                        warn("cd %s", path);
                }

                return;
        }

        char *fname = find_program(c);

        if (fname == NULL) {
                fprintf(stderr, "%s: command not found\n", c->argv[0]);
                return;
        }

        pid_t pid = fork();

        if (pid) {
                int stat_loc = 0;
                waitpid(pid, &stat_loc, 0);
        } else {
                char **envp = { NULL };
                execve(fname, c->argv, envp);

                // we only get here if there was an error running fname
                err(127, "%s", c->argv[0]);
        }
}

void
free_command(Command *self)
{
        for (char **s = self->argv; *s; s++) {
                free(*s);
        }

        free(self->argv);
        free(self);
}

int
main(int argc, char *argv[])
{
        char buf[BUFSIZE];

        for (;;) {
                get_line(buf, sizeof(buf));
                Command *c = parse_command(buf);
                run(c);
                free_command(c);
        }
}
