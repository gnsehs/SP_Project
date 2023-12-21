#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_MAX 3
#define DIRECTION_MAX 256
#define VALUE_MAX 256

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1
#define PWM 0
#define PWM_1 1

/* pi3 전역변수 0 -> 열기 1 -> 닫기 */
int Motor_Front = -1; // 입구 앞
int Motor_Back = -1;  // 입구 뒤

/* PWM FUNCTIONS */
static int PWMExport(int pwmnum)
{
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    int fd, byte;

    // TODO: Enter the export path.
    fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open export for export!\n");
        return (-1);
    }

    byte = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
    write(fd, buffer, byte);
    close(fd);

    sleep(1);

    return (0);
}

static int PWMEnable(int pwmnum)
{
    static const char s_enable_str[] = "1";

    char path[DIRECTION_MAX];
    int fd;


    snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm0/enable", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in enable!\n");
        return -1;
    }

    write(fd, s_enable_str, strlen(s_enable_str));
    close(fd);

    return (0);
}

static int PWMWritePeriod(int pwmnum, int value)
{
    char s_value_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;


    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm0/period", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in period!\n");
        return (-1);
    }
    byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

    if (-1 == write(fd, s_value_str, byte))
    {
        fprintf(stderr, "Failed to write value in period!\n");
        close(fd);
        return -1;
    }
    close(fd);

    return (0);
}

static int PWMWriteDutyCycle(int pwmnum, int value)
{
    char s_value_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;


    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm0/duty_cycle", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in duty cycle!\n");
        return (-1);
    }
    byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

    if (-1 == write(fd, s_value_str, byte))
    {
        fprintf(stderr, "Failed to write value in duty cycle!\n");
        close(fd);
        return -1;
    }
    close(fd);

    return (0);
}

static int PWMEnable_2(int pwmnum) // PWM MULTI CHANNEL
{
    static const char s_enable_str[] = "1";

    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm1/enable", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in enable!\n");
        return -1;
    }

    write(fd, s_enable_str, strlen(s_enable_str));
    close(fd);

    return (0);
}

static int PWMWritePeriod_2(int pwmnum, int value)
{
    char s_value_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;


    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm1/period", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in period!\n");
        return (-1);
    }
    byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

    if (-1 == write(fd, s_value_str, byte))
    {
        fprintf(stderr, "Failed to write value in period!\n");
        close(fd);
        return -1;
    }
    close(fd);

    return (0);
}

static int PWMWriteDutyCycle_2(int pwmnum, int value)
{
    char s_value_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;

    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm1/duty_cycle", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in duty cycle!\n");
        return (-1);
    }
    byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

    if (-1 == write(fd, s_value_str, byte))
    {
        fprintf(stderr, "Failed to write value in duty cycle!\n");
        close(fd);
        return -1;
    }
    close(fd);

    return (0);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void *teample(void *arg)
{
    printf("hello World - Client \n");
}

int main(int argc, char *argv[])
{
    int status;
    int sock;
    struct sockaddr_in serv_addr;
    int str_len;
    pthread_t users;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // 서보모터 -90도 : 1주기(20ms) 중 0.5ms

    // 서보모터    0도 : 1주기(20ms) 중 1.5ms

    // 서보모터 +90도 : 1주기(20ms) 중 2.5ms

    /* PWM SETTING START*/
    printf("PWM SETTING--\n");
    PWMExport(PWM);
    PWMExport(PWM_1);

    PWMWritePeriod(PWM, 20000000); // 1주기 (20ms)
    PWMWritePeriod_2(PWM_1, 20000000);

    /*******************************
     * DUTY CLCLE.
     * 2500000 닫혀있음
     * 2000000 열려있음
     * 약 45도 각도로 열고 닫기 
    ********************************/

    PWMWriteDutyCycle(PWM, 2000000);      // 입구 : 초기 닫혀. 
    PWMWriteDutyCycle_2(PWM_1, 2000000); // 출구 초기 : 닫혀있음.

    PWMEnable(PWM);
    PWMEnable_2(PWM_1);

    sleep(1); // open 상태 설정 동작을 위한 1초 대기

    /* SOCKET FUNCTIONS START*/
    printf("SOCKET SETTING--\n");
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    printf("Connection established\n");

    pthread_create(&users, NULL, teample, (void *)&sock);
    pthread_join(users, (void **)&status);

    /* READ & WIRTE START */
    printf("R & W SETTING--\n");
    while (1)
    {
        printf("TEST RECV\n");
        str_len = recv(sock, &Motor_Back, sizeof(Motor_Back), 0);
        if (str_len == -1)
            error_handling("recv() error");
        str_len = recv(sock, &Motor_Front, sizeof(Motor_Front), 0);
        printf("BACK : %d, FRONT : %d\n", Motor_Back, Motor_Front);

        if (str_len == -1)
            error_handling("recv() error");
/* FRONT MOTOR */
        if (Motor_Front == 0) // open 
        {
            PWMWriteDutyCycle(PWM, 2000000);
        }
        else if (Motor_Front == 1) // close
        {
            PWMWriteDutyCycle(PWM, 2500000);
        }
        else
        {
            printf("ERROR MOROR FRONT\n");
        }
/* BACK MOTOR */
        if (Motor_Back == 0)
        {
            PWMWriteDutyCycle_2(PWM_1, 2000000); // open
        }
        else if (Motor_Back == 1)
        {
            PWMWriteDutyCycle_2(PWM_1, 2500000); // close
        }
        else
        {
            printf("ERROR MOROR BACK\n");
        }


    }
    close(sock);

    return (0);
}
