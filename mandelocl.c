#include "SDL.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <CL/cl.h>
#define MAX_SOURCE_SIZE (0x100000)
#define TRUE 1
#define FALSE 0
#define DESCRIPTION "\n This is a simple program which should display Mendelbrot set. Works with normal cpu or OpenCL. \n\n"
#define HEIGHT 512
#define WIDTH 512
#define pel(surf, x, y, rgb) ((unsigned char *)(surf->pixels))[y*(surf->pitch)+x*3+rgb] // Macro for accessing pixel on surface (SDL)
#define LEFT     -2.0
#define RIGHT    2.0
#define TOP      2.0
#define BOTTOM   -2.0
#define MAX_ITER 50
#define KERNELFILENAME "mandelocl.cl"
#define KERNELNAME "experimental4"

// To compile run:
//64 bit
//gcc -c -I /opt/AMDAPP/include -std=c99 my_prog_disp_7_oc.c -o my_prog_disp7oc.o `sdl-config --cflags --libs`
//gcc my_prog_disp7oc.o -o disp7oc -L /opt/AMDAPP/lib/x86_64/ -l OpenCL `sdl-config --cflags --libs`
//32 bit
//gcc -c -I /opt/AMDAPP/include -std=c99 my_prog_disp_7_oc.c -o my_prog_disp7oc.o `sdl-config --cflags --libs`
//gcc my_prog_disp7oc.o -o disp7oc -L /opt/AMDAPP/lib/x86/ -l OpenCL `sdl-config --cflags --libs`



	int fDisplaySet(int x,int y, SDL_Surface *scr, int *setarray); //copies from our scratch buffer to screen using SDL
	int fTestRoutine(int x,int y, SDL_Surface *scr, int *setarray); // generates test pattern to use with fDisplaySet
	int fDoMandelbrot5(float startx, float starty, float endx, float endy, int *setarray);
	int fDoMandelbrot6(float startx, float starty, float endx, float endy, int *setarray, cl_kernel kernel, cl_command_queue command_queue, cl_context context);
	int MaxIters = MAX_ITER;
	int x, y, t, count; // same here
	const int LIST_SIZE = WIDTH * HEIGHT;

int main(int argc, char *argv[])
{

	int setarray[LIST_SIZE];

//
// SDL initialization starts here
//
    SDL_Event event;	    /* Event information is placed in here */
    SDL_Surface *scr;	    /* This will be used as our "handle" to the screen surface */
    SDL_Init(SDL_INIT_VIDEO);
    scr = SDL_SetVideoMode(WIDTH, HEIGHT, 24, SDL_SWSURFACE);
    assert(scr);
    SDL_LockSurface(scr);	/* Ensures we have exclusive access to the pixels */

	fTestRoutine(x,y,scr,setarray);	// Draws test pattern to display buffer
	fDisplaySet(x,y,scr,setarray); //  Copies display buffer to scr

    SDL_UnlockSurface(scr);
    SDL_UpdateRect(scr, 0, 0, 0, 0);	/* Copies the `scr' surface to the _actual_ screen */
//
// SDL code init ends here
//


//
// OpenCL init
//
	//Loading kernel source file
	    FILE *fp;
	    char *source_str;
	    size_t source_size;

	    fp = fopen(KERNELFILENAME, "r");
	    if (!fp) {
			fprintf(stderr, "Failed to load kernel.\n");
			exit(1);
	    }
	    source_str = (char*)malloc(MAX_SOURCE_SIZE);
	    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
	    fclose( fp );

	// Get platform and device information
	    cl_platform_id platform_id = NULL;
	    cl_device_id device_id = NULL;
	    cl_uint ret_num_devices;
	    cl_uint ret_num_platforms;
	    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1,
		    &device_id, &ret_num_devices);

	    // Create an OpenCL context
	    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

	    // Create a command queue
	    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1,
            (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, KERNELNAME, &ret);

//
//	OpenCL init ends here
//

	float magnify=1.0, vectorx =0.0, vectory=0.0, vectordisp=0.2;
	float startx, endx, starty, endy;
	int OpenCLToggle = FALSE;

// This is main SDL event handle, everything in OpenCL has to be initialized till here
    while(SDL_WaitEvent(&event))	    /* Now we wait for an event to arrive */
    {
    	if (event.type == SDL_QUIT)
    		break; // This one exits while loop
	if (event.type == SDL_KEYUP)
	{
		switch(event.key.keysym.sym){
			case SDLK_UP:
				vectory+=vectordisp;
				break;
			case SDLK_DOWN:
				vectory-=vectordisp;
				break;
			case SDLK_LEFT:
				vectorx-=vectordisp;
				break;
			case SDLK_RIGHT:
				vectorx+=vectordisp;
				break;
			case SDLK_z:
				magnify*=0.75;
				break;
			case SDLK_x:
				magnify*=1.25;
				break;
			case SDLK_a:
				MaxIters+=1;
				break;
			case SDLK_s:
				MaxIters-=1;
				break;
			case SDLK_c:
				if(OpenCLToggle == FALSE) {
				OpenCLToggle = TRUE;
				}
				else {
				OpenCLToggle = FALSE;
				}
				break;
			default:
				break;
			}
			startx = (LEFT + vectorx)*magnify;
			starty = (TOP + vectory)*magnify;
			endx = (RIGHT + vectorx)*magnify;
			endy = (BOTTOM + vectory)*magnify;



			//redraw screen after keypress
		    SDL_LockSurface(scr);
			//fTestRoutine(x,y,scr,setarray);



		if(OpenCLToggle == FALSE) {
		    fDoMandelbrot5(startx, starty, endx, endy, setarray); //Draws Mandelbrot to Display buffer
		}
		else {
			fDoMandelbrot6(startx, starty, endx, endy, setarray, kernel,command_queue,context); //Draws Mandelbrot to Display buffer
		}
		
		// fDoMan
		    fDisplaySet(x,y,scr,setarray); //Copies display buffer to scr
		    SDL_UnlockSurface(scr);
		    SDL_UpdateRect(scr, 0, 0, 0, 0);
	}
    }
    SDL_Quit(); // This one quits SDL after we exit from while loop

//CleanUP OpenCL

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    return EXIT_SUCCESS;
}

//fDisplaySet - copies Display buffer we work on to the scr. Possibly slow, but we do not care for now.
int fDisplaySet(int x,int y, SDL_Surface *scr, int *setarray)
{
	for(y = 0; y < scr->h; y++)
	    	for(x = 0; x < scr->w; x++)
	    	{
			if(setarray[(y)*(scr->w)+x]==TRUE)
			{
				pel(scr,x,y,0)=255;
				pel(scr,x,y,1)=255;
				pel(scr,x,y,2)=255;

			}
			else
			{
				pel(scr,x,y,0)=0;
				pel(scr,x,y,1)=0;
				pel(scr,x,y,2)=0;
			}

		}
}

//fTestRoutine copies test pattern to buffer. Useful for troubleshooting everything. Helped in debugging of fDisplaySet
int fTestRoutine(int x,int y, SDL_Surface *scr, int *setarray)
{
    	for(y = 0; y < scr->h; ++y)
	    	for(x = 0; x < scr->w; ++x)
	    	{
			if(x%2==1 && y%2==1)
				{
					setarray[(y)*WIDTH+x]=TRUE;
				}
			else
				{
					setarray[(y)*WIDTH+x]=FALSE;
				}

		}
}

//CPU version of Mandelbrot drawing routine. Copies Mandelbrot to our Display buffer.
//Slow if there are many Mandelbrot pixels on the screen. 512 * 512 * 200 ~ 52.5 million iterations of code to finish.
int fDoMandelbrot5(float startx, float starty, float endx, float endy, int *setarray)
{

    int	x, y, count;
    float zr, zi, cr, ci;
    float rsquared, isquared;

    for (y = 0; y < WIDTH; y++)
    {
        for (x = 0; x < HEIGHT; x++)
        {
            zr = 0.0;
            zi = 0.0;
            cr = startx + x * (endx - startx) / WIDTH;
            ci = starty + y * (endy - starty) / HEIGHT;
            rsquared = zr * zr;
            isquared = zi * zi;

            for (count = 0; rsquared + isquared <= 4.0
                            && count < MaxIters; count++)
            {
                zi = zr * zi * 2;
                zi += ci;

                zr = rsquared - isquared;
                zr += cr;

                rsquared = zr * zr;
                isquared = zi * zi;
            }

            if (rsquared + isquared <= 4.0)
                setarray[(y)*WIDTH+x]=TRUE;
            else
                setarray[(y)*WIDTH+x]=FALSE;
        }
    }
}

int fDoMandelbrot6(float startx, float starty, float endx, float endy, int *setarray, cl_kernel kernel, cl_command_queue command_queue, cl_context context)
{

	cl_int ret;


	float *A = (float*)malloc(sizeof(float)*LIST_SIZE);
	float *B = (float*)malloc(sizeof(float)*LIST_SIZE);



// here comes complex number C initialization A is Re, B is Im
	for(int y=0;y<HEIGHT; y++){
		for(int x=0; x<WIDTH; x++) {
			A[y*WIDTH+x] = startx + (endx - startx)/WIDTH * x;
			B[y*WIDTH+x] = starty + (endy - starty)/HEIGHT * y;
		}
	}

    // Create memory buffers on the device for each vector
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            WIDTH * HEIGHT * sizeof(float), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            WIDTH * HEIGHT * sizeof(float), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            WIDTH * HEIGHT * sizeof(float), NULL, &ret);
    cl_mem d_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            WIDTH * HEIGHT * sizeof(float), NULL, &ret);

    // Copy the lists to their respective memory buffers

	ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
            WIDTH * HEIGHT * sizeof(float), A, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0,
            WIDTH * HEIGHT * sizeof(float), B, 0, NULL, NULL);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&d_mem_obj);

    // Execute the OpenCL kernel on the list
    size_t global_item_size = HEIGHT * WIDTH; // Process the entire lists
    size_t local_item_size = 32; // Process in groups of 512
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
            &global_item_size, &local_item_size, 0, NULL, NULL);

	float *C = (float*)malloc(sizeof(float)*LIST_SIZE);
	float *D = (float*)malloc(sizeof(float)*LIST_SIZE);

    // Read the memory buffer C on the device to the local variable C
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0,
            WIDTH * HEIGHT * sizeof(int), C, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, d_mem_obj, CL_TRUE, 0,
            WIDTH * HEIGHT * sizeof(int), D, 0, NULL, NULL);

    for (y = 0; y < WIDTH; y++)
    {
        for (x = 0; x < HEIGHT; x++)
        {
            if (C[(y)*WIDTH+x]*C[(y)*WIDTH+x] + D[(y)*WIDTH+x]*D[(y)*WIDTH+x] <= 4.0)
                setarray[(y)*WIDTH+x]=TRUE;
            else
                setarray[(y)*WIDTH+x]=FALSE;
        }
    }

    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseMemObject(d_mem_obj);


}
