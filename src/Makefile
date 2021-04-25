#Makefile

make: ftpclient.c ftpserver.c
	gcc -pthread -g -Wall ftpclient.c -o ftpclient
	
	gcc -pthread -g -Wall ftpserver.c -o ftpserver
	

clean:
	rm ./ftpclient ./ftpserver
