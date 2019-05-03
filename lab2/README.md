# Lab2 编写shell程序

#### 支持cd、pwd、exit内建命令：

##### cd命令：

执行完后continue执行下一条指令

```c
      	if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            continue;
        }
```

##### pwd命令：

执行完后continue执行下一条指令

```c
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            continue;
        }
```

##### exit命令：

exit指令后程序return退出

```c
	if (strcmp(args[0], "exit") == 0)
            return 0;
```



#### 支持export修改环境变量

调用`setenv`函数设置环境变量，例如`export MY_OWN_VAR=1`（`=`前后不能加空格）

```c
        if (strcmp(args[0], "export") == 0){
            char *eq = strchr(args[1],'=');
            if(eq == NULL)return 0;
            *eq = '\0';
            setenv(args[1], eq + 1,1);
            continue;
        }
```



#### 支持管道

拆解完命令行后处理`|`，`pipe_num`记录管道数量，函数`parse_pipe`处理管道命令，将`|`替换为`NULL`即将前后命令自动隔开。

```c
int parse_pipe(char *args[128], int cmd_pos[128]){
    /*处理pipe*/
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
```

递归执行管道命令，`i == pipe_num`时为执行最后一个命令，不含管道，否则创建管道，递归执行。

```c
void do_pipe(char *args[128],int cmd_pos[128],int i,int pipe_num,int arg_num){
     if(i == pipe_num){
        /*最后一个命令不含pipe*/
        ……
    }
    
    /*执行管道*/
    int fd[2];
    if(pipe(fd) < 0){
        perror("Failed to create pipe!");
        exit(255);
    };
    if (fork() == 0){
        close(fd[0]);
        dup2(fd[1],1);
        close(fd[1]);
        execvp(args[cmd_pos[i]],args+cmd_pos[i]);
        exit(255);
    }
    close(fd[1]);
    dup2(fd[0], 0);
    close(fd[0]);
    do_pipe(args, cmd_pos, i+1, pipe_num,arg_num);
}

```



#### 支持`>`、`<`、`>>`文件重定向

当`do_pipe`函数处理到`i == pipe_num`时为最后一个命令或命令不含管道，此时判断是否含有重定向。

```c
    if(i == pipe_num){
        /*最后一个命令不含pipe*/
        if(arg_num - cmd_pos[i] > 2){
            /*处理重定向*/
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
```



#### 处理命令中多余空格

在`parse_cmd`中，设置`space`变量用于标记是否为多余空格，`space`为0时为第一个空格，此时不忽略并将`space`设为1。`space `为1时为多余空格，忽略。

```c
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
```



#### 增加错误处理

##### fork失败

若`fork()`失败则返回-1

```c
        pid_t pid = fork();
        if (pid == 0) {
            ……
        }
        else perror("Faild to fork!");
```

##### execvp失败

`execvp()`在调用成功后不会返回，否则返回-1。

`main`函数中：

```c
        if (pid == 0) {
            /* 子进程 */
            do_pipe(args,cmd_pos,0,pipe_num,arg_num);
            perror("Faild to execute!");
            return 255;
        }
```

`do_pipe`函数中：

```c
    execvp(args[cmd_pos[i]], args+cmd_pos[i]);
        exit(255);
```

##### 管道创建失败

若`pipe()`失败则返回-1

```c
    if(pipe(fd) < 0){
        perror("Failed to create pipe!");
        exit(255);
    };
```





#### 测试样例

```shell
# cd /
# pwd
/
# ls
bin    dev   initrd.img      lib64	 mnt   root  snap      sys  var
boot   etc   initrd.img.old  lost+found  opt   run   srv       tmp  vmlinuz
cdrom  home  lib	     media	 proc  sbin  swapfile  usr  vmlinuz.old
# ls | wc
     27      27     162
# ls | cat | wc
     27      27     162
# export MY_OWN_VAR=1
# env | grep MY_OWN_VAR
MY_OWN_VAR=1
# cd    /home/ningyuting/lab/shell
# echo Hello > hello.txt
# cat < hello.txt
Hello
# ls | wc >> hello.txt
# cat   <    hello.txt
Hello
      6       6      41
# exit
```
