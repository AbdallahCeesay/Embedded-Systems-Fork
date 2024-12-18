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
 
Mutex mtx, conumer_to_sdcard;

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
        
        printf("\n******* FLUSHING *******\n");
        
        while(!mail_box.empty()) 
        {
            Envdata_t *rx = mail_box.try_get();
            if (rx) 
            {
                Envdata_t received_data = *rx;

                /*send data to the sdcard*/
                if (sdcard_queue.try_put(&received_data)) printf("Data added to the Sd card queue.\n");
             
                else printf("SD card queue FULL\n");
            }
                /*Full mailbox memory*/
                if (mail_box.free(rx) == osOK) printf("Mail free successful\n"); 
                
                else printf("Mail free failed\n");     
        }

        // once empty and flushing completed, send signal to the producer thread to begin adding to the mailbox again
        producerThread.flags_set(8);
        sdcardThread.flags_set(10); // set flag to run the sdcard thread

    }

}



// thread to get the sampled data from the consumer
// void SDcard () {

//     ThisThread::flags_wait_any(10); // waiting for a flag from the consumer thread
//    // ThisThread::flags_clear(10);

//     char buffer[128];

//     /*process data in the SDcard queue*/

//     while (!sdcard_queue.empty()) {
//         Envdata_t *sd_data;

//         if (sdcard_queue.try_get(&sd_data)) {
//             snprintf(buffer, sizeof(buffer),"\nTemperature: %3.1fC\nPressure: %4.1fmbar\nLight Level: %1.2f\n",
//                          sd_data->temperature, sd_data->pressure, sd_data->lightLevel);
//         }

//         /*check if the sd card is inserted*/

//         if (!sd.card_inserted())  printf("SD card not inserted\n");
        
//         if (sd.write_file("SampleData.txt",buffer)) printf("Data written to the SD Card successfull\n"); 
        
//         else   printf("Failed to write to the SD card\n");
          
//         /*print to the terminal*/
//         printf("SD card Data:\n%s", buffer);
//     }
// }

void SDcard() {
    while (true) {
        // Wait for a flag from the consumer thread to process the SD card queue
        ThisThread::flags_wait_any(10);
        ThisThread::flags_clear(10);

        printf("\n--- Processing SD card queue ---\n");

        char buffer[128];
        bool success = true;

        /* Process and write all data in the SD card queue */
        while (!sdcard_queue.empty()) {
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
                if (sd.write_file("SampleData.txt", buffer)) {
                    printf("Data written to the SD card successfully:\n%s", buffer);
                } else {
                    printf("Failed to write to the SD card.\n");
                    success = false;
                    break; // Exit if writing fails
                }

            } else {
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



/*ISR*/

void sampling_ISR ()
{

    sensorSampler.samplingThread.flags_set(2); // sends a flag to the sampleData method in the class to begin sampling.
}
