# Lab3 实现简单HTTP服务器

### 实验设计：

#### 线程池

先考虑多进程VS多线程：

1. 每次连接都fork开太大。每个子进程都要复制父进程页表，而所有线程共享一个页表。
2. 当并发量较大时，进程调度器压力太大。

基于以上原因，选择多线程。而每次创建新线程需要额外的开销，因此采用线程池设计。预先创建一组线程，在请求到达时分配，减少额外的创建开销。



线程池设计:

```c
struct threadpool
{
    int thread_num;                   //线程池中开启线程的个数
    struct job *head;                 //指向job的头指针
    struct job *tail;                 //指向job的尾指针
    pthread_t *pthreads;              //线程池中所有线程的pthread_t
    pthread_mutex_t mutex;            //互斥信号量
    pthread_cond_t queue_empty;       //队列为空的条件变量
    pthread_cond_t queue_not_empty;   //队列不为空的条件变量
    int queue_cur_num;                //队列当前的job个数
    int pool_close;                   //线程池是否已经关闭
};
```

`void *threadpool_function(void *arg)`为线程函数，在线程池正常工作时调用`handle_clnt`函数执行。参数`void *arg`指向线程池，无返回值。

`void *threadpool_init(int thread_num)`为线程池初始化函数。参数`int thread_num`为线程池开启的线程个数，若执行成功返回线程池地址。

`int threadpool_add_job(struct threadpool *pool, int clnt_sock)`为向线程池中添加任务的函数。参数`pool`是线程池地址，`clnt_sock`是客户端套接字。成功返回0，失败返回-1。

`int threadpool_destroy(struct threadpool *pool)`为销毁线程池的函数。其实程序并不会执行到`threadpooj_destroy`，但是为了线程池功能的完整性此处仍然保留了销毁函数。成功返回0，失败返回-1。

#### 解析和检验HTTP头

* 读取请求：

  由于`read`可能一次不能读完请求，所以需要循环读取。

  ```c
  		req[0] = '\0';
      char* buf = (char*) malloc(MAX_RECV_LEN * sizeof(char));
      ssize_t read_len;
      while(1)
      {
          read_len = read(clnt_sock, buf, MAX_RECV_LEN - 1);
        	if (read_len < 0) handle_error("failed to read clnt_sock");
          buf[read_len] = '\0';
          strcat(req, buf);
          if(buf[read_len - 4] == '\r' && buf[read_len - 3] == '\n' && buf[read_len - 2] == '\r' && buf[read_len - 1] == '\n') break;
      }
      *req_len = strlen(req);
  ```

  `buf`临时保存读到的内容，`read_len`为一次读取到的数据。如果`read`函数返回值为负数，则出错，进行错误处理，否则将新读到的内容接在已读到的内容后面。如果读到的内容最后几位为`"\r\n\r\n"`则已读完，跳出循环。

* 最大路径长度：

  Linux系统对路径长度有限制，通过`getconf PATH_MAX/usr`命令可以得到路径长度限制为4096。因此设置`MAX_PATH_LEN`为4096。

* 解析HTTP头：

  如果读到的请求长度小于5字节，则没有'GET /'，请求不正确，返回`500 Internal Server Error`。

  判断前五个字节是否为'GET /'，如果不是，则在本实验中也返回`500 Internal Server Error`。

  s1指向第一个空格，s2从'/'开始逐个向后。当`s2-s1`即路径长度大于`MAX_PATH_LEN`或`req[s2]`为空格时退出循环，否则检查s2是否为`/`。k为一个标记变量，初始化为0，如果路径中出现了`/../`，则向上跳一级目录，k-1;如果路径中出现了`/`，则进入下一级目录，k+1。在此过程中，任何时刻k为负值表示跳出了当前目录，返回错误`500 Internal Server Error`。

  若跳出循环时是`s2-s1>MAX_PATH_LEN`，则路径长度超过Linux路径长度限制，返回错误`500 Internal Server Error`。否则，找到了第二个空格，将`req[s1]`改为`.`，`req[s2]`改为`\0`，此时将`req+s1`传入`open`，则可以打开文件。如果返回值小于0，则打开失败，返回错误`404 Not Found`，否则检查是否为常规文件，如果不是，返回错误`500 Internal Server Error`，否则正常返回文件描述符。

* 返回值-2代表`500 Internal Server Error`，-1代表`404 Not Found`。



#### 错误和异常处理

错误处理使用宏handle_error完成：

```c
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)
```



socket函数错误处理：

```c
int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock == -1)
        handle_error("socket failed");
```

bind 函数错误处理:

```c
if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        handle_error("failed to bind");
```

listen函数错误处理:

```c
if (listen(serv_sock, SOMAXCONN)==-1)
        handle_error("failed to listen");
```

malloc函数错误处理（以malloc req_buf为例）：

```c
char *req_buf = (char *)malloc(MAX_RECV_LEN * sizeof(char));
    if (req_buf == NULL)
        handle_error("failed to malloc req_buf\n");
```

Handle_clnt函数中对parse_request返回值进行错误处理（也包括对write函数的错误处理）：

如果返回值是-1，为404 Not Found类型，返回的数据为：

```c
if (file_fd == -1)
    {// 404
        sprintf(response, "HTTP/1.0 %s\r\nContent-Length: %zd\r\n\r\n", HTTP_STATUS_404, (size_t)0);
        size_t response_len = strlen(response);
        if (write(clnt_sock, response, response_len) == -1)
            handle_error("failed to write response when 404");
    }
```

如果返回值是-2，为500 Internal Server Error类型，返回的数据为:

```c
else if (file_fd == -2)
    {// 500
        sprintf(response, "HTTP/1.0 %s\r\nContent-Length: %zd\r\n\r\n", HTTP_STATUS_500, (size_t)0);
        size_t response_len = strlen(response);
        if (write(clnt_sock, response, response_len) == -1)
            handle_error("failed to write response when 500");
    }
```

否则返回的是文件描述符。

read和write函数错误处理：

```c
while (response_len = read(file_fd, response, MAX_SEND_LEN)){
            if (response_len == -1)
                handle_error("failed to read file");
            if (write(clnt_sock, response, response_len) == -1)
                handle_error("failed to write file to clnt_sock");
        }
```

对HTTP的错误处理已在`解析和检验HTTP头`部分有详细解释。



### 编译/运行方法

```
gcc server.c -o server -pthread
./server
```



### 测试结果

使用`Siege`进行在Linux虚拟机上测试：

`siege -c 255 -r 10 http://127.0.0.1:8000/kernel7.img`（其中kernel7.img是lab1裁剪内核过程中裁过的一个大小为4.9M的文件）

得到的性能指标为

```
Transactions:2550 hits
Availability:100.00%
Elapsed time:17.70 secs
Data transferred:12001.26 MB
Response time:1.57 secs
Transaction rate:122.07 trans/sec
Throughput:678.04 MB/sec
Concurrency:226.12
Successful transactions:2550
Failed transactions:0
Longest transaction:9.56
Shortest transaction:0.01
```
![测试截图](https://github.com/nnnyt/OSH-2019-Labs/blob/master/lab3/docs/测试截图.png)
