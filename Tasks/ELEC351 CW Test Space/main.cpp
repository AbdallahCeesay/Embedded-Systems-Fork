#include "mbed.h"
#include "uop_msb.h"
#include <chrono>
#include <SensorSampler.hpp>
#include <cstdio>
#include <chrono>

// parameters
constexpr uint8_t size = 4;                                     // size of Mail Queue
constexpr chrono::milliseconds T_sampling = 500ms;              // Sampling period, in ms


typedef struct {

    float temperature;
    float pressure;
    float lightLevel;

} Envdata_t;


volatile Envdata_t data;
 
Mutex mtx;

Ticker tmr;
SensorSampler sensorSampler;
Thread t1, t2;
Mail<Envdata_t, size> mail_box;


void producer();
void consumer();

void sampling_ISR ();


int main()
{

    t1.start(producer);
    t2.start(consumer);

    tmr.attach(&sampling_ISR,T_sampling);
    sensorSampler.start_Sampling();

}

/*Threads*/
void producer() 
{

    while(1) {

        printf("Putting data onto mail...\n");

        ThisThread::flags_wait_any(4);
        ThisThread::flags_clear(4);
        // printf("I am alive");

        // put onto mail here...

        Envdata_t* data_ptr = mail_box.try_alloc();
        if (data_ptr == NULL) {

            // sends full signal to consumer thread
            t2.flags_set(5);
            
            printf("Buffer FULL\n");
            
            // wait for empty flag from t2
            ThisThread::flags_wait_any(7);
            ThisThread::flags_clear(7);
        }

        else {

            ///////// critical section begin /////////
            mtx.lock();
        
            // obtained pointer to data
            // const_cast to remove the volatile qualifier.
            *data_ptr = const_cast<Envdata_t&>(data);


            mtx.unlock();
            ///////// critical section end /////////



            // put data on mailbox/buffer
            mail_box.put(data_ptr);

        }
    }
}

void consumer()
{

    while(1) {

        // once buffer is full
        ThisThread::flags_wait_any(5);
        ThisThread::flags_clear(5);
        
        // start flushing

        printf("//////// FLUSHING ////////\n");
        
        while(!mail_box.empty()) {

            // retrieve pointer, pointing to the data, from buffer
            Envdata_t* rx = mail_box.try_get();
            // make a copy of this data because... (may not be needed)
            Envdata_t received_data = *rx;
            // ...the memory is now freed
            // check why the "free" is failing!!!
            if(mail_box.free(rx) == osOK) printf("success!\n");
            else printf("failed!\n");

            printf("\nTemperature:\t%3.1fC\nPressure:\t%4.1fmbar\nLight Level:\t%1.2f\n", received_data.temperature,received_data.pressure,received_data.lightLevel);

            
        }

        // once empty, signal producer thread
        t1.flags_set(7);

        
    }

}


/*ISR*/
void sampling_ISR ()
{
    sensorSampler.samplingThread.flags_set(2);
}

