#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>
#include <pthread.h>

int contador;


//Estructura necesaria para acesso excluyente
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *AtenderCliente (void *socket)
{
	//Generamos las variables
	int sock_conn, ret;
	int *s;
	s= (int *) socket;
	sock_conn= *s;
	
	char pwd[20];
	int playerID;
	char consulta[80];
	
	char peticion[512];
	char respuesta[512];
	
	//TODO LO NECESARIO PARA SQL
	
	// Estructura especial para almacenar resultados de consultas -Wall -pedantic-errors -O0 -lm
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	MYSQL_ROW row_user;
	MYSQL_ROW row_connected;
	MYSQL *conn;
	int err;
	
	//Creamos la conexión al servidor MYSQL
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
	
	//Bucle de atención al cliente (Aqui encontramos todas las posibles peticiones)
	int terminar = 0;
	while (terminar == 0)
	{
		// Ahora recibimos su nombre, que dejamos en buff. Recoge la peticion
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		
		// Tenemos que añadirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		
		//Peticion de determinacion de lo que piden
		//Escribimos el nombre en la consola
		
		printf ("\tLa peticion del socket es: %s\n", peticion);
		
		
		char *p = strtok( peticion, "/"); //Split, cogemos el primer numero
		int codigo =  atoi (p); // Convertimos "1" en integer  5/1/Ems
		char nombre[30]; 
		

		if (codigo != 0)
		{
			
			
			printf("\tEl codigo es diferente de 0\n");
			p = strtok( NULL, "/");
			strcpy (nombre, p);
			printf ("\tCodigo: %d, Nombre: %s\n", codigo, nombre);
			
			
		}
		if (codigo == 0)
		{
			
			
			printf("\tEl codigo es igual a 0, se finaliza el programa\n } \n");
			terminar = 1;
			
			
		}
		
		if (codigo == 1) //Queremos saber las victorias de un jugador concreto
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			printf("\tEntrado en contar victorias del jugador (codigo==1)\n");
			
			strcpy(consulta, "SELECT COUNT(winner) FROM GAME WHERE winner ='");
			strcat(consulta, nombre);
			strcat(consulta,"'");
			
			printf("\tHacemos consulta a mySQL\n");
			
			//hacemos la consulta
			err=mysql_query(conn, consulta);
			if(err!=0)
			{
				printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
				exit(1);
				
			}
			
			printf("\tConsulta realizada con exito\n");
			
			//Recogemos el resultado de la consulta
			resultado = mysql_store_result (conn); 
			row = mysql_fetch_row (resultado);
			
			if (row == NULL){
				
				printf ("\t\tNo se han obtenido datos en la consulta\n");}
			
			else{
				// El resultado debe ser una matriz con una sola fila
				// y una columna que contiene el numero de victorias
				printf ("\tEl numero de victorias es: %s\n", row[0] );
				sprintf(respuesta, "%s", row[0]);
				
			}
			printf("\tSe acaba la consulta 1\n"); 
			
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		if (codigo == 2) //Consulta para saber que día hubo más partidas
		{	
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			printf("\tEntramos en la consulta del día con mas partidas(codigo==2)\n");
			
			strcpy(consulta, "SELECT day, COUNT(day) AS total FROM GAME GROUP BY day ORDER BY total DESC");
			
			printf("\tHacemos consulta a mySQL\n");
			
			err=mysql_query(conn, consulta);
			if(err!=0)
			{
				printf("Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
				exit(1);
			}
			
			printf("\tConsulta realizada con exito\n");
			//Recogemos el resultado de la consulta
			resultado = mysql_store_result (conn); 
			row = mysql_fetch_row (resultado);
			
			if (row == NULL){
				
				printf ("No se han obtenido datos en la consulta\n");}
			
			else{
				printf ("\t El día obtenido es: %s \n", row[0] );
				sprintf(respuesta, "%s", row[0]);
				
			}
			printf("\tAcaba la consulta con codigo 2\n");
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
			
		}
		
		
		
		if(codigo == 3) //Seleccionamos el id más grande que haya en la tabla player
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			printf("\tEntramos en la consulta de id más grande (codigo==3)\n");
			
			err=mysql_query (conn, "SELECT MAX(id) FROM PLAYER");
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			printf("\tConsulta a MySQL realiada con exito\n");
			resultado = mysql_store_result(conn);
			row = mysql_fetch_row(resultado); 
			
			if (row == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
			{	
				printf("El maximo id es: %s \n", row[0]); 
				strcpy(respuesta, row[0]);
			}
			printf("\tSe acaba la consulta 3\n"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		if (codigo == 15) //Consultamos el username de un id introducido
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora	
			printf ("\tEntramos en la consulta del username (codigo==15)\n");
			
			strcpy(consulta, "SELECT username FROM PLAYER WHERE id = ");
			printf ("%s\n", consulta);
			char id[20];
			sprintf(id, "%s", nombre); //copiamos el strtok nombre a la variable id
			strcat(consulta, id); //incluimos con strcat el id a la consulta
			
			printf ("\tLa consulta que mandamos a MySQL es: %s\n", consulta); 
		
			err=mysql_query (conn, consulta);
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			resultado = mysql_store_result(conn);
			row_user = mysql_fetch_row(resultado);
						
			printf("\tConsulta realizada con exito\n");
			
			if (row_user == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
			{
				printf ("\tEl usuario encontrado es: %s\n", row_user[0]);
				sprintf(respuesta, "%s", row_user[0]);
			}
			printf("\tSe acaba la consulta 15\n"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		
		
		if (codigo == 4) //LOG IN
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			printf("\tEntramos en la consulta de Log In (consulta==4)\n");
			
			strcpy(consulta, "SELECT pwd FROM PLAYER WHERE username ='");
			strcat(consulta, nombre);
			strcat(consulta,"'");
			
			printf ("\tLa consulta que mandamos a MySQL es: %s\n", consulta); 
			
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
			
			printf("\tConsulta realizada con exito\n");
			
			if (row == NULL){
				
				printf ("No se han obtenido datos en la consulta\n");
				strcpy(respuesta, "NO");
				
			}
			
			
			else{
				// El resultado debe ser una matriz con una sola fila
				// y una columna que contiene el numero de victorias
				printf ("\tHemos obtenido la contraseña con exito\n");
				sprintf(respuesta, "%s", row[0]);
			}	
			printf("\tSe acaba la consulta de Log In\n"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
			
			
		}
		if (codigo == 5) //REGISTER
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			// entramos el strtok como: nombre/contraseña
			
			printf("\tEntramos en Register, (consulta==5)\n");
			
			strcpy(consulta, "SELECT id FROM PLAYER WHERE username ='");
			strcat(consulta, nombre);
			strcat(consulta,"'");
			
			printf ("\tLa consulta que enviamos es: %s\n", consulta);
			
			err=mysql_query (conn, consulta);
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			resultado = mysql_store_result(conn);
			row = mysql_fetch_row(resultado); 
			
			printf("\tConsulta realizada con exito\n\t{\n");
			
			if (row == NULL)
			{
				printf("\t  Como el usuario no esta en el registro lo incluimos\n");
				printf ("\t  Usuario todavia no registrado\n");
				
				
				p = strtok (NULL, "/");
				char pwd [30];
				strcpy(pwd, p);
				strcpy(consulta, "INSERT INTO PLAYER (username, pwd) VALUES ('");
				strcat(consulta, nombre);
				strcat(consulta, "', '");
				strcat(consulta, pwd);
				strcat(consulta, "')");
				
				printf ("\t  La consulta que enviamos es: %s\n", consulta);
				
				err=mysql_query(conn, consulta);
				if(err!=0)
				{
					printf("\t  Error al consultar datos de la base %u%s\n", mysql_errno(conn), mysql_error(conn));
					printf ("\t  No se ha podido registrar al usuario\n");
					sprintf(respuesta, "%s", "NO");
					exit(1);
					
				}
				else
				   printf("\t  Usuario registrado con exito\n\t}\n");
				   sprintf(respuesta, "%s", "REGISTRADO");
				
				
				
			}
			
			else
			{
				printf ("\tYa existe un usuario con ese username, escoja otro por favor\n");
				sprintf(respuesta, "%s", "NO");
			}
			printf("\tSe acaba la consulta de Register\n"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
			
		}
		if (codigo == 96) //BUSCAR ID
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			//METODO EXCLUYENTE
			
			
			printf("\tEntramos en Busca de ID, (consulta==96)\n");
			
			strcpy(consulta, "SELECT id FROM PLAYER WHERE username = '");
			strcat(consulta, nombre);
			strcat(consulta, "'");
			printf("%s\n", consulta); 
			
			printf ("\tLa consulta que enviamos es: %s\n", consulta);
			
			err=mysql_query (conn, consulta);
			if (err!=0) {
				printf ("\t	Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			resultado = mysql_store_result(conn);
			row = mysql_fetch_row(resultado); 
			printf("\tConsulta realizada con exito\n");
			
			if (row == NULL)
				printf ("\t No se han obtenido datos en la consulta\n");
			else
			{
				printf("\t El id es: %s", row[0]);
				sprintf(respuesta,"%s", row[0]);
			}
			
			//METEMOS EL EXCLUYENTE
			
			
			printf("\tSe acaba la consulta de Buscar ID\n");
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		
		if (codigo == 6) //PREGUNTAR QUE SOCKET ME TOCA
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			printf ("\tEntramos en la consulta SOCKET, (consulta==6)\n");
			
			strcpy(consulta, "SELECT MAX(socket) FROM CONNECT");
			
			printf ("%s\n", consulta);
			
			err=mysql_query (conn, consulta);

			resultado = mysql_store_result(conn);
			row = mysql_fetch_row(resultado); 
			
			printf("\tConsulta realizada con exito\n");
			
			if (row == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
			{
				printf("\tEl socket asignado es: %s",row[0]);
				sprintf(respuesta,"%s", row[0]);
			}
			printf("\tSe acaba la consulta SOCKET\n"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		
		if (codigo == 7) //INSERTAR VALORES EN CONECTADO EN LOG IN 
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			// la estructura que nos llega de nombre es: socket/id
			
			printf ("\tEntramos en la consulta CONECTADO, (consulta==7)\n");
			
			strcpy(consulta, "INSERT INTO CONNECT (socket, id_P) VALUES (");
			strcat(consulta, nombre);
			
			strcat(consulta,", ");
			
			p = strtok (NULL, "/");
			char socket [30];
			strcpy (socket, p);
			strcat(consulta, socket);
			
			strcat(consulta, ")");
			
			printf ("la consulta es: %s\n", consulta);
			
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
				printf("\tConsulta realizada con exito\n");
				printf("\tSe ha introducido correctamente en la lista\n");
				strcpy(respuesta, "SI");
			}
			printf("\tSe acaba la consulta CONNECT\n"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
			
		}
		
		
		if (codigo == 8) //ELIMINAR DATOS DE CONECTADO 
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			// nos llega el strtok como: id
			printf ("\tEntramos en la consulta DESCONECTADO, (consulta==8)\n");
			
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
				printf("\tSe ha eliminado correctamente en la lista\n");
				strcpy(respuesta, "SI");
			}
			printf("\tSe acaba la consulta DISCONNECT\n"); 
			
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
			
		}
		if (codigo == 9) //ENCONTRAR NUMBERO DE CONECTADOS
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			printf ("\tEntramos en la consulta Numero de Conectados, (consulta==9)\n");
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
			{
				printf("El numero de conectados es: %s", row[0]);
				sprintf(respuesta,"%s", row[0]);			
			}
			printf("\tSe acaba la consulta Numero de Conectados\n"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		if (codigo == 10) //DAME EL NOMBRE JUGADOR TAL ONLINE
		{
			// Recibimos del strtok = id_C
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			printf ("\tEntramos en la consulta Obtener Conectado, (consulta==10)\n");
			
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
				printf ("\tEl usuario no esta conectado\n");
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
					printf ("\tEl usuario no esta conectado\n");
					printf ("No se han obtenido datos en la consultaaaa\n");
					strcpy(respuesta, "NO");
					
				}
				
				
				else	
				{
					/*El resultado debe ser una matriz con una sola fila*/
					/*y una columna que contiene el numero de victorias*/
					printf ("\tEl usuario: %s esta conectado",row[0]);
					sprintf(respuesta, "%s", row[0]);
					
				}
			}
			printf("\tSe acaba la consulta Obtener Conectado\n"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		if (codigo == 11)
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			sprintf(respuesta, "%d", contador); 
			printf ("El contador ha sido enviado"); 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		
		
		if (codigo != 0)
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			printf ("%s\n", respuesta);
			// Y lo enviamos
			write (sock_conn, respuesta, strlen(respuesta));
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme
		}
		if ((codigo == 1) || (codigo = 2) || (codigo = 3) || (codigo = 4) || (codigo = 5) || (codigo = 6) || (codigo = 7) || (codigo = 8) || (codigo = 9) || (codigo = 10) || (codigo = 15) || (codigo = 96) )
		{
			pthread_mutex_lock( &mutex ); //NO me interrumpas ahora
			contador ++; 
			pthread_mutex_unlock( &mutex);//Ya puedes interrumpirme

		}
		
	}
	
	// Se acabo el servicio para este cliente
	close(sock_conn); 
	
	
	
	
}



int main(int argc, char *argv[])
{
	//MAIN DEL SERVIDOR
	
	
	//GENERAMOS LAS VARIABLES DE LOS SOCKETS 
	int sock_conn, sock_listen;
	struct sockaddr_in serv_adr;

	// INICIALIZACIONES
	
	// Abrimos el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	
	// Hacemos el bind al puerto
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la maquina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// escucharemos en el port 9080
	serv_adr.sin_port = htons(9007);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	//La cola de peticiones pendientes no podr? ser superior a 3
	if (listen(sock_listen, 3) < 0) 
		printf("Error en el Listen");
	
	//Generamos las variables para el for
	contador = 0;
	int i;
	int sockets[100]; //Vector de Sockets, que serán las conexiones de los usuarios
	pthread_t thread[100]; //Vector de threads
	
	// Atenderemos peticiones, en este caso 100, pero queremos infinitas (preguntar)
	for(i=0;i<100;i++){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL); //El servidor espera hasta que alguien se conecta
		printf ("He recibido conexion\n");
		printf("\n");
		sockets[i] = sock_conn;
		//sock_conn es el socket que usaremos para este cliente
		
		//Crear thread y decirle lo que tiene que hacer
		printf("Creamos thread\n{\n");
		pthread_create (&thread[i], NULL, AtenderCliente,&sockets[i]);			
		
	}
}
