#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

/*Pins denitios*/
constexpr uint8_t TFT_CS = 27;
constexpr uint8_t TFT_DC = 23;
constexpr uint8_t TFT_RST = 22;
constexpr uint8_t TFT_SCLK = 14;
constexpr uint8_t TFT_MISO = 12;
constexpr uint8_t TFT_MOSI = 13;
constexpr uint8_t BUTTON_PIN = 21;
constexpr uint8_t BUZZER_PIN = 32;

/*object for wificlient:socket*/
WiFiClient client;

/*object for for tft display*/
Adafruit_ST7735 tft(TFT_CS, TFT_DC,TFT_MOSI, TFT_SCLK, TFT_RST);

/*Init state, and text color*/
unsigned int currentState = LOW;
uint16_t color = ST7735_CYAN;
constexpr uint8_t size = 2;

/*for signaling message from host*/
bool flag = false;
bool long_alert = false;

/*time measure*/
const long max_delay = 5000;
unsigned long start = 0;
unsigned long now = 0;

/*wifi credentials*/
const char* ssid = "*******";
const char* password = "******";

/*socket port and host ip*/
const uint16_t port = 1234;
const char * host = "********"; //ip address

/*function declarations*/
void beep();
void beep2();
void set_wifi();
void read_msg();
void send_msg();
void set_socket();
void clearDisplay();
bool buttonPressed();
void alert_display();
void normal_display();
void debug(const char* str);
void showText(int x, int y, const char* text);

void setup(){
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  debug("Hello! ST7735 TFT Test");
  tft.initR(INITR_BLACKTAB);   /*inicializar*/
  tft.setRotation(3); /*rodar o display*/
  tft.fillScreen(ST7735_BLACK);
  normal_display();
  set_wifi();
  set_socket();
}

void loop() {
  // put your main code here, to run repeatedly:
  read_msg(); /*esperar receber mensagem*/

  if(flag){   
    /*if this flag true, means recieved message from sys 1*/
    alert_display();
    beep();
    flag = false;
    start = millis();
  }
  
  if((millis()-start) >= max_delay && (start != 0)){
    /*beep longer after 5 seconds of alert*/
    start = 0;
    while(!buttonPressed()){
      beep2();
    }
  }
  
  if(buttonPressed()){
    start = 0;
    debug("Just bc...");
  }
}

void beep(){
  int i = 0;
  while((!buttonPressed()) && (i<8)){
    digitalWrite(BUZZER_PIN, HIGH);
    delay(75);
    digitalWrite(BUZZER_PIN, LOW);
    delay(75);
    i++;
  } 
}

void beep2(){
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  delay(75);
}

void alert_display(){
  clearDisplay();
  color = ST7735_RED;
  showText(0, 50, "Driver State:");
  showText(30, 70, "Drowsy :(");
}

void normal_display(){
  clearDisplay();
  color = ST7735_CYAN;
  showText(35, 25, "Driver's");
  showText(30, 45, "Attention");
  showText(20, 65, "Monitoring");
  showText(45, 85, "System");
}

bool buttonPressed(){
  currentState = digitalRead(BUTTON_PIN);
  if(currentState){
    Serial.println("The button is pressed");
    send_msg();
    normal_display();
    return true;
  } 
  return false;
}  

void showText(int x, int y, const char* text){
  /*!
    @param x: position of text in x-axis
    @param y: position of text in y-axis
    @param text: text to be printed to display
  */
   tft.setTextSize(size);
   tft.setTextColor(color);        
   tft.setCursor(x, y);            
   tft.println(text);              
}

void clearDisplay(){
  /*!
    @brief function to clear display
  */
  tft.fillScreen(ST7735_BLACK);
  delay(2000);
}

void set_wifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  debug("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void set_socket(){
  debug("setting socket");

  if (!client.connect(host, port)) {
    debug("Falha de conexao");
    delay(1000);
    return;
  }
 
  Serial.println("Socket set");
}

void read_msg(){
  char buf;
  while(client.available()){
    if( (buf = client.read()) != -1){
      Serial.print(buf);
      flag = true;
    }
  }
}

void send_msg(){
  client.write("Driver is okay...");
  debug("Disconnecting...");
}

void debug(const char* str){
  Serial.println(str);
}