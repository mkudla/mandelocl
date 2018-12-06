# mandelocl - using OpenCL from C - demonstration with Mandelbrot set

![Screenshot](https://github.com/mkudla/madelocl/blob/master/screen1.png)
![Screenshot](https://github.com/mkudla/madelocl/blob/master/screen2.png)

## use

Use keys to move around 

z - zoom in

x - zoom out

a - increase iterations +1

s - decrease iterations -1

c - toggle on / off openCL


## compilation

// To compile run:
//64 bit:

gcc -c -I /opt/AMDAPP/include -std=c99 my_prog_disp_7_oc.c -o my_prog_disp7oc.o `sdl-config --cflags --libs`
gcc my_prog_disp7oc.o -o disp7oc -L /opt/AMDAPP/lib/x86_64/ -l OpenCL `sdl-config --cflags --libs`

//32 bit:

gcc -c -I /opt/AMDAPP/include -std=c99 my_prog_disp_7_oc.c -o my_prog_disp7oc.o `sdl-config --cflags --libs`
gcc my_prog_disp7oc.o -o disp7oc -L /opt/AMDAPP/lib/x86/ -l OpenCL `sdl-config --cflags --libs`

## requirements
It requires SDL and OpenCL libraries. You can use ones provided by either Intel, AMD or NVIDIA. Above compilation instructions assume the use of AMD libraries, because they will run even if no openCL gpu is present. In this case they will just try to serialize CPU (no matter Intel or AMD), still running faster than single threaded conventional code.
