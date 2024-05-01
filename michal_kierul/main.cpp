// Michał Kierul 322178

#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <chrono>
#include <poll.h>
#include "utils.hpp"
#include "heap.hpp"

#define timeout 20
#define poll_timeout 10
#define window_size 1000
#define window_num 1000

using namespace std::chrono;

int main(int argc, char *argv[]){
    if(argc != 5)
        ERROR("wrong number of arguments");

    char* ip = argv[1];
    char* port = argv[2];
    char* filename = argv[3];
    char* size = argv[4];
    char* tmp;
    FILE *file = fopen(filename,"w");

    if(!is_number(port))
        ERROR("wrong port argument");

    if(!is_number(size))
        ERROR("wrong size argument");

    int port_ = strtol(port,&tmp,10);
    unsigned int size_ = strtol(size,&tmp,10);

    if(size_ > 10000000)
        ERROR("wrong size argument");
    
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
        ERROR("socket error");

    struct sockaddr_in my_address;
    memset(&my_address, 0, sizeof(my_address));
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock_fd, (struct sockaddr*)&my_address, sizeof(my_address)) < 0)
        ERROR("bind error");

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_);
    if(inet_pton(AF_INET, ip, &server_address.sin_addr) != 1)
        ERROR("wrong ip address");

    int packs = size_/window_size;
    if(size_%window_size != 0)
        packs += 1;
    int beg_idx = 0;
    int beg_win = 0;

    int* heap = new int[window_num];
    int heap_size = packs;
    if(heap_size >= window_num)
        heap_size = window_num;

    int* heap_pos = new int[window_num];

    bool* ack = new bool[window_num];
    u_int8_t** wind = new u_int8_t*[window_num];
    long long* t = new long long[window_num];
    for(int i = 0; i < window_num && i < packs; i++){
        wind[i] = new u_int8_t[window_size];
        ack[i] = false;
        t[i] = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() - timeout;
        heap[i] = i;
        heap_pos[i] = i;
    }

    struct pollfd *socket = new struct pollfd[1];
    socket[0].fd = sock_fd;
    socket[0].events = POLLIN;

    for(;beg_idx < packs;){
        if(poll(socket,1,poll_timeout) < 0)
            ERROR("poll error");

        if(socket[0].revents & POLLIN){
            struct sockaddr_in sender;
            socklen_t sender_len = sizeof(sender);
            u_int8_t buffer[IP_MAXPACKET+1];

            ssize_t datagram_len = recvfrom(sock_fd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
            if (datagram_len < 0)
                ERROR("recvfrom error");

            char sender_ip_str[20];
            inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));

            if(strcmp(sender_ip_str,ip) == 0 && ntohs(sender.sin_port) == port_){
                int pos = 5;
                int start = read_int(buffer, pos);
                pos++;
                int ac_size = read_int(buffer, pos);
                pos++;
                int idx = start/window_size;
                int win = (idx-beg_idx+beg_win)%window_num;

                if(idx >= beg_idx && idx < beg_idx + window_num && !ack[win]){
                    ack[win] = true;
                    for(int i = 0; i < ac_size; i++)
                        wind[win][i] = buffer[pos+i];
                    remove_heap(heap, heap_size, heap_pos[win], heap_pos, t);
                }

                for(;ack[beg_win];){
                    if(beg_idx < packs-1 || size_%window_size == 0 ){
                        if(fwrite(wind[beg_win],1,window_size,file) != window_size)
                            ERROR("fwrite error");
                    }else{
                        if(fwrite(wind[beg_win],1,size_%window_size,file) != size_%window_size)
                            ERROR("fwrite error");
                    }

                    ack[beg_win] = false;
                    t[beg_win] = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() - timeout;
                    if(beg_idx+window_num < packs)
                        insert_heap(heap, window_num, heap_size, beg_win, heap_pos, t);
                    beg_win = (beg_win+1)%window_num;
                    beg_idx++;
                    if(!ack[beg_win] && beg_idx < packs)
                        printf("Pobrano %d B\n", beg_idx*window_size);
                }
            }
        }else if(socket[0].revents != 0){
            ERROR("socket error");
        }else{
            long long ac_t = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();

            for(;heap_size > 0;){
                int win = heap[0];
                int idx = win-beg_win;
                if(idx<0)
                    idx += window_num;
                idx += beg_idx;

                if(ac_t-t[win] < timeout){
                    break;
                }else if(!ack[win]){
                    remove_heap(heap, heap_size, 0, heap_pos, t);
                    t[win] = ac_t;
                    insert_heap(heap, window_num, heap_size, win, heap_pos, t);

                    char msg[30] = "GET ";
                    int pos = write_int_to_char(msg, idx*window_size, 4);
                    msg[pos] = ' ';
                    if(idx < packs-1 || size_%window_size == 0)
                        pos = write_int_to_char(msg, window_size, pos+1);
                    else
                        pos = write_int_to_char(msg, size_%window_size, pos+1);
                    msg[pos] = '\n';
                    unsigned int msg_len = strlen(msg);

                    if (sendto(sock_fd, msg, msg_len, 0, (struct sockaddr*)&server_address, sizeof(server_address)) != msg_len)
                        ERROR("sendto error");
                }else{
                    remove_heap(heap, heap_size, 0, heap_pos, t);
                }
            }
        }
    }
    printf("Ukonczono!\n");
    close (sock_fd);
    fclose(file);
    delete[] socket;
    delete[] heap_pos;
    delete[] heap;
    delete[] t;
    delete[] ack;
    for(int i = 0; i < window_num && i < packs; i++)
        delete[] wind[i];
    delete[] wind;
    return EXIT_SUCCESS;
}

