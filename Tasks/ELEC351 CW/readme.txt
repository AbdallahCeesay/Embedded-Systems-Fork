This is a readme file intended to showcase what requirements have been met and how to verify them. Note that there is a video demonstration for
the requirements met here:
Link 1: https://www.youtube.com/watch?v=MDD_zEEyxJ0
Link 2: https://www.youtube.com/watch?v=y3SJlXltFVI

Requirements met: the numbers below correspond to the requrements on the coursework document 

1. The device periodically samples every 10s. This is made the highest priority thread as well. Sample jitter is also minimized by using signal wait and
the timer class to accurately sample at the set sample rate. This has also been encapsulated into its separate .cpp and .hpp file.


2. partially implemented: The time and date aspect has not been implemented but the sensor data is written to the sd card with every sample.

3. this has been fully implemented. the mbed mail box has been used as a FIFO buffer. The mailbox size has been set to 60. so it only writes to the sd card after 60 seconds

4. Partially implemented: thresholds have been set but hysteresis has not been implemented

5. fully implemented: All threads are synchronized using Mutexs, flags and signal wait. The signl wait is an ISR attached to a timer that wakes the sampling thread every 10s to sample and go back to the waiting stage. Also separate threads have been implemented for each component of the project.

6. partially implemented

7. partially implemented. x to stop and start sampling has been implemented. data in the buffere is written to the sd card then emptied. the other two function have not been implemented (datetime and select [x])

8. Mutexes have been used to avoid deadlocks and a watch dog timer has been implemented to restart the system when there is such an error. This comes in useful when you insert the sdcard. for some reason, the corrupts atimes. I am unable to determine why this occurs but the system will restart when the sdcard is corrupted prompting you to format and then try again.

9. this has not been implemented. it is the only task not implemented at All

10. this has been fully implemented.

Bugs: the sd card is a very tricky part of this whole system and I am unable to understand why this this the case.
the sd card can cause the system to crash (gets corrupted easily) so make sure that once the system crashes without any reason, remove the sd card and connect to a computer. You will be prompted to format the sd card. do so and connect it back to the uopmsb and it should start working.
this has been a major pain in the system. sometimes, you can connect the sd card and after a while it starts to work. 

warning: the sd card when not working stops on the following line "Initialize and write to a file" this then restarts the system


how to verify each requirement?

Requirement:

1. you can set a timer and observe when the system starts to sample. a much more accurate way to do this would be to have a that prints to the terminal the time taken to sample

2. you can verify this by plugging the sd card to a computer and observe the data within the sampleData.txt file.

3. prints to the terminal every minutes the samples that have been buffered and written to the sd card. this is verified throught the terminal

4. you can verify that the terminal is working by pressing 'x' on your keyboard to observe that sampliing stops and then starts again when x is pressed again.

I think these are the requirements that can be tested. Other are either done visually or audibly.