#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>

int main(int argc, char *argv[])
{
	int sock_conn, sock_listen, ret;
	struct sockaddr_in serv_adr;
	char peticion[512];
	char respuesta[512];
	
	MYSQL *conn;
	int err;
	// Estructura especial para almacenar resultados de consultas -Wall -pedantic-errors -O0 -lm
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	MYSQL_ROW row_user;
	MYSQL_ROW row_connected;
	//char username[30];
	char pwd[20];
	int playerID;
	char consulta[80];

	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	
	
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la maquina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// escucharemos en el port 9080
	serv_adr.sin_port = htons(9100);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	//La cola de peticiones pendientes no podr? ser superior a 3
	if (listen(sock_listen, 3) < 0) 
		printf("Error en el Listen");
	
	//Creamos la conexi�n al servidor MYSQL
	conn = mysql_init(NULL);
	if (conn==NULL) {
		printf ("Error al crear la conexion: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	//inicializar la conexion
	conn = mysql_real_connect (conn,"localhost","root","mysql","datagame",0,NULL,0);
	if (conn==NULL) {
		printf ("Error al inicializar la conexion: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	int i;
	// Atenderemos solo 5 peticiones
	for(;;){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL); //El servidor espera hasta que alguien se conecta
		printf ("He recibido conexion\n");
		//sock_conn es el socket que usaremos para este cliente
		
		//Bucle de atenci�n al cliente
		int terminar = 0;
		while (terminar == 0)
		{
			// Ahora recibimos su nombre, que dejamos en buff. Recoge la peticion
			ret=read(sock_conn,peticion, sizeof(peticion));
			printf ("Recibido\n");
			
			// Tenemos que a�adirle la marca de fin de string 
			// para que no escriba lo que hay despues en el buffer
			peticion[ret]='\0';
			
			//Peticion de determinacion de lo que piden
			//Escribimos el nombre en la consola
			
			printf ("El username introducido es: %s\n", peticion);
			
			
			char *p = strtok( peticion, "/"); //Split, cogemos el primer numero
			int codigo =  atoi (p); // Convertimos "1" en integer  5/1/Ems
			char nombre[30]; 
			
			if(codigo != 0)
			{
				p = strtok( NULL, "/");
				strcpy (nombre, p);
				printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
			}
			if (codigo == 0)
				terminar = 1;
			
			if (codigo == 1) 
			{
				//sprintf (respuesta,"%d",strlen (nombre)); //piden la longitd del nombre
				strcpy(consulta, "SELECT COUNT(winner) FROM GAME WHERE winner ='");
				strcat(consulta, nombre);
				strcat(consulta,"'");
				
				
				//hacemos la consulta
				err=mysql_query(conn, consulta);
				if(err!=0)
				{
					printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
					exit(1);
					
				}
				
				//Recogemos el resultado de la consulta
				resultado = mysql_store_result (conn); 
				row = mysql_fetch_row (resultado);
				
				if (row == NULL){
					
					printf ("No se han obtenido datos en la consulta\n");}
				
				else{
					// El resultado debe ser una matriz con una sola fila
					// y una columna que contiene el numero de victorias
					printf ("Numero de victorias: %s\n", row[0] );
					sprintf(respuesta, "%s", row[0]);
					
				}
			}
			if (codigo == 2)
			{
				//SELECT day, COUNT(day) AS total FROM GAME GROUP BY day ORDER BY total DESC;
				
				
				strcpy(consulta, "SELECT day, COUNT(day) AS total FROM GAME GROUP BY day ORDER BY total DESC");
				
				err=mysql_query(conn, consulta);
				if(err!=0)
				{
					printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
					exit(1);
				}
				
				//Recogemos el resultado de la consulta
				resultado = mysql_store_result (conn); 
				row = mysql_fetch_row (resultado);
				
				if (row == NULL){
					
					printf ("No se han obtenido datos en la consulta\n");}
				
				else{
					printf ("D�a", row[0] );
					sprintf(respuesta, "%s", row[0]);
					
				}
				
				
			}

			
				
			if(codigo == 3)
			{
				err=mysql_query (conn, "SELECT MAX(id) FROM PLAYER");
				if (err!=0) {
					printf ("Error al consultar datos de la base %u %s\n",
							mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				resultado = mysql_store_result(conn);
				row = mysql_fetch_row(resultado); 
				
				if (row == NULL)
					printf ("No se han obtenido datos en la consulta\n");
				else
					strcpy(respuesta, row[0]);
			}
			if (codigo == 15)
				{
				
					printf ("Entro en indice 2\n");
					strcpy(consulta, "SELECT username FROM PLAYER WHERE id = ");
					printf ("%s\n", consulta);
					char id[20];
					sprintf(id, "%s", nombre);
					strcat(consulta, id);
					printf ("%s\n", consulta);
					
					err=mysql_query (conn, consulta);
					if (err!=0) {
						printf ("Error al consultar datos de la base %u %s\n",
								mysql_errno(conn), mysql_error(conn));
						exit (1);
					}
					resultado = mysql_store_result(conn);
					row_user = mysql_fetch_row(resultado);
					
/*					strcpy(consulta, "SELECT connected FROM CONNECT WHERE id_P = ");*/
/*					strcat(consulta, id);*/
					
/*					err=mysql_query (conn, consulta);*/
/*					if (err!=0) {*/
/*						printf ("Error al consultar datos de la base %u %s\n",*/
/*								mysql_errno(conn), mysql_error(conn));*/
/*						exit (1);*/
/*					}*/
/*					resultado = mysql_store_result(conn);*/
/*					row_connected = mysql_fetch_row(resultado);*/
					
					if (row_user == NULL)
						printf ("No se han obtenido datos en la consulta\n");
					else
						printf ("%s\n", row_user[0]);
						sprintf(respuesta, "%s", row_user[0]);
					
				}
			
			
			if (codigo == 4) //LOG IN
			{
				
					
					strcpy(consulta, "SELECT pwd FROM PLAYER WHERE username ='");
					strcat(consulta, nombre);
					strcat(consulta,"'");
					
					//hacemos la consulta
					err=mysql_query(conn, consulta);
					if(err!=0)
					{
						printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
						exit(1);
						
					}
					//Recogemos el resultado de la consulta
					resultado = mysql_store_result (conn); 
					row = mysql_fetch_row (resultado);
					
					if (row == NULL){
						
						printf ("No se han obtenido datos en la consulta\n");
						strcpy(respuesta, "NO");
					
					}
					
					
					else{
						// El resultado debe ser una matriz con una sola fila
						// y una columna que contiene el numero de victorias
						printf ("SI");
						sprintf(respuesta, "%s", row[0]);
					}	
				
				
				
			}
			if (codigo == 5) //REGISTER
			{
				// nombre/contrase�a
				
				strcpy(consulta, "SELECT id FROM PLAYER WHERE username ='");
				strcat(consulta, nombre);
				strcat(consulta,"'");
				
				printf ("%s\n", consulta);
				
				err=mysql_query (conn, consulta);
				if (err!=0) {
					printf ("Error al consultar datos de la base %u %s\n",
							mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				resultado = mysql_store_result(conn);
				row = mysql_fetch_row(resultado); 
				
				
				
				if (row == NULL)
				{
					
					printf ("Usuario todavia no registrado\n");
					
					
					p = strtok (NULL, "/");
					char pwd [30];
					strcpy(pwd, p);
					strcpy(consulta, "INSERT INTO PLAYER (username, pwd) VALUES ('");
					strcat(consulta, nombre);
					strcat(consulta, "', '");
					strcat(consulta, pwd);
					strcat(consulta, "')");
					
					printf ("%s\n", consulta);
					
					err=mysql_query(conn, consulta);
					if(err!=0)
					{
						printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
						printf ("No se ha podido registrar al usuario");
						sprintf(respuesta, "%s", "NO");
						exit(1);
						
					}
					else
						sprintf(respuesta, "%s", "REGISTRADO");
					
					
					
				}
				
				else
				{
					printf ("%s\n", "NO");
					sprintf(respuesta, "%s", "NO");
				}	
				
			}
			if (codigo == 96) //BUSCAR ID
			{
				strcpy(consulta, "SELECT id FROM PLAYER WHERE username = '");
				strcat(consulta, nombre);
				strcat(consulta, "'");
				printf("%s\n", consulta); 
				
				err=mysql_query (conn, consulta);
				if (err!=0) {
					printf ("Error al consultar datos de la base %u %s\n",
							mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				resultado = mysql_store_result(conn);
				row = mysql_fetch_row(resultado); 
				
				if (row == NULL)
					printf ("No se han obtenido datos en la consulta\n");
				else
				{
					sprintf(respuesta,"%s", row[0]);
					
					
				}
			}
			
			if (codigo == 6) //PREGUNTAR QUE SOCKET ME TOCA
			{
				printf ("%s\n", "SOCKET");
				
				strcpy(consulta, "SELECT MAX(socket) FROM CONNECT");
				
				printf ("%s\n", consulta);
				
				err=mysql_query (conn, consulta);
				
				
				
				
/*				if (err == NULL)*/
/*				{*/
/*					printf("Error al consultar datos de la base %u\n ", mysql_errno(conn) );*/
/*					printf("Error %s\n", mysql_error(conn));*/
/*					strcpy(respuesta, "NO");*/
/*					exit(1);*/
					
/*				}*/
				
			
				resultado = mysql_store_result(conn);
				row = mysql_fetch_row(resultado); 
				
				if (row == NULL)
					printf ("No se han obtenido datos en la consulta\n");
				else
					sprintf(respuesta,"%s", row[0]);
				
				
			}
			
			if (codigo == 7) //INSERTAR VALORES EN CONECTADO EN LOG IN 
			{
				// socket/id
				strcpy(consulta, "INSERT INTO CONNECT (socket, id_P) VALUES (");
				strcat(consulta, nombre);
				printf ("%s\n", consulta);
				strcat(consulta,", ");
				printf ("%s\n", consulta);
				p = strtok (NULL, "/");
				char socket [30];
				strcpy (socket, p);
				strcat(consulta, socket);
				printf ("%s\n", consulta);
				strcat(consulta, ")");
				
				printf ("%s\n", consulta);
				
				//hacemos la consulta
				err=mysql_query(conn, consulta);
				if(err!=0)
				{
					printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
					strcpy(respuesta, "NO");
					exit(1);
					
				}
				else
				{
				   strcpy(respuesta, "SI");
				}
				
			}
			
			
			if (codigo == 8) //ELIMINAR DATOS DE CONECTADO 
			{
				// id
				strcpy(consulta, "DELETE FROM CONNECT WHERE id_P =");
				strcat(consulta, nombre);
				
				
				//hacemos la consulta
				err=mysql_query(conn, consulta);
				if(err!=0)
				{
					printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
					strcpy(respuesta, "NO");
					exit(1);
					
				}
				else
				{
					strcpy(respuesta, "SI");
				}
				
			}
			if (codigo == 9) //ENCONTRAR NUMBERO DE CONECTADOS
			{
				strcpy(consulta, "SELECT MAX(id_C) FROM CONNECT");
				
				//hacemos la consulta
				err=mysql_query(conn, consulta);
				if(err!=0)
				{
					printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
					strcpy(respuesta, "NO");
					exit(1);
					
				}
				
				resultado = mysql_store_result(conn);
				row = mysql_fetch_row(resultado); 
				
				if (row == NULL)
					printf ("No se han obtenido datos en la consulta\n");
				else
					sprintf(respuesta,"%s", row[0]);			
			}
			if (codigo == 10) //DAME EL NOMBRE JUGADOR TAL ONLINE
			{
				// id_C
				strcpy(consulta, "SELECT id_P FROM CONNECT WHERE id_C = ");
				strcat(consulta, nombre);
				
				//hacemos la consulta
				err=mysql_query(conn, consulta);
				if(err!=0)
				{
					printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
					strcpy(respuesta, "NO");
					exit(1);
					
				}
				
				resultado = mysql_store_result(conn);
				row = mysql_fetch_row(resultado); 
				
				if (row == NULL)
				{
					printf ("No se han obtenido datos en la consulta\n");
					strcpy(respuesta, "NO");
					
				}
				else
				{	
					strcpy(consulta, "SELECT username FROM PLAYER WHERE id = ");
					strcat(consulta, row[0]);


					err=mysql_query(conn, consulta);
					if(err!=0)
					{
						printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
						exit(1);

					}

					/*Recogemos el resultado de la consulta*/
					resultado = mysql_store_result (conn); 
					row = mysql_fetch_row (resultado);

					if (row == NULL){

						printf ("No se han obtenido datos en la consultaaaa\n");
						strcpy(respuesta, "NO");

					}


					else	
					{
						/*El resultado debe ser una matriz con una sola fila*/
						/*y una columna que contiene el numero de victorias*/
						printf ("SI");
						sprintf(respuesta, "%s", row[0]);
					
					}
				}
			}
			
			
			if (codigo != 0)
			{
				printf ("%s\n", respuesta);
				// Y lo enviamos
				write (sock_conn, respuesta, strlen(respuesta));
			}
		
				
		}
		
		// Se acabo el servicio para este cliente
		close(sock_conn); 
				
				
	}
}
