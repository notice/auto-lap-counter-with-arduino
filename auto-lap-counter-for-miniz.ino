/*
 * Auto Lap Counter for MINIZ beta-5
 * add start signals and printing results
 * 
 * revises
 * beta-4: using PSD(Position Sensitive Detector) GP2Y0A21YK
 * beta-3: add lap signal LEDs
 * 
 * author: kan
 */
#define ANALOG_PORT 5
#define ANALOG_THRESHOLD 300
#define CHATTERING 2

#define BOOING_LEVEL 3

#define DELAY 40

#define LAP 1
#define MAX_LAP 10

#define RED_LED 9
#define GREEN_LED 10

int debug = 0;

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

void setup() {
  Serial.begin(9600);
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
    analogWrite(RED_LED, 0);
    analogWrite(GREEN_LED, 0);
    printLapTimes();
  } else {
    if (finishedFlash == 0) {
      finishedFlash = 1;
      analogWrite(GREEN_LED, 255);
      delay(500);    
    } else {
      finishedFlash = 0;
      analogWrite(GREEN_LED, 0);
      delay(500);            
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
    Serial.print("lap:"); Serial.print(lapCount); Serial.print(" "); printTime(interval);
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

void frashLapSignal()
{
  if (lapSignalCount > 0) {
    analogWrite(lapSignalPort, 255);
    --lapSignalCount;
    if (lapSignalCount == 0) {
      analogWrite(lapSignalPort, 0);
      if ((lapCount + 1) >= MAX_LAP) { // final lap
        Serial.println("final lap!!");
        delay(500);  
        // flashing LED
        analogWrite(lapSignalPort, 255);
        delay(500);  
        analogWrite(lapSignalPort, 0);
        delay(500);  
      }
    }
  }
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

void startSignal()
{
  Serial.println("Auto Lap Counter beta-5 Ready.");

  for (int i = 0; i < 3; ++i) {
   analogWrite(RED_LED, 255);
   delay(1000);    
   analogWrite(RED_LED, 0);
   delay(1000);    
  }
  
  analogWrite(GREEN_LED, 255);
  delay(500);     
  analogWrite(GREEN_LED, 0);
  beforeTime = millis(); 
  Serial.println("Go!");
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
