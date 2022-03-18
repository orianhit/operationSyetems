#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>


void printPrompt() {
     time_t t = time(NULL);
     struct tm tm = *localtime(&t);
     char hostname[HOST_NAME_MAX+1];
     char cwd[PATH_MAX];
     gethostname(hostname, sizeof(hostname));
     getcwd(cwd, sizeof(cwd));
     printf("%d-%02d-%02d %02d:%02d:%02d %s $ %s : \n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, hostname, cwd);
}

char *concatenate(char *a, char *b, char *c)
{
  int size = strlen(a) + strlen(b) + strlen(c) + 1;
  char *str = malloc(size);
  strcpy (str, a);
  strcat (str, b);
  strcat (str, c); 

  return str;
}

int runcmd(char *cmd) {
     int child_status;
     char *argv_for_program[] = { cmd, NULL };
     pid_t pid = fork();

     if (pid == -1) {
          printf("failed to create proc \n");
          return 0;
     } else if (pid == 0) {
          execv(cmd, argv_for_program);
          _exit(127);
     } else {
          int child_pid = wait(&child_status);
          if (child_pid > 0) {
               if(WIFEXITED(child_status)) {
                    if (WEXITSTATUS(child_status) != 127) {
                         return 1;
                    }
               } else {
                    return 0;
               }
          }
     }
     return 0;
}


void main() {
     char command[PATH_MAX+1];
     int status = 0;
     char *full_command;
     char paths_env[PATH_MAX+1];
     char *path_env;
     const char *paths_env_c = getenv("PATH");

     while(1) {
          printPrompt();
          scanf(" %s", command);
          if (! strcmp(command, "leave")) break;

          strcpy(paths_env, paths_env_c);
          
          path_env = strtok(paths_env, ":");
          
          while( path_env != NULL ) {
               full_command = concatenate(path_env, "/", command);
               status = runcmd(full_command);

               free(full_command);

               if(status) break;
               path_env = strtok(NULL, ":");
          }
          if (!status) printf("no such command %s \n", command);

     }
}