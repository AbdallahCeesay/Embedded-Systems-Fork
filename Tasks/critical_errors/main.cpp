/******************************************************************************
 * File:        main.cpp
 * Author:      Andrew Norris
 * Institution: Plymouth University
 * Date:        25-11-2024
 * Description: Example of how to detect critical errors when using Mbed Mail or Semaphore
 * 
 *              This example shows how to enduce and detect errors when using an Mbed Mailbox and semaphore
 *              2 threads are used. One is the producer, the other the consumer.
 *              A Ticker is used to set a sempahore which unblocks the producer thread so it can take a sample
 *              The producer then places the data into a mailbox. When the mailbox has samples available, the 
 *              consumer thread unblocks and reads the data before freeing the memory.
 *              If button A on the MSB is held, the ticker will not release the semaphore. This simulates the 
 *              sampling being interrupted by the ticker failing.
 *              The try_aquire_for() function allows us to set a timeout and detect this. If just acquire() is used,
 *              we have no mechanism for detecting and handling the error.
 *              Similarly, pressing the B button will stop the producer from placing data into the mailbox.
 *              This simulates the producer thread locking as no new data is added to the mailbox
 *              This will cause the mailbox to empty and the consumer will see no data and report an error.
 *              Pressing the blue button will cause the consumer to stop pulling data out of the mailbox.
 *              This simulates that the consumer thread has locked and data is not pulled from thr FIFO.
 *              This will cause mailbox to fill up and the producer will have no space for new samples

******************************************************************************/



#include "mbed.h"

// Our data type which will be used in the mailbox
typedef struct{
    int a;
    int b;
    int c;
}data_t;

// Create the mailbox (10 samples of type data_t)
Mail<data_t,10> mailbox;

// Timers and RTOS components
Thread producer;
Thread consumer;
Ticker sampling_ticker;
Semaphore sampling_sem;

// Buttons to enducer errors
DigitalIn sem_stopper(PG_0);
DigitalIn mail_stopper(PG_1);
DigitalIn c_stopper(BUTTON1);


void sampling_ticker_isr(){
    // Ticker ISR, if Button A is pressed, the semaphore is not released (enduces error)
    if(!sem_stopper){
        sampling_sem.release();
    }
}
void producer_thread(){
    int x=0;
    while(1){
        // Try to aquire the semaphore, time out in 1000ms if it is not available
        bool got_sem = sampling_sem.try_acquire_for(1000ms);
        // If we do aquire the semaphore...
        if(got_sem){ 
            if(!mail_stopper){  // If Button B is pressed it will prevent the data being placed into the mailbox(enduce error)
                data_t *mail = mailbox.try_alloc_for(1000ms);   // try to get space in the mailbox for 1 sec
                if(mail==nullptr){      // If no space is available after the timeout period we have an error
                    // critical error - no space in mailbox
                    printf("Critical Error: No space in mailbox (consumer thread may have locked)\nExiting...\n");
                    exit(1);
                }
                else{                   // Otherwise add new samnple to the mailbox
                    mail->a = x;
                    mail->b = x;
                    mail->c = x;
                    mailbox.put(mail);
                    x++;
                }
            }
        }
        // If we do not aquire the semaphore...
        else{
            //critical error - sempahore not acquired
            printf("Critical Error: Sampling has been interrupted (sempahore not released)\nExiting...\n");
            exit(1);
        }
        
    }
}

void consumer_thread(){
    while(1){

        if(!c_stopper){ // If the Blue button is pressed, samples will not be removed from mailbox (enduces an error) 
            data_t* rec_mail = mailbox.try_get_for(4000ms); // Try to get sample from mailbox (timeout in 4 secs)
            // If no new data is available, report error
            if (rec_mail == nullptr) {
                //critical error - No data available
                printf("Critical Error: No data available (producer may have locked)\nExiting...\n");
                exit(1);
            }
            // Otherwise retrieve sample,and free the memory
            else{
                printf("%d,%d,%d\n",rec_mail->a,rec_mail->b,rec_mail->c);
                mailbox.free(rec_mail);
            }
        }  
    }
}

int main()
{
    // Start threads and Ticker
    consumer.start(consumer_thread);
    producer.start(producer_thread);
    sampling_ticker.attach(sampling_ticker_isr,500ms);
    // No operations required in loop(put main thread to sleep)
    while (true) {
        ThisThread::sleep_for(osWaitForever);
    }
}

