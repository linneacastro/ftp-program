all: myftpserve myftp
myftpserve: myftpserve.o myftp.h
	gcc -g -o myftpserve myftpserve.o
myftpserve.o: myftpserve.c myftp.h
	gcc -g -c myftpserve.c
myftp: myftp.o myftp.h
	gcc -g -o myftp myftp.o
myftp.o: myftp.c myftp.h
	gcc -g -c myftp.c
clean:
	rm myftpserve.o myftpserve myftp.o myftp
