#include "mbed.h"
#include "uop_msb.h"
#include <chrono>
#include <SensorSampler.hpp>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <_Terminal.hpp>
#include <ctime>

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
 
Mutex mtx, consumer_to_sdcard;

Ticker tmr;
SensorSampler sensorSampler;

Thread producerThread, consumerThread, sdcardThread, serialThread, timeThread;

Mail<Envdata_t, size> mail_box;
Queue<Envdata_t, size> sdcard_queue; // queue for sdcard thread

// Buffered serial for real-time control
BufferedSerial serial(USBTX, USBRX, 9600); // tx, rx, baud rate
bool samplingEnabled = true;              // Sampling state
EventQueue serialQueue(32 * EVENTS_EVENT_SIZE);

void producer();
void consumer();
void sampling_ISR();
void SDcard();
void monitorSerial();
void processSerialInput();
void setDateAndTime();

int main()
{
    uint64_t currentTime = 1734622120;
    set_time(currentTime);

    Watchdog &watchdog = Watchdog::get_instance();
    watchdog.start(WATCHDOG_TIMEOUT_MS);

    producerThread.start(producer);
    consumerThread.start(consumer);
    sdcardThread.start(SDcard);
    serialThread.start(monitorSerial);
    timeThread.start(setDateAndTime);

    tmr.attach(&sampling_ISR, T_sampling);
    sensorSampler.start_Sampling();

    // Dispatch the serial event queue
    serialQueue.dispatch_forever();
}

/* Threads */
void producer() 
{
    while (true) {
        Watchdog::get_instance().kick();
        printf("Putting data onto mail...\n");

        ThisThread::flags_wait_any(4); // waits for a flag from the sampleData method after it's done sampling
        ThisThread::flags_clear(4);

        /* Tries to allocate space in the mailbox */
        Envdata_t* data_ptr = mail_box.try_alloc();

        if (data_ptr == NULL) {
            consumerThread.flags_set(6); // sends full signal to consumer thread to start flushing the mailbox
            printf("\nBuffer FULL\n");
            
            // wait for empty flag from t2
            ThisThread::flags_wait_any(8);
            ThisThread::flags_clear(8);
        } else {
            /******** Critical section begin ********/
            mtx.lock();
            // const_cast to remove the volatile qualifier
            *data_ptr = const_cast<Envdata_t&>(data); // obtained pointer to data
            mtx.unlock();
            /******** Critical section end ********/
            
            mail_box.put(data_ptr); // put data in the mailbox
        }
    }
}

void consumer()
{
    while (true) {
        Watchdog::get_instance().kick();
        // Once buffer is full
        ThisThread::flags_wait_any(6); // waits for signal from producer thread
        ThisThread::flags_clear(6);
        
        printf("\n******* FLUSHING *******\n");
        
        while (!mail_box.empty()) 
        {
            Envdata_t *rx = mail_box.try_get();
            if (rx) 
            {
                Envdata_t received_data = *rx;

                /* Send data to the SD card */
                if (sdcard_queue.try_put(&received_data)) {
                    printf("Data added to the SD card queue.\n");
                } else {
                    printf("SD card queue FULL\n");
                }
            }
            /* Free mailbox memory */
            if (mail_box.free(rx) == osOK) {
                printf("Mail free successful\n"); 
            } else {
                printf("Mail free failed\n");
            }
        }

        // Once empty and flushing completed, send signal to the producer thread to begin adding to the mailbox again
        producerThread.flags_set(8);
        sdcardThread.flags_set(10); // set flag to run the sdcard thread
    }
}

void SDcard() 
{// implement data time as well
     while (true) 
    {
        Watchdog::get_instance().kick();

        // Wait for a flag from the consumer thread to process the SD card queue
        ThisThread::flags_wait_any(10);
        ThisThread::flags_clear(10);

        printf("\n--- Processing SD card queue ---\n");

        char buffer[256];
        bool success = true;

        /* Process and write all data in the SD card queue */
        while (!sdcard_queue.empty()) {
            Envdata_t *sd_data;

            if (sdcard_queue.try_get(&sd_data)) {
                // Get the current system time
                time_t timeNow = time(NULL);
                struct tm* timeInfo = localtime(&timeNow);

                if (timeInfo) {
                    // Format the data into the buffer with the date and time
                    snprintf(buffer, sizeof(buffer),
                             "\nDate: %04d-%02d-%02d Time: %02d:%02d:%02d\nTemperature: %3.1fC\nPressure: %4.1fmbar\nLight Level: %1.2f\n",
                             timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday,
                             timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec,
                             sd_data->temperature, sd_data->pressure, sd_data->lightLevel);
                } else {
                    // If time fetching fails, log an error
                    snprintf(buffer, sizeof(buffer),
                             "\nTime unavailable\nTemperature: %3.1fC\nPressure: %4.1fmbar\nLight Level: %1.2f\n",
                             sd_data->temperature, sd_data->pressure, sd_data->lightLevel);
                }

                /* Check if the SD card is inserted */
                consumer_to_sdcard.lock();
                if (!sd.card_inserted()) {
                    printf("SD card not inserted. Skipping write operation.\n");
                    success = false;
                    break; // Exit early if SD card is not available
                }

                /* Write to the SD card */
                if (!sd.write_file("SampleData.txt", buffer)) {
                    printf("Data written to the SD card successfully:\n%s", buffer);
                } else {
                    printf("Failed to write to the SD card.\n");
                    success = false;
                    break; // Exit if writing fails
                }
                consumer_to_sdcard.unlock();
            } 
            else {
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


/* Serial Monitoring */
void monitorSerial() {
    while (true) 
    {
        Watchdog::get_instance().kick();
        processSerialInput();
    }
}

/* Updated ISR */
void sampling_ISR() {
    if (samplingEnabled) {
        // Only send a flag if sampling is enabled
        sensorSampler.samplingThread.flags_set(2);
    }
}

void processSerialInput() {
    char c;
    while (serial.read(&c, 1) > 0) {
        if (c == 'x') {
            samplingEnabled = !samplingEnabled; // Toggle sampling
            printf("Sampling %s\n", samplingEnabled ? "ON" : "OFF");

            if (!samplingEnabled) {
                // Signal the sampling thread to pause
                sensorSampler.samplingThread.flags_set(4); // Set a 'disable' flag
            } else {
                // Signal to resume sampling by clearing the disable flag
                ThisThread::flags_clear(4); // Clear the 'disable' flag
            }
        }
    }
}

void setDateAndTime() {
    
 while (true) {
        time_t timeNow = time(NULL);
        struct tm* time = localtime(&timeNow);

        if (time) {
            printf("Current Time: %s\n", asctime(time));
        } else {
            printf("Error retrieving time\n");
        }
    }
}