#include <WiFi101.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include "AS3935.h"

#define INT_PIN 5
#define LED_PIN 6
#define MAX_WIFI_TRIES 5
#define HUB_RESPONSE_TIME 1000

static const String deviceID = "1";

// Azure IoT Hub Config
char iotHubURL[] = "[IoT Hub URL]";   // IoT Hub URL
char path[] = "/devices/[Device Name]/messages/events?api-version=2016-02-03"; //Header PATH
char SAS[] = "[Generated SAS]"; 
int port = 443;

AS3935 AS;
WiFiSSLClient client;
// WiFi settings
char ssid[] = "[Access Point Name]";        // your network SSID (name)
char pass[] = "[WPA Password]";       // your network password
int status = WL_IDLE_STATUS;    // the Wifi radio's status


volatile int lastEvent = 0;
volatile uint8_t lastLightDistance = 0;
volatile bool eventOccured = false;

int lightnings = 0;
int disturbers = 0;

unsigned long epochTime = 0;

void setup() {

    uint8_t connectionRetries = 0;

    Serial.begin(115200);
    while(!Serial); // Wait for Serial
    
    pinMode(INT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // If shield not present
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present");
        // Don't continue
        for (;;);
    }
  

    // While not connected and haven't reached maximum tries
    while (status != WL_CONNECTED && connectionRetries < MAX_WIFI_TRIES) {
        Serial.print("Connecting to ");
        Serial.print(ssid);
        Serial.println("...");
        // Connect to Wifi
        status = WiFi.begin(ssid, pass);
        // Wait 3 seconds for connection
        delay(3000);
        // If connected
        if (status == WL_CONNECTED) {
            connectionRetries = MAX_WIFI_TRIES;
            Serial.println("Connected to Wi-Fi");
        } else {
            // Increment retries
            connectionRetries++;
            // If max retries
            if (connectionRetries == MAX_WIFI_TRIES) {
                Serial.println("Failed to connect to Wi-Fi");
            }
        }
    }

    // If connected to WiFi
    if (status == WL_CONNECTED) {
        // Get time to use as seed
        epochTime = readLinuxEpochUsingNTP();
    }
    
    Serial.println("Initialising sensor...");
    AS.init();
    AS.getInterruptCause(); // Clear any pending interrupts
    attachInterrupt(INT_PIN, intHandler, RISING);
    Serial.println("All done");
    Serial.println("---------------------------------------");
}

void loop() {

    static char serialData[10];
    static uint8_t dataIndx = 0;
  
    if (eventOccured) {
        eventOccured = false;
            detachInterrupt(INT_PIN);
            digitalWrite(LED_PIN, HIGH);
            // Have to wait at least 2ms before reading sensor registers
            delayMicroseconds(3000);
            lastEvent = AS.getInterruptCause();
            if (lastEvent == 3) {
                lightnings++;
                lastLightDistance = AS.getStormDistance();
            } else if (lastEvent == 2) {
                disturbers++;
            }
        
        Serial.println("==================================");

        if (lastEvent == 3) {
            Serial.print("Lightning detected at ");
            Serial.print(lastLightDistance);
            Serial.println(" km");
        } else if (lastEvent == 2) {
            Serial.println("Disturbed");
        } else if (lastEvent == 1) {
            Serial.println("Noisy");
        } else {
            Serial.println("Unknown");
        }

        if (status == WL_CONNECTED && lastEvent != 0) {
            sendToHub();
        }

        digitalWrite(LED_PIN, LOW);
        attachInterrupt(INT_PIN, intHandler, RISING);
    }

    if (Serial.available() > 0) {
        char c;
        // Get the character 
        c = Serial.read();
        // If end of string
        if (c == '\r' || c == '\n') {
            serialData[dataIndx] = 0;
            parseCommand(serialData);
            dataIndx = 0;
        } else {
            serialData[dataIndx] = c;
            dataIndx++;
        }
    }
}

void sendToHub() {
    
    randomSeed(epochTime + (millis() / 1000));
    uint32_t randNumber = random(9999999999999);
    
    String payload = "{ \"Device\":\"3\",\"Event\":\"" + String(lastEvent) + "\",\"Distance\":\"" + String(lastLightDistance) + "\",\"[Partition Key]\":\"Lightnings\",\"[Row Key]\":\"" + String(randNumber) + "\"}";

    Serial.println("Attempting to connect to server...");
    // if you get a connection, report back via serial:
    if (client.connect(iotHubURL, port)) {
        Serial.println("Connection established");
        // Start sending HTTP header
        client.print("POST ");
        client.print(path);
        client.println(" HTTP/1.1"); 
        client.print("Authorization: ");
        client.println(SAS);  //Authorization SAS token generated from device explorer
        client.println("Content-Type: application/atom+xml;type=entry;charset=utf-8");
        client.print("Host: "); 
        client.println(iotHubURL);
        client.print("Content-Length: ");
        client.println(String(payload.length()));
        client.println();
		// Send payload
        client.println(payload);
        client.flush();
        
        // Wait for IoT Hub to respond
        delay(HUB_RESPONSE_TIME);
        String response = "";
        while (client.available()) {
            char c = client.read();
            response.concat(c);
        }
        if (!response.equals("")) {
            // If response starts with 201 
            if (response.startsWith("HTTP/1.1 204")) {
                // Write was successful
                Serial.println("Written to hub");
            } else {
                Serial.println("Failed to write!!");
            }
        }
        
        // Disconnect from Event Hub
        client.stop();
        Serial.println("Disconnected");
    } else {
        Serial.println("Failed to connect");
    }

}

void parseCommand(char *data) {

    if (data[0] == '?' || data[0] == 'h' || data[0] == 'H') {
        Serial.println("Help:");
        Serial.println(" c - Read Internal Capacitor Value");
        Serial.println(" d - Mask Disturbers (0 or 1)");
        Serial.println(" e - Read Captured Lightnings Since Reset");
        Serial.println(" f - Read Captured Disturbers Since Reset");
        Serial.println(" i - Output LCO at Interrupt pin (0 or 1");
        Serial.println(" l - Indoors (0 or 1)");
        Serial.println(" m - Minimum Strikes Threshold");
        Serial.println(" n - Noise Threshold");
        Serial.println(" r - Read All Registers/Reset Sensor");
        Serial.println(" s - Spike Rejection");
        Serial.println(" w - Watchdog Threshold");
    } else if (data[0] == 'g') {
        parseRead(data);
    } else if (data[0] == 's') {
        parseSet(data);
    }
}

void parseRead(char *data) {
  
    int value, r;

    switch (data[1]) {
        case 'c':  // Send internal capacitor value
            value = AS.getInternalCapacitors();
            Serial.print("Internal Capacitor Value: ");
            Serial.println(value);
            break;
        case 'd':
            Serial.print("Disturbers: ");
            if (AS.getMaskDisturber()) {
                Serial.println("Masked");
            } else {
                Serial.println("Not masked");
            }
            break;
        case 'e':  // Send lightings captured so far
            Serial.print("Lightnings: ");
            Serial.println(lightnings);
            break;
        case 'f':  // Send disturbers
            Serial.print("Disturbers: ");
            Serial.println(disturbers);
            break;
        case 'l':
            Serial.print("Indoors: ");
            if (AS.getIndoors()) {
                Serial.println("TRUE");
            } else {
                Serial.println("FALSE");
            }
            break;
        case 'm':  // Send minimum strikes threshold
            value = AS.getMinimumStrikes();
            Serial.print("Minimum Strikes: ");
            Serial.println(value);
            break;
        case 'n':  // Send noise level
            value = AS.getNoise();
            Serial.print("Noise Level: ");
            Serial.println(value);
            break;
        case 'r':  // Send registers
            for (int i = 0x00; i < 0x33; i++) {
                r = AS.readReg(i);
                Serial.print("Register 0x");
                Serial.print(i, HEX);
                Serial.print(": 0x");
                Serial.println(r, HEX);
            }
            break;
        case 's':  // Send spike rejection
            value = AS.getSpikeRejection();
            Serial.print("Spike Rejection Level: ");
            Serial.println(value);
            break;
        case 'v': // Send firmware version
          break;
        case 'w':  // Send watchdog threshold
            value = AS.getWDT();
            Serial.print("WDT: ");
            Serial.println(value);
            break;
    }
}

void parseSet(char *data) {
  
    switch (data[1]) {
        case 'd':
            if (data[2] == '0') {
                AS.maskDisturber(false);
            } else if (data[2] == '1') {
                AS.maskDisturber(true);
            }
            break;
        case 'i':
            if (data[2] == '0') {
                attachInterrupt(INT_PIN, intHandler, RISING);
                AS.LCOOffIntPin();
            } else if (data[2] == '1') {
                detachInterrupt(INT_PIN);
                AS.LCOOnIntPin();
            }
            break;
        case 'l':
            if (data[2] == '0') {
                AS.setIndoors(false);
            } else if (data[2] == '1') {
                AS.setIndoors(true);
            }
            break;
        case 'm':
            AS.setMinimumStrikes((int)(data[2] - '0')*10 + (int)(data[3] - '0'));
            break;
        case 'n':
            AS.setNoise((int)(data[2] - '0'));
            break;
        case 'r':
            AS.reset();
            AS.clearStatistics();
            break;
        case 's':
            AS.setSpikeRejection((int)(data[2] - '0')*10 + (int)(data[3] - '0'));
            break;
        case 'w':
            AS.setWDT((int)(data[2] - '0')*10 + (int)(data[3] - '0'));
            break;
    }
}

void intHandler() {
    digitalWrite(LED_PIN, HIGH);
    eventOccured = true;
}

// Used for NTP
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP

unsigned long readLinuxEpochUsingNTP() {
    Udp.begin(localPort);
    sendNTPpacket(timeServer); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);

    if ( Udp.parsePacket() ) {
        Serial.println("NTP time received");
        // We've received a packet, read the data from it
        Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, esxtract the two words:

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;

        // now convert NTP time into everyday time:
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:

        Udp.stop();
        return (secsSince1900 - seventyYears);
    } else {
        Udp.stop();
        return 0;
    }
}

unsigned long sendNTPpacket(IPAddress & address) {
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
  
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
  
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}
