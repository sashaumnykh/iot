
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <tcs3200.h>

#define S0 4
#define S1 5
#define S2 37
#define S3 26
#define sensorOut 38

const char* ssid = " ";
const char* password = " ";

int number = 10;
float tol = 0.1;
bool status_ok = true;
int whereismistake;

int red, green, blue, white;
int red_tem, green_tem, blue_tem;
// tcs3200 tcs(0, 4, 15, 13, 38); // (S0, S1, S2, S3, output pin)  //

#define BOTtoken " "

#define CHAT_ID "- "

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/get_colors"){
        bot.sendMessage(chat_id, "getting colors", "");
        for (int i = 0; i <= number; i++) {
          red = getred();
          green = getgreen();
          blue = getblue();
          bool prev_status = status_ok;
          if (abs((red - red_tem)/red_tem) >= tol or abs((green - green_tem)/green_tem) >= tol){
            status_ok = false;
            }
          if (abs((blue - blue_tem)/blue_tem) >= tol){
            status_ok = false;
            }
          if (status_ok != prev_status){
            whereismistake = i;
          }
          delay(100);
        }
        if (status_ok = false){
          bot.sendMessage(chat_id, "Проверка не пройдена!\nНомер первого образца с ошибкой: " + String(whereismistake) + "\nПроведено проверок: " + String(i) + " из " + String(number));
        }
        else{
          bot.sendMessage(chat_id, "Проверка прошла успешно!");
          bot.sendMessage(chat_id, "\nПроведено проверок: " + String(number) + " из " + String(number));
        }
  
        //bot.sendMessage(chat_id, "Red = " + String(red)+ "\nGreen = " + String(green) + "\nBlue = " + String(blue));
    }
    if (text == "/scan_the_template"){
      red_tem = getred();
      green_tem = getgreen();
      blue_tem = getblue();
      bot.sendMessage(chat_id, "Red = " + String(red_tem)+ "\nGreen = " + String(green_tem) + "\nBlue = " + String(blue_tem));
      }
  }
}


int getred(){
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  red = pulseIn(sensorOut, LOW);
  Serial.printf("Red = ", red);
  return red;
}
int getgreen(){
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  green = pulseIn(sensorOut, LOW);
  Serial.printf("Green = ", green);
  return red;
}
int getblue(){
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  blue = pulseIn(sensorOut, LOW);
  Serial.printf("Blue = ", blue);
  Serial.println();
  return blue;
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
 
  //Setting frequency-scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot Started", "");
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
