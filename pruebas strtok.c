#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>

int main(int argc, char *argv[]) {
	
	char usuario[512];
	strcpy(usuario, "5/1/Ems");
	char *p = strtok (usuario, "/"); 
	printf ("%s\n", p);
	
	p = strtok (NULL, "/");
	printf( "%s\n", p);
	
	p = strtok (NULL, "/"); 
	printf ("%s\n", p);
	char nombre[20];
	
	return 0;
}



