#ifndef ITEM
#define ITEM_H

#include "include.h"

#define NUM_OGGETTI_1 5
#define NUM_LOCATION_1 2

//tiene conto degli oggetti raccolti durante la partita 
typedef struct raccolti
{
    char* nomi[3];
    int numero_oggetti;
    int oggetti_totali;
}raccolti;

//mantiene domanda e risposta per gli enigmi
typedef struct enigma
{
    char* domanda;
    char* risposta;
} enigma;

// Definizione della struttura per le location
typedef struct location
{
    char* nome;
    char* descrizione;
    char* sblocca_stato[2];
    int num_sblocchi;
} location;


// Definizione della struttura per gli oggetti
typedef struct oggetto 
{
    char* nome;
    char* risposte[6];      //ad ogni comando corrisponde una risposta in base allo stato dell'oggetto
                            //risposte[0] = comando look con oggetto non nominato
                            //risposte[1] = comando look con oggetto nominato
                            //risposte[2] = comando take 
                            //risposte[3] = comando take con oggetto già raccolto
                            //risposte[4] = comando use
                            //risposte[5] = comando use con oggetto già usato

    int stato;              //stato = 0 --> oggetto non ancora nominato
                            //stato = 1 --> oggetto nominato ma non può essere preso (può essere visto)
                            //stato = 2 --> oggetto che può essere preso
                            //stato = 3 --> oggetto raccolto
                            //stato = 4 --> oggetto che può essere usato (è precedentemente stato preso)
                            //stato = 5 --> oggetto usato
    char* sblocca_stato[2];       // Oggetto che sblocca un altro oggetto (un oggetto può sbloccare al massimo altri 2 oggetti)
    int massimo_stato, num_sblocchi;
    bool moving;            //per la centrifuga
    struct enigma* indovinello;
    
} oggetto;

//mantiene le informazioni per la partita in corso
typedef struct partita
{
    int stanza;
    int num_oggetti, num_location;
    char* soluzione;
    struct oggetto* objs;
    struct location* loc; 
    struct raccolti* token;
    struct partita *next;
}partita;

//inizializza le location e gli oggetti della escape room numero 1
void inizializza_elementi_1(int stanza, partita *current_game);

//dato il nome di un oggetto cerca se è presente tra gli oggetti inizializzati in objs, se presente restituisce l'indice
int cerca_oggetto(char* arg, oggetto objs[]);

//sblocca eventuali oggetti successivamente al comando look [location]
void unlock_state_loc(int j, location loc[], oggetto objs[]);

//sblocca eventuali oggetti successivamente ad un comando su un oggetto
void unlock_state_obj(int j, oggetto objs[], char* comando);

//libera lo spazio allocato per gli oggetti raccolti
void free_raccolti(raccolti* token);

//libera lo spazio allocato per la struct degli oggetti
void free_oggetti(oggetto* objs, int num_oggetti);

//libera lo spazio allocato per le location
void free_location(location* loc, int num_location);

//libera lo spazio allocato per la partita
void free_partita(partita* game);

#endif
