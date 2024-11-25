#include "mbed.h"
#include "ComplexFunctions.hpp"
#include <cstdio>

// TASK - write and test complexConjugate, complexNegate, complexSubtract, complexMagnitude, complexMultiply and complexDivide

int main() {

    printf("\n\nTASK312\n");

    //Create instance of a complex number
    ComplexNumber_C p = {2.0, 3.0};
    ComplexNumber_C q = {1.0, 1.0};

    complexDisplay("p", p);
    complexDisplay("q", q);
 
    ComplexNumber_C sum = complexAdd(p, q);
    complexDisplay("p+q", sum); 

    ComplexNumber_C Conjugate = complexConjugate(q);
    complexDisplay("Complex conjugate", Conjugate);
    
    while (true) {
    }
}
