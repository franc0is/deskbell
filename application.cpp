#include "rest_client.h"

#define SERIAL_DEBUG // define or comment out

#define HOSTNAME "technobly.com"
uint32_t startTime = 0;
bool BUTTON_PRESSED = false;
#define DEBOUNCE_TIME 10 // number of 5ms ticks before button is debounced
uint8_t button_cnt = 0; // used to count up debounce time
int button_state = 0;
bool s = 0; // output state of D7 led

RestClient client = RestClient(HOSTNAME);
String response;

void setup() {
  startTime = millis(); // capture the time that our app starts
  pinMode(D0,INPUT_PULLUP);

#ifdef SERIAL_DEBUG
  Serial.begin(9600); // Make sure serial terminal is closed before powering up Core
  while(!Serial.available()) SPARK_WLAN_Loop(); // Open serial terminal now, and press ENTER
#endif
    
  pinMode(D7,OUTPUT);
}

void loop() {
    
  // Check if THEE BUTTON has been pressed
  if(BtnPressed()) {
    // Publish a value
    //Spark.publish("office-door-bell", "ding", 20);
        
    // Push notify this biatch.
    pushNotify("COOL TITLE","This is the message!");
        
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
