#pragma once

#define COMMAND_LINE_MAX_SIZE 512

void initialize_signal_handling(void);
void print_finished_background_processes(void);
int read_command_line(char *line);
void run_command_line(char *line);
