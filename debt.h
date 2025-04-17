#ifndef DEBT_H
#define DEBT_H

#include <stdbool.h>

/**
 * Executes a command with its arguments.
 *
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @return The exit code of the executed command or -1 if error
 */
int execute_command(int argc, char *argv[]);

/**
 * Handles signals to forward them to the child process.
 *
 * @param setup If true, set up signal handlers; if false, reset to defaults
 */
void handle_signals(bool setup);

/**
 * Extracts filename from full path (cross-platform alternative to basename)
 *
 * @param path The full path including filename
 * @return Pointer to the filename part of the path
 */
char* get_filename(char* path);

/**
 * Builds a command string from argument array
 *
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param start_idx Index to start from in the argv array
 * @return Newly allocated string with the full command (must be freed by caller)
 */
char* build_command(int argc, char *argv[], int start_idx);

#endif /* DEBT_H */
