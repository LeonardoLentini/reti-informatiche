#ifndef PARTITA_H
#define PARTITA_H
//comunica all'utente gli scenari possibili 
void rooms();
//ricevo il tempo rimanente dal server
int ricevi_tempo(int sd);
//sincronizza i giocatori in attesa dell'inizio della partita
void preparazione_partita(int sd, int giocatore);
//spiegazione scenario per il giocatore principale
void inizio_partita();
//spiegazione scenario per l'assistente
void inizio_partita_assistente();

void comandi_giocatore();

void comandi_assistente();
//gestisce la partita del giocatore
void partita_giocatore(int sd);
//gestisce la partita dell'assistente
void partita_assistente(int sd);


#endif