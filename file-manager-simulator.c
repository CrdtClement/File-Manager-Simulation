#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

/*  TIPE 2024-2025, MPI Lycée Louis Thuillier: Cardot Clément et William Guerrin-Garnier

Objectives: simulate a file manager
    - access to a database
    - operations on the database (add a file, rename it)

*/

//══════════════════════════════════════════ FUNCTION FOR 'file_type' ══════════════════════════════════════════

typedef struct {
    int id;
    char name[128];
    char extension[16];
    char address[256];
    long offset;
    long length;
} file_type;

void get_name(sqlite3 *db, file_type *file) {
    char query[512];
    snprintf(query, sizeof(query), "SELECT name FROM files WHERE id = %d;", file->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *name = (const char *)sqlite3_column_text(stmt, 0);
            strncpy(file->name, name, sizeof(file->name));
        }
    }
    sqlite3_finalize(stmt);
}

void get_extension(sqlite3 *db, file_type *file) {
    char query[512];
    snprintf(query, sizeof(query), "SELECT extension FROM files WHERE id = %d;", file->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *extension = (const char *)sqlite3_column_text(stmt, 0);
            strncpy(file->extension, extension, sizeof(file->extension));
        }
    }
    sqlite3_finalize(stmt);
}

void get_address(sqlite3 *db, file_type *file) {
    char query[512];
    snprintf(query, sizeof(query), "SELECT address FROM files WHERE id = %d;", file->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *address = (const char *)sqlite3_column_text(stmt, 0);
            strncpy(file->address, address, sizeof(file->address));
        }
    }
    sqlite3_finalize(stmt);
}

void get_offset(sqlite3 *db, file_type *file) {
    char query[512];
    snprintf(query, sizeof(query), "SELECT offset FROM files WHERE id = %d;", file->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            file->offset = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}

void get_length(sqlite3 *db, file_type *file) {
    char query[512];
    snprintf(query, sizeof(query), "SELECT length FROM files WHERE id = %d;", file->id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            file->length = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}

//═══════════════════════════════════════════ SQL / HEXA FUNCTION ═══════════════════════════════════════════

void extract_name_and_extension(file_type *file) {
    const char *file_name = strrchr(file->address, '/');  // Find the last '/'
    if (!file_name) {
        file_name = file->address; // If no '/' found, use the whole address
    } else {
        file_name++; // Skip the '/'
    }

    const char *ext = strrchr(file_name, '.');
    if (ext) {
        strcpy(file->extension, ext + 1); // Copy the extension without the dot
        strncpy(file->name, file_name, ext - file_name); // Copy the name without the extension
        file->name[ext - file_name] = '\0'; // Null terminate the string
    } else {
        strcpy(file->name, file_name); // No extension found
        strcpy(file->extension, "");
    }
}

int convert_file_to_hex(file_type *file) {
    FILE *binary_file = fopen(file->address, "rb");
    if (!binary_file) {
        fprintf(stderr, "Error opening the file: %s\n", file->address);
        return -1;
    }
    FILE *hex_file = fopen("data_hex.txt", "a");
    if (!hex_file) {
        fprintf(stderr, "Error opening the file data_hex.txt\n");
        fclose(binary_file);
        return -1;
    }
    unsigned char byte;
    while (fread(&byte, 1, 1, binary_file) == 1) {
        fprintf(hex_file, "%02X", byte);
    }
    fclose(binary_file);
    fclose(hex_file);
    return 0;
}

int delete_file(const char *address) {
    if (remove(address) == 0) {
        printf(" File '%s' deleted successfully.\n", address);
        return 0;
    } else {
        perror(" Error deleting the file");
        return -1;
    }
}

int add_file(sqlite3 *database, file_type *file) {
    char *error_message = 0;
    char query[512];
    long offset;
    long length = 0;

    // Extract the name and extension
    extract_name_and_extension(file);

    // Open the file in hexadecimal and determine the offset
    FILE *hex_file = fopen("hexadecimal_file.txt", "a");
    if (!hex_file) {
        fprintf(stderr, "Error opening the file hexadecimal_file.txt\n");
        return -1;
    }
    offset = ftell(hex_file); // Get the current offset

    FILE *binary_file = fopen(file->address, "rb");
    if (!binary_file) {
        fprintf(stderr, "Error opening the file: %s\n", file->address);
        fclose(hex_file);
        return -1;
    }
    
    unsigned char byte;
    while (fread(&byte, 1, 1, binary_file) == 1) {
        fprintf(hex_file, "%02X", byte);
        length++;
    }
    fclose(binary_file);
    fclose(hex_file);

    // Update the offset and length in the structure
    file->offset = offset;
    file->length = length;

    // Insert into the database with the offset and length
    snprintf(query, sizeof(query), 
         "INSERT INTO files (name, extension, address) VALUES ('%s', '%s', '%ld:%ld');",
         file->name, file->extension, file->offset, file->length);

    int result = sqlite3_exec(database, query, 0, 0, &error_message);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Insertion error: %s\n", error_message);
        sqlite3_free(error_message);
        return result;
    }

    fprintf(stdout, "File '%s' with extension '%s' added successfully.\n\n", file->name, file->extension);

    if (delete_file(file->address) != 0) {
        fprintf(stderr, "Error deleting the file: %s\n", file->address);
    }
    return SQLITE_OK;
}

int open_data(sqlite3 *database, file_type *file) {
    char query[256];
    char *error_message = 0;
    sqlite3_stmt *stmt;
    
    snprintf(query, sizeof(query), "SELECT address FROM files WHERE id = '%d';", file->id);
    
    if (sqlite3_prepare_v2(database, query, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Error preparing the query: %s\n", sqlite3_errmsg(database));
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Read the address as a string and extract the offset and length
        const char *address = (const char *)sqlite3_column_text(stmt, 0);
        sscanf(address, "%ld:%ld", &(file->offset), &(file->length));
    } else {
        fprintf(stderr, "File not found in the database.\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);

    // Read the hex file and reconstruct the file
    FILE *hex_file = fopen("hexadecimal_file.txt", "r");
    if (!hex_file) {
        fprintf(stderr, "Error opening the file hexadecimal_file.txt\n");
        return -1;
    }

    FILE *output_file = fopen(file->name, "wb");
    if (!output_file) {
        fprintf(stderr, "Error creating the file: %s.%s\n", file->name, file->extension);
        fclose(hex_file);
        return -1;
    }

    fseek(hex_file, file->offset, SEEK_SET);
    for (long i = 0; i < file->length; i++) {
        unsigned int byte;
        fscanf(hex_file, "%02X", &byte);
        fputc(byte, output_file);
    }
    
    fclose(hex_file);
    fclose(output_file);
    return 0;
}
int delete_file(sqlite3 *database, ftype *file) {
    char query[256];
    char *error_message = 0;
    sqlite3_stmt *stmt;
    long offset, length;
    
    // 1. Retrieve the offset and length of the file to delete
    snprintf(query, sizeof(query), "SELECT address FROM files WHERE id = '%d';", file->id);
    if (sqlite3_prepare_v2(database, query, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Error preparing the query: %s\n", sqlite3_errmsg(database));
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Read the address as a string and extract the offset and length
        const char *address = (const char *)sqlite3_column_text(stmt, 0);
        sscanf(address, "%ld:%ld", &offset, &length);
    } else {
        fprintf(stderr, "File not found in the database.\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);

    // 2. Delete the entry from the database
    snprintf(query, sizeof(query), "DELETE FROM files WHERE id = '%d';", file->id);
    int result = sqlite3_exec(database, query, 0, 0, &error_message);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error deleting: %s\n", error_message);
        sqlite3_free(error_message);
        return result;
    }

    // 3. Read the hexadecimal content of the file into a buffer (necessary to rewrite it later)
    FILE *hex_file = fopen("hexadecimal_file.txt", "r");
    if (!hex_file) {
        fprintf(stderr, "Error opening file hexadecimal_file.txt\n");
        return -1;
    }

    fseek(hex_file, 0, SEEK_END);
    long total_size = ftell(hex_file);
    rewind(hex_file);

    char *buffer = malloc(total_size + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation error.\n");
        fclose(hex_file);
        return -1;
    }
    fread(buffer, 1, total_size, hex_file);
    fclose(hex_file);

    // 4. Remove the data of the file to be deleted by compacting the buffer
    long new_size = total_size - (length * 2); // Each byte = 2 hexadecimal characters
    char *new_buffer = malloc(new_size + 1);
    if (!new_buffer) {
        fprintf(stderr, "Memory allocation error.\n");
        free(buffer);
        return -1;
    }

    // Copy the parts before and after the offset to delete
    memcpy(new_buffer, buffer, offset);
    memcpy(new_buffer + offset, buffer + offset + (length * 2), total_size - offset - (length * 2));

    free(buffer);

    // 5. Rewrite the hexadecimal file with the new content
    hex_file = fopen("hexadecimal_file.txt", "w");
    if (!hex_file) {
        fprintf(stderr, "Error opening file hexadecimal_file.txt for writing\n");
        free(new_buffer);
        return -1;
    }

    fwrite(new_buffer, 1, new_size, hex_file);
    fclose(hex_file);
    free(new_buffer);

    // 6. Update the offsets of other files in the database
    snprintf(query, sizeof(query), "SELECT id, address FROM files;");
    if (sqlite3_prepare_v2(database, query, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Error preparing the query: %s\n", sqlite3_errmsg(database));
        return -1;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *address = (const char *)sqlite3_column_text(stmt, 1);
        long file_offset, file_length;
        sscanf(address, "%ld:%ld", &file_offset, &file_length);

        // If the offset of the file is after the one just deleted, shift it
        if (file_offset > offset) {
            file_offset -= (length * 2); // Update the offset

            // Update the entry in the database
            snprintf(query, sizeof(query), 
                     "UPDATE files SET address = '%ld:%ld' WHERE id = %d;", 
                     file_offset, file_length, id);

            result = sqlite3_exec(database, query, 0, 0, &error_message);
            if (result != SQLITE_OK) {
                fprintf(stderr, "Error updating: %s\n", error_message);
                sqlite3_free(error_message);
                sqlite3_finalize(stmt);
                return result;
            }
        }
    }
    sqlite3_finalize(stmt);

    printf("File '%s' deleted successfully.\n", file->name);
    return 0;
}

//══════════════════════════════════════ DISPLAY AND INTERFACE FUNCTIONS ══════════════════════════════════════
// (┼ ─ ├ ┬ ┴ │ └ ┐ ┘ ┌ ┤ ╬ ═ ╠ ╦ ╩ ║ ╚ ╗ ╝ ╔ ╣)

int callback_function(void *Unused, int column_count, char **values, char **column_names) {
    printf("├──────┼───────────┼─────────────────────┤\n   %s       %s       %s\n", values[0], values[1], values[2]);
    return 0;
}

void display_table(sqlite3 *database, char *select_query) {
    int result;
    char *error_message = 0;
    printf("\n\n\n\n\n════════════════════════════════════════════\n");
    printf("┌──────┬───────────┬─────────────────────┐\n");
    printf("│  id  │ extension │         name        │\n");
    result = sqlite3_exec(database, select_query, callback_function, 0, &error_message);
    printf("└──────┴───────────┴─────────────────────┘");
    if (result != SQLITE_OK) {
        fprintf(stderr, " Error selecting data: %s\n", error_message);
        sqlite3_free(error_message);
    }
}

void list_database(sqlite3 *database) {
    char *select_query = "SELECT * FROM files;";
    display_table(database, select_query);
}

void search(sqlite3 *database, ftype *file) {
    char select_query[256];
    snprintf(select_query, sizeof(select_query), "SELECT * FROM files WHERE extension LIKE '%s';", file->extension);
    // snprintf: printf into a char* buffer (allows receiving the "%s")
    display_table(database, select_query);
}

void rename_file(sqlite3 *database, ftype *file) {
    int result;
    char *error_message = 0;
    char select_query[256];
    snprintf(select_query, sizeof(select_query), "UPDATE files SET name = '%s' WHERE id = %d;", file->name, file->id);
    // snprintf: printf into a char* buffer (allows receiving the "%s")
    result = sqlite3_exec(database, select_query, callback_function, 0, &error_message);
    if (result != SQLITE_OK) {
        fprintf(stderr, " Error selecting data: %s\n", error_message);
        sqlite3_free(error_message);
    }
    snprintf(select_query, sizeof(select_query), "SELECT * FROM files WHERE id = %d;", file->id);
    display_table(database, select_query);
}

void interface(sqlite3 *database) {
    int choice;
    ftype *file = malloc(sizeof(ftype));

    while (1) {
        printf("\n═══════════════════ Menu ═══════════════════\n");
        printf(" 1. Display the database\n");
        printf(" 2. Add a file\n");
        printf(" 3. Open a file\n");
        printf(" 4. Search for files by extension\n");
        printf(" 5. Rename a file\n");
        printf(" 6. Delete a file\n");
        printf(" 7. Quit\n");
        printf("════════════════════════════════════════════\n");
                
        printf(" Enter your choice: ");
        scanf("%d", &choice);
        getchar();  // To consume the newline left by scanf

        switch (choice) {
            case 1:
                list_database(database);
                break;

            case 2:
                printf("\n\n================== Add ===================\n");
                printf(" address of the file to add: ");
                fgets(file->address, sizeof(file->address), stdin);
                file->address[strcspn(file->address, "\n")] = 0;  // Remove the newline
                if (add_file(database, file) == 0) {
                    printf("\n\n File added successfully.\n\n");
                } else {
                    printf("\n\n Error adding the file.\n\n");
                }
                break;

            case 3: 
                printf("\n\n================== Open ===================\n");
                printf(" ID of the file to open: ");
                scanf("%d", &(file->id));
                getchar();  // To consume the newline left by scanf
                // Fill in the name, extension, address, and offset
                name_data(database, file);
                extension_data(database, file);
                address_data(database, file);
                offset_data(database, file);
                
                // After retrieving the data, try to open the file
                if (open_data(database, file) == 0) {
                    printf(" File opened successfully.\n");
                } else {
                    printf(" Error opening the file.\n");
                }
                break;

            case 4:
                printf("\n\n================== Search ===================\n");
                printf(" extension to search: ");
                fgets(file->extension, sizeof(file->extension), stdin);
                file->extension[strcspn(file->extension, "\n")] = 0;  // Remove the newline
                search(database, file);
                break;

            case 5:
                printf("\n\n================== Rename ===================\n");
                printf(" ID of the file to rename: ");
                scanf("%d", &(file->id));
                getchar();  // To consume the newline left by scanf
                printf(" New name: ");
                fgets(file->name, sizeof(file->name), stdin);
                file->name[strcspn(file->name, "\n")] = 0;  // Remove the newline
                rename_file(database, file);
                break;

            case 6:
                printf("\n\n================== Delete ===================\n");
                printf(" ID of the file to delete: ");
                scanf("%d", &(file->id));
                getchar();  // To consume the newline left by scanf
                if (delete_file(database, file) == 0) {
                    printf(" File deleted successfully.\n");
                } else {
                    printf(" Error deleting the file.\n");
                }
                break;

            case 7:
                printf("\n════════════════════════════════════════════\n");
                printf("                  Goodbye !\n\n");
                free(fichier);
                return;

            default:
                printf(" Invalid choice. Please try again.\n");
                break;
        }
    }
}

//══════════════════════════════════════════ MAIN FUNCTION ══════════════════════════════════════════

int main(int argc, char **argv) {
    sqlite3 *database;
    int result;

    result = sqlite3_open("file_manager.db", &database);
    if (result) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
        return -1;
    }

    // Example usage
    file_type file = { .id = 1 }; // Example file ID
    get_name(database, &file);
    get_extension(database, &file);
    get_address(database, &file);
    get_offset(database, &file);
    get_length(database, &file);

    convert_file_to_hex(&file);
    add_file(database, &file);
    open_data(database, &file);

    sqlite3_close(database);
    return 0;
}
