/******************************************************************************
 * File:        main.cpp
 * Author:      Andrew Norris
 * Institution: Plymouth University
 * Date:        25-11-2024
 * Description: Example of how to use an Mbed EventQueue to serialise asychronous tasks
 * 
 *              This example shows how Eventqueues can be used to ensure that tasks are executed sequentially
 *              This protects against race conditions and data corruption as it prevents preemption when reading
 *              and writing from different threads.
 *              4 threads are used including main, which all print data to the terminal. This means that they are all 
 *              fighting for control of a shared resource (our serial port).
 *              When the code is run, initially there is no attempt to protect this shared resource and the data is 
 *              often incoherent or garbled.
 *              Pressing the blue button toggles a flag which informs the program to use the event queue.
 *              When the queue is used, each thread places its data into the event queue. The queue will ensure that
 *              each print job is complete before commencing the next and the data is printed clearly.

******************************************************************************/


#include "mbed.h"



EventQueue queue;           // The Event Queue we will be using
Thread queue_thread;        // A thread for the queue to run in
bool use_queue = false;     // Boolean variable to set whether we just print, or use the queue

// Threads which will be printing info
Thread th1(osPriorityHigh2);
Thread th2;
Thread th3(osPriorityHigh3);

// Other objects
UnbufferedSerial serial(USBTX,USBRX,9600);
InterruptIn interrupt_in(BUTTON1);

// Function Prototypes
void interrupt_in_isr();
void debounce_ticker_isr();

// The function which all threds will call to print data
void print_text(char* txt,int len){
    if(use_queue){
        queue.call(callback(&serial,&UnbufferedSerial::write),txt,len);       
    }
    else{
        serial.write(txt,len);
    }
}

// Example of threads doing concurrent work
void thread1(){
    char th1_txt[32] = "Hello from thread 1\n";
    while(1){
        print_text(th1_txt,sizeof(th1_txt));
        ThisThread::sleep_for(500ms);
    }
}

void thread2(){
    char th2_txt[32] = "Hello from thread 2\n";
    while(1){
        print_text(th2_txt,sizeof(th2_txt));
        ThisThread::sleep_for(500ms);
    }
}

void thread3(){
    char th3_txt[32] = "Hello from thread 3\n";
    while(1){
        print_text(th3_txt,sizeof(th3_txt));
        ThisThread::sleep_for(500ms);
    }
}


// Ticker object to debounce the interrupt in
Ticker debounce_ticker;


void debounce_ticker_isr(){
    interrupt_in.rise(interrupt_in_isr);
}
// The blue button interrupt isr (inverts the state of bool use_queue)
void interrupt_in_isr(){
    use_queue = !use_queue;
    interrupt_in.rise(NULL);
    debounce_ticker.attach_us(&debounce_ticker_isr,500000);
}


 

int main()
{
    // Start the threads
    th1.start(thread1);
    th2.start(thread2);
    th3.start(thread3);

    // Start the eventqueue in it's own thread
    queue_thread.start(callback(&queue, &EventQueue::dispatch_forever));

    // Enable the blue button interrupt
    interrupt_in.rise(interrupt_in_isr);

    // Main thread also prints to the terminal
    char main_txt [32] = "Hello from main thread\n";
    while (true) {
        print_text(main_txt,sizeof(main_txt));
        ThisThread::sleep_for(200ms);
    }
}

