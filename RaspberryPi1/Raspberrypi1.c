#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <jansson.h>

#define MAX_SIZE 4096

int pi_cnt = 0;                            // RaspberryPi 연결 개수
int pid;                                   // Unique PID Number
struct sockaddr_in serv_addr, clnt_addr;   // Socket Address

int serv_sock;                             // Server Sockets
int clnt_sock;                             // Client Sockets

int clnt_pids[MAX_SIZE];                   // Client's 개수

// ************** 전역변수 **************
// ** Pi_1
int Catch_Fish = 0;                         // 물고기 잡았는지 여부 
int User_Motor_button = 0;                 // User의 버튼
int User_LED_button = 0;                 // User의 버튼
int User_Speaker_button = 1;                 // User의 버튼
int User_Water_button = 0;
int User_GPS_button = 0;

int PID_BUFFER[MAX_SIZE];

// ************** 전역변수 for Android **************
char android_buffer[MAX_SIZE];

FILE *fr;
double lat; // 위도
double lon; // 경도 
char gps_number[20];

// ** Pi_2
// ..?

// ** Pi_3
int Motor_Front = 0;                        // 입구 앞
int Motor_Back = 0;                         // 입구 뒤

// ** Pi_4
int Water_Level = 0;                        // 수질 등급
int User_shoot = 0;

// ************************************
void * Pi_Flask(void * arg);
// void * Pi_One(void * arg);                  // Owns (Motor Sensor, Flask, Android)
// void * Pi_Android(void * arg);                  // Owns (Motor Sensor, Flask, Android)
// void * Pi_Two(void * arg);                  // Light, LED Sensors
void * Pi_Three(void * arg);                // Motor x 2
// void * Pi_Fourth(void * arg);               // Speaker, GPS, Water Sensors
// void * motor_function(void * arg);               // User Motor x 2
void * water_function(void * arg);               // User function x 2

void error_handling(char * msg);    

int main(int argc, char *argv[]) {
    socklen_t clnt_addr_size;               // Client address
    pthread_t t_id;                         // Thread 식별을 위한 변수
    // *** pthread_t
    pthread_t p_thread[2];
    char p1[] = "thread_1";
    char p2[] = "thread_2";

    if (argc != 2) { printf("Usage : %s <port>\n", argv[0]);  exit(1); }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);               // Server Socket
    if (serv_sock == -1) error_handling("socket() error");     
    memset(&serv_addr, 0, sizeof(serv_addr));                  // Setting for Server address
	serv_addr.sin_family=AF_INET; 
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) error_handling("bind() error");
    if (listen(serv_sock, 5) == -1) error_handling("listen() error");
    int pid_num = 0;
    int pi_android = 0;

    while (1) {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);        // Client Lock
        printf("pid_num : %d \n", pid_num);
        
        if (pid_num == 0) {
            clnt_pids[0] = clnt_sock;
            pthread_create(&t_id, NULL, Pi_Flask, (void*)&clnt_sock);
        } else if (pid_num == 1) {
            clnt_pids[1] = clnt_sock;
            // pthread_create(&t_id, NULL, Pi_Two, (void*)&clnt_sock);
        } else if (pid_num == 2) {
            clnt_pids[2] = clnt_sock;
            pthread_create(&t_id, NULL, Pi_Three, (void*)&clnt_sock);
            // pthread_create(&p_thread[0], NULL, motor_function, (void *)clnt_sock);
        } else if (pid_num == 3) {
            clnt_pids[3] = clnt_sock;
            // pthread_create(&t_id, NULL, Pi_Fourth, (void*)&clnt_sock);
            pthread_create(&p_thread[1], NULL, water_function, (void *)clnt_sock);
        } else {
            while (1) {
                User_Speaker_button = 1;
                if (pi_android != 0) {
                    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);        // Client Lock
                }
                ssize_t bytesRead = recv(clnt_sock, android_buffer, sizeof(android_buffer), 0);
                if (bytesRead > 0) {
                    android_buffer[bytesRead] = '\0';
                    if (strcmp("sound", android_buffer) == 0) {       // Sound 한번
                        User_Speaker_button = 0;
                        printf("Memory - sounds\n");
                        send(clnt_pids[3], &User_Speaker_button, sizeof(User_Speaker_button), 0);     
                        memset(&android_buffer, 0, sizeof(android_buffer));
                    } else if (strcmp("on", android_buffer) == 0) {   // LED ON
                        User_LED_button = 1;
                        printf("Memory - on\n");
                        send(clnt_pids[1], &User_LED_button, sizeof(User_LED_button), 0);
                        memset(&android_buffer, 0, sizeof(android_buffer));
                    } else if (strcmp("off", android_buffer) == 0) {  // LED OFF 
                        User_LED_button = 0;
                        printf("off \n");
                        send(clnt_pids[1], &User_LED_button, sizeof(User_LED_button), 0);
                        memset(&android_buffer, 0, sizeof(android_buffer));
                    } else if (strcmp("open", android_buffer) == 0) {   // Motor Open
                        // User_Motor_button = 1;
                        Motor_Back = 0;
                        Motor_Front = 0;
                        send(clnt_pids[2], &Motor_Back, sizeof(Motor_Back), 0);      
                        send(clnt_pids[2], &Motor_Front, sizeof(Motor_Front), 0);    

                        printf("open \n");
                        memset(&android_buffer, 0, sizeof(android_buffer));
                    } else if (strcmp("close", android_buffer) == 0) {  // Motor Off 
                        Motor_Back = 1;
                        Motor_Front = 1;
                        send(clnt_pids[2], &Motor_Back, sizeof(Motor_Back), 0);      
                        send(clnt_pids[2], &Motor_Front, sizeof(Motor_Front), 0);    
                        printf("close \n");
                        memset(&android_buffer, 0, sizeof(android_buffer));
                    } else if (strcmp("gps", android_buffer) == 0) {   // GPS 
                        User_GPS_button = 1;
                        printf("require\n");
                        fr = fopen("data.txt", "r");
                        fscanf(fr, "%lf", &lat);
                        fscanf(fr, "%lf", &lon);
                        sprintf(gps_number, "%lf,%lf\n", lat,lon);
                        send(clnt_sock, gps_number, sizeof(gps_number), 0);      
                        memset(&android_buffer, 0, sizeof(android_buffer));
                        fclose(fr);
                    } else if (strcmp("water", android_buffer) == 0) {   // 수질 등급
                        printf("water \n");
                        int waters = Water_Level;
                        char wateee[10];
                        sprintf(wateee, "%d\n", waters);
                        printf("Waters : %s \n", wateee);
                        send(clnt_sock, &wateee, sizeof(wateee), 0);     
                        memset(&android_buffer, 0, sizeof(android_buffer));
                    } else if (strcmp("shoot", android_buffer) == 0) {   // 미끼
                        User_shoot = 3;
                        printf("shoot \n");
                        send(clnt_pids[3], &User_shoot, sizeof(User_shoot), 0);          
                        memset(&android_buffer, 0, sizeof(android_buffer));
                    } else if (strcmp("fish", android_buffer) == 0) {   // 미끼
                        char fishsing[10];
                        sprintf(fishsing, "%d\n", Catch_Fish);
                        send(clnt_sock, &fishsing, sizeof(fishsing), 0);          
                        memset(&android_buffer, 0, sizeof(android_buffer));
                    }
                }
                pi_android = 1;
                close(clnt_sock);
            }
        }
        pid_num++;
    }
    return 0;
}

// void * Pi_Two(void * arg) {                // Light, LED Sensors
//     int LED = 0;

//     while (1) {
//         if ( User_LED_button == 1 ) {
//             LED = 1;
//             send(clnt_pids[1], &LED, sizeof(LED), 0);    // ON 요청
//         } else {
//             LED = 0;
//             send(clnt_pids[1], &LED, sizeof(LED), 0);    // ON 요청
//         }
//         User_LED_button = 0;
//     } 
// }

void * Pi_Three(void * arg) {              // Motor x 2
    int num = 0;
    int three_fork;
    // three_fork = fork();

    while (1) {
        // printf("Catch_Fish : %d \n", Catch_Fish);
        if (Catch_Fish == 1) {      // 물고기가 들어온 경우
            Motor_Back = 1;
            Motor_Front = 1;
            printf("ddddd ddddd ddddd \n");
            send(clnt_pids[2], &Motor_Back, sizeof(Motor_Back), 0);     
            send(clnt_pids[2], &Motor_Front, sizeof(Motor_Front), 0);   
            }
        sleep(3);
    } 
}
           

void * Pi_Flask(void * arg) {
    CURL *curl;
    CURLcode res;
    const char *image_path = "test.jpg";
    const char *url = "http://192.168.221.3:9999/upload";
    int received_value;
    char buffer[1024];

    int flask_pi;
    // flask_pi = fork();              // 2가지 분기
    while (1) {
            system("raspistill -o test.jpg");
            struct curl_httppost *formpost = NULL;
            struct curl_httppost *lastptr = NULL;
            curl_global_init(CURL_GLOBAL_ALL);
            curl = curl_easy_init();
            if (curl) {
                curl_formadd(&formpost, &lastptr,\
                            CURLFORM_COPYNAME, "image",\
                            CURLFORM_FILE, image_path,\
                            CURLFORM_END);
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
                res = curl_easy_perform(curl);
                curl_formfree(formpost);
                curl_easy_cleanup(curl);
            }              
            recv(clnt_pids[0], buffer, sizeof(buffer), 0);
            printf("REceived : %s\n", buffer);
            memset(&buffer, 0, sizeof(buffer));

            if (strcmp(buffer, "one") == 0) {
                Catch_Fish = 1;
            } else {
                Catch_Fish = 0;
            }
        }
}

void error_handling(char *message) {    
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}


void *water_function(void * arg) {
    while (1) {
        sleep(3);
        read(clnt_pids[3], &Water_Level, sizeof(Water_Level));
        printf("Water : %d\n", Water_Level);
    }
}