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
extern Thread producerThread;
extern Mutex mtx;


typedef struct {
    float temperature;
    float pressure;
    float lightLevel;

} Envdata_t;


/*this constructor is to make sure that whenever an object of this class is created, it is set to the highest priority thread and sets the stack size*/
SensorSampler::SensorSampler() : samplingThread(osPriorityHigh, 2048) {
    /*sets the thread priority and also the stact size. */
}

void SensorSampler::start_Sampling() {
    samplingThread.start( callback(this, &SensorSampler::sampleData)); /*passing a reference to the function start using callback*/
}

void SensorSampler::sampleData() {

    
    extern volatile Envdata_t data;
    
    while (true) {
        
        /* wait for sampling_ISR flag */
        ThisThread::flags_wait_any(2);
        ThisThread::flags_clear(2);

        /******** critical section begin ********/
        mtx.lock();

        /*read data*/
        data.temperature = env.getTemperature();
        data.pressure = env.getPressure();
        data.lightLevel = ldr.read();


        mtx.unlock();
        /******** critical section end ********/

        producerThread.flags_set(4); // set a flag to wake up the producer thread to take the sampled data
    }
    
}

