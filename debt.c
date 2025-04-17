#include "debt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #define PATH_SEPARATOR '\\'
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <libgen.h>
    #define PATH_SEPARATOR '/'
#endif

#ifdef _WIN32
    HANDLE child_process = NULL;
#else
    pid_t child_pid = -1;
#endif

// Platform-specific signal handling
#ifndef _WIN32
void forward_signal(int sig) {
    if (child_pid > 0) {
        kill(child_pid, sig);
    }
}

void handle_signals(bool setup) {
    struct sigaction sa;

    if (setup) {
        sa.sa_handler = forward_signal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGHUP, &sa, NULL);
        sigaction(SIGQUIT, &sa, NULL);
    } else {
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGHUP, &sa, NULL);
        sigaction(SIGQUIT, &sa, NULL);
    }
}
#else
BOOL WINAPI console_handler(DWORD signal) {
    if (child_process != NULL) {
        if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT) {
            TerminateProcess(child_process, 1);
            return TRUE;
        }
    }
    return FALSE;
}

void handle_signals(bool setup) {
    if (setup) {
        SetConsoleCtrlHandler(console_handler, TRUE);
    } else {
        SetConsoleCtrlHandler(console_handler, FALSE);
    }
}
#endif

// Extract filename from path (cross-platform alternative to basename)
char* get_filename(char* path) {
    char* filename = strrchr(path, PATH_SEPARATOR);
    return filename ? filename + 1 : path;
}

// Build command string from arguments
char* build_command(int argc, char *argv[], int start_idx) {
    int total_length = 0;
    for (int i = start_idx; i < argc; i++) {
        // Add space for quotes around arguments with spaces
        bool needs_quotes = strchr(argv[i], ' ') != NULL;
        total_length += strlen(argv[i]) + (needs_quotes ? 2 : 0) + 1; // +1 for space
    }

    char *command = malloc(total_length + 1); // +1 for null terminator
    if (!command) {
        perror("Failed to allocate memory");
        return NULL;
    }

    command[0] = '\0';
    for (int i = start_idx; i < argc; i++) {
        bool needs_quotes = strchr(argv[i], ' ') != NULL;

        if (needs_quotes) strcat(command, "\"");
        strcat(command, argv[i]);
        if (needs_quotes) strcat(command, "\"");

        if (i < argc - 1) {
            strcat(command, " ");
        }
    }

    return command;
}

int execute_command(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s command [args]\n", argv[0]);
        fprintf(stderr, "       Strips leading $ or > from commands for easy copy-pasting\n");
        return 1;
    }

    // Set up signal handlers
    handle_signals(true);

#ifdef _WIN32
    // Windows implementation
    char *command = build_command(argc, argv, 1);
    if (!command) return -1;

    char *comspec = getenv("COMSPEC");
    if (!comspec) comspec = "cmd.exe";

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // First try direct execution
    if (CreateProcess(NULL, command, NULL, NULL, TRUE,
                     0, NULL, NULL, &si, &pi)) {
        child_process = pi.hProcess;

        // Wait for the process to complete
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);

        // Cleanup
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        free(command);
        handle_signals(false);

        return (int)exit_code;
    }

    // If direct execution fails, try with cmd.exe
    char *cmd_command = malloc(strlen(command) + 10); // /c + space + command
    if (!cmd_command) {
        free(command);
        perror("Failed to allocate memory");
        return -1;
    }

    sprintf(cmd_command, "/c %s", command);
    free(command);

    if (CreateProcess(comspec, cmd_command, NULL, NULL, TRUE,
                     0, NULL, NULL, &si, &pi)) {
        child_process = pi.hProcess;

        // Wait for the process to complete
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);

        // Cleanup
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        free(cmd_command);
        handle_signals(false);

        return (int)exit_code;
    }

    // Both methods failed
    fprintf(stderr, "Failed to execute command: %s\n", cmd_command);
    free(cmd_command);
    handle_signals(false);
    return -1;
#else
    // Unix implementation
    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        handle_signals(false);
        return -1;
    }

    if (child_pid == 0) {
        // Child process - directly execute the command without an intermediate shell
        execvp(argv[1], &argv[1]);

        // If execvp fails, try with shell (for commands with shell operators)
        char *shell = getenv("SHELL");
        if (!shell) {
            #if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
                shell = "/bin/bash";
                if (access(shell, X_OK) != 0) {
                    shell = "/bin/sh";
                }
            #else
                shell = "/bin/sh";
            #endif
        }

        // Build command for shell execution only if execvp fails
        char *command = build_command(argc, argv, 1);
        if (!command) {
            exit(EXIT_FAILURE);
        }

        execl(shell, shell, "-c", command, NULL);
        perror("exec failed");
        free(command);
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(child_pid, &status, 0);

        // Clean up
        handle_signals(false);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }

        return -1;
    }
#endif
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    char *program_name = get_filename(argv[0]);
#else
    char *program_name = basename(argv[0]);
#endif

    // Check if we're being called as $ or >
    if (strcmp(program_name, "$") == 0 || strcmp(program_name, ">") == 0) {
        return execute_command(argc, argv);
    }

    // If we're called as 'debt', check if the first argument starts with $ or >
    if (argc > 1) {
        if (argv[1][0] == '$' || argv[1][0] == '>') {
            // Shift the command one character forward
            argv[1] = &argv[1][1];

            // If the command is now empty, remove it
            if (argv[1][0] == '\0') {
                for (int i = 1; i < argc - 1; i++) {
                    argv[i] = argv[i + 1];
                }
                argc--;
            }

            return execute_command(argc, argv);
        } else {
            // Regular command execution
            return execute_command(argc, argv);
        }
    } else {
        fprintf(stderr, "Usage: %s command [args]\n", argv[0]);
        fprintf(stderr, "       Strips leading $ or > from commands for easy copy-pasting\n");
        return 1;
    }
}
