#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TOTAL_CLIENTES 50
#define MAX_DENTRO 20
#define MAX_SOFA 4
#define MAX_SILLAS 3

sem_t sem_entrar;      // Hasta 20 personas en la peluquería
sem_t sem_sofa;        // Hasta 4 personas sentadas en el sofá
sem_t sem_sillas;      // Hasta 3 personas en silla de corte
sem_t sem_clientes;    // Clientes listos para corte

pthread_mutex_t mutex_id;
int siguiente_id = 1;

void *cliente(void *arg) {
    int id;

    // Esperar si hay 20 personas dentro
    sem_wait(&sem_entrar);

    // Asignar ID protegido por mutex
    pthread_mutex_lock(&mutex_id);
    id = siguiente_id++;
    pthread_mutex_unlock(&mutex_id);

    printf("👤 Cliente %d entra a la peluquería.\n", id);

    // Esperar sofá
    sem_wait(&sem_sofa);
    printf("🛋️ Cliente %d se sienta en el sofá.\n", id);

    // Esperar silla
    sem_wait(&sem_sillas);
    sem_post(&sem_sofa); // Se levanta del sofá
    printf("💈 Cliente %d va a la silla del peluquero.\n", id);

    // Avisar al peluquero
    sem_post(&sem_clientes);

    // Esperar mientras le cortan el pelo (simulado)
    sleep(5);
    printf("✅ Cliente %d ha terminado y se va.\n", id);

    // Liberar silla y lugar en la peluquería
    sem_post(&sem_sillas);
    sem_post(&sem_entrar);

    return NULL;
}

void *peluquero(void *arg) {
    long id = (long)arg;

    while (1) {
        sem_wait(&sem_clientes); // Espera a que haya un cliente listo
        printf("✂️ Peluquero %ld está cortando el pelo...\n", id);
        sleep(5); // Simula el corte
        printf("💇 Peluquero %ld terminó de cortar.\n", id);
    }

    return NULL;
}

int main() {
    pthread_t clientes[TOTAL_CLIENTES];
    pthread_t peluqueros[3];

    // Inicialización de semáforos y mutex
    sem_init(&sem_entrar, 0, MAX_DENTRO);
    sem_init(&sem_sofa, 0, MAX_SOFA);
    sem_init(&sem_sillas, 0, MAX_SILLAS);
    sem_init(&sem_clientes, 0, 0);
    pthread_mutex_init(&mutex_id, NULL);

    // Crear peluqueros
    for (long i = 0; i < 3; i++) {
        pthread_create(&peluqueros[i], NULL, peluquero, (void *)i);
    }

    // Crear clientes uno por segundo
    for (int i = 0; i < TOTAL_CLIENTES; i++) {
        pthread_create(&clientes[i], NULL, cliente, NULL);
        sleep(1);
    }

    // Esperar a todos los clientes
    for (int i = 0; i < TOTAL_CLIENTES; i++) {
        pthread_join(clientes[i], NULL);
    }

    // Los peluqueros se quedan trabajando infinitamente

    return 0;
}
