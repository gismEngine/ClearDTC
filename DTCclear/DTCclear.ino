#include <mcp_can.h>
#include <SPI.h>

#define DEBUG Serial

// Default SPI pins on ESP32
// MOSI:  D23
// MISO:  D19
// SCK:   D18

#define CAN_INT 15
#define CAN_CS 4

#define LED_OK 2         // 100 ohm R

const char* PROGMEM FIRMWARE_NAME = "DTC Clear";
const char* PROGMEM FIRMWARE_VER = "0.1";

MCP_CAN CAN(CAN_CS);

uint32_t clearTime = 0;
uint32_t clearTimeExt = 0;
uint32_t clearTimeUds = 0;
const uint32_t DTC_CLEAR_CYCLE = 2000;          // in ms
const uint32_t DELETE_TIME = 2000;              // in ms

void setup() {
  initDebug();

  // Init CAN (MCP2515) interfaces:
  Serial.println(F("CAN-bus Setup:"));
  
  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK){
    Serial.println(F("MCP2515 Initialized Successfully!"));
  }else{
    Serial.println(F("Error Initializing MCP2515..."));
  }
  CAN.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
  
  pinMode(CAN_INT, INPUT);  

  // LED OK
  pinMode(LED_OK, OUTPUT);
  digitalWrite(LED_OK, LOW);
}

bool stdOk = false;
bool extOk = false;
bool udsOk = false;

bool led_status = false;

void loop() {

  if (millis() > clearTime){
    if(clearDTC()){
      clearTime = millis() + DTC_CLEAR_CYCLE;
      stdOk = true;
      delay(150);
    }
  }


  if (millis() > clearTimeExt){
    if(clearDTCExt()){
      clearTimeExt = millis() + DTC_CLEAR_CYCLE;
      extOk = true;
      delay(150);
    }
  }

  if (millis() > clearTimeUds){
    if(clearDtcUds()){
      clearTimeUds = millis() + DTC_CLEAR_CYCLE;
      udsOk = true;
      delay(150);
    }
  }

  if(stdOk && extOk && udsOk){
    led_status = !led_status;
    digitalWrite(LED_OK, led_status);
    
    stdOk = false;
    extOk = false;
    udsOk = false;
  }

}

uint32_t obdId = 0x7DF;
uint32_t obdIdExt = 0x18DB33F1;   // BROADCAST ID
uint32_t obdIdExtUds = 0x18DA00FA;   // BROADCAST ID

// Clear/reset Emission-Related Diagnostic Information
byte clearObdDtc[8] = {0x01, 0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
byte clearUdsDtc[8] = {0x04, 0x14, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool clearDTC(void){

  byte sndStat = CAN.sendMsgBuf(obdId, 0, 8, clearObdDtc);
  
  if(sndStat == CAN_OK){
    Serial.println("s");
    return true;
  }
  Serial.println("S");
  return false;
  
}

bool clearDTCExt(void){

  
  byte sndStat = CAN.sendMsgBuf(obdIdExt, 1, 8, clearObdDtc);
  
  if(sndStat == CAN_OK){
    Serial.println("e");
    return true;
  }
  Serial.println("E");
  return false;
  
}


bool clearDtcUds(void){

  byte sndStat = CAN.sendMsgBuf(obdIdExtUds, 1, 8, clearUdsDtc);
  
  if(sndStat == CAN_OK){
    Serial.println("u");
    return true;
  }
  Serial.println("U");
  return false;
  
}

void initDebug(void){
  DEBUG.begin(115200);
  
  DEBUG.print(F("\n\n\n"));                         //  Print firmware name and version const string
  DEBUG.print(FIRMWARE_NAME);
  DEBUG.print(F(" - "));
  DEBUG.println(FIRMWARE_VER);

  DEBUG.print(F("CPU@ "));      
  DEBUG.print(getCpuFrequencyMhz());    // In MHz
  DEBUG.println(F(" MHz"));

  DEBUG.print(F("Xtal@ "));      
  DEBUG.print(getXtalFrequencyMhz());    // In MHz
  DEBUG.println(F(" MHz"));

  DEBUG.print(F("APB@ "));      
  DEBUG.print(getApbFrequency());        // In Hz
  DEBUG.println(F(" Hz"));

  EspClass e;
  
  DEBUG.print(F("SDK v: "));
  DEBUG.println(e.getSdkVersion());

  DEBUG.print(F("Flash size: "));
  DEBUG.println(e.getFlashChipSize());
 
  DEBUG.print(F("Flash Speed: "));
  DEBUG.println(e.getFlashChipSpeed());
 
  DEBUG.print(F("Chip Mode: "));
  DEBUG.println(e.getFlashChipMode());

  DEBUG.print(F("Sketch size: "));
  DEBUG.println(e.getSketchSize());

  DEBUG.print(F("Sketch MD5: "));
  DEBUG.println(e.getSketchMD5());
}
