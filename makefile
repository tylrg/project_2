all:
	gcc -g -Wall -o driver.exe driver.c
driver: driver.c
	gcc -o -Wall -c driver driver.c
clean:
		-rm driver.exe
