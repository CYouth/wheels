#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   
#include <unistd.h>   
#include <arpa/inet.h>   
#include <sys/socket.h>   
#include <sys/poll.h>   
   
#define BUF_SIZE 100   
#define POLL_MAX 50000 
void error_handling(char *buf);   
   
int main(int argc, char *argv[])   
{   
    int serv_sock, clnt_sock;   
    struct sockaddr_in serv_adr, clnt_adr;
    struct pollfd fds[POLL_MAX]; /*用于poll函数第一个参数的数组*/  
   
    socklen_t adr_sz;   
    int nfds,tmp, str_len, fd_num, i;   
    char buf[BUF_SIZE];  //缓冲区
    if(argc!=2) {   
        printf("Usage : %s <port>\n", argv[0]);   
        exit(1);   
    }   
   
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);  //监听套接字 
    //设置监听地址
    memset(&serv_adr, 0, sizeof(serv_adr));   
    serv_adr.sin_family=AF_INET;   
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);   
    serv_adr.sin_port=htons(atoi(argv[1]));   
       
    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)   //绑定
        error_handling("bind() error");   
    if(listen(serv_sock, 5)==-1)   
        error_handling("listen() error");   
    
    tmp=0;//当前索引
    //注册
    fds[tmp].fd=serv_sock;
    fds[tmp].events=POLLIN;
    
    while(1)   
    {    
        nfds=tmp;
        if((fd_num=poll(fds, nfds+1, -1))==-1)  //poll
            break;   
           
        if(fd_num==0)   
            continue;   
   
        for(i=0; i<nfds+1; i++)   //有事件发生，要遍历一次数组
        {   
            if(fds[i].fd!=-1 && (fds[i].revents&POLLIN))   
            {   
                if(fds[i].fd==serv_sock)     // connection request!   
                {   
                    adr_sz=sizeof(clnt_adr);   
                    clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);   //客户端socket
                    fds[tmp+1].fd=clnt_sock;
                    fds[tmp+1].events=POLLIN;
                    tmp++;
                    printf("connected client: %d \n", clnt_sock);   
                }   
                else    // read message!   
                {   
                    str_len=read(fds[i].fd, buf, BUF_SIZE);   
                    if(str_len==0)    // close request!   
                    {   
                        fds[i].fd=-1;  //设置为不可用 
                        close(fds[i].fd);   
                        printf("closed client: %d \n", fds[i].fd);   
                    }   
                    else   
                    {   
                        write(fds[i].fd, buf, str_len);    // echo!   
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

 