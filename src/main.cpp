/******************************************************************
                        LoLin32 html slider
                                                қuran march 2024
******************************************************************/
#include <Arduino.h>
#include <stdio.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "serviceSetIdentifier.h"

#define TRUE                            1
#define FALSE                           0
#define LEN                             21
#define WAIT_ONE_SEC                    10000
#define WAIT_250_MSEC                   2500
#define WAIT_10_MSEC                    100
#define ON_BOARD_LED                    5
#define ON_BOARD_LED_ON                 0
#define ON_BOARD_LED_OFF                1


#define WHEEL_L                         2
#define WHEEL_R                         A4
#define WHEEL_L_DIRECTION               15 
#define WHEEL_R_DIRECTION               A5
#define BATTERY_LEVEL                   A3      // GPIO 39

// -------  global Variables --------------------------------------

volatile int oneSecFlag, qSecFlag, tenMSecFlag;
volatile int vL, vR;
volatile int LDir, RDir;

hw_timer_t *timer = NULL;
void IRAM_ATTR myTimer(void);
volatile int startWiFi = 0;


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const int led5 = ON_BOARD_LED; 
bool ledState = 1;

void initSPIFFS()
{
    if (!SPIFFS.begin(true))  Serial.println("An error has occurred while mounting SPIFFS");
    else                      Serial.println("SPIFFS mounted successfully!");
}

void initWiFi()
{
    char text[LEN];
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("Connection to WiFi . . .");
    while ((WiFi.status() != WL_CONNECTED) && (startWiFi < 21))
    {
        delay(1000);   
        printf(" .");  
        startWiFi++;
    }

    uint32_t ip = (uint32_t) WiFi.localIP();
    sprintf(text, "%u.%u.%u.%u", ip & 0xFF, (ip>>8) & 0xFF, (ip>>16) & 0xFF, (ip>>24) & 0xFF );
    printf("\nIP: %s\n", text);
}

String processor(const String& var)
{
    String ret = "";

    printf("processor: %s:\n", var);

    if (var == "STATE") 
    {
        if (digitalRead(led5) == ON_BOARD_LED_ON)
        {
            printf("-ON-");
            ledState = 1; ret = "-ON-";

        }
        else
        {
            printf("-OFF-");
            ledState = 0; ret = "-OFF-"; // wird beim ersten Druchlauf ausgegeben... 
        }
    }

    printf("\n------------------------\n");

    return ret;
}

void notifyClients(String state)
{
    printf("notifyClients: %s\n", state);
    ws.textAll(state);
}

void handleWebSocketMessage(void *arg, uint8_t * data, size_t len)
{
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String sliderValue;
    String val;
    int length;

    if (info->final && info->index == 0 && info->len && info->opcode == WS_TEXT)
    {
        data[len] = 0;

        if (strcmp((char*)data, "bON") == 0)
        {
            ledState = 1;
            notifyClients("ON!!!"); // unter laufenden Betrieb ausgegeben
            printf("handleWebSocketMessage: on\n");

        }
        else if (strcmp((char*)data, "bOFF") == 0)
        {
            ledState = 0;
            notifyClients("OFF");
            printf("handleWebSocketMessage: off\n");
        }
        else if(strncmp((char*)data, "sLa", 3) == 0)
        {
            sliderValue = (const char*)data;
            length = sliderValue.length();
            val = sliderValue.substring(3, length);
            vL = val.toInt();
            printf("%s\n", sliderValue);
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient * client, AwsEventType type, 
             void * arg, uint8_t * data, size_t len)
{
    switch(type)
    {
        case WS_EVT_CONNECT: 
             printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;

        case WS_EVT_DISCONNECT:
             printf("WebSocket client #%u disconnected\n", client->id());
        break;

        case WS_EVT_DATA:
             handleWebSocketMessage(arg, data, len);
        break;

        case WS_EVT_PONG: 
        case WS_EVT_ERROR:
        break;
    }
}

void initWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void setup() 
{
    Serial.begin(115200);
    printf("start!");

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &myTimer, true);
    timerAlarmWrite(timer, 100, true);  // 0.1 msec
    timerAlarmEnable(timer);
    
    oneSecFlag = qSecFlag = tenMSecFlag = FALSE; 

    pinMode(ON_BOARD_LED, OUTPUT);
    pinMode(WHEEL_L, OUTPUT);
    pinMode(WHEEL_R, OUTPUT);
    pinMode(WHEEL_L_DIRECTION, OUTPUT);
    pinMode(WHEEL_R_DIRECTION, OUTPUT);
   
    digitalWrite(ON_BOARD_LED, LOW); // on ! ... blue 
    digitalWrite(WHEEL_L_DIRECTION, LOW );
    digitalWrite(WHEEL_R_DIRECTION, HIGH);
    digitalWrite(WHEEL_L, LOW); // stop !
    digitalWrite(WHEEL_R, LOW); // stop !
  
    LDir = 0; 
    RDir = 1;
    vL = vR = 0;

    initSPIFFS();
    initWiFi();
    initWebSocket();

    server.on("/", HTTP_GET, 
    [](AsyncWebServerRequest * request) {
        request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });

    server.serveStatic("/", SPIFFS, "/");
    
    server.on("/logo", HTTP_GET, 
    [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/logo.png", "image/png");
    } );

    // Start server:
    server.begin();
}

void loop() 
{
    ws.cleanupClients();

    if (oneSecFlag == TRUE)  // all Seconds 
    {
        oneSecFlag = FALSE;

        if (ledState == 1) digitalWrite(ON_BOARD_LED, ON_BOARD_LED_ON);  // on = LOW! 
        else               digitalWrite(ON_BOARD_LED, ON_BOARD_LED_OFF); // off

    }

    if (qSecFlag)  // all 250 msec ... 
    {
        qSecFlag = FALSE;

    }
    
    if (tenMSecFlag)
    {
        tenMSecFlag = FALSE;
    }

}    

//******************************   Timer Interrupt:   **************************************

void IRAM_ATTR myTimer(void)   // periodic timer interrupt, expires each 0.1 msec
{
    static int32_t otick  = 0;
    static int32_t qtick = 0;
    static int32_t mtick = 0;
    static unsigned char ramp = 0;
    
    otick++;
    qtick++;
    mtick++;
    ramp++;

    if (otick >= WAIT_ONE_SEC) 
    {
        oneSecFlag = TRUE;
        otick = 0; 
    }

    if (qtick >= WAIT_250_MSEC) 
    {
        qSecFlag = TRUE;
        qtick = 0; 
    }

    if (mtick >= WAIT_10_MSEC) 
    {
        tenMSecFlag = TRUE;
        mtick = 0; 
    }

    // PWM:

    if (ramp > vL) digitalWrite(WHEEL_L, LOW);  else digitalWrite(WHEEL_L, HIGH);
    if (ramp > vR) digitalWrite(WHEEL_R, LOW);  else digitalWrite(WHEEL_R, HIGH);

}

