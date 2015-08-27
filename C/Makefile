a.out: libsc.a
	gcc main.c libsc.a -lpthread

libsc.a:
	gcc -c digest.c encodings.c sc.c sceda.c
	ar rcs libsc.a digest.o encodings.o sc.o sceda.o
	rm digest.o encodings.o sc.o sceda.o
