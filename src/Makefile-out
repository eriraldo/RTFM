#Makefile

make: ftpclient.c ftpserver.c
	gcc -pthread -g -Wall ftpclient.c -o ftpclient
	
	gcc -pthread -g -Wall ftpserver.c -o ftpserver ../lib/utils.c
	

clean:
	rm ../bin/ftpclient ../bin/ftpserver
