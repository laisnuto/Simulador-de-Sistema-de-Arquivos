#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pwd.h>
#include <time.h> 


void change_directory(char *path);
void remove_file(char *filename);
void uname_a();
void exibe_prompt();
void le_comando(char *comando, char *parametros[]);




