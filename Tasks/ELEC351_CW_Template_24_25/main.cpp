/* 
 * Filename: main.cpp
 * Author: Andrew Norris
 * Institution: Plymouth University
 * Date: 21/10/24
 * Description: Starter Code for ELEC351 Coursework 24/25
 * 
 * Notes:
 *
 * For documentation regarding the module support board library, please see the Readme.md on github
 * https://github.com/PlymouthUniversityEEER/PlymouthUniversityModuleSupportBoard/blob/main/README.md
 *
 * You will need to set the module support board version that you have in MSB_Config.h
 * The default if V4. If you are using a V2 comment out "#define MSB_VER 4" and uncomment "#define MSB_VER 2"
 * 
 * 
 */

#include "uop_msb.h"
#include "mbed.h"
#include <chrono>


//These objects have already been created for you in uop_msb.h
extern EnvSensor env;
extern LatchedLED latchedLEDs;
extern SDCard sd;
extern LCD_16X2_DISPLAY disp;

// Global variables
uint32_t sample_num = 0;

int main()
{

    // Set output enable on the latched LEDs.
    latchedLEDs.enable(true);
    
    // Set the time on the RTC (You can use https://www.epochconverter.com/ for testing)
    uint64_t now = 1729519318;
    set_time(now);

    // Write some text to the SD card
    if(sd.card_inserted()){ // Check to see if the card is present (polls PF_4)
        int err = sd.write_file("test.txt", "Plymouth University - ELEC351 Coursework 24-25\n");    // Attempt to write text to file
        if(err == 0){   // If is successful, read the content of the file back
            printf("Successfully written to SD card\n");
            printf("---------------------------\nFile contents:\n");
            sd.print_file("test.txt",false);
            printf("---------------------------\n");
        }
        else{
            printf("Error Writing to SD card\n");
        }
    }
    else{
        printf("No SD Card Detected\n");
    }


    // Print the time and date
    time_t time_now = time(NULL);   // Get a time_t timestamp from the RTC
    struct tm* tt;                  // Create empty tm struct
    tt = localtime(&time_now);      // Convert time_t to tm struct using localtime
    printf("%s\n",asctime(tt));     // Print in human readable format

    while (true) {
        // Sample the data
        float temp = env.getTemperature();
        float pressure = env.getPressure();
        float light_level = ldr.read();

        // Print the samples to the terminal
        printf("\n----- Sample %d -----\nTemperature:\t%3.1fC\nPressure:\t%4.1fmbar\nLight Level:\t%1.2f\n", sample_num,temp,pressure,light_level);

        sample_num++;

        // Write the current light level as a float to the 7-segment display
        latchedLEDs.write_seven_seg(light_level);

        // If the current light level is above a threshold take action
        if(light_level > 0.5f){
            for(int i=0;i<4;i++){
                buzz.playTone("C");                     // Play tone on buzzer
                latchedLEDs.write_strip(0xFF,RED);      // Turn on all LEDs on RGB bar
                latchedLEDs.write_strip(0xFF,GREEN);
                latchedLEDs.write_strip(0xFF,BLUE);
                ThisThread::sleep_for(200ms);

                buzz.rest();                            // Stop buzzer
                latchedLEDs.write_strip(0x0,RED);       // Turn off all LEDs on RGB bar
                latchedLEDs.write_strip(0x0,GREEN);
                latchedLEDs.write_strip(0x0,BLUE);
                ThisThread::sleep_for(200ms);
            }
        }
        else{
            latchedLEDs.write_strip(0x0,RED);           // Turn off all LEDs on RGB bar
            latchedLEDs.write_strip(0x0,GREEN);
            latchedLEDs.write_strip(0x0,BLUE);
        }

        // Print the time and date
        time_t time_now = time(NULL);   // Get a time_t timestamp from the RTC
        struct tm* tt;                  // Create empty tm struct
        tt = localtime(&time_now);      // Convert time_t to tm struct using localtime
        printf("%s\n",asctime(tt));     // Print in human readable format

        // Write the time and date on the LCD
        disp.cls();                     // Clear the LCD                 
        char lcd_line_buffer[17];           
        
        strftime(lcd_line_buffer, sizeof(lcd_line_buffer), "%a %d-%b-%Y", tt);  // Create a string DDD dd-MM-YYYY
        disp.locate(0,0);                                                       // Set LCD cursor to (0,0)
        disp.printf("%s",lcd_line_buffer);                                      // Write text to LCD
        
        strftime(lcd_line_buffer, sizeof(lcd_line_buffer), "     %H:%M", tt);   // Create a string HH:mm
        disp.locate(1,0);                                                       // Set LCD cursor to (0,0)
        disp.printf("%s",lcd_line_buffer);                                      // Write text to LCD

        // Wait 5 seconds before next sample
        ThisThread::sleep_for(std::chrono::seconds(5));
    }
}

