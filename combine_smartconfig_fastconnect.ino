#include <SPI.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Client.h>
//#include <avr/pgmspace.h>

// Pins
#define CC3000_INT      2   // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN       7   // Can be any digital pin
#define CC3000_CS       10  // Preferred is pin 10 on Uno

// Connection info data lengths
#define IP_ADDR_LEN     4   // Length of IP address in bytes

// Constants
unsigned int timeout = 30000;             // Milliseconds

unsigned int num_pings = 3;    // Number of times to ping
char server[] = "data.sparkfun.com";

// Global Variables
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
SFE_CC3000_Client client = SFE_CC3000_Client(wifi);


/////////////////
// Phant Stuff //
/////////////////
const String publicKey = "yA6dXAGooNcw1Rdwld6j";
const String privateKey = "[PRIVATE KEY]";
const byte NUM_FIELDS = 1;
const String fieldNames[NUM_FIELDS] = {"photosensor"};
String fieldData[NUM_FIELDS];


const int lightPin = A0;


void setup() {


  pinMode(lightPin, INPUT_PULLUP);
  
  
  ConnectionInfo connection_info;
  IPAddress ip_addr;
  IPAddress remote_ip;
  PingReport ping_report = {0};
  int i;
  
  // Initialize Serial port
  Serial.begin(115200);
  Serial.println();
  Serial.println("-----------------------------");
  Serial.println("SparkFun CC3000 - FastConnect");
  Serial.println("-----------------------------");
  
  // Initialize CC3000 (configure SPI communications)
  if ( wifi.init() ) {
    Serial.println("CC3000 initialization complete");
  } else {
    Serial.println("Something went wrong during CC3000 init!");
  }

  
    
   // Connect to WiFi network stored in non-volatile memory
  Serial.println("Connecting to network stored in profile...");
  if ( !wifi.fastConnect(timeout) ) {

  Serial.println("-----------------------------");
  Serial.println("SparkFun CC3000 - FastConnect");
  Serial.println("-----------------------------");  
    // Start SmartConfig and wait for IP address from DHCP
  Serial.println("Starting SmartConfig");
  Serial.println("Send connection details from app now!");
  Serial.println("Waiting to connect...");
    if ( !wifi.startSmartConfig(timeout) ) {
      Serial.println("Error: Could not connect with SmartConfig");
    }
 
 }


  
  // Gather connection details and print IP address
  if ( !wifi.getConnectionInfo(connection_info) ) {
    Serial.println("Error: Could not obtain connection details");
  } else {
    Serial.println("Connected!");
    Serial.print("IP Address: ");
    for (i = 0; i < IP_ADDR_LEN; i++) {
      Serial.print(connection_info.ip_address[i]);
      if ( i < IP_ADDR_LEN - 1 ) {
        Serial.print(".");
      }
    }
    Serial.println();
  }
 
  
  Serial.print(" ");
  Serial.print(num_pings, DEC);
  Serial.println(" times...");
  if ( !wifi.ping(remote_ip, ping_report, num_pings, 56, 1000) ) {
    Serial.println("Error: no ping response");
  } else {
    Serial.println("Pong!");
    Serial.println();
    Serial.print("Packets sent: ");
    Serial.println(ping_report.packets_sent);
    Serial.print("Packets received: ");
    Serial.println(ping_report.packets_received);
    Serial.print("Min round time (ms): ");
    Serial.println(ping_report.min_round_time);
    Serial.print("Max round time (ms): ");
    Serial.println(ping_report.max_round_time);
    Serial.print("Avg round time (ms): ");
    Serial.println(ping_report.avg_round_time);
    Serial.println();
  }
  
 
  
  // Done!
  Serial.println("Finished SmartConfig test");
  
}

void loop() {


  // Gather data:
    fieldData[0] = String(analogRead(lightPin));
    

    // Post data:
    Serial.println("Posting!");
    postData(); // the postData() function does all the work, 
                // check it out below.
  
    delay(1000);
  
} 



void postData()
{

  // Make a TCP connection to remote host
  if ( !client.connect(server, 80) )
  {
    // Error: 4 - Could not make a TCP connection
    Serial.println("Error: 4");
  }

  // Post the data! Request should look a little something like:
  // GET /input/publicKey?private_key=privateKey&photosensor=sensorValues&time=5201 HTTP/1.1\n
  // Host: data.sparkfun.com\n
  // Connection: close\n
  // \n
  client.print("GET /input/");
  client.print(publicKey);
  client.print("?private_key=");
  client.print(privateKey);
  for (int i=0; i<NUM_FIELDS; i++)
  {
    client.print("&");
    client.print(fieldNames[i]);
    client.print("=");
    client.print(fieldData[i]);
  }
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Connection: close");
  client.println();

  while (client.connected())
  {
    if ( client.available() )
    {
      char c = client.read();
      Serial.print(c);
    }      
  }
  Serial.println();
}

