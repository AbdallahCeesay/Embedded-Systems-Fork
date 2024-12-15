
/* 
* Filename: SensorSampler.hpp
* Author: Abdallah Ceesay 10726858
* Module: ELEC 351 Advanced Embedded Programming
* Institution: Plymouth University
* Professor: Yu Yao. Honourable mention: Nickolas Outram, Andrew Norris
* Date: 25/11/24
* Description: Class and Function declaration
*/


#ifndef SensorSampler_HPP
#define SensorSampler_HPP

#include "mbed.h"
#include <chrono>

class SensorSampler {
    public:
        /* constructor of this class */
        SensorSampler();

        /*method to start the sampling thread - this is done every 10 secs*/
        void start_Sampling();

    /*using encapsulation here, the implementation of the sampling of the sensors is hidden from the end user */
    private:
        Thread samplingThread;
        Timer timer;

        /*method for sampling the data. This is what reads from the senor and the LDR at a set sample rate*/
        void sampleData();
};


#endif