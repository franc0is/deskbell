// This #include statement was automatically added by the Spark IDE.
#include "LiquidCrystalSPI.h"

#include "rest_client.h"

//#define SERIAL_DEBUG // define or comment out

#define HOSTNAME "technobly.com"
uint32_t startTime = 0;
bool BUTTON_PRESSED = false;
#define DEBOUNCE_TIME 10 // number of 5ms ticks before button is debounced
uint8_t button_cnt = 0; // used to count up debounce time
int button_state = 0;
bool s = 0; // output state of D7 led
bool red_led = 0;
bool green_led = 0;
char lcd_message[32];
bool update_lcd = 0;

RestClient client = RestClient(HOSTNAME);
String response;

// Create an instance of the library for SOFTWARE SPI mode (define SS "latch" pin, SCLK pin, SDAT pin)
// These can be ANY of the A0 - A7 or D0 - D7 pins. 
// Just make sure you don't redefine them as some other peripheral later in your code.
LiquidCrystal lcd(D2, D3, D4);

void setup() {
  startTime = millis(); // capture the time that our app starts
  pinMode(D0,INPUT_PULLUP);

#ifdef SERIAL_DEBUG
  Serial.begin(9600); // Make sure serial terminal is closed before powering up Core
  while(!Serial.available()) SPARK_WLAN_Loop(); // Open serial terminal now, and press ENTER
#endif

  // initialize the SPI ( Must call this before begin()! )
  lcd.initSPI();
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Pebble Bell!");
    
  pinMode(D7,OUTPUT);
  pinMode(A0,OUTPUT);
  pinMode(A1,OUTPUT);
  digitalWrite(A0,LOW);
  digitalWrite(A1,LOW);
  
  //https://api.spark.io/v1/devices/53ff6a065067544838360587/led?access_token=0e1046822854d1a108bbe9c2ee54c1f63dc0b710&args=busy
  Spark.function("msg",updateBell);
}

void loop() {
    
  // Check if THEE BUTTON has been pressed
  if(BtnPressed()) {
    // Publish a value
    //Spark.publish("office-door-bell", "ding", 20);
        
    // Push notify this biatch.
    pushNotify("OFFICE","Hey Brett are you there?");
        
    // Do some flashy stuff
    /*
    RGB.control(true);
    for(uint8_t x=0; x<1; x++) { // Call the police!
      RGB.color(255,0,0); //red
      delay(200);
      RGB.color(0,0,255); //blue
      delay(200);
    }
    RGB.control(false);
    */
  }
    
  // Make it blinky to indicate Total World Domination in progress
  if(millis() - startTime > 100UL) {
    startTime = millis();
    s = !s;
    digitalWrite(D7,s);
  }
  
  if(update_lcd == 1) {
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(lcd_message);
    
    if(green_led == 1) {
        digitalWrite(A0,HIGH);
        green_led = 0;
    }
    else if(red_led == 1) {
        digitalWrite(A1,HIGH);
        red_led = 0;
    }
  
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    digitalWrite(A0,LOW);
    digitalWrite(A1,LOW);
    update_lcd = 0;
  }
}

bool BtnPressed() {
  // Process Inputs
  //-----------------------------------------
  // Read and debounce SAVE input
  // Only accumulate a SAVE "press" if save_state is inactive
  if(digitalRead(D0) == LOW && !BUTTON_PRESSED) {
    button_cnt++;
    if(button_cnt > DEBOUNCE_TIME) {
      button_cnt = DEBOUNCE_TIME;
      BUTTON_PRESSED = true; // BUTTON input is active
      button_state = 1;
      return 1;
    }
  }
  else if(digitalRead(D0) == HIGH && BUTTON_PRESSED) {
    if(button_cnt > 0) button_cnt--;
    else {
      BUTTON_PRESSED = false;
    }
  }
  return 0;
}

void pushNotify(const char* title, const char* message){
  response = "";
  
  // Build the query string
  char querystring[140] = "title=";
  strcat(querystring,title);
  strcat(querystring,"&message=");
  strcat(querystring,message);
  
#ifdef SERIAL_DEBUG
  Serial.print("Query string: ");
  Serial.println(querystring);
#endif
  
  // Build the security header
  char header[24] = "Referer: ";
  strcat(header,HOSTNAME); // "Referer: technobly.com"
  client.setHeader(header);
  
  // Push, push, in the George Bush.
  int statusCode = client.post("/pushover.php", querystring, &response);
  
#ifdef SERIAL_DEBUG
  Serial.print("Status code from server: ");
  Serial.println(statusCode);
  Serial.print("Response body from server: ");
  Serial.println(response);
#endif

  delay(1000);
}

int updateBell(String cmd) {
    // Grab the quick command
    if(cmd.substring(0,1) == "1") {
      green_led = 1;
    }
    else if(cmd.substring(0,1) == "2") {
      red_led = 1;
    }
    // Patch up and parse message beginning at second character
    cmd.replace("%20"," ");
    cmd.replace("%27","'");
    const char *cmd_ptr = cmd.c_str();
    strcpy(lcd_message, cmd_ptr + 1);
    update_lcd = 1;
    return 200;
}