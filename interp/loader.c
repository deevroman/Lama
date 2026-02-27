#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "loader.h"

bytefile* load_bytecode_file(const char* filename)
{
    DEBUG_LOG("[DEBUG] load_bytecode_file: opening %s\n", filename);

    FILE* f = fopen(filename, "rb");
    if (!f)
    {
        ERROR_LOG("Cannot open file: %s: %s\n", filename, strerror(errno));
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0)
    {
        perror("Failed to seek file");
        fclose(f);
        return NULL;
    }

    const long end_pos = ftell(f);
    if (end_pos < 0)
    {
        perror("Failed to tell file size\n");
        fclose(f);
        return NULL;
    }

    const size_t size = (size_t)end_pos;
    DEBUG_LOG("[PARSE] size=%zu\n", size);
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        perror("Failed to seek file\n");
        fclose(f);
        return NULL;
    }

    if (size <= sizeof(bytefile_data))
    {
        ERROR_LOG("Invalid bytecode file: too small to contain header\n");
        fclose(f);
        return NULL;
    }

    bytefile* file = malloc(sizeof(bytefile));
    file->data = malloc(size);
    if (!file->data)
    {
        ERROR_LOG("Failed to alloc %zu\n", size);
        fclose(f);
        return NULL;
    }

    const size_t read_bytes = fread(&file->data->stringtab_size, 1, size, f);
    if (read_bytes != size)
    {
        perror("Failed to read file\n");
        free(file->data);
        return NULL;
    }
    fclose(f);

    DEBUG_LOG("[PARSE] Read %zu bytes\n", read_bytes);
    DEBUG_LOG("[PARSE] stringtab_size=%d (0x%x)\n", file->data->stringtab_size, file->data->stringtab_size);
    DEBUG_LOG("[PARSE] global_area_size=%d (0x%x)\n", file->data->global_area_size, file->data->global_area_size);
    DEBUG_LOG("[PARSE] public_symbols_number=%d (0x%x)\n", file->data->public_symbols_number,
              file->data->public_symbols_number);

    const size_t payload_bytes = size - sizeof(bytefile_data);
    const size_t public_bytes = file->data->public_symbols_number * sizeof(public_symbol_t);
    const size_t string_bytes = file->data->stringtab_size;

    DEBUG_LOG("[PARSE] size=%zu, payload=%zu\n", size, payload_bytes);
    DEBUG_LOG("[PARSE] public_symbols_number=%d, public_bytes=%zu\n", file->data->public_symbols_number, public_bytes);
    DEBUG_LOG("[PARSE] stringtab_size=%d, string_bytes=%zu\n", file->data->stringtab_size, string_bytes);

    if (public_bytes > payload_bytes || public_bytes + string_bytes > payload_bytes)
    {
        ERROR_LOG(
            "Invalid bytecode: tables out of range: string_bytes=%zu, public_bytes=%zu, but file_size_without_header=%zu\n",
            string_bytes, public_bytes, payload_bytes
        );
        free(file->data);
        return NULL;
    }

    if (string_bytes + public_bytes == payload_bytes)
    {
        ERROR_LOG("Empty code section\n");
        free(file->data);
        return NULL;
    }

    file->public_ptr = (int32_t*)file->data->buffer;
    file->string_ptr = file->data->buffer + public_bytes;
    file->code_ptr = file->string_ptr + string_bytes;
    file->code_size = payload_bytes - public_bytes - string_bytes;

    DEBUG_LOG("[DEBUG] First 20 bytes of buffer: ");
    for (int i = 0; i < 20 && i < (int)payload_bytes; i++)
    {
        DEBUG_LOG("%02x ", (unsigned char)file->data->buffer[i]);
    }
    DEBUG_LOG("\n");
    DEBUG_LOG("[DEBUG] buffer addr=%p, string_ptr offset=%ld, code_ptr offset=%ld\n",
              (void*)file->data->buffer,
              (long)(file->string_ptr - file->data->buffer),
              (long)(file->code_ptr - file->data->buffer));

    DEBUG_LOG("[DEBUG] Bytecode structure:\n");
    DEBUG_LOG("[DEBUG]   stringtab_size=%d\n", file->data->stringtab_size);
    DEBUG_LOG("[DEBUG]   global_area_size=%d\n", file->data->global_area_size);
    DEBUG_LOG("[DEBUG]   public_symbols_number=%d\n", file->data->public_symbols_number);
    DEBUG_LOG("[DEBUG]   public_bytes=%zu\n", public_bytes);
    DEBUG_LOG("[DEBUG]   string_bytes=%zu\n", string_bytes);
    DEBUG_LOG("[DEBUG]   code_size=%zu\n", file->code_size);
    DEBUG_LOG("[DEBUG]   First code bytes: %02x %02x %02x %02x\n",
              (unsigned char)file->code_ptr[0],
              (unsigned char)file->code_ptr[1],
              (unsigned char)file->code_ptr[2],
              (unsigned char)file->code_ptr[3]);

    return file;
}
