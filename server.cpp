#include "server.h"

using namespace std;
map<string, struct client_inform> client_list;
map<string, struct client_inform>::iterator iter;
int online_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg);
int offline_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg);
int busy_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg);
int query_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg);
int req_connect_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg);
int (*op_array[])(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg) = {
online_op, offline_op, busy_op, query_op, req_connect_op
};
int send_status_to_client(string client_name, int status, int sockfd);



void udp_respon(int sockfd, struct sockaddr_in my_recv_addr)
{
    unsigned int arr_len = sizeof(my_recv_addr);
    int recv_len;
    char msg[MAX_MSG_SIZE];
    printf("wait for message\n");
    while(1)
    {
        recv_len = recvfrom(sockfd, msg, MAX_MSG_SIZE, 0, (struct sockaddr *)&my_recv_addr, (socklen_t*)&arr_len);
        if (recv_len < 0)
        {
            printf("recive package error\n");
            continue;
        }
        printf("recv op\n");
        //msg[recv_len] = '\0';
        int* op_index = (int *)msg;
        printf("op is %d", *op_index);
        if (*op_index < 0 || *op_index > 5)
        {
            printf("operator option error\n");
            continue;
        }
        if (!op_array[*op_index](&my_recv_addr, sockfd, msg))
        {
            printf("operator failed\n");
            continue;
        }
    }
};

int main()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creat fail\n");
        exit(1);
    }
    struct sockaddr_in address, client_addr;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //address.sin_addr.s_addr = htonl(inet_addr("127.0.0.1"));
    //address.sin_port = htons(SERVER_PORT);
    address.sin_port = (SERVER_PORT);
    printf("%d\n", address.sin_addr.s_addr);
    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("bind error \n");
        exit(1);
    }
    udp_respon(sockfd, client_addr);
    close(sockfd);
    return 0;
}

int online_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg)
{
    struct client_inform online_inform;
    string client_addr(inet_ntoa(my_recv_addr -> sin_addr));
    iter = client_list.find(client_addr);
    online_inform.status = ONLINE;
    online_inform.port = my_recv_addr -> sin_port;
    if (iter == client_list.end())
    {
        client_list.insert(pair<string, struct client_inform>(client_addr, online_inform));
    }
    else {
        iter -> second = online_inform;
    }
    char response[400];
    int status = SUCCESS;
    memcpy(response, &status, sizeof(int));
    if (sendto(sockfd, response, sizeof(int), 0, (struct sockaddr*)my_recv_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("response error\n");
        return -1;
    }
    else
    {
        send_status_to_client(client_addr, ONLINE, sockfd);
    }
    return 1;
}

int offline_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg)
{
    struct client_inform offline_inform;
    string client_addr(inet_ntoa(my_recv_addr -> sin_addr));
    iter = client_list.find(client_addr);
    offline_inform.status = OFFLINE;
    offline_inform.port = my_recv_addr -> sin_port;
    if (iter != client_list.end())
    {
        iter->second = offline_inform;
    }
    char response[400];
    int status = SUCCESS;
    memcpy(response, &status, sizeof(int));
    if (sendto(sockfd, response, sizeof(int), 0, (struct sockaddr*)my_recv_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("response error\n");
        return -1;
    }
    else
    {
        send_status_to_client(client_addr, OFFLINE, sockfd);
    }
    return 1;
}

int query_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg)
{
    string client_addr(inet_ntoa(my_recv_addr -> sin_addr));
    int msg_len = 0;
    int client_num = 0;
    int client_name_length = 0;
    char client_ip_array[400];
    for (iter = client_list.begin(); iter != client_list.end(); iter++)
    {
        const char* tmp_client = iter -> first.c_str();
        struct client_inform tmp_client_inform = iter -> second;
        client_name_length = strlen(tmp_client);
        memcpy(client_ip_array+msg_len, &client_name_length, sizeof(int));
        msg_len += sizeof(int);
        memcpy(client_ip_array+msg_len, tmp_client, client_name_length);
        msg_len += client_name_length;
        memcpy(client_ip_array+msg_len, &tmp_client_inform, sizeof(tmp_client_inform));
        msg_len += sizeof(tmp_client_inform);
        client_num++;
    }
    char response[400];
    int status = SUCCESS;
    memcpy(response, &status, sizeof(int));
    memcpy(response+sizeof(int), &client_num, sizeof(int));
    memcpy(response+2*sizeof(int), &client_ip_array, msg_len);
    msg_len += 2*sizeof(int);
    if (sendto(sockfd, response, msg_len, 0, (struct sockaddr*)my_recv_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("response error\n");
        return -1;
    }
    return 1;
}
int busy_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg)
{
    struct client_inform busy_inform;
    string client_addr(inet_ntoa(my_recv_addr -> sin_addr));
    iter = client_list.find(client_addr);
    busy_inform.port = my_recv_addr -> sin_port;
    busy_inform.status = BUSY;
    if (iter != client_list.end())
    {
        iter -> second = busy_inform;
    }
    char response[400];
    int status = SUCCESS;
    memcpy(response, &status, sizeof(int));
    if (sendto(sockfd, response, sizeof(int), 0, (struct sockaddr*)my_recv_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("response error\n");
        return -1;
    }
    return 1;
}
int send_status_to_client(string client_name, int status, int sockfd)
{
    struct sockaddr_in respon_addr;
#if DEBUG
    printf("send to client list\n");
#endif
    for (iter = client_list.begin(); iter != client_list.end(); iter++)
    {
        if (strcmp(client_name.c_str(), iter->first.c_str()) != 0)
        {
            struct client_inform  tmp_client = iter->second;
            respon_addr.sin_family = AF_INET;
            respon_addr.sin_port = (STATE_PORT);
            //respon_addr.sin_port = htons(tmp_client.port);
            //respon_addr.sin_addr.s_addr = htonl(inet_addr(iter->first.c_str()));
            respon_addr.sin_addr.s_addr = (inet_addr(iter->first.c_str()));
            char msg[400];
            const char* ip_inform = iter->first.c_str();
            int len = strlen(ip_inform);
            int s = SUCCESS;
            memcpy(msg, &s, sizeof(int));
            memcpy(msg+sizeof(int), &len, sizeof(int));
            memcpy(msg+2*sizeof(int), ip_inform, len);
            memcpy(msg+2*sizeof(int)+len, &status, sizeof(int));
            if (sendto(sockfd, msg, len+3*sizeof(int), 0, (struct sockaddr *)&respon_addr, sizeof(respon_addr)) == -1)
            {
                printf("send back error ip %s\n", iter->first.c_str());
                continue;
            }
        }
    }
    return 1;
}
int req_connect_op(struct sockaddr_in *my_recv_addr, int sockfd, const char* msg)
{
    struct sockaddr_in respon_addr;
    int* msg_len = (int *)(msg + sizeof(int));
    char client_name[400];
    memcpy(client_name, msg+2*sizeof(int), (size_t)*msg_len);
    struct client_inform client_in;
    iter = client_list.find(string(client_name));
    client_in = iter -> second;
    respon_addr.sin_family = AF_INET;
    respon_addr.sin_port = (client_in.port);
    //respon_addr.sin_port = htons(client_in.port);
    //respon_addr.sin_addr.s_addr = htonl(inet_addr(client_name));
    respon_addr.sin_addr.s_addr = (inet_addr(client_name));
    char send_msg[400];
    char *send_addr = inet_ntoa(my_recv_addr -> sin_addr);
    int str_len = strlen(send_addr);
    memcpy(send_msg, &str_len, sizeof(int));
    memcpy(send_msg+sizeof(int), send_addr, str_len);
#if DEBUG
    printf("tcp host name is %s\n", client_name);
    printf("file host name is %s\n", send_addr);
#endif
    if (sendto(sockfd, send_msg, sizeof(int)+str_len, 0, (struct sockaddr*)&respon_addr, sizeof(respon_addr)) == -1)
    {
        printf("send to client error\n");
        return -1;
    }
    else
    {
        printf("send to client success\n");
    }
    return 1;

}
