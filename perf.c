#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <regex.h>

struct systemcall
{
	float per_time;
	char *name;
};
struct systemcall syscall[300];

char* substr(const char*str, unsigned start, unsigned end)
{
  unsigned n = end - start;
  static char stbuf[256];
  strncpy(stbuf, str + start, n);
  stbuf[n+1] = '\0';
  return stbuf;
}
int main(int argc, char *argv[]) {
	/*--------读取命令行参数--------*/
	for (int i = 0; i < argc; i++) {
		assert(argv[i]); // specification
	    printf("argv[%d] = %s\n", i, argv[i]);
	}
	assert(!argv[argc]); // specification	
	if(argc == 1){
		printf("Usage: stat command [argument]");
		exit(1);
	}
	/*--------连接管道，创建子进程--------*/
	int fd[2] = {0, 0};
	if(pipe(fd) != 0){
		printf("pipe error\n");
		exit(1);
	}
	pid_t pid = fork();
	if(pid == 0){
		//pid为0是子进程，子进程调用execve执行strace去读系统调用次数
		char *child_argv[100]={"strace","-w","-c", "", "","","","","","","","","",""};
		char **temp = argv;
		for(int i = 1; i<argc; i++){
			child_argv[i+2] = (char*)temp[i];
			printf("child_argv[i+2]:%s\n", child_argv[i+2]);
		}
		printf("argc:%d\n", argc);
		child_argv[argc+2] = NULL;
		close(fd[0]);
		dup2(fd[1],2);	//把strace的输出连接到子进程的写管道
		execvp("strace", child_argv);
		printf("hahaha\n");
		//close(fd[1]);
	}
	else{
		//父进程， 要通过管道读取strace的输出
		
		sleep(1);
		close(fd[1]);			
		printf("this is father out while\n");
		char buf[1024][100];
		ssize_t len = read(fd[0], buf, sizeof(buf));
		printf("len:%d\n", len);	
		char *pattern = "[A-Za-z]+";
		regex_t reg;
		int p = regcomp(&reg,pattern,REG_ICASE);regmatch_t pm[1];	
		//char *r = "forty";

		char *temp = strtok(buf[0], " ");
		temp = strtok(temp, "\n");
		for(int i = 0; i<300; i++){
			//temp += strlen(temp) + 1;	//手动移指针
			temp = strtok(NULL, "\n");
			printf("temp%d:%s\n ", i, temp);			
			if(temp == NULL){
				printf("\n");
				break;
			}	
			
			else{
				printf("len: %d\n", strlen(temp));
				if(i == 2){
					if(p!=0){
						printf("error!");
					}	
					else{
					char *r; strcpy(r, temp);
					p=regexec(&reg,r,1,pm,0);
					substr(r,pm[0].rm_so,pm[0].rm_eo);
					printf("r:%s\n", r);
					}
				}

			}
		}
		regfree(&reg);
				
			exit(0);
	}
	return 0;
}
