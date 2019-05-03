#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>



void do_pipe(char *args[128],int cmd_pos[128],int i,int pipe_num,int arg_num){
    if(i == pipe_num){
        if(arg_num - cmd_pos[i] > 2){
            if(strcmp(args[arg_num-2],">") == 0){
                args[arg_num-2] = NULL;
                int fd = open(args[arg_num - 1], O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
                dup2(fd, 1);
                close(fd);
            }
            else if (strcmp(args[arg_num-2],">>") == 0){
                args[arg_num-2] = NULL;
                int fd = open(args[arg_num - 1], O_CREAT|O_RDWR|O_APPEND, S_IRUSR|S_IWUSR);
                dup2(fd,1);
                close(fd);
            }
            else if(strcmp(args[arg_num-2],"<") == 0){
                args[arg_num-2] = NULL;
                int fd = open(args[arg_num-1], O_RDONLY);
                dup2(fd, 0);
                close(fd);
            }
        }
        execvp(args[cmd_pos[i]], args+cmd_pos[i]);
        exit(255);
    }
    
    int fd[2];
    if(pipe(fd) < 0){
	perror("Failed to create pipe!");
	exit(255);
    }
    pid_t pid = fork();
    if (pid == 0){
        close(fd[0]);
        dup2(fd[1],1);
        close(fd[1]);
        execvp(args[cmd_pos[i]],args+cmd_pos[i]);
        exit(255);
    }
    else if(pid < 0)
	perror("Failed to fork!");
    close(fd[1]);
    dup2(fd[0], 0);
    close(fd[0]);
    do_pipe(args, cmd_pos, i+1, pipe_num,arg_num);
}

int parse_cmd(char cmd[256],char *args[128]){
    /* 拆解命令行 */
    
    int i;
    int space;  //处理多余空格
    
    args[0] = cmd;
    for (i = 0; *args[i]; i++){
        space = 0;
        for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++){
            if (*args[i+1] == ' ') {
                space = 1;
                *args[i+1] = '\0';
            }
            else {
                if(space == 1) break;
            }
        }
    }
    args[i] = NULL;
    return i;
}

int parse_pipe(char *args[128],int cmd_pos[128]){
	/* 处理pipe */
        int pipe_num = 0;
        for(int i = 0;args[i];i++){
            if(strcmp(args[i],"|") == 0){
                args[i] = NULL; //将|换成NULL，自动隔开前后命令
                pipe_num ++;
                cmd_pos[pipe_num] = i + 1;
            }
        }
	return pipe_num;
}

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    /* 记录每个单独指令在args中所在位置 */
    int cmd_pos[128];
    int arg_num = 0;
    
    while (1) {
        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        cmd[i] = '\0';
        
        /* 拆解命令行 */
        arg_num = parse_cmd(cmd,args);

	/* 处理pipe */
        memset(cmd_pos,0,sizeof(cmd_pos));
        int pipe_num = parse_pipe(args,cmd_pos);

        /* 没有输入命令 */
        if (!args[0])
            continue;
        
        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            continue;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;
        if (strcmp(args[0], "export") == 0){
            char *eq = strchr(args[1],'=');
            if(eq == NULL) continue;
            *eq = '\0';
            setenv(args[1], eq + 1,1);
            continue;
	}
        
        /* 外部命令 */
        pid_t pid = fork();
        if (pid == 0) {
            /* 子进程 */
            do_pipe(args,cmd_pos,0,pipe_num,arg_num);
            perror("Failed to execute!");
	    return 255;
        }
	else if(pid < 0){
	    perror("Failed to fork aa!");
	    continue;
	}
            
        /* 父进程 */
        int status;
        waitpid(pid,&status,0);
    }
}
