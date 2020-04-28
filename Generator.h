#ifndef GENERATOR_H
#define GENERATOR_H

#include "Model.h"

class Generator
{
public:
	static Model generateSphere(int prec=48);
	static Model generateRingHab(double habWidth=0.2, double wallThick=0.01,
	                             double wallHeight=0.05, int prec=48);
	static Model generateBox();
};

#endif
