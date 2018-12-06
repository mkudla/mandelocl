__kernel void experimental4(__global float *A, __global float *B, __global float *C, __global float *D) {
    
    // Get the index of the current element
    int i = get_global_id(0);

    // Do the operation
	float zr = A[i];
	float zi = B[i];
	float tempzi;

	//barrier(CLK_LOCAL_MEM_FENCE);

		for(int iter = 0; iter < 30; iter++)
		{
			tempzi = zi;
			zi = 2 * zi * zr; // Im part needs to come first, because it depends on C
						// Ay, I need scratchpad memory!!!
			//barrier(CLK_LOCAL_MEM_FENCE);
			zr = zr * zr - tempzi * tempzi; // more

		//fence
			//barrier(CLK_LOCAL_MEM_FENCE);
			zr += A[i];
			zi += B[i];
		//loop end
			//barrier(CLK_LOCAL_MEM_FENCE);
		}

C[i] = zr;
D[i] = zi;

}
