#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<stdlib.h>
#include <assert.h>

struct systemcall
{
	float per_time;
	char *name;
};
struct systemcall syscall[300];

void do_str(char *s){
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
//		char *app="";
//		app = argv[1];	
//		for(int i = 2; i<argc; i++){
//			sprintf(app, "%s %s",app ,argv[i]);
//			printf("this is app:%s\n", app);
//		}
		close(fd[0]);
		//char fd_str[11];
		//sprintf(fd_str, "%d", fd[1]);	//此处fd[1]是子进程的写管道！！！尝试是不是用字符串表示！！！
		//printf("fd_str:%s\n", fd_str);
		
		char another_argv[100][100]; 
		printf("hahah\n");
		for(int i = 0; i<argc; i++)
			strcpy(another_argv[i], argv[i]);
		for(int i = 1; i<argc; i++)
			strcpy(argv[i+2], argv[i]);
		strcpy(argv[0], "strace");
		strcpy(argv[1], "-w");
		strcpy(argv[2], "-c");
		for(int i = 0; i<argc+2; i++)
			printf("argv[%d]:%s\n", i, argv[i]);
		dup2(fd[1],2);	//把strace的输出连接到子进程的写管道
		execvp("strace", argv);
		//char *child_envp[ ]={"PATH=/bin", NULL};
		//execve("/bin/strace", child_argv); 
		close(fd[1]);
	}
	else{
		//父进程， 要通过管道读取strace的输出
		sleep(5);
		printf("this is father out while\n");
		close(fd[1]);	//把父进程的写管道关掉
		/*---------初步尝试---------*/
		char *buf;
		while(1){
			printf("this is father\n");
			ssize_t len = read(fd[0], buf, 1024);
			if(len > 0)
				printf("read:\n%s\n", buf);
			else{
				printf("this is in else\n");
				exit(0);
			}
		}
		
	}
	return 0;
}
