/******************************************************************************
 * File:        main.cpp
 * Author:      Andrew Norris
 * Institution: Plymouth University
 * Date:        25-11-2024
 * Description: Example to demonstrate how to notify a thread from an ISR (or another thread)
 * 
 *              This example uses 2 thread which both just flash an LED
 *              The timings of both are controlled by an Mbed ticker
 *              One thread blocks waiting for a semaphore, the other waits for an RTOS flag
 *              Both threads block in the WAITING state which means they sleep until the signal is given
 *              This means that even though redThread is high priority, greenThread is still allowed CPU time when redThread sleeps
 *              If the blue button is pressed, the sempahore is not released and the flag is not set, this will
 *              keep both threads blocking and the LED state will not change.

 *              Using a timer to signal a thread allows us to use the accuracy of a timer to ensure our real-time tasks
 *              occur at a fixed and deterministic rate. Whist allowing the thread to do the heavy lifting and performing operations
 *              which are not allowed in ISRs (printf, I2C,SPI, etc)

******************************************************************************/

#include "mbed.h"

DigitalOut redLED(LED3);
DigitalOut greenLED(LED1);
DigitalIn din (PC_13);

Ticker redTicker;
Ticker greenTicker;

Semaphore redSem;

Thread redThread(osPriorityHigh7);
Thread greenThread;


void redLEDThread(){
    while(1){
        redSem.acquire();
        redLED = !redLED;
    }
}

void greenLEDThread(){
    while(1){
        ThisThread::flags_wait_any(1);
        greenLED = !greenLED;
    }
}

void red_ticker_isr(){
    if(!din){
        redSem.release();
    }
}


void green_ticker_isr(){
    if(!din){
        greenThread.flags_set(1);
    }
}


int main()
{
    redThread.start(redLEDThread);
    greenThread.start(greenLEDThread);

    redTicker.attach_us(&red_ticker_isr, 1000000);
    greenTicker.attach_us(&green_ticker_isr, 500000);

    while (true) {

    }
}

