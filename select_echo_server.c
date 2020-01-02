#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   
#include <unistd.h>   
#include <arpa/inet.h>   
#include <sys/socket.h>   
#include <sys/time.h>   
#include <sys/select.h>   
   
#define BUF_SIZE 100   
void error_handling(char *buf);   
   
int main(int argc, char *argv[])   
{   
    int serv_sock, clnt_sock;   
    struct sockaddr_in serv_adr, clnt_adr;   
    struct timeval timeout;   
    fd_set reads, cpy_reads;//用两个位数组，有长度限制，在32位系统中长度为1024bit   
   
    socklen_t adr_sz;   
    int fd_max, str_len, fd_num, i;   
    char buf[BUF_SIZE];   
    if(argc!=2) {   
        printf("Usage : %s <port>\n", argv[0]);   
        exit(1);   
    }   
   
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);  //监听套接字
    //设置监听地址
    memset(&serv_adr, 0, sizeof(serv_adr));   
    serv_adr.sin_family=AF_INET;   
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);   
    serv_adr.sin_port=htons(atoi(argv[1])); //端口  
       
    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)   
        error_handling("bind() error");   
    if(listen(serv_sock, 5)==-1)   
        error_handling("listen() error");   
    
    FD_ZERO(&reads); //初始化  
    FD_SET(serv_sock, &reads);  //注册 
    fd_max=serv_sock;   //最大fd
   
    while(1)   
    {   
        cpy_reads=reads;   //因为cpy_reads是用在select函数之中的，所以返回结果之后里面的数据就乱了，或者有了新的fd注册，所以需要重新赋值
        timeout.tv_sec=5;   
        timeout.tv_usec=5000;   
   
        if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1)  //只监听read事件
            break;   
           
        if(fd_num==0)   
            continue;   
   
        for(i=0; i<fd_max+1; i++)   //有事件发生，要遍历一次位数组
        {   
            if(FD_ISSET(i, &cpy_reads))   
            {   
                if(i==serv_sock)     // connection request!   
                {   
                    adr_sz=sizeof(clnt_adr);   
                    clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);   //客户端socket
                    FD_SET(clnt_sock, &reads);   //注册
                    if(fd_max<clnt_sock)   //最大fd
                        fd_max=clnt_sock;   
                    printf("connected client: %d \n", clnt_sock);   
                }   
                else    // read message!   
                {   
                    str_len=read(i, buf, BUF_SIZE);   
                    if(str_len==0)    // close request!   
                    {   
                        FD_CLR(i, &reads);   
                        close(i);   
                        printf("closed client: %d \n", i);   
                    }   
                    else   
                    {   
                        write(i, buf, str_len);    // echo!   
                    }   
                }   
            }   
        }   
    }   
    close(serv_sock);   
    return 0;   
}   
   
void error_handling(char *buf)   
{   
    fputs(buf, stderr);   
    fputc('\n', stderr);   
    exit(1);   
}  

 