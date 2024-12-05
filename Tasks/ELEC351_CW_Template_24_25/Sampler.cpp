/* 
* Filename: MySensorData
* Author: Abdallah Ceesay 10726858
*Module: ELEC 351 Advanced Embedded Programming
* Institution: Plymouth University
*Professor: Yu Yao. Honourable mention: Nickolas Outram, Andrew Norris
* Date: 25/11/24
* Description: method, constructor and Function definitions for this project
*/

#include "mbed.h"
#include "uop_msb.h"
#include <chrono>
#include <cstdio>
#include "Sensor_Sampler.hpp"

extern EnvSensor env;
extern AnalogIn ldr;

/*this constructor is to make sure that whenever an object of this class is created, it is set to the highest priority thread and sets the stack size*/

SensorSampler::SensorSampler() : samplingThread(osPriorityHigh, 1024) {
    /*sets the thread priority and also the stact size. */
    // note to self, don't forget to change the stack size to accommodate for more local vaiables
}

void SensorSampler::start() {
    samplingThread.start( callback(this, &SensorSampler::sampleData)); /*passing a reference to the function start using callback*/
}

void SensorSampler::sampleData() {
    
    while (true) {
        
        /*record the start time for this method. this will be used to account for sample jitter*/
        auto start = timer.elapsed_time();    /*I am not sure what type elapsed_time will return so I used auto to let the compiler decide that*/
        
        float temperature = env.getTemperature();
        float pressure = env.getPressure();

        float lightLevel = ldr.read();
        printf("Temperature: \t %.2f°C, Pressure: %.2fmbar \n, Light Level:  %.2f\n", temperature, pressure, lightLevel);

        /*this is to ensure that the sampling is done every 10 sec exactly. It measures the time is takes for the data to be sampled and subtracts that from the sleep time of the thread*/
        auto elapsed = timer.elapsed_time() - start;

        ThisThread::sleep_for(10s - std::chrono::duration_cast<std::chrono::seconds>(elapsed)); // signaling example 
        /*std::chrono::duration_cast<std::chrono::seconds>
        this is to make sure that both operands are of the same type */
    }
}