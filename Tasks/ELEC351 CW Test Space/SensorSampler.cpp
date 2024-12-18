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

/* Constructor */
SensorSampler::SensorSampler() : samplingThread(osPriorityHigh, 2048) {
    /* Sets the thread priority and also the stack size */
}

void SensorSampler::start_Sampling() {
    samplingThread.start(callback(this, &SensorSampler::sampleData)); /* Pass a reference to the function start using callback */
}

void SensorSampler::sampleData() {
    extern volatile Envdata_t data;

    while (true) {
        // Wait for sampling_ISR flag
        uint32_t flags = ThisThread::flags_wait_any(2 | 4); // Wait for sampling or disable flag

        if (flags & 4) {
            // Disable sampling: wait until re-enabled
            continue;
        }

        /******** critical section begin ********/
        mtx.lock();

        /* Read data */
        data.temperature = env.getTemperature();
        data.pressure = env.getPressure();
        data.lightLevel = ldr.read();

        mtx.unlock();
        /******** critical section end ********/

        producerThread.flags_set(4); // Set a flag to wake up the producer thread
    }
}

