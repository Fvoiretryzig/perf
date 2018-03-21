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
	char per_time[10];
	char name[256];
};
struct systemcall syscall[300];
void substr_name(const char*str, unsigned start, unsigned end, int count)
{
  unsigned n = end - start;
  strcpy(syscall[count].name, "");
  strncpy(syscall[count].name, str + start, n);
  syscall[count].name[n+1] = '\0';
}
void substr_per(const char*str, unsigned start, unsigned end, int count)
{
  unsigned n = end - start;
  strcpy(syscall[count].per_time, "");
  strncpy(syscall[count].per_time, str + start, n);
  syscall[count].per_time[n+1] = '\0';
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
		}
		printf("argc:%d\n", argc);
		child_argv[argc+2] = NULL;
		close(fd[0]);
		dup2(fd[1],2);	//把strace的输出连接到子进程的写管道
		execvp("strace", child_argv);
		//close(fd[1]);
	}
	else{
		//父进程， 要通过管道读取strace的输出
		sleep(1);
		close(fd[1]);			
		char buf[1024][100];
		ssize_t len = read(fd[0], buf, sizeof(buf));
		int cnt = 0;
		if(len<0)
			exit(1);
		char *temp = strtok(buf[0], " ");
		for(int i = 0; i<300; i++){
			temp = strtok(NULL, "\n");		
			if(temp == NULL){
				printf("\n");
				break;
			}	
			else{
				if(i >= 2){
					char *pattern_name = "[A-Za-z,_]+([0-9]+)?";	//用来匹配名字的
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
						p_name = regexec(&reg_name,temp,1,pm_name,0);
						substr_name(temp,pm_name[0].rm_so,pm_name[0].rm_eo,cnt);
						if(!((syscall[cnt].name[0]>=65 && syscall[cnt].name[0]<=90) ||(syscall[cnt].name[0]>=97 && syscall[cnt].name[0]<=122)))
							break;
						regfree(&reg_name);
						
						p_per = regexec(&reg_per, temp, 1, pm_per, 0);
						substr_per(temp,pm_per[0].rm_so,pm_per[0].rm_eo, cnt);
						regfree(&reg_per);
						
						cnt++;
					}
				}
			}	
		}		
		for(int i = 0; i<cnt; i++)
			printf("%s:%s%\n", syscall[i].name,syscall[i].per_time);
		exit(0);
		
	}
	return 0;
}
