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


#define BASE 100
#define SPI_CHAN 0

// 사용할 아날로그 채널
#define ADC_CHANNEL 1
  // 조도 센서의 핀 번호

#define LED_PIN 17           // LED의 핀 번호
#define USER_ACT -1

int main() {

    if(wiringPiSetup()==-1) {
        printf("wrong\n");
    }

    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi initialization failed. Exiting...\n");
    }

    printf("wiringPiSPISetup return =%d\n", wiringPiSPISetup(0,500000));

    mcp3004Setup(BASE, SPI_CHAN);

    int adcChannel = BASE + ADC_CHANNEL;

    pinMode(LED_PIN, OUTPUT);

    while (1) {
        // 조도 센서의 값을 읽어옴
        int analogValue = analogRead(adcChannel);
        printf("%d\n",analogValue);

        if (analogValue<50&&USER_ACT==-1) {
            // 조도 센서 값이 50 이하로 내려가면 LED를 켬
            digitalWrite(LED_PIN, HIGH);
        } else if(USER_ACT==-1) {
            // 조도 센서 값이 50을 넘으면 LED를 끔
            digitalWrite(LED_PIN, LOW);
        }

        delay(1000);  // 1초 대기
    }

    return 0;
}
