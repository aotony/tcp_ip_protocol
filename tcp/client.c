#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFLEN 10

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in s_addr;
    socklen_t len;
    unsigned int port;
    char buf[BUFLEN];

    /*建立socket*/
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(errno);
    }
    /*设置服务器端口*/
    if(argv[2])
        port = atoi(argv[2]);
    else
        port = 7777;
    /*设置服务器ip*/
    bzero(&s_addr, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    if (inet_aton(argv[1], (struct in_addr *)&s_addr.sin_addr.s_addr) == 0) {
        perror(argv[1]);
        exit(errno);
    }
    /*开始连接服务器*/
    if(connect(sockfd,(struct sockaddr*)&s_addr,sizeof(struct sockaddr)) == -1){
        perror("connect");
        exit(errno);
    }else
        printf("*****************client start***************\n");

    while(1){
        /******接收消息*******/
        bzero(buf,BUFLEN);
        len = recv(sockfd,buf,BUFLEN,0);
        if(len > 0)
            printf("receive massage:%s\n",buf);
        else{
            if(len < 0 )
                printf("receive failed\n");
            else
                printf("server stop\n");
            break;
        }
    _retry:
        /******发送消息*******/
        bzero(buf,BUFLEN);
        printf("enter your words:");
        /*fgets函数：从流中读取BUFLEN-1个字符*/
        fgets(buf,BUFLEN,stdin);
        /*打印发送的消息*/
        //fputs(buf,stdout);
        if(!strncasecmp(buf,"quit",4)){
            printf("client stop\n");
            break;
        }
        /*如果输入的字符串只有"\n"，即回车，那么请重新输入*/
        if(!strncmp(buf,"\n",1)){

            goto _retry;
        }
        /*如果buf中含有'\n'，那么要用strlen(buf)-1，去掉'\n'*/
        if(strchr(buf,'\n'))
            len = send(sockfd,buf,strlen(buf)-1,0);
        /*如果buf中没有'\n'，则用buf的真正长度strlen(buf)*/
        else
            len = send(sockfd,buf,strlen(buf),0);
        if(len > 0)
            printf("send successful\n");
        else{
            printf("send failed\n");
            break;
        }
    }
    /*关闭连接*/
    close(sockfd);

    return 0;
}


/**
 * Using command tcpdump to trace the data:
 * sudo tcpdump -vvv -X -i lo tcp port 7777
 *
 * 1. start server
 * 2. start client
 * 3 way handshake is as follows:
08:19:58.593130 IP (tos 0x0, ttl 64, id 50801, offset 0, flags [DF], proto TCP (6), length 60)
    localhost.52715 > localhost.7777: Flags [S], cksum 0xfe30 (incorrect -> 0x88f2), seq 2691414960, win 43690, options [mss 65495,sackOK,TS val 1444052417 ecr 0,nop,wscale 7], length 0
    0x0000:  4500 003c c671 4000 4006 7648 7f00 0001  E..<.q@.@.vH....
    0x0010:  7f00 0001 cdeb 1e61 a06b bbb0 0000 0000  .......a.k......
    0x0020:  a002 aaaa fe30 0000 0204 ffd7 0402 080a  .....0..........
    0x0030:  5612 7dc1 0000 0000 0103 0307            V.}.........
08:19:58.593145 IP (tos 0x0, ttl 64, id 0, offset 0, flags [DF], proto TCP (6), length 60)
    localhost.7777 > localhost.52715: Flags [S.], cksum 0xfe30 (incorrect -> 0xe320), seq 2043172900, ack 2691414961, win 43690, options [mss 65495,sackOK,TS val 1444052417 ecr 1444052417,nop,wscale 7], length 0
    0x0000:  4500 003c 0000 4000 4006 3cba 7f00 0001  E..<..@.@.<.....
    0x0010:  7f00 0001 1e61 cdeb 79c8 5824 a06b bbb1  .....a..y.X$.k..
    0x0020:  a012 aaaa fe30 0000 0204 ffd7 0402 080a  .....0..........
    0x0030:  5612 7dc1 5612 7dc1 0103 0307            V.}.V.}.....
08:19:58.593159 IP (tos 0x0, ttl 64, id 50802, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.52715 > localhost.7777: Flags [.], cksum 0xfe28 (incorrect -> 0xb565), seq 1, ack 1, win 342, options [nop,nop,TS val 1444052417 ecr 1444052417], length 0
    0x0000:  4500 0034 c672 4000 4006 764f 7f00 0001  E..4.r@.@.vO....
    0x0010:  7f00 0001 cdeb 1e61 a06b bbb1 79c8 5825  .......a.k..y.X%
    0x0020:  8010 0156 fe28 0000 0101 080a 5612 7dc1  ...V.(......V.}.
    0x0030:  5612 7dc1                                V.}.
 *
 * 3. server send "stest". (client reply ACK)
08:35:42.853990 IP (tos 0x0, ttl 64, id 9700, offset 0, flags [DF], proto TCP (6), length 57)
    localhost.7777 > localhost.52715: Flags [P.], cksum 0xfe2d (incorrect -> 0xce4b), seq 2043172901:2043172906, ack 2691414961, win 342, options [nop,nop,TS val 1444288482 ecr 1444052417], length 5
    0x0000:  4500 0039 25e4 4000 4006 16d9 7f00 0001  E..9%.@.@.......
    0x0010:  7f00 0001 1e61 cdeb 79c8 5825 a06b bbb1  .....a..y.X%.k..
    0x0020:  8018 0156 fe2d 0000 0101 080a 5616 17e2  ...V.-......V...
    0x0030:  5612 7dc1 7374 6573 74                   V.}.stest
08:35:42.854046 IP (tos 0x0, ttl 64, id 50803, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.52715 > localhost.7777: Flags [.], cksum 0xfe28 (incorrect -> 0x8117), seq 1, ack 5, win 342, options [nop,nop,TS val 1444288482 ecr 1444288482], length 0
    0x0000:  4500 0034 c673 4000 4006 764e 7f00 0001  E..4.s@.@.vN....
    0x0010:  7f00 0001 cdeb 1e61 a06b bbb1 79c8 582a  .......a.k..y.X*
    0x0020:  8010 0156 fe28 0000 0101 080a 5616 17e2  ...V.(......V...
    0x0030:  5616 17e2                                V...
 *
 * 4. client send "test". (server reply ACK)
08:35:42.854180 IP (tos 0x0, ttl 64, id 50804, offset 0, flags [DF], proto TCP (6), length 56)
    localhost.52715 > localhost.7777: Flags [P.], cksum 0xfe2c (incorrect -> 0x9931), seq 1:5, ack 5, win 342, options [nop,nop,TS val 1444288482 ecr 1444288482], length 4
    0x0000:  4500 0038 c674 4000 4006 7649 7f00 0001  E..8.t@.@.vI....
    0x0010:  7f00 0001 cdeb 1e61 a06b bbb1 79c8 582a  .......a.k..y.X*
    0x0020:  8018 0156 fe2c 0000 0101 080a 5616 17e2  ...V.,......V...
    0x0030:  5616 17e2 7465 7374                      V...test
08:35:42.854199 IP (tos 0x0, ttl 64, id 9701, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.7777 > localhost.52715: Flags [.], cksum 0xfe28 (incorrect -> 0x8113), seq 5, ack 5, win 342, options [nop,nop,TS val 1444288482 ecr 1444288482], length 0
    0x0000:  4500 0034 25e5 4000 4006 16dd 7f00 0001  E..4%.@.@.......
    0x0010:  7f00 0001 1e61 cdeb 79c8 582a a06b bbb5  .....a..y.X*.k..
    0x0020:  8010 0156 fe28 0000 0101 080a 5616 17e2  ...V.(......V...
    0x0030:  5616 17e2                                V...
 *
 * 5. server send "quit"
09:46:23.764733 IP (tos 0x0, ttl 64, id 61534, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.7777 > localhost.41078: Flags [F.], cksum 0xfe28 (incorrect -> 0x08a7), seq 131692629, ack 706057033, win 342, options [nop,nop,TS val 1456487601 ecr 1456179211], length 0
    0x0000:  4500 0034 f05e 4000 4006 4c63 7f00 0001  E..4.^@.@.Lc....
    0x0010:  7f00 0001 1e61 a076 07d9 7855 2a15 9349  .....a.v..xU*..I
    0x0020:  8011 0156 fe28 0000 0101 080a 56d0 3cb1  ...V.(......V.<.
    0x0030:  56cb 880b                                V...
09:46:23.764851 IP (tos 0x0, ttl 64, id 6981, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.41078 > localhost.7777: Flags [F.], cksum 0xfe28 (incorrect -> 0x53fb), seq 1, ack 1, win 342, options [nop,nop,TS val 1456487601 ecr 1456487601], length 0
    0x0000:  4500 0034 1b45 4000 4006 217d 7f00 0001  E..4.E@.@.!}....
    0x0010:  7f00 0001 a076 1e61 2a15 9349 07d9 7856  .....v.a*..I..xV
    0x0020:  8011 0156 fe28 0000 0101 080a 56d0 3cb1  ...V.(......V.<.
    0x0030:  56d0 3cb1                                V.<.
09:46:23.764872 IP (tos 0x0, ttl 64, id 61535, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.7777 > localhost.41078: Flags [.], cksum 0xfe28 (incorrect -> 0x53fb), seq 1, ack 2, win 342, options [nop,nop,TS val 1456487601 ecr 1456487601], length 0
    0x0000:  4500 0034 f05f 4000 4006 4c62 7f00 0001  E..4._@.@.Lb....
    0x0010:  7f00 0001 1e61 a076 07d9 7856 2a15 934a  .....a.v..xV*..J
    0x0020:  8010 0156 fe28 0000 0101 080a 56d0 3cb1  ...V.(......V.<.
    0x0030:  56d0 3cb1                                V.<.
 *
 * 6. client send "quit"
09:56:38.635064 IP (tos 0x0, ttl 64, id 53196, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.41198 > localhost.7777: Flags [F.], cksum 0xfe28 (incorrect -> 0xe1cf), seq 1553847980, ack 1263348770, win 342, options [nop,nop,TS val 1456641318 ecr 1456597184], length 0
    0x0000:  4500 0034 cfcc 4000 4006 6cf5 7f00 0001  E..4..@.@.l.....
    0x0010:  7f00 0001 a0ee 1e61 5c9d d6ac 4b4d 2c22  .......a\...KM,"
    0x0020:  8011 0156 fe28 0000 0101 080a 56d2 9526  ...V.(......V..&
    0x0030:  56d1 e8c0                                V...
09:56:38.635255 IP (tos 0x0, ttl 64, id 13696, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.7777 > localhost.41198: Flags [F.], cksum 0xfe28 (incorrect -> 0x3568), seq 1, ack 1, win 342, options [nop,nop,TS val 1456641318 ecr 1456641318], length 0
    0x0000:  4500 0034 3580 4000 4006 0742 7f00 0001  E..45.@.@..B....
    0x0010:  7f00 0001 1e61 a0ee 4b4d 2c22 5c9d d6ad  .....a..KM,"\...
    0x0020:  8011 0156 fe28 0000 0101 080a 56d2 9526  ...V.(......V..&
    0x0030:  56d2 9526                                V..&
09:56:38.635284 IP (tos 0x0, ttl 64, id 53197, offset 0, flags [DF], proto TCP (6), length 52)
    localhost.41198 > localhost.7777: Flags [.], cksum 0xfe28 (incorrect -> 0x3568), seq 1, ack 2, win 342, options [nop,nop,TS val 1456641318 ecr 1456641318], length 0
    0x0000:  4500 0034 cfcd 4000 4006 6cf4 7f00 0001  E..4..@.@.l.....
    0x0010:  7f00 0001 a0ee 1e61 5c9d d6ad 4b4d 2c23  .......a\...KM,#
    0x0020:  8010 0156 fe28 0000 0101 080a 56d2 9526  ...V.(......V..&
    0x0030:  56d2 9526                                V..&
 *
 * 7. TCP connection setup refused:
09:25:33.014788 IP (tos 0x0, ttl 64, id 21696, offset 0, flags [DF], proto TCP (6), length 60)
    localhost.41077 > localhost.7777: Flags [S], cksum 0xfe30 (incorrect -> 0x5ebe), seq 2517377921, win 43690, options [mss 65495,sackOK,TS val 1456174913 ecr 0,nop,wscale 7], length 0
    0x0000:  4500 003c 54c0 4000 4006 e7f9 7f00 0001  E..<T.@.@.......
    0x0010:  7f00 0001 a075 1e61 960c 2381 0000 0000  .....u.a..#.....
    0x0020:  a002 aaaa fe30 0000 0204 ffd7 0402 080a  .....0..........
    0x0030:  56cb 7741 0000 0000 0103 0307            V.wA........
09:25:33.014821 IP (tos 0x0, ttl 64, id 21450, offset 0, flags [DF], proto TCP (6), length 40)
    localhost.7777 > localhost.41077: Flags [R.], cksum 0x3969 (correct), seq 0, ack 2517377922, win 0, length 0
    0x0000:  4500 0028 53ca 4000 4006 e903 7f00 0001  E..(S.@.@.......
    0x0010:  7f00 0001 1e61 a075 0000 0000 960c 2382  .....a.u......#.
    0x0020:  5014 0000 3969 0000                      P...9i..
 *
 */
