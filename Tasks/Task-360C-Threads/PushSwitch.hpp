#ifndef __PUSHSWITCH_HPP__
#define __PUSHSWITCH_HPP__
#include "mbed.h"

class PushSwitch {

private:
    typedef enum {RISING_EVENT=1, FALLING_EVENT=2} SWITCH_EVENT;
    InterruptIn switchInterrupt;
    osThreadId_t threadID;

    /* these methods are called when a switched is pressed */
    /* and they are passed by reference to their corresponding methods that are public. each method also has it's own thread ID */
    void switchPressed() {
        switchInterrupt.rise(NULL);
        osSignalSet(threadID, RISING_EVENT);
    }
    void switchReleased() {
        switchInterrupt.fall(NULL);
        osSignalSet(threadID, FALLING_EVENT);
    }
    void switchChanged() {
        switchInterrupt.rise(NULL);
        switchInterrupt.fall(NULL);
        osSignalSet(threadID, RISING_EVENT | FALLING_EVENT);
    }
    
public:
/*when you create an object of this calss,
1.  Initialize the object with the pin name, in this case an INTERRUPT
2.  Get the thread ID for the object in order to know what thread this object is running in and what thead to INTERRUPT when the interrupt is called*/
    PushSwitch(PinName pin) : switchInterrupt(pin) 
    {
       threadID = ThisThread::get_id(); 
    }
    /*Destructor then sets the value for the rise (1) and fall (2) to Zero (0)*/
    ~PushSwitch() {
        switchInterrupt.rise(NULL);
        switchInterrupt.fall(NULL);
    }

    void waitForPress() {       
        switchInterrupt.rise(callback(this, &PushSwitch::switchPressed)); /*callback used to get the address of a member function*/
        ThisThread::flags_wait_any(RISING_EVENT);    /* this is blocking. WAITING for a press. From the way RISING_EVENT has been set in the struct, it seems
        line that the ::flags-wait_any takes only integers.
        Now that I have confirmed, it does take in 32 bit integers*/
    }

    void waitForRelease() {       
        switchInterrupt.fall(callback(this, &PushSwitch::switchReleased));
        ThisThread::flags_wait_any(FALLING_EVENT);      
    }

    operator int() {
        switchInterrupt.rise(callback(this, &PushSwitch::switchChanged));
        switchInterrupt.fall(callback(this, &PushSwitch::switchChanged));
        ThisThread::flags_wait_any(RISING_EVENT | FALLING_EVENT); 
        return switchInterrupt.read();
    }
};
#endif