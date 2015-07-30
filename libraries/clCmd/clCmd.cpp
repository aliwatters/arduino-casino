#include "Arduino.h"
#include "clCmd.h"

#include "../Adafruit_CC3000/Adafruit_CC3000.h"
#include <SPI.h>

// WIFI Board and Network Setup
// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!

#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);

#define WLAN_SSID       "CL Eng Devices" // supersecret - hide before checkin...
#define WLAN_PASS       "mak3 all the Things talk" // hide this too.
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define LISTEN_PORT     7

Adafruit_CC3000_Server server(LISTEN_PORT);



ClCmd::ClCmd() {
  // this is the constructor
  _complete = false;
  _cc3000avail = false; // TODO - use the wifi if available.
}

void ClCmd::init(int baud) {
  Serial.begin(baud);
  while (! Serial); // wait until ready.

  Serial.println(F("Looking for cc3000"));

  if (!cc3000.begin()) {
    Serial.println(F("No CC3000 or bad wiring."));
  } else {
    Serial.println(F("CC3000 detected - requesting IP"));
    // DO rest of cc3000
    Serial.println("Attempting to connect to " + String(WLAN_SSID));
    if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
      Serial.println(F("Failed!"));
      while(1);
    }
    while (!cc3000.checkDHCP())
    {
      delay(100); // ToDo: Insert a DHCP timeout!
    }

    while (! wifiDetails()) {
      delay(500);
    }

    server.begin();
    _cc3000avail = true;
  }

  Serial.println(F("clCmd Setup Completed"));
}

Command ClCmd::getCmd() {
  // read what ever is on the serial buffer.

  Command cmd;
  char ch;
  cmd.name = "none";
  String text;

  while(Serial.available() && _complete == false) {
      ch = Serial.read();
      if (ch == ';') {
        _complete = true;
      } else {
        _buffer += ch;
      }
  }

  if (_complete) {
    Serial.println("Echo: " + _buffer);
    cmd = parse(_buffer);
    clear();
  }

  return cmd;
}

Command ClCmd::parse(String text) {
  Command cmd;
  cmd.name = "parse_error";

  // parse the string here... according to our protocol.
  // <name>:<arg1>:<arg2>:<arg3>

  int p1 = text.indexOf(':');
  int p2 = text.indexOf(':', p1 + 1);
  int p3 = text.indexOf(':', p2 + 1);

  if (p1 != -1 && p2 != -1 && p3 != -1) {
    cmd.name = _buffer.substring(0,p1);
    cmd.arg1 = _buffer.substring(p1+1, p2);
    cmd.arg2 = _buffer.substring(p2+1, p3);
    cmd.arg3 = _buffer.substring(p3+1);
  } else {
    Serial.println(F("Invalid command"));
  }

  return cmd;
}

void ClCmd::clear() {
  _buffer = "";
  _complete = false;
}

bool ClCmd::wifiDetails()
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
