#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

struct systemcall
{
	float per_time;
	char *name;
};
struct systemcall syscall[300];

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
		
		sleep(2);
		close(fd[1]);			
		printf("this is father out while\n");
		printf("fd[0]:%d fd[1]:%d\n", fd[0], fd[1]);
		char buf[1024][100];
		ssize_t len = read(fd[0], buf, sizeof(buf));
		printf("len:%d\n", len);	
		char *temp = strtok(buf[0], " ");
		for(int i = 0; i<300; i++){
			temp = strtok(NULL, "\n");
			printf("temp%d:%s\n ", i, temp);			
			if(temp == NULL){
				printf("\n");
				break;
			}	
			
			else{
				printf("len: %d\n", strlen(temp));
				int cnt = 0; char *tmp = strtok(temp, " ");
				printf("tmp:%s\n", tmp);
			/*	for(int j = 0; j<strlen(temp); j++){
					cnt++;
					tmp = strtok(NULL, "");
					if(tmp == NULL)
						break;
					else{
						if(tmp[0]>58)
							printf("name:%s\n", tmp);
					}
				}*/
			}
		}
		/*char *temp = strtok(buf[0], " ");
		printf("temp1:%s\n", temp);
		temp = strtok(NULL, " ");
		printf("temp2:%s\n", temp);*/
				
			exit(0);
	}
	return 0;
}
