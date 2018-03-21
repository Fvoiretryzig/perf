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

char* substr_name(const char*str, unsigned start, unsigned end)
{
  unsigned n = end - start;
  static char stbuf_name[256];
  strncpy(stbuf_name, "", 256);
  strncpy(stbuf_name, str + start, n);
  stbuf_name[n+1] = '\0';
  return stbuf_name;
}
char* substr_per(const char*str, unsigned start, unsigned end)
{
  unsigned n = end - start;
  static char stbuf_per[256];
  strncpy(stbuf_per, "", 256);
  strncpy(stbuf_per, str + start, n);
  stbuf_per[n+1] = '\0';
  return stbuf_per;
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
		if(len<0)
			exit(1);
		//printf("len:%d\n", len);	
		char *temp = strtok(buf[0], " ");
		//temp = strtok(temp, "\n");
		for(int i = 0; i<300; i++){
			temp = strtok(NULL, "\n");
			//printf("temp%d:%s\n ", i, temp);			
			if(temp == NULL){
				printf("\n");
				break;
			}	
			
			else{
				//printf("len: %d\n", strlen(temp));
				if(i >= 2){
					char *pattern_name = "[A-Za-z]+([0-9]+)?";	//用来匹配名字的
					regex_t reg_name;
					int p_name = regcomp(&reg_name,pattern_name, REG_EXTENDED);
					
					char *pattern_per = "([0-9]{1,2})(.[0-9]{2})";	//匹配百分号的
					regex_t reg_per;
					int p_per = regcomp(&reg_per, pattern_per, REG_EXTENDED);
					
					regmatch_t pm_name[1]; regmatch_t pm_per[1];	
					if(p_name!=0 || p_per!=0){
						printf("error!");
					}	
					else{
						//printf("temp:%s\n", temp);
						p_name = regexec(&reg_name,temp,1,pm_name,0);
						//char *r_name = substr(temp,pm_name[0].rm_so,pm_name[0].rm_eo);
						syscall[i-2].name = substr_name(temp,pm_name[0].rm_so,pm_name[0].rm_eo);
						if(!((syscall[i-2].name[0]>=65 && syscall[i-2].name[0]<=90) ||(syscall[i-2].name[0]>=97 && syscall[i-2].name[0]<=122)))
							break;
						printf("r:%s\n", syscall[i-2].name);
						regfree(&reg_name);
						
						p_per = regexec(&reg_per, temp, 1, pm_per, 0);
						printf("r:%s\n", syscall[i-2].name);
						char *r_per = substr_per(temp,pm_per[0].rm_so,pm_per[0].rm_eo);
						printf("r:%s\n", syscall[i-2].name);
						printf("per:%s\n\n", r_per);
						//printf("%s: %s%\n\n", r_name, r_per);
						regfree(&reg_per);
					}
				}

			}
		}
		
				
			exit(0);
	}
	return 0;
}
