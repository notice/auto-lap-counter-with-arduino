/*
 * Auto Lap Counter for MINIZ beta-2
 * author: kan
 */
#define ANALOG_PORT 5
#define ANALOG_THRESHOLD 800
#define CHATTERING 2
#define BOOING_LEVEL 3
#define DELAY 40
#define LAP 1

#define RED_LED 9
#define GREEN_LED 10

unsigned long beforeTime = 0;
unsigned long bestTime = 0xffffffff;
int threshold = ANALOG_THRESHOLD;
int chattering = 0;
int debug = 0;

int status = 0;
int lapCount = 0;
int booingLevel = 0;

int lapSignalCount = 0;
int lapSignalPort = 0;

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
        lapSignalCount = 2;
        if (bestTime > interval) {
          bestTime = interval;
          Serial.print("You did it! the best LAP!\n");
          lapSignalPort = GREEN_LED;
        } else {
          lapSignalPort = RED_LED;
          ++booingLevel;
          if (booingLevel >= BOOING_LEVEL) {
            booingLevel = 0;
            Serial.print("Boo! Go for it!!\n");            
          }
        }
      }
    }
  }
  if (lapSignalCount > 0) {
    analogWrite(lapSignalPort, 255);
    --lapSignalCount;
    if (lapSignalCount == 0) {
      analogWrite(lapSignalPort, 0);
    }
  }
  delay(DELAY);
}

void printTime(unsigned long time)
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
