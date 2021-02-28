#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define N_PARADAS 5// número de paradas de la ruta
#define EN_RUTA 0// autobús en ruta
#define EN_PARADA 1// autobús en la parada
#define MAX_USUARIOS 40 // capacidad del autobús
#define USUARIOS 4// numero de usuarios

// Variables Globales

pthread_t th_user[USUARIOS], th_bus;// Crear el array de Users

pthread_mutex_t cerrojo;

pthread_cond_t subida;
pthread_cond_t bajada;
pthread_cond_t autobus;

int estado = EN_RUTA;// estado inicial
int parada_actual = 0;// parada en la que se encuentra el autobus
int n_ocupantes= 0;// ocupantes que tiene el autobús

int esperando_parada[N_PARADAS] = {0,0,0,0,0}; // personas que desean subir en cada parada
int esperando_bajar[N_PARADAS]  = {0,0,0,0,0}; // personas que desean bajar en cada parada

void Usuario(int id_usuario, int origen, int destino);

void Autobus_En_Parada();
void Conducir_Hasta_Siguiente_Parada();

void Subir_Autobus(int id_usuario, int origen);
void Bajar_Autobus(int id_usuario, int destino);

void * thread_autobus(void * args);
void * thread_usuario(void * arg);


/*
Ajustar el estado y bloquear al autobús hasta que no haya pasajeros que quieran
bajar y/o subir la parada actual. Después se pone en marcha */
void Autobus_En_Parada(){
	pthread_mutex_lock(&cerrojo);
	estado = EN_PARADA;
	// Si soy el ultimo entonces despierto a todos
	while((esperando_bajar[parada_actual] > 0 ) || (esperando_parada[parada_actual] > 0 )){
	
	    pthread_cond_broadcast(&autobus);
		while(esperando_bajar[parada_actual] > 0){
			pthread_cond_wait(&bajada, &cerrojo);
		}
		
		while(esperando_parada[parada_actual] > 0){
			pthread_cond_wait(&subida, &cerrojo);
		}
	}
	pthread_mutex_unlock(&cerrojo);
}
// Retardo
// Actualizar numero de parada
void Conducir_Hasta_Siguiente_Parada(){
	pthread_mutex_lock(&cerrojo);
	estado = EN_RUTA;
	printf("----------  Autobus en ruta  ----------\n");
	pthread_mutex_unlock(&cerrojo);

        sleep(3);

	pthread_mutex_lock(&cerrojo);
	parada_actual = (parada_actual+1) % N_PARADAS; // Ya que es una ruta circular
	printf("Autobus acaba de llegar a la parada [%d].\n", parada_actual);
	pthread_mutex_unlock(&cerrojo);
}
/*
El usuario indicará que quiere subir en la parada ’origen’, esperará a que el
autobús se pare en dicha parada y subirá. El id_usuario puede utilizarse para
proporcionar información de depuración */
void Subir_Autobus(int id_usuario, int origen){
	pthread_mutex_lock(&cerrojo);
	esperando_parada[origen]++;
	
	printf("Usuario [%d] esperando en la parada [%d] para SUBIR.\n", id_usuario, origen);

	while((parada_actual != origen) || (estado != EN_PARADA)){
		pthread_cond_wait(&autobus, &cerrojo); // Habiamos puesto subida
	}
	esperando_parada[origen]--;
	n_ocupantes++;
	
	printf("Usuario [%d] en la parada [%d] acaba de SUBIR al autobus.\n", id_usuario, origen);
	
	if(esperando_parada[origen] == 0) {pthread_cond_broadcast(&subida);}

	pthread_mutex_unlock(&cerrojo);
}
/*
El usuario indicará que quiere bajar en la parada ’destino’, esperará a que el
autobús se pare en dicha parada y bajará. El id_usuario puede utilizarse para
proporcionar información de depuración */
void Bajar_Autobus(int id_usuario, int destino){
	pthread_mutex_lock(&cerrojo);
	esperando_bajar[destino]++;

	printf("Usuario [%d] esperando para BAJAR en la parada [%d].\n", id_usuario, destino);

	while((parada_actual != destino) || (estado != EN_PARADA)){
		pthread_cond_wait(&autobus, &cerrojo);	
	}
	
	esperando_bajar[destino]--;
	n_ocupantes--;

	printf("Usuario [%d] en la parada [%d] acaba de BAJAR del autobus.\n", id_usuario, destino);
	
	if(esperando_bajar[destino] == 0) {pthread_cond_broadcast(&bajada);}

	pthread_mutex_unlock(&cerrojo);

}
// Definiciones globales (comunicación y sincronización)
void * thread_autobus(void * args){
	while (1) {// esperar a que los viajeros
	    Autobus_En_Parada();// suban y bajen

		sleep(1);// insertar sleep(1) para retardo

		Conducir_Hasta_Siguiente_Parada();// conducir hasta siguiente parada
	}
}
void * thread_usuario(void * arg){
	int id_usuario = *((int*) arg);
	int a, b;
	// obtener el id del usario
	while (1) {
		a = rand() % N_PARADAS;
		do{
			b = rand() % N_PARADAS;
		} while(a == b);
		printf("--- El usuario [%d] ha decidido hacer la ruta [%d] -> [%d].\n", id_usuario, a, b);
		Usuario(id_usuario,a,b);
	}
}
void Usuario(int id_usuario, int origen, int destino){
	// Esperar a que el autobus esté
	Subir_Autobus(id_usuario, origen);// en parada origen para subir
	Bajar_Autobus(id_usuario, destino);// Bajarme en estación destino
}
int main(int argc, char* argv[]){
	int i;
	pthread_mutex_init(&cerrojo, NULL);
	pthread_cond_init(&bajada, NULL);
	pthread_cond_init(&subida, NULL);
	pthread_cond_init(&autobus, NULL);

    // Crear el thread Autobus
	pthread_create(&th_bus, NULL, thread_autobus, NULL);
	for (i = 0; i < USUARIOS; i++){		
		pthread_create(&th_user[i], NULL, thread_usuario, (void*)&i);
	}
	
	// Esperar terminación de los hilos
	pthread_join(th_bus,NULL);
	for(i=0; i <USUARIOS; i++){
		pthread_join(th_user[i],NULL);
	}
	
	return 0;
}

