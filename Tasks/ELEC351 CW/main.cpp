/* 
* Filename: main.cpp
* Author: Abdallah Ceesay 10726858
* Module: ELEC 351 Advanced Embedded Programming
* Institution: Plymouth University
* Professor: Yu Yao. Honourable mention: Nickolas Outram, Andrew Norris
* Date: 25/11/24
*/

#include "mbed.h"
#include "uop_msb.h"
#include <chrono>
#include <SensorSampler.hpp>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <iostream>


static UnbufferedSerial serial_port(USBTX, USBRX); // Create a BufferedSerial object (USART) with a default baud rate

// externs
extern volatile bool samp_toggle;
extern Mutex samp_mtx;
extern SDCard sd;
extern Buzzer buzz;
extern EnvSensor env;
extern AnalogIn ldr;

// parameters
constexpr uint8_t size = 60;                                     // size of Mail Queue
constexpr chrono::milliseconds T_sampling = 10000ms;              // Sampling period, in ms
constexpr uint32_t WATCHDOG_TIMEOUT_MS = 5000;
uint32_t sampling_toggle_flag = 1024;


typedef struct {
    float temperature;
    float pressure;
    float lightLevel;
} Envdata_t;


volatile Envdata_t data; // shared variable between consumer, producer thread and sd card thread

/*Mutexes*/
Mutex mtx, conumer_to_sdcard, light;


/*Threads, mailboxes and queues*/
Ticker tmr;
SensorSampler sensorSampler;
Thread producerThread, consumerThread, sdcardThread, controlThread, emergencyThread;
Mail<Envdata_t, size> mail_box;
Queue<Envdata_t, size> sdcard_queue; // queue for sdcard thread


/*function decalartion*/
void producer();
void consumer();
void sampling_ISR();
void SDcard();
void control_thread();
void emergency();

int main()
{

    // Set desired properties (9600-8-N-1).
    serial_port.baud(9600);
    serial_port.format(
        /* bits */ 8,
        /* parity */ SerialBase::None,
        /* stop bit */ 1
    );

    printf("\n\n\n\n\n\n ````````SYSTEM RESET`````````` \n\n\n\n\n\n\n");
    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.start(WATCHDOG_TIMEOUT_MS);

    producerThread.start(producer);
    consumerThread.start(consumer);
    sdcardThread.start(SDcard);
    controlThread.start(control_thread);
    emergencyThread.start(emergency);

    tmr.attach(&sampling_ISR,T_sampling);
    sensorSampler.start_Sampling();
}

/*Threads*/
void producer() 
{
    while(true) 
    {
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
        else 
        {
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

        Watchdog::get_instance().kick();

        // once buffer is full
        ThisThread::flags_wait_any(6); // waits for this signal from the producer thread then start flusing the mailbox
        ThisThread::flags_clear(6);
        
        printf("\n******* FLUSHING *******\n");
        
        while(!mail_box.empty()) 
        {
            Envdata_t *rx = mail_box.try_get();
            if (rx) 
            {
                Envdata_t received_data = *rx;
                if (!sdcard_queue.try_put(&received_data)) printf("SD card queue FULL\n");
            }

            /*Full mailbox memory*/
            if (mail_box.free(rx) != osOK) printf("Mail free failed\n");     
        }
        // once empty and flushing completed, send signal to the producer thread to begin adding to the mailbox again
        producerThread.flags_set(8);
        sdcardThread.flags_set(10); // set flag to run the sdcard thread
    }
}

void SDcard() 
{
    while (true) 
    {
        // Wait for a flag from the consumer thread to process the SD card queue
        ThisThread::flags_wait_any(10);
        ThisThread::flags_clear(10);

        printf("\n--- Processing SD card queue ---\n");

        char buffer[128];
        bool success = true;

        /* Process and write all data in the SD card queue */
        // WARNING: THIS NEVER EXITS UNDER CERTAIN CONDITIONS
        while (!sdcard_queue.empty()) {

            Watchdog::get_instance().kick();
            Envdata_t *sd_data;

            if (sdcard_queue.try_get(&sd_data)) {
                // Format the data into the buffer
                snprintf(buffer, sizeof(buffer),
                         "\nTemperature: %3.1fC\nPressure: %4.1fmbar\nLight Level: %1.2f\n",
                         sd_data->temperature, sd_data->pressure, sd_data->lightLevel);

                /* Check if the SD card is inserted */
                if (!sd.card_inserted()) {
                    printf("SD card not inserted. Skipping write operation.\n");
                    success = false;
                    break; // Exit early if SD card is not available
                }

                /* Write to the SD card */
                // success returns 0; else -1
                if (sd.write_file("SampleData.txt", buffer) == 0) {
                    printf("Data written to the SD card successfully:\n%s", buffer);
                } else {
                    printf("Failed to write to the SD card.\n");
                    success = false;
                    break; // Exit if writing fails
                }

            } else 
            {
                printf("Failed to retrieve data from the SD card queue.\n");
                success = false;
                break;
            }
        }

        /* Acknowledgment */
        if (success) {
            printf("\n--- All samples successfully written to the SD card. Buffer emptied. ---\n");
        } else {
            printf("\n--- Error occurred while processing SD card data. ---\n");
        }
    }
}


void control_thread() 
{
    char c;
    while(1) 
    {
        // poll the RXNE bit
        // wait for something...
        while(!(USART3->SR & 0x20)) {

            Watchdog::get_instance().kick();
            ThisThread::sleep_for(500ms);
        }

        c = USART3->DR; // a read clears the RXNE bit

        if(c == 'x') {

            samp_mtx.lock();
            samp_toggle = !samp_toggle;
            samp_mtx.unlock();

            printf("\n!!!!!!!!!!!!!!SAMPLING IS: %s!!!!!!!!!!!!!!\n!", (samp_toggle) ? "ENABLED" : "DISABLED");

            // only wake up thread upong re-enabling logging
            if(samp_toggle) {
                
                sensorSampler.samplingThread.flags_set(sampling_toggle_flag);
                std::cout << "flag sent" << std::endl;
            }
        }
    }
}




// note: check crashing issue in this thread
void emergency () 
{
    while (true) 
    {
        // light check
        mtx.lock();
        if (ldr.read() > 0.8f || ldr.read() < 0.2f) 
        {

            for (int a = 0; a < 4; a++) {
                
                buzz.playTone("A");
                ThisThread::sleep_for(100ms);

            }
        }
        mtx.unlock();
        buzz.rest();

        // temperature check
        mtx.lock();
        if (env.getTemperature() > 28.0f || env.getTemperature() < 10.0f) 
        {
            for (int a = 0; a < 4; a++) {
                
                buzz.playTone("B");
                ThisThread::sleep_for(100ms);

            }
        }
        mtx.unlock();
        buzz.rest();

        // pressure check
        mtx.lock();
        if (env.getPressure() > 1100.0f || env.getPressure() < 900.0f) 
        {
            for (int a = 0; a < 4; a++) {
                
                buzz.playTone("C");
                ThisThread::sleep_for(100ms);
            }
        }
        mtx.unlock();
        buzz.rest();

        ThisThread::sleep_for(1s);      
    }
}


/*ISR*/
void sampling_ISR ()
{
    sensorSampler.samplingThread.flags_set(2); // sends a flag to the sampleData method in the class to begin sampling.
}