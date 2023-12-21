#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <mcp3004.h>
#include <wiringPiSPI.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>


// SPI
#define BASE 100
#define SPI_CHAN 0

// 사용할 아날로그 채널
#define ADC_CHANNEL 1

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define POUT 17
#define BUFFER_MAX 3
#define DIRECTION_MAX 256
#define VALUE_MAX 256

//PWM
#define PWM 0
#define PWM_1 1

/* pi3 전역변수 0 -> 열기 1 -> 닫기 */
int Motor_Front = 0; // 입구 앞
int Motor_Back = 0;  // 입구 뒤
int str_len; 
int value;

int sock;
int Water_Level = 0; // 수질 등급
int Speaker;         // 값 1 : 소리 X, 값 0 : 소리 O

void error_handling(char *message);
void * Water(void * arg);
void * Speaker_Motor(void * arg);


void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

/* GPIO functions */
static int GPIOExport(int pin) {
#define BUFFER_MAX 3
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIODirection(int pin, int dir) {
  static const char s_directions_str[] = "in\0out";

  char path[DIRECTION_MAX];
  int fd;

  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio direction for writing!\n");
    return (-1);
  }

  if (-1 ==
      write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
    fprintf(stderr, "Failed to set direction!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

static int GPIOWrite(int pin, int value) {
  static const char s_values_str[] = "01";

  char path[VALUE_MAX];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for writing!\n");
    return (-1);
  }

  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
    fprintf(stderr, "Failed to write value!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

static int GPIOUnexport(int pin) {
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open unexport for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

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

    // TODO: Enter the enable path.
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

    // TODO: Enter the period path.
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

    // TODO: Enter the duty_cycle path.
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


/* Thread functions */
void * Water(void * arg){
    //WiringPi , SPI Setup
     if (wiringPiSetup() == -1) {
        printf("WiringPi initialization failed. Exiting...\n");
    }

    printf("wiringPiSPISetup return =%d\n", wiringPiSPISetup(0,500000));

    mcp3004Setup(BASE, SPI_CHAN);

    int adcChannel = BASE + ADC_CHANNEL;

    while(1) {
        // 아날로그 값 읽기
        int analogValue = analogRead(adcChannel);

        // 센서에서 읽은 아날로그 값을 tdsvalue값으로 변환(보정 공식 사용)
       float averageVoltage = analogValue * (float)3.3 / 1024.0;
       float compensationCoefficient = 1.0 + 0.02*(25.0 - 25.0);

       float compensationVoltage = averageVoltage/compensationCoefficient;
       float tdsValue = (133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;

        // 결과 출력
        printf("Analog Value: %d, tdsValue: %.2f ppm\n", analogValue, tdsValue);

        // Water_Level 측정
        if(tdsValue > 0 && tdsValue <50)
        {
            Water_Level = 1;
        }
        else if(tdsValue >= 50 && tdsValue <200)
        {
            Water_Level = 2;
        }
        else if(tdsValue >=200 && tdsValue <300)
        {
            Water_Level = 3;
        }
        else if(tdsValue >=300 && tdsValue <500)
        {
            Water_Level = 4;
        }
        else if(tdsValue >= 500)
        {
            Water_Level = 5;
        }

        // server에 Water_Level 값 전송
        send(sock, &Water_Level, sizeof(Water_Level), 0);
        

        // 측정 주기 (3초)
        delay(3000);
    }

}


void * Speaker_Motor(void * arg)
{
    // GPIO Setting
    GPIOExport(POUT);
    GPIODirection(POUT, OUT);
    GPIOWrite(POUT, 1);

    while(1){
    // 서버에서 값 read
    read(sock, &value, sizeof(value));
    if(value == 0) // 읽은 값이 0일 경우 부저 작동
    {
        GPIOWrite(POUT, 0);
        printf("Speaker Value: %d", Speaker);

    sleep(1);
    GPIOWrite(POUT, 1);

    }

    else if(value == 3) // 읽은 값이 3일 경우 모터 작동
    {
        PWMWriteDutyCycle(PWM, 2500000);
        sleep(1);
        PWMWriteDutyCycle(PWM, 1500000);
    }

    }
}

/* Main function */
int main(int argc, char *argv[]) {

    int status;                        
    struct sockaddr_in serv_addr;  
    pthread_t users1, users2;    

     if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }   

    /* SOCKET FUNCTIONS START*/     

    sock = socket(PF_INET, SOCK_STREAM, 0);         
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Socket 생성 및 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        printf("CONNECTION ESTABLISHED");
    }

    // 스레드 생성
    pthread_create(&users1, NULL, Water, (void*)&sock);
    pthread_create(&users2, NULL, Speaker_Motor, (void*)&sock); 

    /* PWM SETTING START*/
    PWMExport(PWM);
    

    PWMWritePeriod(PWM, 20000000);

    PWMWriteDutyCycle(PWM, 500000);
    
    PWMEnable(PWM);


	pthread_join(users1, (void**)&status);
    pthread_join(users2, (void**)&status);

    close(sock);

    return 0;
}