#include "mbed.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;

// main() runs in its own thread in the OS
int main()
{
    // Part 1 of the coursework
    //----------------------------------------------------------------------------------------------

    // created a structure for the two parts of the complex number
    typedef struct {
        double real;
        double img;
    } complexNumber;

    // created a variable with a datatype of complexNumber

    complexNumber a; 
    a.img = 3;
    a.real = 4;

    // part 2 of the coursework
    // ------------------------------------------------------------------------------------------
    const char* binaryStr = "1101";
    long hexadecimalValue = strtol(binaryStr, nullptr, 2); // Convert binary to decimal


    while (true) {

        // this prints both the absoulute value of the imaginary number and the conversion to the serial terminal
        double result = sqrt(pow(a.real, 2) + pow(a.img, 2));
        printf("absolute value: %f\n\n", result);
        wait_us(1000000);

        printf("Binary: %s\nHexadecimal: %lX\n", binaryStr, hexadecimalValue);
        wait_us(1000000);
        cout << "Hello";
    }
}

