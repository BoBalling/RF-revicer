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



// test data
//unsigned int value = 4294961097;   //15-1023-999-201
//unsigned int value = 4294967295;   //15-1023-1023-255
//unsigned int value = 4026531841;   //15-0-0-1

}

int main(int argc, char *argv[]) {

    float startTime = clock();


   

    // we get lots of duplicates in the same transmission so we'll remember
    // the last value received so we don't react twice to the same value
    // however, if more than 30s passed more than likely this is a new
    // transmission so treat it as a new value so it gets posted (see if below)
    unsigned int lastValue = 0;

     int PIN = 4;

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

              // if t2 and t3 are 0, it's motion sensor data; otherwise is temp/humid/batt
              if(t2 == 0 && t3 ==0) {
                  isMotion = true;
                  // real values
                  stationCode = t1;    // numeric code 0-15
                  motion = t4;         // 0=off,1=on
                  printf("single values: %u-%u\n", stationCode, motion);
                  data.stationCode = stationCode;
                  data.motion = motion;
                  postData();
              } else {
                  isMotion = false;
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
                  postData();
              }

              // store the new value as lastValue
              lastValue = value;
          }
        }

        mySwitch.resetAvailable();
      }
    }

    if(readyToSendToServer) {
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
//    sqlite3_finalize(dbRes);
    sqlite3_close(dbConn);

    exit(0);
}

// data doesn't get modified so pass by value
void postData()
{
    // able to use the same curl because it is not dependent on a url
    postSparkfun();
    postDweet();
    postThingspeak();
}

void postSparkfun() {
    buffer[0] = '\0';
    // post to data.sparkfun.com
    if(isMotion) {
        // send motion sensor data
        // http://data.sparkfun.com/input/[publicKey]?private_key=[privateKey]&station=[value]&motion=[value]
        sprintf(buffer,
            "%s/input/%s?private_key=%s&%s=%u&%s=%u", sparkfunServerUrl, publicKeyPIR, privateKeyPIR,
            stationKey, data.stationCode, motionKey, data.motion);
    } else {
        // send temp/humid/batt sensor data
        // http://data.sparkfun.com/input/[publicKey]?private_key=[privateKey]&station=[value]&humidity=[value]&temp=[value]&voltage=[value]
        sprintf(buffer,
            "%s/input/%s?private_key=%s&%s=%u&%s=%.1f&%s=%.1f&%s=%.1f", sparkfunServerUrl, publicKeyDHT, privateKeyDHT,
            stationKey, data.stationCode, humidityKey, data.humid, tempKey, data.temp, voltageKey, data.batt);
    }
    doPost(buffer);
}

void postDweet() {
    buffer[0] = '\0';
    // post to dweet.io
    if(isMotion) {
        // send motion sensor data
        // https://dweet.io/dweet/for/my-thing-name?station=[value]&motion=[value]
        sprintf(buffer,
            "%s%s?%s=%u&%s=%u", dweetServerUrl, dweetPIRName,
            stationKey, data.stationCode, motionKey, data.motion);
    } else {
        // send temp/humid/batt sensor data
        // https://dweet.io/dweet/for/my-thing-name?station=[value]&humidity=[value]&temp=[value]&voltage=[value]
        sprintf(buffer,
            "%s%s?%s=%u&%s=%.1f&%s=%.1f&%s=%.1f", dweetServerUrl, dweetDHTName,
            stationKey, data.stationCode, humidityKey, data.humid, tempKey, data.temp, voltageKey, data.batt);
    }
    doPost(buffer);
}

void postThingspeak() {
    buffer[0] = '\0';
    // post to api.thingspeak.com
    if(isMotion) {
        // send motion sensor data
        // http://api.thingspeak.com/update?api_key=[privateKey]&field1=[value]&field2=[value]
        sprintf(buffer,
            "%s/update?api_key=%s&%s=%u&%s=%u", thingspeakServerUrl, tsPrivateKeyPIR,
            tsStationKey, data.stationCode, tsMotionKey, data.motion);
    } else {
        // send temp/humid/batt sensor data
        // http://api.thingspeak.com/update?api_key=[privateKey]&field1=[value]&field2=[value]&field3=[value]&field4=[value]
        sprintf(buffer,
            "%s/update?api_key=%s&%s=%u&%s=%.1f&%s=%.1f&%s=%.1f", thingspeakServerUrl, tsPrivateKeyDHT,
            tsStationKey, data.stationCode, tsHumidityKey, data.humid, tsTempKey, data.temp, tsVoltageKey, data.batt);
    }
    // write to db
    doPost(buffer);
    unsigned short posted = 1;
    if(responseCode == 0) {
        // if we got 0 in the responseCode, the post was not accepted (less than 15s): posted = 0
        posted = 0;
    }
    // post to db (only when posting to thinkspeak for now)
    postDb(posted);
}

void doPost(char buffer[]) {
    printf("\n%s\n", buffer);
    if(readyToSendToServer) {
        curl_easy_setopt(curl, CURLOPT_URL, buffer);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
    }
}

void postDb(int posted) {
    dbBuffer[0] = '\0';
    if(isMotion) {
        sprintf(dbBuffer, sqlInsertPIR, data.stationCode, motion, posted);
    } else {
        sprintf(dbBuffer, sqlInsertDHT, data.stationCode, data.temp, data.humid, data.batt, posted);
    }
    puts(dbBuffer);
    dbError = sqlite3_exec(dbConn, dbBuffer, 0, 0, 0);
    if (dbError) {
        puts("Can not insert into database");
    }
}