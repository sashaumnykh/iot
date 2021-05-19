#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <tcs3200.h>

#define S0 4
#define S1 5
#define S2 37
#define S3 26
#define sensorOut 38

const char* ssid = "wienski";
const char* password = "zTreparas1";

int n;
bool status_ok = true;

int red, green, blue, white;
int red_tem, green_tem, blue_tem;
// tcs3200 tcs(0, 4, 15, 13, 38); // (S0, S1, S2, S3, output pin)  //

#define BOTtoken "1773050156:AAGEVcSbHNNO7A6_HpfS0WdKN3TAeRigSFc"  // your Bot Token (Get from Botfather)

#define CHAT_ID "-1001406607356"

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
        //red = tcs.colorRead('r');
        //green = tcs.colorRead('g');
        //blue = tcs.colorRead('b');
        red = getred();
        green = getgreen();
        blue = getblue();
        bot.sendMessage(chat_id, "Red = " + String(red)+ "\nGreen = " + String(green) + "\nBlue = " + String(blue));
        if (abs(red - red_tem) > 10 or abs(green - green_tem) > 10 or abs(blue - blue_tem) > 10){
          status_ok = false;
          bot.sendMessage(chat_id, "Проверка не пройдена!");
        }
        else{
          bot.sendMessage(chat_id, "Проверка прошла успешно!");
          }
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
