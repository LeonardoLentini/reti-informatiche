#include "item.h"

#define DIM_MSG 256     /* dimensione massima dei messaggi per la comunicazione client-server */

void inizializza_elementi_1(int stanza, partita *current_game)
{
    int i;
    struct location loc[NUM_LOCATION_1];
    struct oggetto objs[NUM_OGGETTI_1];
    
    current_game->objs = (struct oggetto*)malloc(NUM_OGGETTI_1 * sizeof(struct oggetto));
    if (current_game->objs == NULL) {
        perror("Errore nell'allocazione di current_game->objs");
        free(current_game->loc);  // Libera la memoria di current_game->loc
        free(current_game);       // Libera la memoria di current_game
        exit(1);
    }
    current_game->loc = (struct location*)malloc(NUM_LOCATION_1 * sizeof(struct location));
    if (current_game->loc == NULL) {
        perror("Errore nell'allocazione di current_game->loc");
        free(current_game);  // Libera la memoria di current_game
        exit(1);
    }
    current_game->token = (struct raccolti*)malloc(sizeof(struct raccolti));
    if (current_game->token == NULL) {
        perror("Errore nell'allocazione di current_game->token");
        free(current_game);  // Libera la memoria di current_game
        exit(1);
    }

    //inizializza le variabili per la partita nella stanza 1
    current_game->stanza = stanza;
    current_game->num_location = NUM_LOCATION_1;
    current_game->num_oggetti = NUM_OGGETTI_1;
    current_game->next = NULL;
    current_game->token->numero_oggetti = 0;
    current_game->token->oggetti_totali = 3;
    current_game->soluzione = strdup("C6H12O6");
    
    //inizializzo le location
    // TAVOLO
    loc[0].nome = strdup("tavolo");
    loc[0].descrizione = strdup("Al centro del tavolo c’è una **centrifuga** collegata alla corrente e in funzione con alcune **provette** al suo interno.");
    //sblocca lo stato di centrifuga e provette
    loc[0].sblocca_stato[0] = strdup("centrifuga");
    loc[0].sblocca_stato[1] = strdup("provette");
    loc[0].num_sblocchi = 2;
    //DEPOSITO
    loc[1].nome = strdup("deposito");
    loc[1].descrizione = strdup("Nel deposito delle attrezzature vedi molti strumenti da laboratorio, c’è anche un **microscopio**.");
    //sblocca lo stato di microscopio
    loc[1].sblocca_stato[0] = strdup("microscopio");
    loc[1].num_sblocchi = 1;
    
    
    //inizializzo gli oggetti - centrifuga, provette, presa, microscopio, campione
    //CENTRIFUGA
    objs[0].nome = strdup("centrifuga");
    objs[0].stato = 0;
    objs[0].sblocca_stato[0] = strdup("presa");
    objs[0].risposte[0] = strdup("Oggetto non valido\n");     //ha provato a fare il comando look sull'oggetto in stato = 0
    objs[0].risposte[1] = strdup("La centrifuga gira alla massima velocità. Non ci sono pulsanti visibili per fermarla, ma puoi accedere alla **presa** di corrente a cui è collegata.\n");
    objs[0].massimo_stato = 1;
    objs[0].num_sblocchi= 1;
    objs[0].moving = true;
    objs[0].indovinello = NULL;
    
    //PROVETTE
    objs[1].nome = strdup("provette");
    objs[1].stato = 0;
    objs[1].risposte[0] = strdup("Oggetto non valido\n");
    objs[1].risposte[1] = strdup("Non riesci a vedere le provette perché la centrifuga sta girando\n");
    objs[1].risposte[2] = strdup("L'OGGETTO È BLOCCATO, RISOLVI L'ENIGMA\n"); 
    objs[1].risposte[3] = strdup("L'oggetto è già stato raccolto\n");
    objs[1].risposte[4] = strdup("L'oggetto non ha alcun uso da solo\n"); //caso in cui l'oggetto è usato da solo
    objs[1].risposte[5] = strdup("L'oggetto è già stato usato\n");
    objs[1].indovinello = (enigma*)malloc(sizeof(enigma));  //alloco l'indovinello sulle provette
    if (objs[1].indovinello != NULL) 
    {
        objs[1].indovinello->domanda = strdup("Quale elemento è rappresentato dal simbolo chimico 'Na'?");
        objs[1].indovinello->risposta = strdup("sodio");
    }
    objs[1].num_sblocchi= 0;
    objs[1].massimo_stato = 5;
    objs[1].moving = false;
    
    //PRESA
    objs[2].nome = strdup("presa");
    objs[2].stato = 0;
    objs[2].risposte[0] = strdup("Oggetto non valido\n");
    objs[2].risposte[1] = strdup("Vedi un cavo elettrico collegato con una presa al muro, non sembra molto resistente...\n");
    objs[2].risposte[2] = strdup("Hai staccato il cavo e raccolto la presa! La centrifuga adesso è ferma.\n");
    objs[2].risposte[3] = strdup("L'oggetto è già stato raccolto\n");
    objs[2].risposte[4] = strdup("L'oggetto non sembra avere alcun uso\n");
    objs[2].indovinello = NULL;
    objs[2].moving = false;
    objs[2].massimo_stato = 4;
    objs[2].num_sblocchi= 1;
    objs[2].sblocca_stato[0] = strdup("centrifuga");

    //MICROSCOPIO
    objs[3].nome = strdup("microscopio");
    objs[3].stato = 0;
    objs[3].risposte[0] = strdup("Oggetto non valido\n");
    objs[3].risposte[1] = strdup("Il microscopio con una luce LED illumina un vetrino con un **campione** di cellule virali. Attraverso la lente riesci a leggere una scritta minuscola sulla parte alta del vetrino che recita: “parte dell'antidoto”\n");
    objs[3].indovinello = NULL;
    objs[3].moving = false;
    objs[3].massimo_stato = 1;
    objs[3].num_sblocchi= 1;
    objs[3].sblocca_stato[0] = strdup("campione");
    
    //CAMPIONE
    objs[4].nome = strdup("campione");
    objs[4].stato = 0;
    objs[4].risposte[0] = strdup("Oggetto non valido\n");
    objs[4].risposte[1] = strdup("Il campione di cellule infettate dal virus è posizionato all’interno del microscopio.\n");
    objs[4].risposte[2] = strdup("L'OGGETTO È BLOCCATO, RISOLVI L'ENIGMA\n");
    objs[4].risposte[3] = strdup("L'oggetto è già stato raccolto\n");
    objs[4].risposte[4] = strdup("L'oggetto non ha alcun uso da solo\n");
    objs[4].risposte[5] = strdup("L'oggetto è già stato usato\n");
    objs[4].indovinello = (enigma*)malloc(sizeof(enigma));  //alloco l'indovinello sul campione
    if (objs[4].indovinello != NULL) 
    {
        objs[4].indovinello->domanda = strdup("Qual è il pH di una soluzione neutra?");
        objs[4].indovinello->risposta = strdup("7");
    }
    objs[4].massimo_stato = 5;
    objs[4].num_sblocchi= 0;
    objs[4].moving = false;
    //inserisco location e oggetti inizializzati nella partita attuale
    for (i = 0; i < NUM_LOCATION_1; i++) 
    {
        current_game->loc[i] = loc[i];
    }
    for (i = 0; i < NUM_OGGETTI_1; i++) 
    {
        current_game->objs[i] = objs[i];
    }

}

int cerca_oggetto(char* arg, oggetto objs[])
{
    int i;
    for (i = 0; i < NUM_OGGETTI_1; i++)
    {
        if(strcmp(arg, objs[i].nome) == 0)
        {
            return i;
        }
    }
    return -1;
}

void unlock_state_loc(int j, location loc[], oggetto objs[])
{
    int i, k;
    for(i = 0; i < loc[j].num_sblocchi; i++)
    {
        for(k = 0; k < NUM_OGGETTI_1; k++)
        {
            if(strcmp(loc[j].sblocca_stato[i], objs[k].nome) == 0)
            {
                //ho trovato l'oggetto da sbloccare 
                if(objs[k].stato < 1)   //lo sblocco solo se era ancora in stato 0
                {
                    objs[k].stato = 1;
                }
                
            }
        }
    }
}

void unlock_state_obj(int j, oggetto objs[], char* comando)
{
    /*  objs[j] è l'oggetto che potrebbe sbloccare altri oggetti
        ciclo finché non trovo l'oggetto corrispondente a objs[j].sblocca_stato[i]
        quando lo trovo controllo ed agisco in base al comando
    */
    int i, k;
    for(i=0;i<objs[j].num_sblocchi;i++)
    {
        for(k=0;k<NUM_OGGETTI_1;k++)
        {
            if(strcmp(objs[j].sblocca_stato[i], objs[k].nome) == 0)
            {
                if(strcmp(comando, "look") == 0)
                {
                    if(objs[k].stato < 1)
                    {
                        objs[k].stato = 1;
                        break;
                    }
                }
                else
                {
                    /*è il comando take presa, nella sblocca stato c'è "centrifuga", 
                    ma devo cambiare anche la risposte[1] di provette se la centrifuga si stava muovendo*/
                    if(objs[k].moving == true)
                    {
                        objs[k].moving = false;
                        objs[k].risposte[objs[k].stato] = strdup("La centrifuga è ferma, al suo interno vedi 3 **provette**\n");
                        objs[1].risposte[1] = strdup("Vedi 3 **provette** con su scritto delle sostanze chimiche\n");
                        break;
                    }
                }
            }
        }
    }
}

void free_raccolti(raccolti* token) 
{
    // Se token è NULL, non fare nulla
    if (token != NULL) 
    {
        free(token); // Libera il puntatore stesso
    }
}

void free_oggetti(oggetto* objs, int num_oggetti) 
{
    int i, j;
    if(objs == NULL) return;
    for (i = 0; i < num_oggetti; i++) 
    {
        // Libera i nomi
        free(objs[i].nome);
        
        // Libera le risposte
        for (j = 0; j < objs[i].massimo_stato; j++) 
        {
            free(objs[i].risposte[j]);
        }
        // Libera l'enigma se esiste
        if (objs[i].indovinello != NULL) 
        {
            free(objs[i].indovinello->domanda);
            free(objs[i].indovinello->risposta);
            free(objs[i].indovinello);
        }
    }
    free(objs); // Libera il puntatore agli oggetti
}

void free_location(location* loc, int num_location) 
{
    int i, j;
    if(loc == NULL) return;
    for (i = 0; i < num_location; i++) 
    {
        free(loc[i].nome);  //libera il nome
        free(loc[i].descrizione);   //libera la descrizione
        for (j = 0; j < loc[i].num_sblocchi; j++) 
        {
            free(loc[i].sblocca_stato[j]);  //libera gli eventuali sblocchi
        }
    }
    free(loc); // Libera il puntatore alle locations
}

void free_partita(partita* game) 
{
    if (game != NULL) 
    {

        // Libera gli oggetti
        free_oggetti(game->objs, game->num_oggetti);
        
        // Libera le locations
        free_location(game->loc, game->num_location);

        // Libera il token
        free_raccolti(game->token);

        // Libera il puntatore alla partita
        free(game);
    }
}
