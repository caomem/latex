#include <Arduino_FreeRTOS.h>
void vTask1( void * pvParameters ){
  for( ;; ){
    Serial.println("*");
  }
}
void vTask2( void * pvParameters ){
  for( ;; ){
    Serial.println("+");
  }
}

void setup(){
  Serial.begin(115200);
  xTaskCreate( vTask1, "Periodic", 128, NULL, 1, NULL );
  xTaskCreate( vTask2, "Periodic", 128, NULL, 1, NULL );
}

void loop(){
}
