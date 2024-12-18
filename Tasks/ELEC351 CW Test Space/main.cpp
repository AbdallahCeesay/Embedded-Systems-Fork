#include "mbed.h"
#include "uop_msb.h"
#include <chrono>
#include <SensorSampler.hpp>
#include <cstdint>
#include <cstdio>
#include <chrono>

// parameters
constexpr uint8_t size = 4;                                     // size of Mail Queue
constexpr chrono::milliseconds T_sampling = 500ms;              // Sampling period, in ms
const uint32_t WATCHDOG_TIMEOUT_MS = 5000;
extern SDCard sd;


typedef struct {

    float temperature;
    float pressure;
    float lightLevel;

} Envdata_t;


volatile Envdata_t data; // shared variable between consumer and producer thread
 
Mutex mtx;

Ticker tmr;
SensorSampler sensorSampler;
Thread producerThread, consumerThread, sdcardThread;
Mail<Envdata_t, size> mail_box;
Queue<Envdata_t, size> sdcard_queue; // queue for sdcard thread


void producer();
void consumer();
void sampling_ISR ();
void SDcard();

int main()
{
    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.start(WATCHDOG_TIMEOUT_MS);

    producerThread.start(producer);
    consumerThread.start(consumer);
    sdcardThread.start(SDcard);

    tmr.attach(&sampling_ISR,T_sampling);
    sensorSampler.start_Sampling();
}

/*Threads*/
void producer() 
{

    while(true) {
        Watchdog::get_instance().kick();
        printf("Putting data onto mail...\n");

        ThisThread::flags_wait_any(4); // waits for a flag from the sampleData method after it's done sampling
        ThisThread::flags_clear(4);

        /* tries to allocate space in the mailbox */
        Envdata_t* data_ptr = mail_box.try_alloc();

        if (data_ptr == NULL) {

            consumerThread.flags_set(6); // sends full signal to consumer thread so it can start flusing the mailbox
            
            printf("\nBuffer FULL\n");
            
            // wait for empty flag from t2
            ThisThread::flags_wait_any(8);
            ThisThread::flags_clear(8);
        }

        else {
            /******** critical section begin ********/
            mtx.lock();
            // const_cast to remove the volatile qualifier.
            *data_ptr = const_cast<Envdata_t&>(data); // obtained pointer to data

            mtx.unlock();
            /******** critical section end ********/
            
            mail_box.put(data_ptr); // put data in the mailbox
        }
    }
}

void consumer()
{
    while(true) {

        // once buffer is full
        ThisThread::flags_wait_any(6); // waits for this signal from the producer thread then start flusing the mailbox
        ThisThread::flags_clear(6);
        
        printf("******* FLUSHING *******\n");
        
        while(!mail_box.empty()) {

            // retrieve pointer, pointing to the data from buffer
            Envdata_t* rx = mail_box.try_get();
            // make a copy of this data because... (may not be needed)
            Envdata_t received_data = *rx;
            // ...the memory is now freed
            // check why the "free" is failing!!!
            if(mail_box.free(rx) == osOK) printf("success!\n");
            else printf("failed!\n");

            printf("\nTemperature:\t%3.1fC\nPressure:\t%4.1fmbar\nLight Level:\t%1.2f\n", received_data.temperature,received_data.pressure,received_data.lightLevel);
        }

        // once empty and flushing completed, send signal to the producer thread to begin adding to the mailbox again
        producerThread.flags_set(8);
        sdcardThread.flags_set(10); // set flag to run the sdcard thread

    }

}


/*ISR*/

void sampling_ISR ()
{

    sensorSampler.samplingThread.flags_set(2); // sends a flag to the sampleData method in the class to begin sampling.
}

// thread to get the sampled data from the consumer
void SDcard () {

    ThisThread::flags_wait_any(10); // waiting for a flag from the consumer thread
    ThisThread::flags_clear(10);

    if(!sd.card_inserted()){
        printf("\nInsert the SDCard\n");
        return;
    }
    sd.write_file("SampleData", "hello");

}
