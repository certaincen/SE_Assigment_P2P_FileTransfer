#include "client.h"
using namespace std;
map<string, struct client_inform> client_list;
map<string, struct client_inform>::iterator iter;
int udp_request_server(int sockfd, const struct sockaddr_in *address, int len);
int send_online_msg(int sockfd, const struct sockaddr_in *address, int len);
int send_offline_msg(int sockfd, const struct sockaddr_in *address, int len);
int send_query_msg(int sockfd, const struct sockaddr_in *address, int len);
int recv_from_serv(int sockfd, const struct sockaddr_in *address, int len);
int send_file_msg(int sockfd, const struct sockaddr_in *address, int len);
int send_busy_msg(int sockfd, const struct sockaddr_in *address, int len);
int get_firend_fd();
int recv_firend_state(int statefd);

int (*msg_op_array[5])(int sockfd, const struct sockaddr_in *address, int len) = {
    send_online_msg, send_offline_msg, send_query_msg, recv_from_serv, send_file_msg
};

void show_client_list();
void* recv_friend_inf(void* s)
{
    int sockfd;
    struct sockaddr_in address, peer;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&address, sizeof(sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = (STATE_PORT);
    address.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("bind error \n");
        exit(1);
    }
    unsigned int arr_len = sizeof(peer);
    int recv_len;
    char msg[MAX_MSG_SIZE];
    printf("wait for message\n");
    while(1)
    {
        recv_len = recvfrom(sockfd, msg, MAX_MSG_SIZE, 0, (struct sockaddr *)&peer, (socklen_t*)&arr_len);
        if (recv_len < 0)
        {
            //printf("recive package error\n");
            continue;
        }
        int *state = (int *)msg;
        int *str_len = (int *)(msg + sizeof(int));
        char ip_inf[200];
        memcpy(ip_inf, msg+2*sizeof(int), *str_len);
        ip_inf[*str_len] = '\0';
        int *client_state = (int *)(msg + 2*sizeof(int) + *str_len);
        const char* s_ch;
        if (*client_state == 1)
            s_ch = "ONLINE";
        else if (*client_state == -1)
            s_ch = "OFFLINE";
        else if (*client_state == 0)
            s_ch = "BUSY";
        else
            s_ch = "ERROR";
        if (*state == SUCCESS)
        {
            printf("%s now state is %s\n", ip_inf, s_ch);
        }
    }
};

int main()
{
    int sockfd;
    struct sockaddr_in address;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&address, sizeof(sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = (SERVER_PORT);
    //address.sin_port = htons(SERVER_PORT);
    //address.sin_addr.s_addr = htonl(inet_addr("127.0.0.1"));
    address.sin_addr.s_addr = (inet_addr("192.168.0.17"));
    //printf("%d\n", htonl(inet_addr("127.0.0.1")));
    pthread_t p_threads[MAX_THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(p_threads, NULL, recv_friend_inf, NULL);
    udp_request_server(sockfd, &address, sizeof(struct sockaddr_in));
    close(sockfd);

    return 0;
}
int udp_request_server(int sockfd, const struct sockaddr_in *address, int len)
{
    char msg[MAX_MSG_SIZE];
    int recv_len;
    while(1)
    {
        printf("choose server \n");
        printf("input 0  :  online\n");
        printf("input 1  :  offline\n");
        printf("input 2  :  query client status\n");
        printf("input 3  :  recv information from server\n");
        printf("input 4  :  send files\n");
        printf("input 5  :  quit system\n");
        int option;
        scanf("%d", &option);
        if (option < 0 || option > 5)
        {
            printf("option error\n");
            continue;
        }
        if (option == 5)
        {
            return 1;
        }
     //   if (FD_ISSET(sockfd, &rset))
        {
            msg_op_array[option](sockfd, address, len);
        }
        //  printf("*****\n");
       // msg_op_array[option](sockfd, address, len);
    }
}

int send_online_msg(int sockfd, const struct sockaddr_in *address, int len)
{
    printf("get in online \n");
    struct sockaddr_in peer;
    int op = R_ONLINE;
    char msg[400];
    memcpy(msg, &op, sizeof(int));
    if (sendto(sockfd, msg, sizeof(int), 0, (struct sockaddr*)address, sizeof(struct sockaddr)) == -1)
    {
        printf("send to server error, try argin\n");
        return -1;
    }
    else {
        printf("send success\n");
        int addr_len = sizeof(struct sockaddr);
        int recv_len = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr*)&peer, (socklen_t *)&addr_len);
        int* status = (int *)msg;
        //printf("***%d\n", *status);
        if (*status == SUCCESS)
        {
            printf("online success\n");
            return 0;
        }
        else {
            printf("recive from server failed\n");
            return -1;
        }
    }
    return 0;
}
int send_offline_msg(int sockfd, const struct sockaddr_in *address, int len)
{
    int op = R_OFFLINE;
    char msg[400];
    memcpy(msg, &op, sizeof(int));
    if (sendto(sockfd, msg, sizeof(int), 0, (struct sockaddr*)address, sizeof(struct sockaddr)) == -1)
    {
        printf("send to server error, try argin\n");
        return -1;
    }
    else {
        int addr_len = sizeof(struct sockaddr);
        int recv_len = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr*)address, (socklen_t*)&addr_len);
        int* status = (int *)msg;
        if (*status == SUCCESS)
        {
            printf("offline success\n");
            return 0;

        }
        else {
            printf("recive from server failed\n");
            return -1;

        }
    }
    return 0;
}

int send_query_msg(int sockfd, const struct sockaddr_in *address, int len)
{
    int op = R_QUERY;
    char msg[400];
    memcpy(msg, &op, sizeof(int));
    struct sockaddr_in peer;
    if (sendto(sockfd, msg, sizeof(int), 0, (struct sockaddr*)address, sizeof(struct sockaddr)) == -1)
    {
        printf("send to server error, try argin\n");
        return -1;
    }
    else {
        int addr_len = sizeof(struct sockaddr);
        int recv_len = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr*)&peer, (socklen_t*)&addr_len);
        int* status = (int *)msg;
        if (*status != SUCCESS)
        {
            printf("recive from server failed\n");
            return -1;
        }
        else {
            client_list.erase(client_list.begin(), client_list.end());
            int *client_num = (int *)(msg+sizeof(int));
            int offset = 2 * sizeof(int);
            struct client_inform *tmp_client;
            int* client_name_len;
            char client_name[400];
            //char *client_name;
#if DEBUG
            printf("msg len is %d\n", recv_len);
#endif
            for (int i=0; i<*client_num; i++)
            {
                client_name_len = (int *)(msg + offset);
                offset += sizeof(int);
                // client_name = (char*)(msg+offset);
                memcpy(client_name, msg+offset, (size_t)*client_name_len);
                client_name[*client_name_len] = '\0';
#if DEBUG
                printf("this client name len is %d\n", *client_name_len);
                printf("this client name is %s", client_name);
                printf("length is %d\n", strlen(client_name));
#endif
                offset += *client_name_len;
                tmp_client = (struct client_inform*)(msg+offset);
                offset += sizeof(*tmp_client);
                string client(client_name);
                client_list.insert(pair<string, struct client_inform>(client, *tmp_client));
            }
        }

    }
    show_client_list();
    return 0;

}
void show_client_list()
{
    printf("client list \n");
    printf("client name     |port            | status\n");
    char status[200];
    for (iter = client_list.begin(); iter != client_list.end(); iter++)
    {
        if (iter->second.status == 1)
        {
            strcpy(status, "ONLINE");
        }
        else if (iter->second.status == -1)
        {
            strcpy(status, "OFFLINE");
        }
        else if (iter->second.status == 0)
        {
            strcpy(status, "BUSY");
        }
        else {
            strcpy(status, "ERROR");
        }
        printf("%-16s|%-16d  |%-16s\n", iter->first.c_str(),iter->second.port, status);
    }
}

bool is_valid(string client_name)
{
    iter = client_list.find(client_name);
    if (iter != client_list.end())
    {
        if (iter->second.status == ONLINE)
            return 1;
    }
    return 0;
}

int send_file_msg(int sockfd, const struct sockaddr_in* address, int len)
{
    show_client_list();
    string client_name;
    do
    {
        printf("choose a online client to transfer files\n");
        cin>>client_name;
    }while(!is_valid(client_name));
    char msg[400] ={0};
    int msg_len = strlen(client_name.c_str());
    int op = R_CONN;
    memcpy(msg, &op, sizeof(int));
    memcpy(msg+sizeof(int), &msg_len, sizeof(int));
    memcpy(msg+sizeof(int)*2, client_name.c_str(), msg_len);
    if(sendto(sockfd, msg, msg_len+2*sizeof(int), 0, (struct sockaddr*)address, len) == -1)
    {
        printf("send to sever failed\n");
        return -1;
    }
    int tcp_sock = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in tcp_addr;
    tcp_addr.sin_family = PF_INET;
    tcp_addr.sin_port = (FILE_PORT);
    tcp_addr.sin_addr.s_addr = (INADDR_ANY);
    //tcp_addr.sin_port = htons(FILE_PORT);
    //tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(tcp_sock, (struct sockaddr*)&tcp_addr, sizeof(tcp_addr)) == -1)
    {
        printf("bind error\n");
        return -1;
    }
    listen(tcp_sock, 4);

    int  sclient;
    struct sockaddr_in remote_addr;
    int remote_addr_len;
    while (1)
    {
        remote_addr_len = sizeof(remote_addr);
        sclient = accept(tcp_sock, (struct sockaddr*)&remote_addr, (socklen_t*)&remote_addr_len);
        if (strcmp(inet_ntoa(remote_addr.sin_addr), client_name.c_str()) != 0)
        {
            printf("not correct client transfer failed\n");
            return -1;
        }
        char infile_name[200];
        printf("input transfer file name\n");
        scanf("%s", infile_name);
        FILE* fin = fopen(infile_name, "r");
        char msg[MAX_MSG_SIZE];
        int msg_len;
        send(sclient, infile_name, strlen(infile_name), 0);
        msg_len = fread(msg, sizeof(char), MAX_MSG_SIZE, fin);
        printf("one time length is %d\n", msg_len);
        while (msg_len>0)
        {
            if (send(sclient, msg, msg_len, 0) == -1)
            {
                printf("send error\n");
            }
            msg_len = fread(msg, sizeof(char), MAX_MSG_SIZE, fin);
            printf("one time length is %d\n", msg_len);
        }
        fclose(fin);
        close(sclient);
        close(tcp_sock);
        return 1;
    }
}
int make_connect(string client_name)
{
    int tcp_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = FILE_PORT;
    addr.sin_addr.s_addr = (inet_addr(client_name.c_str()));
    if (connect(tcp_sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1)
    {
        printf("connect error\n");
        return -1;
    }
    char msg[MAX_MSG_SIZE];
    int msg_len = recv(tcp_sockfd, msg, MAX_MSG_SIZE, 0);
    FILE* fout = fopen(msg, "w");
    msg_len = recv(tcp_sockfd, msg, MAX_MSG_SIZE, 0);
    while (msg_len>0)
    {
#if DEBUG
        printf("recv one time is %d\n", msg_len);
#endif
        fwrite(msg, sizeof(char), msg_len, fout);
        msg_len = recv(tcp_sockfd, msg, MAX_MSG_SIZE, 0);
    }
    fclose(fout);
    close(tcp_sockfd);
    return 0;
}
int send_busy_msg(int sockfd, const struct sockaddr_in *address, int len)
{
    int op = R_BUSY;
    char msg[400];
    memcpy(msg, &op, sizeof(int));
    struct sockaddr_in peer;
    if (sendto(sockfd, msg, sizeof(int), 0, (struct sockaddr*)address, sizeof(struct sockaddr)) == -1)
    {
        printf("send to server error, try argin\n");
        return -1;
    }
    else {
        int addr_len = sizeof(struct sockaddr);
        int recv_len = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr*)&peer, (socklen_t *)&addr_len);
        int* status = (int *)msg;
        if (*status == FAILED)
        {
            printf("recive from server failed\n");
            return -1;
        }
        else {
            printf("online success\n");
            return 0;
        }
    }
    return 0;

}
int recv_from_serv(int sockfd, const struct sockaddr_in *address, int len)
{
    struct sockaddr_in peer;
    int addr_len = sizeof(peer);
    char msg[400];
    int msg_len = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr*)&peer, (socklen_t *)&addr_len);
    if (msg_len == 0)
        return  -1;
    int *str_len = (int *)msg;
    char* client_name = (char *)(msg + sizeof(int));
    int length = strlen(client_name);
#if DEBUG
    printf("length1 = %d\n", *str_len);
    printf("length2 = %d\n", length);
#endif
    if (length != *str_len)
    {
        *(client_name + *str_len) = '\0';
    }
    make_connect(string(client_name));
    return 0;
}
int get_firend_fd()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = STATE_PORT;
    address.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        printf("bind error\n");
    }
    return sockfd;
}
int recv_firend_state(int statefd)
{
    struct sockaddr_in peer;
    unsigned int arr_len = sizeof(peer);
    int recv_len;
    char msg[MAX_MSG_SIZE];
    printf("wait for message\n");
    recv_len = recvfrom(statefd, msg, MAX_MSG_SIZE, 0, (struct sockaddr *)&peer, (socklen_t*)&arr_len);
    if (recv_len < 0)
    {
        printf("recive package error\n");
        return 1;
    }
    int *state = (int *)msg;
    int *str_len = (int *)(msg + sizeof(int));
    char ip_inf[200];
    memcpy(ip_inf, msg+2*sizeof(int), *str_len);
    ip_inf[*str_len] = '\0';
    int *client_state = (int *)(msg + 2*sizeof(int) + *str_len);
    const char* s_ch;
    if (*client_state == 1)
        s_ch = "ONLINE";
    else if (*client_state == -1)
        s_ch = "OFFLINE";
    else if (*client_state == 0)
        s_ch = "BUSY";
    else
        s_ch = "ERROR";
    if (*state == SUCCESS)
    {
        printf("%s now state is %s\n", ip_inf, s_ch);
    }
    return 1;
}
