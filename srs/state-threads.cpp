#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <st.h>

#define LISTEN_PORT 8000

#define ERR_EXIT(m) \
  do                \
  {                 \
    perror(m);      \
    exit(-1);       \
  } while (0)

void *client_thread(void *arg)
{
  st_netfd_t client_st_fd = (st_netfd_t)arg;
  // 用于获取与 st_netfd_t 对象关联的文件描述符（File Descriptor）。它返回一个整数值，表示文件描述符的值。
  // 将 st_netfd_t 对象转换为普通的文件描述符
  int client_fd = st_netfd_fileno(client_st_fd);

  sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  // 获取与套接字连接的对端的地址信息
  int ret = getpeername(client_fd, (sockaddr *)&client_addr, &client_addr_len);
  if (ret == -1)
  {
    printf("[WARN] Failed to get client ip: %s\n", strerror(ret));
  }

  char ip_buf[INET_ADDRSTRLEN];
  // 内存区域清零
  // bzero(ip_buf, sizeof(ip_buf)); // 此函数已经过时
  memset(ip_buf, 0, sizeof(ip_buf));
  inet_ntop(client_addr.sin_family, &client_addr.sin_addr, ip_buf,
            sizeof(ip_buf));

  while (1)
  {
    char buf[1024] = {0};
    // 从给定的套接字中读取指定字节数的数据，并将其存储在提供的缓冲区 buf 中
    ssize_t ret = st_read(client_st_fd, buf, sizeof(buf), ST_UTIME_NO_TIMEOUT);
    if (ret == -1)
    {
      printf("client st_read error\n");
      break;
    }
    else if (ret == 0)
    {
      printf("client quit, ip = %s\n", ip_buf);
      break;
    }

    printf("recv from %s, data = %s", ip_buf, buf);

    ret = st_write(client_st_fd, buf, ret, ST_UTIME_NO_TIMEOUT);
    if (ret == -1)
    {
      printf("client st_write error\n");
    }
  }
}

void *listen_thread(void *arg)
{
  while (1)
  {
    st_netfd_t client_st_fd =
        st_accept((st_netfd_t)arg, NULL, NULL, ST_UTIME_NO_TIMEOUT);
    if (client_st_fd == NULL)
    {
      continue;
    }

    printf("get a new client, fd = %d\n", st_netfd_fileno(client_st_fd));

    st_thread_t client_tid =
        st_thread_create(client_thread, (void *)client_st_fd, 0, 0);
    if (client_tid == NULL)
    {
      printf("Failed to st create client thread\n");
    }
  }
}

int main()
{
  // 用于设置 ST 库的事件系统。
  int ret = st_set_eventsys(ST_EVENTSYS_ALT);
  if (ret == -1)
  {
    printf("st_set_eventsys use linux epoll failed\n");
  }
  // st初始化
  ret = st_init();
  if (ret != 0)
  {
    printf("st_init failed. ret = %d\n", ret);
    return -1;
  }
  // 创建套接字
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1)
  {
    ERR_EXIT("socket");
  }

  int reuse_socket = 1;

  // 设置套接字选项
  ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_socket,
                   sizeof(int));
  if (ret == -1)
  {
    ERR_EXIT("setsockopt");
  }

  struct sockaddr_in server_addr;            // 用于表示 IPv4 地址的结构体
  server_addr.sin_family = AF_INET;          // 地址族，一般为 AF_INET
  server_addr.sin_port = htons(LISTEN_PORT); // 端口
  server_addr.sin_addr.s_addr = INADDR_ANY;  // ipv4地址结构
  // 将套接字与特定的 IP 地址和端口号进行绑定
  ret =
      bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
  if (ret == -1)
  {
    ERR_EXIT("bind");
  }

  ret = listen(listen_fd, 128);
  if (ret == -1)
  {
    ERR_EXIT("listen");
  }
  // st_netfd_open_socket() 是 State Threads (ST) 库中的一个函数，用于创建一个 st_netfd_t 类型的文件描述符对象，以便进行异步 I/O 操作。
  st_netfd_t st_listen_fd = st_netfd_open_socket(listen_fd);
  if (!st_listen_fd)
  {
    printf("st_netfd_open_socket open socket failed.\n");
    return -1;
  }

  st_thread_t listen_tid =
      st_thread_create(listen_thread, (void *)st_listen_fd, 1, 0);
  if (listen_tid == NULL)
  {
    printf("Failed to st create listen thread\n");
  }

  while (1)
  {
    st_sleep(1); /*用于让出CPU执行权，重新调度就绪的协程。*/
  }

  return 0;
}
