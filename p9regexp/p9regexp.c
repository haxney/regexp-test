/**
 * Runner for plan9 regexp library
 */

#include "lib9.h"
#include <regexp9.h>
#include <stdio.h>


int main(int argc, char **argv) {

        Reprog *compiled = regcomp(argv[1]);
        char *line;
        size_t len = 0;
        ssize_t read_length;

        while ((read_length = getline(&line, &len, stdin)) != -1) {
                if (regexec(compiled, line, 0, 0) == 1) {
                        fprintf(stdout, "%s", line);
                };
        }

        exit(0);
}

/* Local Variables: */
/* c-basic-offset: 8 */
/* End: */
