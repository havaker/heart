/******************************************************************************
 *                       Code generated with SymPy 1.12                       *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                      This file is part of 'renderer'                       *
 ******************************************************************************/
#include "surface.h"
#include <math.h>

void surface(double u, double v, double *out_3397100410920502345) {

   out_3397100410920502345[0] = (15*sin(u) - 4*sin(3*u))*sin(v);
   out_3397100410920502345[1] = 8*cos(v);
   out_3397100410920502345[2] = (15*cos(u) - 5*cos(2*u) - 2*cos(3*u) - cos(4*u))*sin(v);

}

void normal(double u, double v, double *out_5155765418550029069) {

   out_5155765418550029069[0] = (-120*sin(u) + 80*sin(2*u) + 48*sin(3*u) + 32*sin(4*u))*pow(sin(v), 2)/(sqrt(576*pow(5*cos(u) - 4*cos(3*u), 2)*pow(sin(v), 2) + 64*pow(-15*sin(u) + 10*sin(2*u) + 6*sin(3*u) + 4*sin(4*u), 2)*pow(sin(v), 2) + (1.0/4.0)*pow(720*pow(sin(u), 4)*cos(u) + 192*pow(sin(u), 4)*cos(3*u) - 480*pow(sin(u), 4) + 1200*pow(sin(u), 2) - 151*cos(u) + 63*cos(3*u) + 68*cos(5*u) - 16*cos(7*u) + 78, 2)*pow(cos(v), 2))*fabs(sin(v)));
   out_5155765418550029069[1] = (-90*pow(1 - cos(2*u), 2)*cos(u) - 24*pow(1 - cos(2*u), 2)*cos(3*u) + 16*cos(u)*cos(6*u) + (151.0/2.0)*cos(u) + 180*cos(2*u) - 63.0/2.0*cos(3*u) + 30*cos(4*u) - 42*cos(5*u) - 249)*sin(v)*cos(v)/(sqrt(576*pow(5*cos(u) - 4*cos(3*u), 2)*pow(sin(v), 2) + 64*pow(-15*sin(u) + 10*sin(2*u) + 6*sin(3*u) + 4*sin(4*u), 2)*pow(sin(v), 2) + (1.0/4.0)*pow(720*pow(sin(u), 4)*cos(u) + 192*pow(sin(u), 4)*cos(3*u) - 480*pow(sin(u), 4) + 1200*pow(sin(u), 2) - 151*cos(u) + 63*cos(3*u) + 68*cos(5*u) - 16*cos(7*u) + 78, 2)*pow(cos(v), 2))*fabs(sin(v)));
   out_5155765418550029069[2] = (-120*cos(u) + 96*cos(3*u))*pow(sin(v), 2)/(sqrt(576*pow(5*cos(u) - 4*cos(3*u), 2)*pow(sin(v), 2) + 64*pow(-15*sin(u) + 10*sin(2*u) + 6*sin(3*u) + 4*sin(4*u), 2)*pow(sin(v), 2) + (1.0/4.0)*pow(720*pow(sin(u), 4)*cos(u) + 192*pow(sin(u), 4)*cos(3*u) - 480*pow(sin(u), 4) + 1200*pow(sin(u), 2) - 151*cos(u) + 63*cos(3*u) + 68*cos(5*u) - 16*cos(7*u) + 78, 2)*pow(cos(v), 2))*fabs(sin(v)));

}
