#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>

int main(void)
{
        int sockfd;
        struct sockaddr_in server;
        char msg[20]={0};

        sockfd = socket(AF_INET,SOCK_DGRAM,0);
        if (sockfd < 0) {
                perror("socket error!\n");
                exit(-1);
        }

        memset(&server,0,sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("192.168.1.1");
        server.sin_port = htons(7777);

        strncpy(msg,"hello",sizeof("hello"));

        printf("send message:%s\n",msg);

        if (sendto(sockfd,msg,20,0,(struct sockaddr *)&server,sizeof(server)) != 20) {
                perror("sendto error!\n");
                exit(-1);
        }

        exit(0);
}


/**
 * Using command tcpdump to trace the data:
 * sudo tcpdump -vvv -X udp port 7777
 *
 * the following is shown in terminal:
17:34:03.862718 IP (tos 0x0, ttl 64, id 5885, offset 0, flags [DF], proto UDP (17), length 48)
    6846f66f356f.59906 > 192.168.1.1.7777: [bad udp cksum 0xaa83 -> 0x092a!] UDP, length 20
        0x0000:  4500 0030 16fd 4000 4011 796a c0a8 2804  E..0..@.@.yj..(.
        0x0010:  c0a8 0101 ea02 1e61 001c aa83 6865 6c6c  .......a....hell
        0x0020:  6f00 0000 0000 0000 0000 0000 0000 0000  o...............
 *
 * After changing server addr to 192.168.1.2, following is shown:
17:37:02.483615 IP (tos 0x0, ttl 64, id 27643, offset 0, flags [DF], proto UDP (17), length 48)
    6846f66f356f.53546 > 192.168.1.2.7777: [bad udp cksum 0xaa84 -> 0x2201!] UDP, length 20
        0x0000:  4500 0030 6bfb 4000 4011 246b c0a8 2804  E..0k.@.@.$k..(.
        0x0010:  c0a8 0102 d12a 1e61 001c aa84 6865 6c6c  .....*.a....hell
        0x0020:  6f00 0000 0000 0000 0000 0000 0000 0000  o...............
 *
 * Here we can see, the total length is 48, which is 0030, the second word of data;
 * the third word is just the id number;
 * the seventh word and eighth word compose the local ip address 192.168.40.4;
 * the ninth and tenth word compose the target ip address;
 * the eleventh word is the local port number;
 * the twelveth word is the target port number 7777;
 * the thirteenth word is the length;
 * the fourteenth word is the checksum;
 * the next 10 words are the message.
 *
 * Then change ip back and send one more word "a", following is shown:
18:07:44.393028 IP (tos 0x0, ttl 64, id 5885, offset 0, flags [DF], proto UDP (17), length 48)
    41b67fc71516.60833 > 192.168.1.1.7777: [bad udp cksum 0xaa85 -> 0x0528!] UDP, length 20
        0x0000:  4500 0030 16fd 4000 4011 7968 c0a8 2806  E..0..@.@.yh..(.
        0x0010:  c0a8 0101 eda1 1e61 001c aa85 6865 6c6c  .......a....hell
        0x0020:  6f61 0000 0000 0000 0000 0000 0000 0000  oa..............
 *
 */
