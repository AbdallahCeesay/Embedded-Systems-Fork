#include "uop_msb.h"
#include <chrono>
#include <ratio>
using namespace uop_msb;
using namespace chrono;

//Count variable
int counter=0;

InterruptIn btnA(BTN1_PIN);
InterruptIn btnB(BTN2_PIN);
Ticker tick; // this is a timer interrupt that takes function as a argument and calls it at a specific rate

DigitalOut redLED(TRAF_RED1_PIN);       //Red Traffic 1
DigitalOut yellowLED(TRAF_YEL1_PIN);    //Yellow Traffic 1
DigitalOut greenLED(TRAF_GRN1_PIN);     //Green Traffic 1

//Dual Digit 7-segment Display
LatchedLED disp(LatchedLED::SEVEN_SEG);


void funcA()
{
    redLED = !redLED;
}

void funcTmr()
{
    greenLED = !greenLED;
}

void funcB ()
{
    yellowLED =!yellowLED;
}

int main()
{
    //Set up interrupts
    btnA.rise(&funcA);
    btnB.fall(&funcB);
    tick.attach(&funcTmr, 500ms); // this interrupt calls int the function funcTmr every 0.5s
    
    //Main loop - mostly sleeps :)
    while (true) {
        sleep(); // puts the CPU in low power mode

        printf("I have been woken %d times\n", ++counter); 
    }
} 







