#include "rgb_lcd.h"
#include <string>
#include <vector>
#include <algorithm>
#include <SPI.h>
#include <WiFi.h>

rgb_lcd display;
char ssid[] = "mPhone"; //  your network SSID (name) 
char pass[] = "edinburgh";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

char currentLetter = 'A';

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "infinite-reaches-69620.herokuapp.com";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

char term = '\n';
const int MAX_TEXT_SIZE = 100000;
char buffer[MAX_TEXT_SIZE];
int bufPos = 0;

std::string response;
unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
boolean lastConnected = false;                  // state of the connection last time through the main loop
const unsigned long postingInterval = 30*1000;

void printWifiStatus();

// BUTTONS
int leftButton = 2;
int rightButton = 6;
int enterButton = 8;
void setup() 
{
  Serial.begin(9600);
  display.begin(2, 16);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);
  pinMode(enterButton, INPUT);
  Serial.println("Hello World");
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 

  String fv = WiFi.firmwareVersion();
  if( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);
  
    // wait 10 seconds for connection:
    delay(10000);
  } 
  Serial.println("Connected to wifi");
  printWifiStatus();
  
  display.write(currentLetter);
}

bool isDelimChar(char c) { 
  const char delimChars[] = "?-!, ;_";
  char testedChar[2];
  testedChar[0] = c;
  
  return (strstr(delimChars, testedChar) != NULL);
}

std::vector <std::string> split_in_words(char str[]) {
  int index = 0;
  std::string word;
  std::vector <std::string> words;
  
  while(str[index]) {
    if(!isDelimChar(str[index])) {
      word += str[index];    
    }
    else {
      word += str[index];
      words.push_back(word);
      word.clear();
    }
    ++index;
  }

  return words;
}


int k = 0;
char tmp[MAX_TEXT_SIZE];

int displayed_counter = 0;
void displayBuffer() {
  std::vector <std::string> words = split_in_words(buffer);

  int row = 1;
  int currentLength = 0;
  for(int i = 0; i < words.size(); ++i) {
      if(currentLength + words[i].size() > 16) {
        for(int j = currentLength; j < 40; ++j) {
          display.write(' ');
        }
        currentLength = 0;
        if(row == 2) {
          delay(3000);
          display.clear();
          row = 1;
        }
        else {
          row = 2;
        }
      }
      
      for(int j = 0; j < words[i].size(); ++j) {
        display.write(words[i][j]);
        //Serial.print(words[i][j]);
      }
      //Serial.println(' ');
      currentLength += words[i].size();
  }

  for(int j = currentLength; j < 40; ++j) {
    display.write(' ');
  }
  if(row == 1) {
    for(int j = 0; j < 40; ++j) {
      display.write(' ');
    }
  }
  delay(3000);
  display.clear();

  ++displayed_counter;
}

void setColor(std::string tweetClass) {
  Serial.print("SCHIMBAM CULOAREA: ");
  Serial.print(tweetClass[0]);
  Serial.print(tweetClass[1]);
  Serial.println(tweetClass[2]);
  
  if(tweetClass == "neu") {
    display.setRGB(255, 255, 0);
  }
  else if(tweetClass == "pos") {
    display.setRGB(0, 255, 0);
  }
  else {
    display.setRGB(255, 0, 0);
  }
}

int leftButtonState = 0;
int rightButtonState = 0;
int enterButtonState = 0;
int lastLeftButtonState = 0;
int lastRightButtonState = 0;
int lastEnterButtonState = 0;

std::string getCommand;
std::string username;

bool inputPhase = true;
void loop() 
{    
  if(inputPhase) {
    leftButtonState = digitalRead(leftButton);
    rightButtonState = digitalRead(rightButton);
    enterButtonState = digitalRead(enterButton);
  
    if (leftButtonState == HIGH) {
      display.clear();
      currentLetter--;
      Serial.println(currentLetter);
      
      char username_c_str[username.size() + 2];
      strcpy(username_c_str, username.c_str());
      display.write(username_c_str);
      display.write(currentLetter);
      
      lastLeftButtonState = 1;
      leftButtonState = LOW;
    }
    
    if (rightButtonState == HIGH) {
      display.clear();
      currentLetter++;
      Serial.println(currentLetter);
  
      char username_c_str[username.size() + 2];
      strcpy(username_c_str, username.c_str());
      display.write(username_c_str);
      display.write(currentLetter);
      
      lastRightButtonState = 1;
      rightButtonState = LOW;
    }
    
    if (enterButtonState == HIGH) {
      if(currentLetter == '!') {
        getCommand = "GET /response/?username=" + username + " HTTP/1.1"; 
        inputPhase = false;
      }
      else {
        username += currentLetter;
        char username_c_str[username.size() + 2];
        strcpy(username_c_str, username.c_str());
        display.write(username_c_str);
      }
      enterButtonState = LOW;
      currentLetter = 'A';
      display.clear();
    }
  
    delay(200);
  }
  else {
    while (client.available()) {
      
      char c = client.read();
      Serial.write(c);
      response += c;
    }
  
    // if there's no net connection, but there was one last time
    // through the loop, then stop the client:
    if (!client.connected() && lastConnected) {
      Serial.println();
      Serial.println("disconnecting.");
      client.stop();
    }
  
    // if you're not connected, and ten seconds have passed since
    // your last connection, then connect again and send data:
    if((millis() - lastConnectionTime > postingInterval)) {
      Serial.println("NEW HTTP REQUEST");
      client.stop();
      httpRequest();
      clearBuffer();
      k = 0;
      response.clear();
    }
    // store the state of the connection for next time through
    // the loop:
    lastConnected = client.connected();
  
    std::string delim = "Connection: close";
    for(int i = 0; i < response.size(); ++i) {
       if(response.substr(i, delim.size()) == delim) {
          std::string tweetClass;
          int j = response.size() - 1;
          while(j >= 0 && response[j] != '\n') {
            tweetClass += response[j];
            --j;
          }
  
          std::reverse(tweetClass.begin(), tweetClass.end());
          
          i = i + delim.size() + 3;
          while(i < j) {
            tmp[k++] = response[i];
            ++i;
          }
  
          setColor(tweetClass);
       } 
    }
    
    readSerial();
  }
}

void clearBuffer() {
  buffer[0] = '\0';
  bufPos = 0;
}

void readSerial() 
{
  int index = 0;
   while (index < k) {
    char inChar = tmp[index++];   // Read single available character, there may be more waiting
    
    if (inChar == term) {     // Check for the terminator (default '\n') meaning end of command
      displayBuffer();
      clearBuffer();
    }
    else if (isprint(inChar)) {     // Only printable characters into the buffer
      if (bufPos < MAX_TEXT_SIZE) {       
        buffer[bufPos++] = inChar;// Put character into buffer     
        buffer[bufPos] = '\0';      // Null terminate
      } 
    }
   }
}

// this method makes a HTTP connection to the server:

//
std::vector <std::string> userNames;
//userNames.push_back("Mihai22e");
//userNames.push_back("Stefan0111");

void httpRequest() {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting");
    
    // Make a HTTP request:
    
    char getCommand_c_str[getCommand.size() + 2];
    strcpy(getCommand_c_str, getCommand.c_str());

    Serial.println(getCommand_c_str);
    client.println(getCommand_c_str);
    client.println("Host: infinite-reaches-69620.herokuapp.com");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
