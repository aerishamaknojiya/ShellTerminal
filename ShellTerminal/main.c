#include <stdio.h>
#include <unistd.h>

#include "myshell.h"

int main(void)
{

  char line[COMMAND_LINE_MAX_SIZE];

  // Set up shell process so interrupts are properly directed to called processes
  initialize_signal_handling();

  while (1)
  {

    // If any background processes finish, print their information
    print_finished_background_processes();

    // Show prompt, but only if stdin is a terminal
    if (isatty(STDIN_FILENO))
      printf("> ");

    // Read and run a command line
    if (read_command_line(line))
      run_command_line(line);
  }
}
