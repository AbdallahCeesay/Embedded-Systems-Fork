#include "uop_msb.h"
#include <chrono>
#include <ratio>
using namespace uop_msb;
using namespace chrono;

class Flashy {
    private:
    //Composition in action here
    DigitalOut led;
    Ticker tick;
    //Internal state
    microseconds _interval;
    PinName _pin;
    bool _enabled = false;

    void timerISR()  { // called periodically by the Ticker object. Toggles the LED between ON and OFF
        led = !led;
    }

    /*if en is true, the Ticker is attached to call timerISR() at the specified interval
     *if en is false, the Ticker is detached and the LED is turned OFF.*/
    void enable(bool en)
    {
        if (en == _enabled) return; // REDUNDANCY CHECK. if requested state (en) is the same as current state (_enable), the method does nothing and returns
        // this avoids unnecessary detachment and attachment of the Ticker

        if (en) {
            tick.attach(callback(this, &Flashy::timerISR), _interval);
        } else {
            tick.detach();
            led = 0;
        }
        _enabled = en;
    }

    public:
    /*constructor that initializes the LED on a given pin with a specific blinking interval*/
    Flashy(PinName pin, microseconds t) : led(pin,0), _pin(pin), _interval(t) { 
        enable(false);
    }
    //OVERLOAD constructor. This constructor defaults the blinking interval to 500ms if no interval is specified
    Flashy(PinName pin) : Flashy(pin, 500ms) {
    }

    /*Destructor; makes sure the LED stops blinking and is turned off when the object is destroyed*/
    ~Flashy() {
        enable(false);
        led = 0;
    }

    //OVERLOAD private enable
    void enable() {
        enable(true);
    }

    void disable() {
        enable(false);
    }

    //Setter and getter for _inteval
    void setInterval(microseconds t) {
        _interval = t;
        if (_enabled) {
            //Reenable to force update
            disable();
            enable();
        }
    }
    microseconds getInterval() {
        return _interval;
    }
};

DigitalIn blueButton(USER_BUTTON);

int main()
{
    /*we first configure the leds by setting the corresponding pins and what interval we want that led to flash*/
    Flashy flashRed(TRAF_RED1_PIN, 125ms);
    Flashy flashYellow(TRAF_YEL1_PIN, 250ms);
    Flashy flashGreen(TRAF_GRN1_PIN); // since no interval is specified, the overloaded constructor in the class sets the interval to 500ms. DEFAULT!

    while (true) {
        flashRed.enable();
        flashYellow.enable();
        flashGreen.enable();
        wait_us(5000000);              //5 seconds
        flashRed.disable();
        flashYellow.disable();
        flashGreen.disable();
        wait_us(5000000);              //5 seconds

        if (blueButton == 1) {
            flashGreen.setInterval(50ms);
        } else {
            flashGreen.setInterval(500ms);
        }
    }
}

