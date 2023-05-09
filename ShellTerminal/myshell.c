#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>

#include "myshell.h"
// running the final command
void runCommand(char **command, int isBackground)
{
  pid_t f;
  int stat_loc;
  if ((f = fork()) < 0)
  {
    perror("fork failed \n");
    exit(1);
  }
  else if (f == 0)
  {
    signal(SIGINT, SIG_IGN);

    if (execvp(command[0], command) < 0)
    {
      perror(command[0]);
      kill(getpid(), SIGKILL);
      exit(0);
    }
  }
  else if (isBackground == 1)
  {
    printf("Background process [%d] started.\n", f);
  }
  else
  {
    waitpid(f, &stat_loc, WUNTRACED);
  }
}
void pipeController(char *arguments[])
{
  char *commandVal[256];
  int fD1[2];
  int fD2[2];
  int ctr = 0;
  pid_t PID;
  int error = -1;
  int termination = 0;
  int index1 = 0, index2 = 0, index3 = 0, index4 = 0;
  while (arguments[index1] != NULL)
  {
    if (strcmp(arguments[index1], "|") == 0)
      ctr++;
    index1++;
  }
  ctr++;
  while (arguments[index2] != NULL && termination != 1)
  {
    index3 = 0;
    while (strcmp(arguments[index2], "|") != 0)
    {
      commandVal[index3] = arguments[index2];
      index2++;
      if (arguments[index2] == NULL)
      {
        termination = 1;
        index3++;
        break;
      }
      index3++;
    }
    commandVal[index3] = NULL;
    index2++;
    if (index4 % 2 != 0)
    {
      pipe(fD1);
    }
    else
    {
      pipe(fD2);
    }
    PID = fork();

    if (PID == -1)
    {
      if (index4 != ctr - 1)
      {
        if (index4 % 2 != 0)
        {
          close(fD1[1]);
        }
        else
        {
          close(fD2[1]);
        }
      }
      //////////
      return;
    }
    if (PID == 0)
    {
      if (index4 == 0)
      {
        dup2(fD2[1], STDOUT_FILENO);
      }
      else if (index4 == ctr - 1)
      {
        if (ctr % 2 != 0)
        {
          dup2(fD1[0], STDIN_FILENO);
        }
        else
        {
          dup2(fD2[0], STDIN_FILENO);
        }
      }
      else
      {
        if (index4 % 2 != 0)
        {
          dup2(fD2[0], STDIN_FILENO);
          dup2(fD1[1], STDOUT_FILENO);
        }
        else
        {
          dup2(fD1[0], STDIN_FILENO);
          dup2(fD2[1], STDOUT_FILENO);
        }
      }
      if (execvp(commandVal[0], commandVal) == error)
      {
        kill(getpid(), SIGTERM);
      }
    }

    if (index4 == 0)
    {
      close(fD2[1]);
    }
    else if (index4 == ctr - 1)
    {
      if (ctr % 2 != 0)
      {
        close(fD1[0]);
      }
      else
      {
        close(fD2[0]);
      }
    }
    else
    {
      if (index4 % 2 != 0)
      {
        close(fD2[0]);
        close(fD1[1]);
      }
      else
      {
        close(fD1[0]);
        close(fD2[1]);
      }
    }
    waitpid(PID, NULL, 0);
    index4++;
  }
}
int numberOfWords(char *line)
{
  int counter = 0;
  printf("%s", line);
  for (int i = 0; line[i] != '\0'; i++)
  {
    if (isspace(line[i]) && !isspace(line[i + 1]))
    {
      counter++;
    }
  }
  return counter;
}

// checking for background commands
void isBackground(char *token[])
{
  int isBackground = 0;
  char *temp[sizeof(char) * COMMAND_LINE_MAX_SIZE];

  int index = 0;
  int index1 = 0;
  while (token[index1] != NULL)
  {
    if (strcmp(token[index1], "&") == 0)
    {
      break;
    }
    temp[index1] = token[index1];
    index1++;
  }
  if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
    exit(0);
  else
  {
    while (token[index] != NULL && isBackground != 1)
    {
      if (strcmp(token[index], "&") == 0)
      {
        isBackground = 1;
        break;
      }
      else if (strcmp(token[index], "|") == 0)
      {
        pipeController(token);
        break;
      }
      temp[index] = token[index];
      index++;
    }

    temp[index] = NULL;
    runCommand(temp, isBackground);
  }
}
// tokenizing the command line
void parsedCommand(char *line)
{
  char *token[sizeof(line)];
  token[0] = strtok(line, " \n\t");
  if (token[0] == NULL)
  {
    exit(1);
  }

  int index = 1;
  while ((token[index]) != NULL)
  {
    token[index] = strtok(NULL, " \n\t");
    index++;
  }
  isBackground(token);
}

/*
 * Initializes the shell process, in particular its signal handling,
 * so that when an keyboard interrupt signal (Ctrl-C) is received by
 * the shell, it is instead directed to the process currently running
 * in the foreground.
 */

void sigint_handler(int sig)
{
  write(0, "Ahhh! SIGINT!\n", 14);
}
void initialize_signal_handling(void)
{

  /* TO BE COMPLETED BY THE STUDENT */
  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
}

/*
 * Checks if any background processes have finished, without blocking
 * to wait for them. If any processes finished, prints a message for
 * each finished process containing the PID of the process.
 */
void print_finished_background_processes(void)
{
  int stat_loc;
  pid_t f;
  while ((f = waitpid(-1, &stat_loc, WNOHANG)) > 0)
  {

    printf("Background process [%d] finished.\n", f);
  }
}

/*
 * Reads a command line from standard input into a line buffer. If a
 * command-line could be read successfully (even if empty), returns a
 * positive value. If the reading process is interrupted by a keyboard
 * interrupt signal (Ctrl-C), returns 0. If a line cannot be read for
 * any other reason, including if an EOF (end-of-file) is detected in
 * standard input, exits the program (using the `exit` system call).
 *
 * The buffer is expected to be at least COMMAND_LINE_MAX_SIZE bytes
 * long. If the command line is longer than supported by this buffer
 * size (including line-feed character and null termination byte),
 * then ignores the full line and returns 0.
 *
 * If the command-line contains a comment character, any text
 * following the comment character is stripped from the command-line.
 *
 * Parameters:
 *  - line: pointer pointing to the start of the line buffer.
 * Returns:
 *  - 1 if a line could be read successfully, even if empty. 0 if the
 *    line could not be read due to a keyboard interrupt or for being
 *    too long.
 */
int read_command_line(char *line)
{
  /* TO BE COMPLETED BY THE STUDENT */
  if (fgets(line, COMMAND_LINE_MAX_SIZE, stdin) == NULL)
  {
    return 0;
  }
  if (!strchr(line, '\n'))
  {
    char chr;
    while ((chr = getchar()) != '\n' && chr != EOF)
      ;
    puts("Command line too long.");
    return 0;
  }
  else if (line[0] == '\n')
  {
    return 1;
  }
  else
  {
    int j = 0;
    char temp[strlen(line)];
    while (line[j] != '#' && line[j] != '\\' && j < strlen(line))
    {
      temp[j] = line[j];
      j++;
    }
    temp[j] = '\0';
    strcpy(line, temp);

    return 1;
  }

  return 1;
}

/*
 * Executes the command (or commands) listed in the specified
 * command-line, according to the rules in the assignment
 * description. The line may contain no command at all, or may contain
 * multiple commands separated by operators like '&', ';' or '|'.
 *
 * Parameters:
 *  - line: string corresponding to the command line to be
 *    executed. This is typically the same line parsed by
 *    read_command_line, though for testing purposes you must assume
 *    that this can be any string containing commands in an
 *    appropriate format.
 */

void run_command_line(char *line)
{
  parsedCommand(line);
}
