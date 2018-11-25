#include <lwip/def.h>

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

#define RESPOND
#define DEBUG_POV_SERIAL(x) Serial.println(x)
//#define DEBUG_POV_SERVER(x) DEBUG_POV_SERIAL(x) 
#define DEBUG_POV_SERVER(x)

#define SERVER_TIMEOUT 5000 //ms to wait for any server status change

#define DEBOUNCE_MIN_REV_CYCLES 5000000

/*
// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
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

typedef struct
{
  uint32_t cyclesPerRev;
  uint32_t dbg;
} PovDebugInfo;

SERVERSTATE servstate =  SST_IDLE;
char response[LD_ACK_SIZE] = {LD_ACK_CHAR};
PovDebugInfo povDbg;
char *pResponse = response;
size_t responseSize = LD_ACK_SIZE;
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

    DEBUG_POV_SERVER("New client");

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
    
        if(bytesread)
        {                   
          DEBUG_POV_SERVER("Bytes rx: " + String(totalbytesread) + "  Expected bytes:" + String(POV_FRAME_SIZE));
          if(totalbytesread == POV_FRAME_SIZE)
          {
            DEBUG_POV_SERVER("Got frame!");          
            currentClient.flush();
            servstate = SST_RESPONDING;
            pResponse = response;
            responseSize = LD_ACK_SIZE;
          }
          else if((totalbytesread == 1) && (leddata[0] != 0xFF))
          {
            // Received data does not look like LED data.
            // Respond with debug info.
            DEBUG_POV_SERVER("Got report request");
            servstate = SST_RESPONDING;
            povDbg.cyclesPerRev = htonl(cyclesPerRev);
            povDbg.dbg = htonl(4711);
            pResponse = (char*)&povDbg;
            responseSize = sizeof(povDbg);            

            // Restore first byte of LED data
            leddata[0] = 0xFF;
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

#ifdef RESPOND      
      if(currentClient.write(pResponse, responseSize) < 0)
      {
        DEBUG_POV_SERVER("failed to tx");
      }
      else
#endif      
      {
        keepCurrentClient = true;
        callYield = true;
      }

      servstate = SST_CONNECTED;
      totalbytesread = 0;
      }
      break;
      
    /*case SST_WAIT_CLOSE:
      // Wait for client to close the connection
      if (millis() - lastStatusChange <= SERVER_TIMEOUT) {
        keepCurrentClient = true;
        callYield = true;
      }
      break;*/

    default:
      //should not happen
      break;
      
    }//switch
  }//connected

  if (!keepCurrentClient) {
    DEBUG_POV_SERVER("Closing connection to client");
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


int loopcnt = 0;

uint32_t lastLoopCycleCnt, newLoopCycleCnt, cyclesPerLoop, maxCyclesPerLoop;

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void loop() {


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
  
  if((loopcnt % 1000) == 0)
  {
    DEBUG_POV_SERIAL(String(cyclesPerRev) + " : " + String(currentSector) + " : " + String(cyclesSinceRevStart) + " : " + String(currentCycleCnt)  + " : " + String(lastCycleCnt) + " : " + String(revs) + " : " + " :: " + String(cyclesPerLoop) + " : " + String(maxCyclesPerLoop));
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

  loopcnt++;

  digitalWrite( PIN_OUT, state );

  yield();
}
