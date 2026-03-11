#include <stdio.h>
#include <stdlib.h>
#include "loader.h"
#include "interpreter.h"
#include "verifier.h"

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

#ifdef VERIFY_BYTECODE
  DEBUG_LOG("Calling verify_bytecode...\n");
  verify_bytecode(bytecode_file);
  DEBUG_LOG("verify_bytecode finished\n");
#endif

  DEBUG_LOG("Calling interpret_bytecode...\n");
  interpret_bytecode(bytecode_file);
  DEBUG_LOG("interpret_bytecode finished\n");

  free(bytecode_file->data);
  free(bytecode_file);

  return 0;
}
