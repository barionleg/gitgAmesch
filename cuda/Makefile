cudatest: 
	nvcc -lib cudamesh.cu -o libcudamesh.a
	g++ main.c -I/usr/include/cuda/ -lcuda -lcudart -L/usr/local/cuda/lib -lcudamesh -L. -o cudatest

clean:
	rm -f *.o *.a *.linkinfo a.out *~ cudatest

