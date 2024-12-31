#include "mbed.h"
#include <cmath>
#include <iostream>
#include <math.h>
#include <string.h>
#include <string>

using namespace std;

class DoubleNumber 
{
    private:

    protected:
        double _real;

    public:
        //Designated constructor. This is the first constructor
        DoubleNumber(double r) {
            _real = r;
            cout << "This is the constructor of Base" << endl;
        }

        //Convenience constructor
        DoubleNumber() : DoubleNumber(0.0) { } // this invokes the first constructor and initilizes its vaue to 0.0

        //Magnitude
        double magnitude() {
            return fabs(_real);
        }

        //Three overloaded functions
        void setValue(double u) {
            _real = u;
        }

        void setValue(int u) {
            _real = (double)u;
        }
        
        void setValue(string strVal) {
            _real = stod(strVal);
        }

        void setValue (DoubleNumber u) { /*u is an object of type DoubleNumber, therefore it allows us to access the methods in the class using the dot operator.*/
            _real = u.getValue();
        }

        double getValue() {
            return _real;
        }

        string asString() {
            return to_string(_real);
        }
};


int main()
{
    //Constructor Function overloading 
    DoubleNumber n0(1.0);
    DoubleNumber n1;
    DoubleNumber n2;

    //setValue function overloading  
    n1.setValue(10);
    n2.setValue("-3.0");
    n0.setValue(n1); // interesting...
    n0.getValue();

    cout << n0.getValue() + n1.getValue() + n2.getValue() << endl;

    while (true) {

    }
}

