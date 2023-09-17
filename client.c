#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>

// The format of asking question: client_id#Q:message
// The format of asking question: client_id#client_id of the person who asked?A:message
// The format of best response: clinet_id#B:best ans

int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}

void alarm_handler(int sig) {
    printf("time up!\n");
}

int main(int argc, char const *argv[]) {
    int fd, room_fd, client_id, port;
    char buff[1024] = {0};
    char buffer[1024] = {0};
    char responds[3][1024] = {0, 0, 0};
    char question[1024] = {0};
    struct sockaddr_in bc_address;
    char major;

    int read_port_flag = 0, read_id_flag = 0;

    fd = connectServer(8080);
    printf("Please enter your major!\n");
    read(0, buff, 1024);
    send(fd, buff, strlen(buff), 0);
    major = buff[0];
    memset(buff, 0, 1024);

    if (recv(fd , buff, 1024, 0) > 0){
        printf("receiving port!\n");
        port = atoi(buff);
        printf("port%d\n", port);
    }
    memset(buff, 0, 1024);
    if (recv(fd , buff, 1024, 0) > 0){
        printf("receiving client id!\n");
        client_id = atoi(buff);
        printf("client_id : %d\n", client_id);
    }
    memset(buff, 0, 1024);

    int broadcast = 1, opt = 1;

    room_fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(room_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(room_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");

    bind(room_fd, (struct sockaddr *)&bc_address, sizeof(bc_address));

    if (client_id == 1){
        printf("enter the first question!\n");
        read(0, buffer, 1024);
        int a = sendto(room_fd, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
        printf("a : %d\n", a);
        
    }
    int responds_cnt = 0;
    while (1) {
            
        memset(buffer, 0, 1024);

        int client_index, QA_index, questioner_index = -1, client_number, questioner;
        char client_number_str[3];
        char questioner_str[3];
        char message[1000];
        char QA;

        memset(buffer, 0, 1024);
        printf("Client %d\n", client_id);
        
        read(0, buffer, 1024);
        printf("reading from room!\n");
        if (strcmp(buffer, "pass\n") == 0){
            printf("p\n");
            responds_cnt++;
        }
        if (strcmp(buffer, "finish\n") == 0){
            close(room_fd);
            break;
        }

        for(int i = 0; i < strlen(buffer); i++){
            if(buffer[i] == '#')
                client_index = i;
            if(buffer[i] == '?')
                questioner_index = i;
            if(buffer[i] == ':'){
                QA_index = i;
                break;
            }
        }

        QA = buffer[QA_index - 1];
        client_number = (buffer[client_index-1] - '0');
        if(QA == 'A')
            questioner = (buffer[questioner_index-1] - '0');

        for(int i = QA_index+1; i < strlen(buffer); i++){
            message[i-QA_index] = buffer[i];
        }

        if(QA == 'Q' && client_number == client_id){
            strcpy(question, buffer);
        }

        if(QA == 'Q' && client_number != client_id){
            // pre_QA = QA;
            memset(buffer, 0, 1024);
            printf("enter the answer!\n");
            signal(SIGALRM, alarm_handler);
            siginterrupt(SIGALRM, 1);
            alarm(60);
            
            int is_read = read(0 , buffer, 1024);
            alarm(0);
            int a = sendto(room_fd, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
            if(is_read == -1){
                int a = sendto(room_fd, "pass\n", strlen("pass\n"), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
                printf("pass\n");
                printf("a : %d\n", a); 
            }
            memset(buffer, 0, 1024); 
        }

        if(QA == 'A' && client_number != client_id && questioner == client_id){
            printf("get the answer!\n");
            strcpy(responds[client_number-1], buffer);
            responds_cnt++;
        }

        if(QA == 'B'){
            continue;
        }

        if(responds_cnt >= 2){
            printf("Select the best answer\n");
            memset(buffer, 0, 1024);
            read(0, buffer, 1024);
            int other_clients[2];
            if(client_id == 1){
                other_clients[0] = 2;
                other_clients[1] = 3;
            }
            if(client_id == 2){
                other_clients[0] = 1;
                other_clients[1] = 3;
            }
            if(client_id == 3){
                other_clients[0] = 1;
                other_clients[1] = 2;
            }

            int best_ans = buffer[0] - '0';
            responds[best_ans-1][strlen(responds[best_ans-1])-1] = '*';
            
            printf("the selected respond : %d\n", best_ans);
            printf("ans : %s\n", responds[best_ans-1]);
            printf("Sending the best response to the clients\n");
            int a = sendto(room_fd, responds[best_ans-1], strlen(responds[best_ans-1]), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
            printf("a : %d\n", a);
            //send the best response to clients

            printf("Sending to server\n");
            char* Qs_and_As;
            Qs_and_As = malloc(strlen(question) + strlen(responds[other_clients[0]-1]) + strlen(responds[other_clients[1]-1]) + 15);
            char field[10];
            strcpy(field, "field:");
            field[6] = major;
            strcpy(Qs_and_As, field);
            strcat(Qs_and_As, "\n");
            strcat(Qs_and_As, question);
            strcat(Qs_and_As, responds[other_clients[0]-1]);
            strcat(Qs_and_As, "\n");
            strcat(Qs_and_As, responds[other_clients[1]-1]);
            send(fd, Qs_and_As, strlen(Qs_and_As), 0);
            memset(responds[other_clients[0]-1], 0, 1024);
            memset(responds[other_clients[1]-1], 0, 1024);
            responds_cnt = 0;
            
        }
        memset(buffer, 0, 1024);
    }
    return 0;
}


