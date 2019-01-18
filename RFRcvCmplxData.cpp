/*
https://github.com/claudiuo/raspberrypi/blob/master/433MHz-Arduino-link/RFRcvCmplxData.cpp
  RFRcvCmplxData: based on RFSniffer: receive 4 simple values
  concatenated in an unsigned int (32 bit) value:
  - 4 bit: station code 0-15
  - 10 bit: temperature in F *10 (to get one decimal) -> need to divide by 10
  - 10 bit: humidity in % *10 (to get one decimal) -> need to divide by 10
  - 8 bit: battery voltage in mV /50 (to get some decimals) -> need to multiply by 50
  Because the data is sent repeatedly, we need to make sure we don't get duplicates.
  So we store the last value received and if we get it again in the next 30s, we
  consider it a duplicate. The temp, humidity and voltage values are sent every few minutes
  that's why we limit the check to 30s: if we receive the same value after 30s, we
  consider it a new value - this is possible if none of tem, humidity or voltage changed.
  For the motion sensor, things are different: we don't check the sensor periodically,
  the Arduino code reacts using interrupts so there is no interval to use to check
  for duplicates. But there is no need to: when the motion sensor is triggered, we get
  a message with motion=1; then the sensor is not triggered again until it resets (a few
  seconds); when it resets, we get a message with motion=0 so the value will be different.
  Then if the sensor is triggered again, we get motion=1 - a new value. And so on.
  RX:
  - Connect pin 1 (on the left) of the sensor to GROUND
  - Connect pin 2 of the sensor to whatever your RXPIN is: in my case PIN 4 (wiringPi)/pin 16 (header)/pin 23 (BCM)
  (see https://projects.drogon.net/raspberry-pi/wiringpi/pins/ for more infor)
  - Connect pin 3 (on the right) of the sensor to +5V.
*/

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAXBUF 512

struct Data
{
    unsigned char stationCode;
    float temp, humid, batt;
};

RCSwitch mySwitch;

unsigned char t1, t4, stationCode = 0;
unsigned short t2, t3 = 0;
float temp, humid, batt = 0;

char buffer[MAXBUF];

struct Data data;
int responseCode = 0;

// test data3-999-201
//unsigned int value = 322055266;   
//unsigned int value = 322317154;   
//unsigned int value = 321793378;   
//unsigned int value = 321793634;

size_t function_pt(char *ptr, size_t size, size_t nmemb, void *stream) {
    printf("%s\n", ptr);
    responseCode = atoi(ptr);
    return size * nmemb;
}

int main(int argc, char *argv[]) {

    float startTime = clock();

    // we get lots of duplicates in the same transmission so we'll remember
    // the last value received so we don't react twice to the same value
    // however, if more than 30s passed more than likely this is a new
    // transmission so treat it as a new value so it gets posted (see if below)
    unsigned int lastValue = 0;

     int PIN = 7;

     if(wiringPiSetup() == -1)
       return 0;

     mySwitch = RCSwitch();
     mySwitch.enableReceive(PIN);

     while(1) {

      if (mySwitch.available()) {

        unsigned int value = mySwitch.getReceivedValue();

        float crtTime = clock();

        if (value == 0) {
          printf("Unknown encoding");
        } else {
          if(value == lastValue && (crtTime-startTime)/CLOCKS_PER_SEC < 30) {
              // nothing to do, it's a duplicate that came in less than 30s
              // printf("Duplicate value\n");
          } else {
              //printf("%f\n", (crtTime - startTime)/CLOCKS_PER_SEC);
              // reset the startTime
              startTime = clock();

              printf("\nReceived %u\n", value);
              // display each simple value combined in the 32 bit uint
              // first value takes only 4 bits so shift by 28
              t1 = value >> 28;
              // second value uses 10 bits: if we shift by 18 (4 first value, 10 this value),
              // we'll end up with 14 bits but we are interested only in last 10 so 0 the others
              t2 = value >> 18 & 0x3FF;   // 3FF = 00001111111111
              // third value uses 10 bits: if we shift by 8 (4 first value, 10 second value, 10 this value),
              // we'll end up with 14 bits but we are interested only in last 10 so 0 the others
              t3 = value >> 8 & 0x3FF;   // 3FF = 00001111111111
              // last value is the last byte, forcing a convertion from int to byte will get the value
              t4 = value;
              printf("raw values: %u-%i-%i-%i\n", t1, t2, t3, t4);

              data.stationCode = 0;
              data.temp = 0.0;
              data.humid = 0.0;
              data.batt = 0.0;

                  // real values
                  stationCode = t1;    // numeric code 0-15
                  temp = t2 / 10.0;    // F
                  humid = t3 / 10.0;   // %
                  batt = t4 * 50.0;    // mV
                  printf("single values: %u-%.1f-%.1f-%.1f\n", stationCode, temp, humid, batt);
                  data.stationCode = stationCode;
                  data.temp = temp;
                  data.humid = humid;
                  data.batt = batt;
                  
              

              // store the new value as lastValue
              lastValue = value;
          }
        }

        mySwitch.resetAvailable();
      }
    }
    exit(0);
}