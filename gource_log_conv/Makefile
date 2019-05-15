all: gource_log_conv

test: gource_log_conv.o
	g++ -o gource_log_conv gource_log_conv.o

test.o:
	g++ -Wall -W -c gource_log_conv.c -o gource_log_conv.o

clean:
	rm -f *.o *.a *.linkinfo a.out *~ gource_log_conv
