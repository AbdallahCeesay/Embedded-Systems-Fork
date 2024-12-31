
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

        Thread samplingThread;
        SensorSampler(); /*constructor*/
        void start_Sampling();
        

    private:
        Timer timer;

        void sampleData();
};


#endif