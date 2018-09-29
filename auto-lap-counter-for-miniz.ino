/*
 * Auto Lap Counter for MINIZ beta-2
 * author: kan
 */
#define ANALOG_PORT 5
#define ANALOG_THRESHOLD 1000
#define CHATTERING 2
#define BOOING_LEVEL 3
#define DELAY 40
#define LAP 1

unsigned long beforeTime = 0;
unsigned long bestTime = 0xffffffff;
int threshold = ANALOG_THRESHOLD;
int chattering = 0;
int debug = 0;

int status = 0;
int lapCount = 0;
int booingLevel = 0;

void setup() {
  Serial.begin(9600);
  Serial.print("Auto Lap Counter beta-2 Ready.\n");     
}

void loop() {
  unsigned int v = analogRead(ANALOG_PORT);
  if (debug) {
    Serial.println(v);
  }
  if (v >= threshold) {
    if (chattering == 0) {
      status = 0;
    } else {
      --chattering;
    }
  } else {
    if (status == 0) {
      status = LAP;
      chattering = CHATTERING;
      if (beforeTime == 0) {
        beforeTime = millis();      
        Serial.println("Go!");     
      } else {
        unsigned long currentTime = millis();
        unsigned long interval = currentTime - beforeTime;
        beforeTime = currentTime;
        ++lapCount;
        Serial.print("lap:"); Serial.print(lapCount); Serial.print(" ");     
        printTime(interval);
        if (bestTime > interval) {
          bestTime = interval;
          Serial.print("You did it! the best LAP!\n");
        } else {
          ++booingLevel;
          if (booingLevel >= BOOING_LEVEL) {
            booingLevel = 0;
            Serial.print("Boo! Go for it!!\n");            
          }
        }
      }
    }
  }
  delay(DELAY);
}

void printTime(int time)
{
  unsigned int secs = time / 1000;
  unsigned int msecs = (time / 10) % 100;
  if (secs < 10) {
    Serial.print(0);
  }
  Serial.print(secs);
  Serial.print(".");
  if (msecs < 10) {
    Serial.print(0);    
  }
  Serial.println(msecs);
}
