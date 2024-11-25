/* this is where functions are defined and classes are implemented
You have to include the header file in both this doucment and the main document.*/

#include "mbed.h"
#include "ComplexFunctions.hpp"

/*
'a' and 'b' are read only and can't be modified in the function
y = a, copies the "Real" and "Imaginary" parts of 'a' to the "Real" and "Imaginary" parts of 'y'
we then return 'y' which is the Real part of "y" and the Imaginary part of "y" 
*/
ComplexNumber_C complexAdd(const ComplexNumber_C a, const ComplexNumber_C b) { 
    ComplexNumber_C y = a; 
    y.real += b.real; /* y.real = y.real + b.real where y = a */
    y.imag += b.imag;
    
    return y; 
}


/*
this function display a complex number
strName represents the name of the complex Number
the variable u represents the real and imaginary parts of the complex number.
*/
void complexDisplay(const char *strName, const ComplexNumber_C u) {
    printf("%s = %.2f + j%.2f\n", strName, u.real, u.imag);
}

/* complex conjugate function*/
ComplexNumber_C complexConjugate (ComplexNumber_C x) {
    ComplexNumber_C y;
    y.imag = -x.imag;

    return y;
}
