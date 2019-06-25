#include <Arduino_FreeRTOS.h>
#include "U8glib.h"
#include "queue.h"

void Update( void *pvParameters );
void UpdateBullet( void *pvParameters );
void Draw( void *pvParameters );
QueueHandle_t mutex, queue;
struct Data
{
   float x, y;
   int id;
};


U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
 const uint8_t bullet[] U8G_PROGMEM = {
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x3c, 0x00, 
  0xff, 0xff, 0xc0, 
  0xff, 0xff, 0xc0, 
  0x00, 0x3c, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00,    
};

const uint8_t smile[] U8G_PROGMEM = {
  0x07, 0xfe, 0x00, 
  0x1e, 0x07, 0x80, 
  0x3c, 0x03, 0xc0, 
  0x70, 0x00, 0xe0, 
  0x60, 0x00, 0x60, 
  0xe0, 0x00, 0x70, 
  0xc6, 0x06, 0x30, 
  0x87, 0x0e, 0x10, 
  0x86, 0x06, 0x10, 
  0x80, 0x00, 0x10, 
  0x80, 0x00, 0x10, 
  0x88, 0x01, 0x10, 
  0x8c, 0x03, 0x10, 
  0xce, 0x07, 0x30, 
  0xe7, 0x0e, 0x70, 
  0x63, 0xfc, 0x60, 
  0x70, 0x00, 0xe0, 
  0x3c, 0x03, 0xc0, 
  0x1e, 0x07, 0x80, 
  0x07, 0xfe, 0x00,  
};

void setup() {
  mutex = xQueueCreate( 1, sizeof(int) );
  if(mutex == 0) return;
  queue = xQueueCreate(10, sizeof(struct Data*));
  if(queue == 0) return;
  
  // Now set up two Tasks to run independently.
  xTaskCreate(
    Update
    ,  (const portCHAR *)"Produtor"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    UpdateBullet
    ,  (const portCHAR *)"Produtor"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );


  xTaskCreate(
    Draw
    ,  (const portCHAR *) "Consumidor"
    ,  128  // Stack size
    ,  NULL
    ,  3  // Priority
    ,  NULL );
}

void loop()
{
  // Empty. Things are done in Tasks.
}

void UpdateBullet( void *pvParameters __attribute__((unused)) )
{
  /*
    UpdateBullet
    Wait for a mutex to fire the bullet 
  */
  int count = 0;
  for (;;){
    if (xQueueReceive(mutex, NULL, (TickType_t)10) == pdTRUE){ //bullet walk
      count+=5;
      struct Data data, *pdata; 
      data.x = ((float)analogRead(0)/1024)*64-4;
      data.y = ((float)analogRead(1)/1024)*128+count;
      data.id = 2;
      
      pdata = &data;
      // try send or block
      while(xQueueSend(queue, (void*)&pdata, (TickType_t)0) == errQUEUE_FULL) vTaskDelay(1);
    }else count = 0;
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void Update( void *pvParameters __attribute__((unused)) )
{
  /*
    Update
    Reads a digital input on pin 2 for fire the bullet 
    and reads analogical inputs A0 and A1 for control of smiles position
  */
  uint8_t button = 2;

  // make the button's pin an input:
  pinMode(button, INPUT_PULLUP);

  for (;;)
  {
    if (!digitalRead(button)){ 
      //bullet walk
      xQueueSend(mutex, NULL, (TickType_t)0);
    }
    //smile :)
    struct Data data, *pdata; 
    data.x = ((float)analogRead(0)/1024)*64-4;
    data.y = ((float)analogRead(1)/1024)*128-4;
    data.id = 1;
    
    pdata = &data;
    // try send or block
    while(xQueueSend(queue, (void*)&pdata, (TickType_t)0) == errQUEUE_FULL) vTaskDelay(1);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void Draw( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    Draw
    Reads the buffer of images description in queue and draw it in the display
  */
  // picture loop
  u8g.firstPage();
  do{
    struct Data *data;
    // try receive or block
    while (xQueueReceive(queue, &data, (TickType_t)10) == pdFALSE) vTaskDelay(1);
    
    if(data->id == 1)
      u8g.drawBitmapP(data->y, data->x, 3, 20, smile);
    else if(data->id == 2){
      u8g.drawBitmapP(data->y, data->x, 3, 10, bullet);
    }
    
    vTaskDelay(1); 
  } while(u8g.nextPage());
}
