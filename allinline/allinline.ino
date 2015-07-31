#include "Arduino.h"

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"


#include <clCmd.h>
// Needed for neopixels - even though completely handled in clPix.
// #include <Adafruit_NeoPixel.h>

#include "udpServer.h"
#define UDP_READ_BUFFER_SIZE 20
#define LISTEN_PORT_UDP 2811


#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);

#define WLAN_SSID       "SSID NAME" // supersecret - hide before checkin...
#define WLAN_PASS       "SUPERSECRET" // hide this too.
#define WLAN_SECURITY   WLAN_SEC_WPA2
// #define LISTEN_PORT     7

// #include <clPixMin.h>
#include <Servo.h>
#include <ClServo.h>


//#define NEOPIN 5
//#define NUMPIXELS 5
//Adafruit_NeoPixel neo = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_RGB + NEO_KHZ800);
  
uint32_t tick; // global timer - pointers used later...

/*
void setup () {}
void loop () {}
*/


// CONSTRUCTORS.

// Adafruit_CC3000_Server server(LISTEN_PORT);
UDPServer echoServer(LISTEN_PORT_UDP);

ClCmd clCmd = ClCmd();

// ClPixMin clPix = ClPixMin(neo, &tick, 5, 5); // 5's are also defined in ClPix - could be cleaner.

Servo servo; // passed in
ClServo clServo = ClServo(servo, 9, 93); // pin no, zero speed (changes everynow and then).


void setup(void)
{

  clServo.init();
  //clPix.init();

/*
  neo.begin();
  for(int i=0; i < NUMPIXELS; i++) { neo.setPixelColor(i, neo.Color(0,100,0)); }
  neo.show();
*/
    

  Serial.begin(115200);
  
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  // Initialise the module
  
  Serial.println(F("Initializing..."));
  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  // Display the IP address DNS, Gateway, etc. 
  while (! wifiDetails()) {
    delay(1000);
  }


Serial.println(F("about ot listen for connections...")); 
  // Start listening for connections
  echoServer.begin();
  
  Serial.println(F("Listening for connections...")); 
  Serial.print(F("Free RAM: after setup ")); Serial.println(getFreeRam(), DEC);

}


int count = 0;
String input = "";
boolean complete = false;

void loop(void) {
  if (count % 1000 == 0) {
    Serial.print(F("Free RAM loop: "));  Serial.print(count); Serial.print(F(" -> "));Serial.println(getFreeRam(), DEC);
  }
  count++;
  
  // Check if there is data available to read.
  
  if (echoServer.available()) {
  
    //Serial.println(F("server avialable:  true"));
    char buffer[UDP_READ_BUFFER_SIZE];
    int n = echoServer.readData(buffer, UDP_READ_BUFFER_SIZE);  // n contains # of bytes read into buffer
    Serial.print(F("n: ")); Serial.println(n);

    for (int i = 0; i < n; ++i) {
       uint8_t c = buffer[i];
       Serial.print(F("c: ")); Serial.print(c); Serial.println(" " + String((char)c));
       // ... Do whatever you want with 'c' here ...
       
       if ((char) c == ';') {
         complete = true;
       } else if (complete == false) {
         input += String((char) c);
       }
       
    }

  } else {
    //Serial.println(F("server NOT avialable:  true"));
  }


  if (complete) {
    Serial.println("Recieved: " + input);
    Command cmd = clCmd.parse(input);
    Serial.println("cmd is: " + String(cmd.name) + " " + String(cmd.arg1));
    if (cmd.name != "none") {
      clServo.update(cmd);
    }
    input = "";
    complete = false;
  }

/*
  // here TODO if complete parse the input and do the thing.
  if (count % 1000 == 0) {

  Serial.println(input);
  Command cmd;

  cmd.name = "clservo-set";
  
  if (count % 2000 == 0) {
    cmd.arg1 = "98";
  } else {
    cmd.arg1 = "102";
  }
  // Serial.println("Servo speed " + cmd.arg1);
  clServo.update(cmd);

  }
*/
  
  clServo.operate();  
}


// INIT Section
/*
void setup() {
  clCmd.init(9600);


  clPix.init();

  if (!cc3000.begin()) {
    // die? really?
    while(1);
  }

 
  Serial.println(F("CC3000 detected - requesting IP"));
  // DO rest of cc3000
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }

  while (!cc3000.checkDHCP()) {
    delay(100);
  }

  while (!wifiDetails()) {
    delay(200);
  }

  server.begin();
  
  clServo.init();
  
}


// MAIN LOOP

void loop() {

Adafruit_CC3000_ClientRef client = server.available();

  uint8_t ch;
  if (client) {
    if (client.available() > 0) {
      ch = client.read();
      client.write(ch);
      Serial.print((char) ch); 
    }
  }
  
  // get a command.
  Command c = clCmd.getCmd(ch);

  // set states below.
  if (c.name != "none") {
    // set new states
    //Serial.println("Execute cmd: " + c.name);
    //Serial.println("Arg1 " + c.arg1 + " Arg2 " + c.arg2 +" Arg3 " + c.arg3);

    clPix.update(c);
    clServo.update(c);
  }

  // start counter loop again.
  tick = millis();
  
  // do stuff
  clPix.operate();
  clServo.operate();
  
}

*/

// wifi helper details.
bool wifiDetails()
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!"));
    return false;
  } else {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.println();
    return true;
  }
}

