/// IFJ24
/// @brief STDIN handler

#include "file.h"
#include <stdio.h>
#include <stdlib.h>

#define TEMP_FILE "temp_code.ifj"

/**
 * @brief Saves input from STDIN to a temporary file.
 */
void save_input_to_file() {
    FILE *file = fopen(TEMP_FILE, "w");
    if (!file) {
        perror("Failed to create temporary file");
        exit(EXIT_FAILURE);
    }

    int c;
    while ((c = getchar()) != EOF) {
        if (fputc(c, file) == EOF) {
            perror("Failed to write to temporary file");
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    if (fclose(file) != 0) {
        perror("Failed to close temporary file after writing");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Opens the temporary file for reading.
 * @return FILE* Pointer to the opened file.
 */
FILE* open_temp_file() {
    FILE *file = fopen(TEMP_FILE, "r");
    if (!file) {
        perror("Failed to open temporary file");
        exit(EXIT_FAILURE);
    }
    return file;
}

/**
 * @brief Rewinds the temporary file to the beginning.
 * @param file Pointer to the file to rewind.
 */
void rewind_file(FILE *file) {
    if (!file) {
        fprintf(stderr, "Error: Invalid file pointer in rewind_file\n");
        return;
    }
    
    if (fseek(file, 0, SEEK_SET) != 0) {
        perror("Error rewinding file");
    }
}

/**
 * @brief Deletes the temporary file.
 */
void cleanup_temp_file() {
    if (remove(TEMP_FILE) != 0) {
        perror("Failed to delete temporary file");
    }
}

/**
 * @brief Prints the content of the temporary file for debugging.
 */
void debug_temp_file() {
    FILE *file = fopen(TEMP_FILE, "r");
    if (!file) {
        perror("Failed to open temporary file for debugging");
        return;
    }

    printf("Temporary file content:\n");
    int c;
    while ((c = fgetc(file)) != EOF) {
        putchar(c);
    }

    fclose(file);
}