#include "mbed.h"
#include <iostream>
#include "uop_msb.h"

using namespace std;

// Declare the volatile shared variable
volatile int counter = 0;

// Instantiate the ticker and button interrupts
Ticker ticker;
InterruptIn buttonA(BTN1_PIN);

// EventQueue for safely printing from the main thread
EventQueue queue(32 * EVENTS_EVENT_SIZE);

// Timer to handle debounce
Timer debounce_timer;

// Define debounce period (in milliseconds)
const float DEBOUNCE_TIME = 50; // 50ms debounce time

// Function to increment the counter in the main thread
void increment_counter() {
    CriticalSectionLock lock; // Lock to prevent race conditions
    counter++;
    queue.call([]() { cout << "Counter incremented: " << counter << endl; }); // Schedule cout in the main thread
}

// ISR to decrement the counter with debounce logic
void decrement_counter() {
    // Check if the debounce time has elapsed
    if (debounce_timer.elapsed_time().count() > DEBOUNCE_TIME) {
        counter--; // Safely decrement the counter
        queue.call([]() { cout << "Counter DECREMENTED: " << counter << endl; }); // Schedule cout in the main thread
        debounce_timer.reset(); // Reset the debounce timer
    }
}

int main() {
    // Start the event queue in a separate thread
    Thread event_thread;
    event_thread.start(callback(&queue, &EventQueue::dispatch_forever));

    // Initialize the debounce timer
    debounce_timer.start();

    // Attach the ticker to the increment function every 1 second
    ticker.attach(&increment_counter, 1s);

    // Attach the button ISR to the decrement function with debounce
    buttonA.fall(&decrement_counter);

    // Keep the main thread running
    while (true) {
        queue.call([]() { cout << "Current counter value: " << counter << endl; }); // Periodically print the counter
        ThisThread::sleep_for(2s);
    }
}
