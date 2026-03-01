#include <stdlib.h>

#include "../interp/loader.h"
#include "analyzer.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.bc\n", argv[0]);
        return 1;
    }

    DEBUG_LOG("Loading bytecode from: %s\n", argv[1]);

    bytefile *bytecode_file = load_bytecode_file(argv[1]);
    if (!bytecode_file) {
        fprintf(stderr, "Failed to load bytecode\n");
        return 1;
    }

    DEBUG_LOG("Calling analyze_bytecode...\n");
    analyze_bytecode(bytecode_file);
    DEBUG_LOG("analyze_bytecode finished\n");
    free(bytecode_file->data);
    free(bytecode_file);

    return 0;
}
