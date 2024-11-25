/* this document declares the functions, structures and clases I want to add to my program*/
#ifndef __ComplexFucntions__
#define __ComplexFucntions__

typedef struct {
  double real;
  double imag;
} ComplexNumber_C; // a complex data type

extern ComplexNumber_C complexAdd(const ComplexNumber_C a, const ComplexNumber_C b);
extern void complexDisplay(const char *strName, const ComplexNumber_C u);
extern ComplexNumber_C complexConjugate (ComplexNumber_C x);

#endif