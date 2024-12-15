#include "mbed.h"
#include <cstdio>

const uint32_t TIMEOUT_MS = 5000;
InterruptIn button(USER_BUTTON);
volatile int countdown = 9; /*this is a shared resource,therefore we use volatile*/

void trigger() /*function that kicks the watchdog timer and resets counter to 9 */
{
    Watchdog::get_instance().kick();
    countdown = 9;
}

int main()
{
    printf("\r\nTarget started.\r\n");

    Watchdog &watchdog = Watchdog::get_instance(); /*get the instance of the watchdog timer*/
    watchdog.start(TIMEOUT_MS); /*start the watchdog and reset the system after the set time (5000ms) has elapsed.*/
    button.rise(&trigger); /* interrupt that kicks the watchdog timer and reset the countdown to 9 */

    uint32_t watchdog_timeout = watchdog.get_timeout();
    printf("Watchdog initialized to %u ms.\r\n", watchdog_timeout); 
    printf("Press BUTTON1 at least once every %u ms to kick the "
           "watchdog and prevent system reset.\r\n", watchdog_timeout);

    while (1) {
        printf("\r%3i", countdown--);
        fflush(stdout); /*ensures that the printed output is immediately displayed*/
        ThisThread::sleep_for(1ms * (TIMEOUT_MS / 10)); 
    }
}

