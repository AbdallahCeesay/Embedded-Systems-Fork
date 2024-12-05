/******************************************************************************
 * File:        main.cpp
 * Author:      Andrew Norris
 * Institution: Plymouth University
 * Date:        25-11-2024
 * Description: Examples of how to dispatch an Event Queue
 *
 *              To dispatch an instance of an EventQueue we can call the dispatch_forever()
 *              member function. However his will take over the thread in which it was called.
 *              IE no other code past this call will run.
 *              Instead it usually makes more sense to run the event queue in a seperate thread
 *              This example shows different ways of achieving this

******************************************************************************/


#include "mbed.h"


EventQueue queue1;
Thread queue1_thread;
DigitalOut led1(LED1);

EventQueue queue2;
Thread queue2_thread;
DigitalOut led2(LED2);

EventQueue queue3;
DigitalOut led3(LED3);


void flip_led1(){
    led1 = !led1;
}

void flip_led2(){
    led2 = !led2;
}
void flip_led3(){
    led3 = !led3;
}

void queue1_starter(){
    queue1.dispatch_forever();
}


int main()
{

    // Here we start a thread which dispatches the thread
    queue1_thread.start(&queue1_starter);
    queue1.call_every(500ms,&flip_led1);

    // This does the same thing but in one line(starts a thread to run the dispatch_forever function from EventQueue class using instance queue2)
    queue2_thread.start(callback(&queue2,&EventQueue::dispatch_forever));
    queue2.call_every(500ms,&flip_led2);


    // This will dispatch queue3 in the context of main and nothing beyond this point will run
    queue3.dispatch_forever();

    printf("Notice that this never prints and LED3 never blinks\n");
    queue3.call_every(500ms,&flip_led3);

}

