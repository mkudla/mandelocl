__kernel void mandelbrot(__global float *cr, __global float *ci, __global float *zr, __global float *zi, __global float *rsquared, __global float *isquared, __global int *map) {
#define TRUE 1
#define FALSE 0
#define ITERNUM 200
	
	int i = get_global_id(0);
/*
	zr[i] = 0.0;
	zi[i] = 0.0;
	rsquared[i] = 0.0;
	isquared[i] = 0.0;
*/
	for(int iter = 0;iter < ITERNUM;iter++){
	zi[i] = zr[i] * zi[i] * 2;
	zi[i] += ci[i];

	zr[i] = rsquared[i] - isquared[i];
	zr[i] += cr[i];

	rsquared[i] = zr[i] * zr[i];
	isquared[i] = zi[i] * zi[i];
	}
	if(rsquared[i]+isquared[i]<=4.0)
		map[i] = TRUE;
	else
		map[i] = FALSE;


}
