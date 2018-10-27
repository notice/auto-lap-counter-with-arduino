/*
 * Auto Lap Counter for MINIZ beta-7
 * uploading the results to Cloud Firestore using Cloud Functions
 * 
 * revises
 * beta-6: sounds to start signal and final lap
 * beta-5: add start signals and printing results
 * beta-4: using PSD(Position Sensitive Detector) GP2Y0A21YK
 * beta-3: add lap signal LEDs
 * 
 * author: kan
 */
#include "ESP8266.h"
#include <SoftwareSerial.h>

#define RX 7
#define TX 6

#define SSID        "ssid"
#define PASSWORD    "password"

#define END_POINT "blahblahblah.cloudfunctions.net"
#define PORT 80
#define ACTION "GET /uploadResults"

SoftwareSerial mySerial(RX, TX); /* RX,TX */
ESP8266 wifi(mySerial);

#define ANALOG_PORT 5
#define ANALOG_THRESHOLD 300
#define CHATTERING 2

#define BOOING_LEVEL 3

#define DELAY 40

#define LAP 1
#define MAX_LAP 10

#define RED_LED 9
#define GREEN_LED 10
#define LED_BRIGHTNESS 32

#define SOUND_PORT 12
#define SOUND_SHORT_LENGTH 200
#define SOUND_LONG_LENGTH 2000
#define SOUND_RED 262
#define SOUND_GREEN 523
#define SOUND_FINAL_LAP 349
#define SOUND_FINISH 494

int debug = 0;

int verbose = 0;
unsigned long beforeTime = 0;
unsigned long bestTime = 0xffffffff;
int threshold = ANALOG_THRESHOLD;
int chattering = 0;

int status = 0;
int lapCount = 0;
int times[MAX_LAP] = {0};
int finished = 0;
int finishedFlash = 0;
int booingLevel = 0;

int lapSignalCount = 0;
int lapSignalPort = 0;

int wifiStatus = 0;
int uploading = 0;
int uploadingDelay = 2;

void setup() {
  wifiStatus = setupWifi();
  startSignal();
}

void loop() {

  if (lapCount >= MAX_LAP) {
    finish();
    return;
  }
  
  unsigned int v = analogRead(ANALOG_PORT);
  
  if (debug) {
    Serial.println(v);
  }
  
  if (v > threshold) {
    lap();

  } else {
    if (chattering == 0) {
      status = 0;
    } else {
      --chattering;
    }
  }

  frashLapSignal();
  
  delay(DELAY);
}

void finish()
{
  if (finished == 0) {
    finished = 1;
    tone(SOUND_PORT, SOUND_FINISH, SOUND_LONG_LENGTH);
    analogWrite(RED_LED, 0);
    analogWrite(GREEN_LED, 0);
    printLapTimes();

  } else {
    if (finishedFlash == 0) {
      finishedFlash = 1;
      analogWrite(GREEN_LED, LED_BRIGHTNESS);
      delay(500);    
    } else {
      finishedFlash = 0;
      analogWrite(GREEN_LED, 0);
      delay(500);            
      if (wifiStatus && !uploading) {
        if (--uploadingDelay <= 0) { // wait for finishing last sound.
          uploadResults(times, MAX_LAP);
          uploading = 1;
        }
      }
    }
  }
}

void lap()
{
  if (status == 0) {
    status = LAP;
    chattering = CHATTERING;
    unsigned long currentTime = millis();
    unsigned long interval = currentTime - beforeTime;
    beforeTime = currentTime;
    times[lapCount] = interval;
    ++lapCount;
    if (verbose) {
      Serial.print("lap:"); Serial.print(lapCount); Serial.print(" "); printTime(interval);      
    }
    lapSignalCount = 2;
    if (bestTime > interval) {
      bestTime = interval;
      if (verbose) {
        Serial.print("You did it! the best LAP!\n");
      }
      lapSignalPort = GREEN_LED;
      booingLevel = 0;
    } else {
      lapSignalPort = RED_LED;
      ++booingLevel;
      if (booingLevel >= BOOING_LEVEL) {
        booingLevel = 0;
        if (verbose) {
          Serial.print("Boo! Go for it!!\n");          
        }  
      }
    }
  }
}

void frashLapSignal()
{
  if (lapSignalCount > 0) {
    analogWrite(lapSignalPort, LED_BRIGHTNESS);
    --lapSignalCount;
    if (lapSignalCount == 0) {
      analogWrite(lapSignalPort, 0);
      if ((lapCount + 1) >= MAX_LAP) { // final lap
        tone(SOUND_PORT, SOUND_FINAL_LAP, SOUND_SHORT_LENGTH);
        if (verbose) {
          Serial.println("final lap!!");
        }
        delay(500);  
        // flashing LED
        analogWrite(lapSignalPort, LED_BRIGHTNESS);
        delay(500);  
        analogWrite(lapSignalPort, 0);
        delay(500);  
      }
    }
  }
}

void startSignal()
{
  Serial.println("Auto Lap Counter beta-7 Ready.");

  for (int i = 0; i < 3; ++i) {
   tone(SOUND_PORT, SOUND_RED, SOUND_SHORT_LENGTH);
   analogWrite(RED_LED, LED_BRIGHTNESS);
   delay(1000);    
   analogWrite(RED_LED, 0);
   delay(1000);    
  }
  if (verbose) {
    Serial.println("Go!");
  }
  tone(SOUND_PORT, SOUND_GREEN, SOUND_LONG_LENGTH);
  analogWrite(GREEN_LED, LED_BRIGHTNESS);
  delay(500);     
  analogWrite(GREEN_LED, 0);
  beforeTime = millis(); 
}

void printTime(long time)
{
  char buffer[8] = {0};
  time2str(time, buffer);
  Serial.println(buffer);
}

int time2str(long time, char *buffer)
{
  int secs = time / 1000;
  int msecs = (time / 10) % 100;
  itoa(secs, buffer, 10);
  buffer[strlen(buffer)] = '.';
  itoa(msecs, buffer + strlen(buffer), 10);
  return strlen(buffer);
}

void printLapTimes()
{
  Serial.println("---- RESULTS ----");
  for (int i = 0; i < MAX_LAP; ++i) {
    Serial.print("lap:"); Serial.print(i + 1); Serial.print(" "); printTime(times[i]);     
  }
  
  int bestLap = 0;
  int bestTime = times[0];
  long totalTime = 0;
  for (int i = 0; i < MAX_LAP; ++i) {
    if (bestTime > times[i]) {
      bestTime = times[i];
      bestLap = i;
    }
    totalTime += times[i];
  }
  
  Serial.println("---- ANALYTICS ----");
  Serial.print("best lap: "); Serial.print(bestLap + 1); Serial.print(" time: "); printTime(times[bestLap]);
  Serial.print("   total: "); printTime(totalTime);
  Serial.print("     avg: "); printTime(totalTime / MAX_LAP);
}

int setupWifi() {

  Serial.begin(9600);

  if (!wifi.setOprToStationSoftAP()) {
    Serial.println("fail to set station + softap.");
    return 0;
  }

  if (!wifi.joinAP(SSID, PASSWORD)) {
    Serial.println("fail to join AP.");
    return 0;
  }
  
  Serial.println("success to join AP.");
  Serial.print("IP:"); Serial.println(wifi.getLocalIP().c_str());
  
  if (!wifi.disableMUX()) {
    Serial.println("fail to disable MUX.");
    return 0;
  }
  return 1;
}

void uploadResults(int *times, int n)
{
  char buffer[256] = {0};

  Serial.print("connecting...");

  if (wifi.createTCP(END_POINT, PORT)) {
    Serial.println("ok");
  } else {
    Serial.println("ng");
    return;
  }

  const char *param = "?results=";
  const char *proto = " HTTP/1.0\r\n";
  const char *host = "Host: ";
  const char *userAgent = "\r\nUser-Agent: auto-lap-counter/beta-7\r\n";
  const char *accept = "Accept: */*\r\n\r\n";

  char results[128] = {0};
  createResults(times, n, results);

  strcpy(buffer, ACTION);
  strcat(buffer, param), strcat(buffer, results), strcat(buffer, proto);
  strcat(buffer, host), strcat(buffer, END_POINT);
  strcat(buffer, userAgent);
  strcat(buffer, accept);
  
  wifi.send((uint8_t *)buffer, strlen(buffer));
}

void createResults(int *times, int n, char *buffer) {
  char *p = buffer;
  
  for (int i = 0; i < n; ++i) {
    if (i > 0) {
      *p++ = ',';
    }
    p += time2str(times[i], p);
  }  
}
