/// IFJ24
/// @brief Main function that accepts arguments from the command line

#include <stdio.h>
#include "scanner.h"
#include "error_codes.h"
#include "parser.h"
#include "file.h"

/**
 * @brief Main program functions
 * @param argc Number of command line arguments
 * @param argv Array of strings containing command line arguments
 * @return Program return code (0 for success, otherwise error code)
 */
int main(int argc, char *argv[]) {

    save_input_to_file();

    FILE *source = open_temp_file();
    if (!source) {
        fprintf(stderr, "Failed to open temporary file\n");
        return EXIT_FAILURE;
    }

    parser_init(source);

    if (parse_program() != 0) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Parsing failed");
    }

    fclose(source);
    cleanup_temp_file();

    return EXIT_SUCCESS;  
}
