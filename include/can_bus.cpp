#include <Arduino.h>

#include <ESP32-TWAI-CAN.hpp>

#define CAN_RX_1 4
#define CAN_TX_1 5
#define CAN_RX_2 34 
#define CAN_TX_2 35
#define CAN_RX_3 18
#define CAN_TX_3 19
#define CAN_RX_4 21
#define CAN_TX_4 22
#define CAN_RX_5 25
#define CAN_TX_5 26

bool enable_can_1 = false;
bool enable_can_2 = false;
bool enable_can_3 = false;
bool enable_can_4 = false;
bool enable_can_5 = false;

bool shutdown_flag = false;

TwaiCAN can_1;
TwaiCAN can_2;
TwaiCAN can_3;
TwaiCAN can_4;
TwaiCAN can_5;

void enable_can(int can_number)
{
    switch(can_number)
    {
        case 1:
            enable_can_1 = true;
            break;
        case 2:
            enable_can_2 = true;
            break;
        case 3:
            enable_can_3 = true;
            break;
        case 4:
            enable_can_4 = true;
            break;
        case 5:
            enable_can_5 = true;
            break;
        default:
            break;
    }
}

void disable_can(int can_number)
{
    switch(can_number)
    {
        case 1:
            enable_can_1 = false;
            break;
        case 2:
            enable_can_2 = false;
            break;
        case 3:
            enable_can_3 = false;
            break;
        case 4:
            enable_can_4 = false;
            break;
        case 5:
            enable_can_5 = false;
            break;
        default:
            break;
    }
}

void disable_all_can()
{
    enable_can_1 = false;
    enable_can_2 = false;
    enable_can_3 = false;
    enable_can_4 = false;
    enable_can_5 = false;
}

void initialize_can()
{
    if(enable_can_1)
    {
        can_1.setPins(CAN_TX_1, CAN_RX_1);
        can_1.setRxQueueSize(5);
        can_1.setTxQueueSize(5);
        can_1.setSpeed(can_1.convertSpeed(128));
        while(!can_1.begin())
        {
            Serial.println("CAN_1 bus failed!");
            delay(1000);
        }

        if(can_1.begin())
        {
            Serial.println("CAN_1 bus started!");
        } else {
            Serial.println("CAN_1 bus failed! could not begin");
        }
    }

    if(enable_can_2)
    {
        can_2.setPins(CAN_TX_2, CAN_RX_2);
        can_2.setRxQueueSize(5);
        can_2.setTxQueueSize(5);
        can_2.setSpeed(can_2.convertSpeed(128));
        while(!can_2.begin())
        {
            Serial.println("CAN_2 bus failed!");
            delay(1000);
        }
        if(can_2.begin())
        {
            Serial.println("CAN_2 bus started!");
        } else {
            Serial.println("CAN_2 bus failed!");
        }
    }

    if(enable_can_3)
    {
        can_3.setPins(CAN_TX_3, CAN_RX_3);
        can_3.setRxQueueSize(5);
        can_3.setTxQueueSize(5);
        can_3.setSpeed(can_3.convertSpeed(128));
        while(!can_3.begin())
        {
            Serial.println("CAN_3 bus failed!");
            delay(1000);
        }
        if(can_3.begin())
        {
            Serial.println("CAN_3 bus started!");
        } else {
            Serial.println("CAN_3 bus failed!");
        }
    }

    if(enable_can_4)
    {
        can_4.setPins(CAN_TX_4, CAN_RX_4);
        can_4.setRxQueueSize(5);
        can_4.setTxQueueSize(5);
        can_4.setSpeed(can_4.convertSpeed(128));
        while(!can_4.begin())
        {
            Serial.println("CAN_4 bus failed!");
            delay(1000);
        }
        if(can_4.begin())
        {
            Serial.println("CAN_4 bus started!");
        } else {
            Serial.println("CAN_4 bus failed!");
        }
    }

    if(enable_can_5)
    {
        can_5.setPins(CAN_TX_5, CAN_RX_5);
        can_5.setRxQueueSize(5);
        can_5.setTxQueueSize(5);
        can_5.setSpeed(can_5.convertSpeed(128));
        while(!can_5.begin())
        {
            Serial.println("CAN_5 bus failed!");
            delay(1000);
        }
        if(can_5.begin())
        {
            Serial.println("CAN_5 bus started!");
        } else {
            Serial.println("CAN_5 bus failed!");
        }
    }
}

void shutdown()
{
    shutdown_flag = true;
}

void turn_on()
{
    shutdown_flag = false;
}

bool is_can_enabled(TwaiCAN *can)
{
    if(can->begin())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void send_message_can(CanFrame *frame)
{
    if(shutdown_flag)
    {
        Serial.println("CAN bus is disabled by shutdown flag");
        return;
    }
    if(enable_can_1)
    {
        is_can_enabled(&can_1);
        can_1.writeFrame(frame);
    }

    if(enable_can_2)
    {
        is_can_enabled(&can_2);
        can_2.writeFrame(frame);
    }

    if(enable_can_3)
    {
        is_can_enabled(&can_3);
        can_3.writeFrame(frame);
    }

    if(enable_can_4)
    {
        is_can_enabled(&can_4);
        can_4.writeFrame(frame);
    }

    if(enable_can_5)
    {
        is_can_enabled(&can_5);
        can_5.writeFrame(frame);
    }
    
    delay(500);
}