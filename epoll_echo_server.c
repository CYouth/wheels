#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   
#include <unistd.h>   
#include <arpa/inet.h>   
#include <sys/socket.h>   
#include <sys/epoll.h>   
   
#define BUF_SIZE 100   
#define EPOLL_SIZE 50   
void error_handling(char *buf);   
   
int main(int argc, char *argv[])   
{   
    int serv_sock, clnt_sock;   
    struct sockaddr_in serv_adr, clnt_adr;   
    socklen_t adr_sz;   
    int str_len, i;   
    char buf[BUF_SIZE];   
   
    struct epoll_event *ep_events;   //返回事件数组
    struct epoll_event event;   //监听事件
    int epfd, event_cnt;   
   
    if(argc!=2) {   
        printf("Usage : %s <port>\n", argv[0]);   
        exit(1);   
    }   
   
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);   //监听socket
    //设置监听地址
    memset(&serv_adr, 0, sizeof(serv_adr));   
    serv_adr.sin_family=AF_INET;   
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);   
    serv_adr.sin_port=htons(atoi(argv[1]));   //port
       
    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)   
        error_handling("bind() error");   
    if(listen(serv_sock, 5)==-1)   
        error_handling("listen() error");   
   
    epfd=epoll_create(1);   //epoll_create，参数大于0就行
    ep_events=malloc(sizeof(struct epoll_event)*EPOLL_SIZE);   
   
    event.events=EPOLLIN;   
    event.data.fd=serv_sock;       
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);   
   
    while(1)   
    {   
        event_cnt=epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);   //epoll
        if(event_cnt==-1)   
        {   
            puts("epoll_wait() error");   
            break;   
        }   
   
        for(i=0; i<event_cnt; i++)   //只需要遍历返回事件数组就行
        {   
            if(ep_events[i].data.fd==serv_sock)   //connection request
            {   
                adr_sz=sizeof(clnt_adr);   
                clnt_sock= accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz); 
                //注册新的事件  
                event.events=EPOLLIN;   
                event.data.fd=clnt_sock;   
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);   
                printf("connected client: %d \n", clnt_sock);   
            }   
            else   
            {
                memset(buf,97,BUF_SIZE);
                str_len=read(ep_events[i].data.fd, buf, BUF_SIZE);   
                if(str_len==0)    // close request!   
                {   
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);   
                    close(ep_events[i].data.fd);   
                    printf("closed client: %d \n", ep_events[i].data.fd);   
                }   
                else   
                {   
                    printf("read str_len is %d: \n", str_len);
                    for(int i=0;i<str_len+3;i++){
                        if(buf[i]=='\n'){
                            printf("%d:%s\n", i,"new");
                        }else{
                            printf("%d:%c\n", i,buf[i]);
                        }
                        
                    }
                    write(ep_events[i].data.fd, buf, str_len);    // echo!   
                }   
    
            }   
        }   
    }   
    close(serv_sock);   
    close(epfd);   //记得关闭epoll fd
    return 0;   
}   
   
void error_handling(char *buf)   
{   
    fputs(buf, stderr);   
    fputc('\n', stderr);   
    exit(1);   
}   

 