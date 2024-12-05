/******************************************************************************
 * File:        main.cpp
 * Author:      Andrew Norris
 * Institution: Plymouth University
 * Date:        25-11-2024
 * Description: Example of how to use member functions with threads and queues 
 *
 *              To start a thread or add an event to a queue with just a simple function we can use the following syntax.
                thread.start(&myfunc);
                queue.call(&myfunc,args);
                However if this function that we wish to run is a member of a class, things are quite so simple. as the call just passes a function pointer
                When a member function is called, it is required to be bound to an instance of the class. Therefore when we want to add the function to an eventqueue
                we need to bind it to the instance of the class we wish to use.
                We do this using Mbed's Callback mechanism.
                The callback requires the instance of the class and the function poiunter to the member function.
                eg callback(&myinstance, MyClass::MemberFunction).
                This allows as to create a callable object which can be passed around.
                When creating a callable object from within the class itself, the instance is likely to be "this" (see MySecondClass)

******************************************************************************/


#include "mbed.h"


class MyFirstClass{
    private:
        DigitalOut _led;
    public:
        MyFirstClass(PinName led_pin) : _led(led_pin){

        } 

        void flip_led(){
            _led = !_led;
        }
        
        void flipper_thread(){
            while(1){
                flip_led();
                ThisThread::sleep_for(200ms);
            }
        }

};

class MySecondClass{
    private:
        DigitalOut _led;
        Thread _thr;        // This class has a private thread
    public:
        MySecondClass(PinName led_pin) : _led(led_pin){
            // Here we start the thread from within the class itself
            // We wish to use flipper_thread member function so a callback is used to 
            // bind the function to the instance "this" of the class
            _thr.start(callback(this,&MySecondClass::flipper_thread));
        } 

        void flip_led(){
            _led = !_led;
        }

        void flipper_thread(){
            while(1){
                flip_led();
                ThisThread::sleep_for(200ms);
            }
        }

};

// Instances of classes
MyFirstClass mfc(LED1);     // LED is toggled using th1 thread in main
MyFirstClass mfc2(LED2);    // LED is toggled using event queue in main
MySecondClass msc(LED3);    // LED is toggled using a thread within the class


// Threads and queues
Thread th1;
Thread th2;
EventQueue queue;


int main()
{

    // Start a thread from main with a member function
    th1.start(callback(&mfc,&MyFirstClass::flipper_thread));
    // Start an event queue in its own thread(calls "dispatch_forever" memeber from "EventQueue" class with instance "queue")
    th2.start(callback(&queue, &EventQueue::dispatch_forever));
    // Add a memember function from MyFirstClass every 500ms
    queue.call_every(500ms,callback(&mfc2,&MyFirstClass::flip_led));

    while (true) {
        ThisThread::sleep_for(osWaitForever);
    }
}

