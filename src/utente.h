#ifndef UTENTE_H
#define UTENTE_H
#include "include.h"

// Definizione della struttura utente
typedef struct utente {
    int id, socket_id;
    bool online, giocatore;
    char* email;      // Puntatore a char per allocazione dinamica
    char* password;   // Puntatore a char per allocazione dinamica
    struct utente* next; // Puntatore al prossimo utente (per lista concatenata)
} utente;

//crea un socket sulla porta e restituisce l'sd
int creazione_sock_server(struct sockaddr_in *my_addr, int porta);

//mostra i comandi del server
void comandi_server();

//stampa gli utenti attualmente registrati 
void stampaUtenti(utente* head);

//data la mail cerca se l'utente è presente e lo restituisce
struct utente* trovaUtente(utente* head, const char* email);

//gestisce il signup dell'utente
int signup(int sd, struct utente** lista_utenti);

//gestisce il login dell'utente
int login(int sd, struct utente** lista_utenti);

// Funzione per liberare una singola struttura utente
void free_utente(utente* u); 

// Funzione per liberare la lista di utenti
void free_lista_utenti(utente* head);

// Mette l'account offline
void logout_utente(struct utente **lista, int socket_id);

#endif
