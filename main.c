#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

struct passwd *getpwuid(uid_t uid);

const char PATH_ENV_SEPERATOR[] = ":";
const char PATH_CONCAT_CHAR[] = "/";
const char EXIT_COMMAND[] = "leave";

void printPrompt() {
     time_t t = time(NULL);
     struct tm tm = *localtime(&t);
     char hostname[HOST_NAME_MAX+1];
     char cwd[PATH_MAX + 1];
     gethostname(hostname, sizeof(hostname));
     getcwd(cwd, sizeof(cwd));
     struct passwd *p = getpwuid(getuid());

     printf("%d-%02d-%02d %02d:%02d:%02d %s@%s:%s$ ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, p->pw_name, hostname, cwd);
}

char *concatenate(char *a, char *c){
     int size = strlen(a) + strlen(PATH_CONCAT_CHAR) + strlen(c) + 1;
     char *str = malloc(size);
     strcpy(str, a);
     strcat(str, PATH_CONCAT_CHAR);
     strcat(str, c); 

     return str;
}

char **getArgs(char *cmd, int *argsSize) {
     char *token;
     char **programArgs = (char **) malloc(sizeof(char *) * 0);

     // Non static strtok
     while ((token = strtok_r(cmd, " ", &cmd))) {
          programArgs = (char **) realloc(programArgs, sizeof(char *) * (*argsSize+1));
          programArgs[(*argsSize)++] = token;
     }
     programArgs[*argsSize] = NULL;
     return programArgs;
}

void freeArr(char **programArgs, int argsSize) {
     for(int i=0; i++; i< argsSize) {
          free(programArgs[i]);
     }
     free(programArgs);
}

int runCmd(char *cmd) {
     int child_status;
     int argsSize = 0;
     char **programArgs = getArgs(cmd, &argsSize);
     pid_t pid = fork();
     int success = 0;

     if (pid == -1) {
          printf("Failed to create proc\n");
     } else if (pid == 0) {
          // programArgs cleared after exit
          execv(programArgs[0], programArgs);
          _exit(127);
     } else {
          int child_pid = wait(&child_status);
          int status = WEXITSTATUS(child_status);
          if (child_pid > 0 && WIFEXITED(child_status) && WEXITSTATUS(child_status) != 127) {
               success = 1;
          }
     }
     freeArr(programArgs, argsSize);
     return success;
}


void main() {
     const char *paths_env_c = getenv("PATH");
     int status = 0;
     char command[PATH_MAX+1];
     char *full_command;
     char *paths_env;
     char *path_env;


     while(1) {
          printPrompt();
          gets(command);
          if (!strcmp(command, EXIT_COMMAND)) break;

          paths_env = strdup(paths_env_c);          
          path_env = strtok(paths_env, PATH_ENV_SEPERATOR);
          
          while( path_env != NULL ) {
               full_command = concatenate(path_env, command);
               status = runCmd(full_command);

               free(full_command);

               if(status) break;
               path_env = strtok(NULL, PATH_ENV_SEPERATOR);
          }
          free(paths_env);
          if (!status) printf("No such command %s\n", command);
     }
}