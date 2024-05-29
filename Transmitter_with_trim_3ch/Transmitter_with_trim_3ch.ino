// 2 Channel Transmitter & Trims 
  #include <esp_now.h>
  #include <WiFi.h>
  #include <EEPROM.h> 

  // define the number of bytes you want to access
  #define EEPROM_SIZE 3

  // REPLACE WITH YOUR RECEIVER MAC Address
  uint8_t receiverMacAddress[] = {0x08,0xd1,0xf9,0x55,0x4d,0x18};  //08:d1:f9:55:4d:18
    
 #define trimbut_1 19                      // Trim button 1 / Pin 19
 #define trimbut_2 18                      // Trim button 2 / Pin 18 
 #define trimbut_3 23                      // Trim button 1 / Pin 23
 #define trimbut_4 22                      // Trim button 2 / Pin 22 
 
 int tvalue1 = EEPROM.read(0) * 16;        // Reading trim values from Eprom
 int tvalue2 = EEPROM.read(2) * 16;        // Reading trim values from Eprom

 int throttle_offset = 0;                  //throttle offset
         
 typedef struct PacketData{
  byte throttle;
  byte steering;
  byte knob1; 
};
 PacketData data;

 esp_now_peer_info_t peerInfo;

  void ResetData() 
{
  data.throttle = 127;                      // Signal lost position 
  data.steering = 127;
  data.knob1 = 127;
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t ");
  Serial.println(status);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Message sent" : "Message failed");
}

  void setup()
{
  // Initializing Serial Monitor 
  Serial.begin(115200);
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  WiFi.mode(WIFI_STA);

  // Initializing ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else
  {
    Serial.println("Succes: Initialized ESP-NOW");
  }
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  else
  {
    Serial.println("Succes: Added peer");
  } 
  ResetData();
 
  pinMode(trimbut_1, INPUT_PULLUP); 
  pinMode(trimbut_2, INPUT_PULLUP);
  pinMode(trimbut_3, INPUT_PULLUP); 
  pinMode(trimbut_4, INPUT_PULLUP);
  tvalue1= EEPROM.read(0) * 16;
  tvalue2= EEPROM.read(2) * 16;
}
// Joystick center and its borders 
  int Border_Map(int val, int lower, int middle, int upper, bool reverse)
{
  val = constrain(val, lower, upper);
  if ( val < middle )
  val = map(val, lower, middle, 0, 128);
  else
  val = map(val, middle, upper, 128, 255);
  return ( reverse ? 255 - val : val );
}
  void loop()
{
// Trims and Limiting trim values 
  if(digitalRead(trimbut_1)==LOW and tvalue1 < 2520) {
    tvalue1=tvalue1+60;
    EEPROM.write(0,tvalue1/16);
    EEPROM.commit(); 
    delay (130);
  }   
  if(digitalRead(trimbut_2)==LOW and tvalue1 > 1120){
    tvalue1=tvalue1-60;
    EEPROM.write(0,tvalue1/16);
    EEPROM.commit();
    delay (130);
  }

  if(digitalRead(trimbut_3)==LOW and tvalue2 < 2520) {
    tvalue2=tvalue2+60;
    EEPROM.write(2,tvalue2/16);
    EEPROM.commit(); 
    delay (130);
  }   
  if(digitalRead(trimbut_4)==LOW and tvalue2 > 1120){
    tvalue2=tvalue2-60;
    EEPROM.write(2,tvalue2/16);
    EEPROM.commit();
    delay (130);
  }
// // Throttle offset for center
//   if(analogRead(32) > 1775 && analogRead(32) < 1975){
//   data.throttle = 128;
//   }
//   else{
//     data.throttle = Border_Map( analogRead(32),0, 1875, 4095, true );   // For Bidirectional ESC  

//   }
  
// Control Stick Calibration for channels         
         
  //data.throttle = Border_Map( analogRead(32),2280, 3200, 4095, false );    // For Single side ESC 
  data.throttle = Border_Map( analogRead(32),0,tvalue1, 4095, true );   // For Bidirectional ESC
  data.steering = Border_Map( analogRead(33), 1220, tvalue2, 2735, true );     // "true" or "false" for signal direction // Center -- 1945
  data.knob1 = Border_Map( analogRead(34), 0, 2047, 4095, false );     // "true" or "false" for signal direction  
  
  // // Default value of throttle in this range
  // if(analogRead(32) > 1600 && analogRead(32) < 2100){
  //   data.throttle = 129;
  // }
  // else{
  //   data.throttle = Border_Map( analogRead(32),0, 1875, 4095, true );   // For Bidirectional ESC  

  // }
   
  esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &data, sizeof(data));
  if (result == ESP_OK) 
  {
    Serial.println("Sent with success");
  }
  else 
  {
    Serial.println("Error sending the data");
  }
  Serial.println(EEPROM.read(0));
  Serial.println(EEPROM.read(2));     
  
  delay(50);
}