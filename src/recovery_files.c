/*  TIPE 2024-2025, MPI Lycée Louis Thuillier: Cardot Clément & William Guerin-Garnier

    Objective: Take "data_hex.txt" as argument and recreate all files
               possibly stored in it, according to the type provided.

    - Requires a text file "data_hex.txt" filled with file data
    - Use "storage_sim.c" to generate this file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    const unsigned char *header;
    const unsigned char *footer;
    size_t header_size;
    size_t footer_size;
} Extension;

int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

unsigned char hex_to_byte(const char *hex) {
    return (hex_char_to_int(hex[0]) << 4) | hex_char_to_int(hex[1]);
}

void extract_file(const char *hex_filename, const Extension *extension) {
    FILE *file = fopen(hex_filename, "r");
    if (file == NULL) {
        perror("Error opening the hexadecimal text file");
        return;
    }

    char buffer[3];
    unsigned char *file_data = NULL;
    size_t file_size = 0;
    int file_counter = 0;
    int inside_file = 0;

    while (fscanf(file, "%2s", buffer) == 1) {
        unsigned char byte = hex_to_byte(buffer);

        // Check for header signature
        if (!inside_file && byte == extension->header[0]) {
            int match = 1;
            for (size_t i = 1; i < extension->header_size; i++) {
                fscanf(file, "%2s", buffer);
                if (hex_to_byte(buffer) != extension->header[i]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                inside_file = 1;
                file_data = malloc(extension->header_size);
                memcpy(file_data, extension->header, extension->header_size);
                file_size = extension->header_size;
                continue;
            }
        }

        // Accumulate bytes until footer is found
        if (inside_file) {
            file_data = realloc(file_data, file_size + 1);
            file_data[file_size++] = byte;

            // Check for footer signature
            if (file_size >= extension->footer_size) {
                int match = 1;
                for (size_t i = 0; i < extension->footer_size; i++) {
                    if (file_data[file_size - extension->footer_size + i] != extension->footer[i]) {
                        match = 0;
                        break;
                    }
                }
                if (match) {
                    // Write the recovered file to disk
                    char output_name[50];
                    sprintf(output_name, "recovered_%d.%s", ++file_counter, extension->name);
                    FILE *output_file = fopen(output_name, "wb");
                    if (output_file != NULL) {
                        fwrite(file_data, 1, file_size, output_file);
                        fclose(output_file);
                        printf("File extracted: %s\n", output_name);
                    } else {
                        perror("Error creating the output file");
                    }

                    free(file_data);
                    file_data = NULL;
                    file_size = 0;
                    inside_file = 0;
                }
            }
        }
    }

    fclose(file);
    if (file_data) free(file_data);
}

int main() {
    // JPEG: header FF D8, footer FF D9
    const unsigned char jpeg_header[] = {0xFF, 0xD8};
    const unsigned char jpeg_footer[] = {0xFF, 0xD9};
    Extension jpeg = { "jpeg", jpeg_header, jpeg_footer, sizeof(jpeg_header), sizeof(jpeg_footer) };

    // TXT: header 0x00, footer 0x0A (unreliable — commented out)
    // const unsigned char txt_header[] = {0x00};
    // const unsigned char txt_footer[] = {0x0A};
    // Extension txt = { "txt", txt_header, txt_footer, sizeof(txt_header), sizeof(txt_footer) };

    // PNG: header 89 50 4E 47, footer 49 45 4E 44 AE 42 60 82
    const unsigned char png_header[] = {0x89, 0x50, 0x4E, 0x47};
    const unsigned char png_footer[] = {0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};
    Extension png = { "png", png_header, png_footer, sizeof(png_header), sizeof(png_footer) };

    extract_file("data_hex.txt", &jpeg);
    // extract_file("data_hex.txt", &txt);
    extract_file("data_hex.txt", &png);

    return 0;
}
