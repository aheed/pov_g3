// Simple strand test for Adafruit Dot Star RGB LED strip.
// This is a basic diagnostic tool, NOT a graphics demo...helps confirm
// correct wiring and tests each pixel's ability to display red, green
// and blue and to forward data down the line.  By limiting the number
// and color of LEDs, it's reasonably safe to power a couple meters off
// the Arduino's 5V pin.  DON'T try that with other code!

//#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#include <ESP8266WiFi.h>
#include "ldprotocol.h"
#include "povgeometry_g3.h"
#include "private.h"

const char* ssid = MYSSID;
const char* password = MYPWD;


//#define NUMSECTORS 100
//#define NUMPIXELS 64 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
//#define DATAPIN    4
#define DATAPIN    13
//#define CLOCKPIN   5
#define CLOCKPIN   14

//Define the LED Pin
#define PIN_OUT (0)
#define LED_ON (LOW)
#define LED_OFF (HIGH)
#define INTERRUPT_PIN 4

//#define DEBUG_POV_SERVER
#define SERVER_TIMEOUT 5000 //ms to wait for any server status change

#define DEBOUNCE_MIN_REV_CYCLES 5000000

/*
//Adafruit_DotStar strip = Adafruit_DotStar(
//  NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);
*/

//#define LEDDATASIZE (NUMSECTORS * (NUMPIXELS + 1) * 4)
uint8_t leddata[POV_FRAME_SIZE];

//volatile byte state = LOW;
volatile int state = LOW;
volatile uint32_t cyclesPerRev = 1000;
volatile uint32_t lastCycleCnt = 0;
volatile uint32_t revs = 0;

int lastSector = 0;

WiFiServer server(LDPORT);

void blink() {
  
  uint32_t newCycleCnt = ESP.getCycleCount();
  uint32_t newCyclesPerRev = newCycleCnt - lastCycleCnt;
  
  if(newCyclesPerRev > DEBOUNCE_MIN_REV_CYCLES)
  {
    // Debounce check passed
    cyclesPerRev = newCyclesPerRev;
    lastCycleCnt = newCycleCnt;  
    
    revs++;
    state = !state;
  }
  
}


///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////

typedef enum SERVERSTATE
{
   SST_IDLE,
   SST_CONNECTED,
   SST_RESPONDING,
   SST_WAIT_CLOSE
} SERVERSTATE;

SERVERSTATE servstate =  SST_IDLE;
unsigned long lastStatusChange;
WiFiClient  currentClient;
int totalbytesread;

void handleServer()
{
  int bytesread;

  if (servstate == SST_IDLE) {
    WiFiClient client = server.available();
    if (!client) {
      return;
    }

#ifdef DEBUG_POV_SERVER
    Serial.println("New client");
#endif

    currentClient = client;
    totalbytesread = 0;
    servstate = SST_CONNECTED;
    lastStatusChange = millis();
  }

  bool keepCurrentClient = false;
  bool callYield = false;

  if (currentClient.connected()) {
    switch (servstate) {
    /*case SST_IDLE:
      // No-op to avoid C++ compiler warning
      break;*/
    case SST_CONNECTED:

      if (currentClient.available()) {
    
        bytesread = currentClient.read(
                         leddata + totalbytesread,  //pointer arithmetic
                         POV_FRAME_SIZE - totalbytesread);
        totalbytesread += bytesread;
        
#ifdef DEBUG_POV_SERVER    
        //Serial.println("Got " + String(bytesread) + " bytes");
#endif
    
        if(bytesread)
        {
#ifdef DEBUG_POV_SERVER
          Serial.println("Bytes read: " + String(totalbytesread) + "  Expected bytes:" + String(POV_FRAME_SIZE));
#endif                        
          if(totalbytesread == POV_FRAME_SIZE)
          {
#ifdef DEBUG_POV_SERVER
            Serial.println("Got frame!");
#endif               
            currentClient.flush();
            servstate = SST_RESPONDING;
          }

          lastStatusChange = millis();
        }        

      } //available

      if (millis() - lastStatusChange <= SERVER_TIMEOUT) {
        keepCurrentClient = true;
      }
      callYield = true;        
      break;

    case SST_RESPONDING:
      {
      char response[LD_ACK_SIZE] = {LD_ACK_CHAR};
      if(currentClient.write(response, LD_ACK_SIZE) < 0)
      {
#ifdef DEBUG_POV_SERVER
        Serial.println("failed to send ACK");
#endif
      }
      else
      {
        keepCurrentClient = true;
        callYield = true;
      }
      servstate = SST_WAIT_CLOSE;
      }
      break;
      
    case SST_WAIT_CLOSE:
      // Wait for client to close the connection
      if (millis() - lastStatusChange <= SERVER_TIMEOUT) {
        keepCurrentClient = true;
        callYield = true;
      }
      break;

    default:
      //should not happen
      break;
      
    }//switch
  }//connected

  if (!keepCurrentClient) {
#ifdef DEBUG_POV_SERVER    
    Serial.println("Closing connection to client");
#endif    
    currentClient = WiFiClient();
    servstate = SST_IDLE;
  }

  if (callYield) {
    yield();
  }
  
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////


void setup() {

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  /*strip.begin(); // Initialize pins for output
  strip.setBrightness(10);
  strip.show();  // Turn all LEDs off ASAP
  */

  SPI.begin();
  SPI.setFrequency(12000000L);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

  int i;
  for(i=0; i<(sizeof(leddata)/sizeof(leddata[0])); i++)
  {
    leddata[i] = 0;
    if( (i > 0) && ((i%4) == 0) )
    {
      leddata[i] = 0xFF; //header
    }
  }

  pinMode( PIN_OUT, OUTPUT );
  digitalWrite( PIN_OUT, LED_ON );

  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), blink, RISING);
   
  Serial.begin ( 115200 );

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  
}

// Runs 10 LEDs at a time along strip, cycling through red, green and blue.
// This requires about 200 mA for all the 'on' pixels + 1 mA per 'off' pixel.

int      head  = 0, tail = -10; // Index of first 'on' and 'off' pixels
uint32_t color = 0xFF0000;      // 'On' color (starts red)


int loopcnt = 0;

uint32_t lastLoopCycleCnt, newLoopCycleCnt, cyclesPerLoop, maxCyclesPerLoop;

void loop() {

  /*
  strip.setPixelColor(head, color); // 'On' pixel at head
  strip.setPixelColor(tail, 0);     // 'Off' pixel at tail
  strip.show();                     // Refresh strip
  delay(20);                        // Pause 20 milliseconds (~50 FPS)
  */


//  delay(5);


  //--------------------------------
  newLoopCycleCnt = ESP.getCycleCount();
  cyclesPerLoop = newLoopCycleCnt - lastLoopCycleCnt;
  lastLoopCycleCnt = newLoopCycleCnt;
  if(cyclesPerLoop > maxCyclesPerLoop)
  {
    //new max cycle count
    maxCyclesPerLoop = cyclesPerLoop;
  }
  //-----------------------

  handleServer();


  uint32_t currentCycleCnt = ESP.getCycleCount();
  uint32_t cyclesSinceRevStart = currentCycleCnt - lastCycleCnt;
  int currentSector =  ((cyclesSinceRevStart * NOF_SECTORS) / cyclesPerRev) % NOF_SECTORS;
  /*int currentSector =  (((cyclesSinceRevStart/100) * NOF_SECTORS) / (cyclesPerRev/100));
  if(currentSector >= NOF_SECTORS)
  {
    // avoid buffer overrun
    currentSector = NOF_SECTORS - 1;
  }*/

  if((loopcnt % 1000) == 0)
  {
    Serial.println(String(cyclesPerRev) + " : " + String(currentSector) + " : " + String(cyclesSinceRevStart) + " : " + String(currentCycleCnt)  + " : " + String(lastCycleCnt) + " : " + String(revs) + " : " + " :: " + String(cyclesPerLoop) + " : " + String(maxCyclesPerLoop));
    /*
    int i;
    for(i=0; i<(NOF_LEDS * LED_DATA_SIZE); i++)
    {
      Serial.print(String(leddata[(currentSector * NOF_LEDS * LED_DATA_SIZE) + i]) + " ");
    }
    Serial.println("");*/    
  }
  else if((loopcnt % 1000) == 1)
  {
    maxCyclesPerLoop = 0;
  }


  //--------
    
//  leddata[head * 4 + 2] = 0X10;
//  leddata[tail * 4 + 2] = 0X00;

  uint8_t preamble[] = {0, 0, 0, 0};

  if(currentSector != lastSector)
  {
    SPI.writeBytes(preamble, 4); // 4 zero bytes preamble
    //SPI.writeBytes(leddata, NOF_LEDS * 4);
    SPI.writeBytes(leddata + currentSector * NOF_LEDS * LED_DATA_SIZE,  //pointer arithmetic
                   NOF_LEDS * LED_DATA_SIZE);
  }
  lastSector = currentSector;

  /*
  SPI.writeBytes(leddata, (NOF_LEDS+1) * 4);
  */

  //--------

  if(++head >= NOF_LEDS) {         // Increment head index.  Off end of strip?
    head = 0;                       //  Yes, reset head index to start
    if((color >>= 8) == 0)          //  Next color (R->G->B) ... past blue now?
      color = 0xFF0000;             //   Yes, reset to red

    color &= 0xFF0000;
  }
  if(++tail >= NOF_LEDS)
  {
    tail = 0; // Increment, reset tail index
  }

  loopcnt++;

  digitalWrite( PIN_OUT, state );

  yield();
}
