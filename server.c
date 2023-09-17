#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define ROOM_CAPACITY 3
#define ROOMS_COUNT 4

typedef struct {
    char name;
    int clients[ROOM_CAPACITY];
    int size;
}
Room;

int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, max_sd;
    char buffer[1024] = {0};
    fd_set master_set, working_set;
       
    server_fd = setupServer(8080);

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    Room rooms[ROOMS_COUNT];
    char rooms_name[ROOMS_COUNT] = {'C', 'B', 'O', 'M'};

    for (int i = 0; i < ROOMS_COUNT; i++){
        rooms[i].name = rooms_name[i];
        rooms[i].size = 0;
    }

    write(1, "Server is running\n", 18);

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                
                if (i == server_fd) {  // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                }
                
                else { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i , buffer, 1024, 0);
                    
                    if (bytes_received == 0) { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }

                    switch (buffer[0])
                    {
                    case 'C':
                        rooms[0].clients[rooms[0].size] = i;
                        rooms[0].size += 1;
                        break;

                    case 'B':
                        rooms[1].clients[rooms[1].size] = i;
                        rooms[1].size += 1;
                        break;

                    case 'O':
                        rooms[2].clients[rooms[2].size] = i;
                        rooms[2].size += 1;
                        break;

                    case 'M':
                        rooms[3].clients[rooms[3].size] = i;
                        rooms[3].size += 1;
                        break;
                    
                    default:
                        break;
                    }

                    for (int j = 0; j < ROOMS_COUNT; j++){
                        if (rooms[j].size >= ROOM_CAPACITY){
                            printf("Create a room\n");                            

                            int port = 8000;
                            printf("port : %d\n", port);

                            char port_str[20];
                            sprintf(port_str, "%d", port);

                            for (int k = 0; k < 3; k++){
                                send(rooms[j].clients[k], port_str, strlen(port_str), 0);
                            }
                            send(rooms[j].clients[0], "3", strlen("3"), 0);
                            send(rooms[j].clients[1], "2", strlen("2"), 0);
                            send(rooms[j].clients[2], "1", strlen("1"), 0);
                            rooms[j].size = 0;
                        }
                    }
                    if(strlen(buffer) > 5){
                        printf("file setting \n");
                        int file_fd;
                        char major = buffer[6];
                        printf("major : %c\n", major);
                        switch (major)
                        {
                        case 'C':
                            file_fd = open("computer.txt", O_APPEND | O_RDWR);
                            write(file_fd, buffer, strlen(buffer));
                            close(file_fd);
                            break;

                        case 'B':
                            file_fd = open("bargh.txt", O_APPEND | O_RDWR);
                            write(file_fd, buffer, strlen(buffer));
                            close(file_fd);
                            break;

                        case 'O':
                            file_fd = open("omran.txt", O_APPEND | O_RDWR);
                            write(file_fd, buffer, strlen(buffer));
                            close(file_fd);
                            break;

                        case 'M':
                            file_fd = open("mechanic.txt", O_APPEND | O_RDWR);
                            write(file_fd, buffer, strlen(buffer));
                            close(file_fd);
                            break;

                        default:
                            break;
                        }
                    }
                    else
                        printf("client %d: %s\n", i, buffer);
                    memset(buffer, 0, 1024);
                        
                    
                }
            }
        }

    }

    return 0;
}