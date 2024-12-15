/* 
* Filename: SensorSampler.cpp
* Author: Abdallah Ceesay 10726858
* Module: ELEC 351 Advanced Embedded Programming
* Institution: Plymouth University
* Professor: Yu Yao. Honourable mention: Nickolas Outram, Andrew Norris
* Date: 25/11/24
* Description: method, constructor and Function definitions for this project
*/

#include "mbed.h"
#include "uop_msb.h"
#include <chrono>
#include <cstdio>
#include "SensorSampler.hpp"

extern EnvSensor env;
extern AnalogIn ldr;

/*this constructor is to make sure that whenever an object of this class is created, it is set to the highest priority thread and sets the stack size*/

SensorSampler::SensorSampler() : samplingThread(osPriorityHigh, 1024) {
    /*sets the thread priority and also the stact size. */
    // note to self, don't forget to change the stack size to accommodate for more local vaiables
}

void SensorSampler::start_Sampling() {
    samplingThread.start( callback(this, &SensorSampler::sampleData)); /*passing a reference to the function start using callback*/
}

void SensorSampler::sampleData() {
    
    while (true) {


        // use signal wait mechanism //
        // 1 create ISR
        // 2 create timer/ticker
        // 3 attach the timer to the ISR every x seconds
        // 4 ISR sends signal/flag to this sampling thread
        // 5 ThisThread::flags_wait_any(flag value here from ISR)
        // 6 unblocks when receiving flag
        // 6 clear the flags - these can queue up (ThisThread::flags_clear(flag value))

        // remove the chrono stuff - timing is handled by ISR and timer!!!
        
        
        float temperature = env.getTemperature();
        float pressure = env.getPressure();

        float lightLevel = ldr.read();
        printf("Temperature: \t %.2f°C, Pressure: %.2fmbar \n, Light Level:  %.2f\n", temperature, pressure, lightLevel);

        // re-enable timer interrupt here
        // tmr.attach(...)
        
    }
