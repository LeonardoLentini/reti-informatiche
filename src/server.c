#include "include.h"
#include "utente.h"
#include "item.h"

#define MAX_CLIENT 2    //numero massimo di client che si possono connettere 
#define DIM_BUFFER 1024 //grandezza buffer del server 
#define DIM_MSG 256     //dimensione massima dei messaggi scambiati tra client e server
#define DIM_COMANDO 6   //dimensione massima del comando inserito dall'utente
#define NUM_OGGETTI 5   //numero di oggetti presenti nello scenario
#define NUM_LOCATION 2  //numero di location presenti nello scenario

/*
struct sockaddr_in {
    sa_family_t     sin_family; // Address family, si mette AF_INET
    in_port_t       sin_port;   // network order
    struct in_addr  sin_addr;   // indirizzo
}

struct in_addr {
    uint32_t s_addr;
};
*/


int main(int argc, char *argv[]) 
{
    int sock_ascolto, sock_comms; // Socket file descriptors
    int ret, len, fd_max = 0, i, j, k, client_connessi = 0, utenti_loggati = 0, utenti_connessi = 0, user;
    int primo_utente = -1, secondo_utente = -1;     //tengono conto dei socket degli utenti durante la partita
    char buf[DIM_BUFFER], comando[DIM_COMANDO], msg[DIM_MSG], arg1[15], arg2[15], esito[20];
    bool acceso = 0;
    fd_set read_fs;
    fd_set master;
    uint16_t porta, network_msg;
    uint8_t stanza = 0;

    struct sockaddr_in my_addr, client_addr;

    struct utente* lista_utenti = NULL;
    struct partita* game = NULL;

    //timer
    time_t start_time = 0, current_time;
    int timer = 300; //5 minuti
    int rimanente = timer;    

    // Inizializza il set di file descriptor
    FD_ZERO(&master);
    FD_ZERO(&read_fs);

    // Aggiungi lo standard input (stdin) al set
    FD_SET(STDIN_FILENO, &master);

    printf("Fine inizializzazione\n");
    // mostro la console dei comandi del server
    comandi_server();

    while(true)
    {
        printf("\n************************************************************************\n");
        // Azzero la variabile dei comandi e buf
        memset(comando, 0, sizeof(comando));
        memset(buf, 0, sizeof(buf));        
        memset(msg, 0, sizeof(msg));   
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));
             
        //salvo la lista di descrittori master in un'altra lista per poi usare la select();
        read_fs = master;
        
        //ciclo per tutto il nuovo set contenete solo i fd pronti
        ret = select(fd_max+1, &read_fs, NULL, NULL, NULL);
        if(ret < 0)
        {
            perror("errore nella select");
            exit(1);
        }

        for(i = 0; i <= fd_max; i++)
        {
            if(FD_ISSET(i, &read_fs)) //Se è pronto in lettura stdin, il socket listener o uno di scambio dati
            { 
                if(i == STDIN_FILENO)
                {
                    // Comando inserito nella console del server
                    // Gestione input
                    fgets(buf, sizeof(buf), stdin);

                    // So di aspettarmi un comando di max 5 caratteri e un intero su 16 bit
                    ret = sscanf(buf, "%5s %hu", comando, &porta);
                    porta = htons(porta);

                    // se ret == 2 vuol dire che ha letto correttamente le due variabili
                    if(ret == 2 && strcmp(comando, "start") == 0)
                    {
                        if(acceso)
                        {
                            printf("Server già acceso sulla porta %d\n", ntohs(porta));
                            continue;
                        }
                         
                        printf("Avvio del server in corso...\n");
                        //inizializzo il socket di ascolto e lo aggiungo al set
                        sock_ascolto = creazione_sock_server(&my_addr, porta);
                        FD_SET(sock_ascolto, &master);
                        if(sock_ascolto > fd_max)
                        {
                            fd_max = sock_ascolto; // fd_max contiene ora il valore massimo dei descrittori di socket master
                        }
                        //metto il socket in ascolto con al massimo MAX_CLIENT in coda
                        ret = listen(sock_ascolto, MAX_CLIENT);
                        if(ret < 0)
                        {
                            printf("Errore nella listen\n");
                            exit(1);
                        }
                        acceso = true;
                    }
                    else if(ret == 1 && (strcmp(comando, "stop") == 0))
                    {
                        if(start_time != 0) //partita in corso
                        {
                            printf("Partita in corso, impossibile chiudere il server\n");
                            continue;
                        }
                        printf("Arresto del server in corso...\n");
                        if(client_connessi > 0)
                        {
                            for(j = 1; j <= fd_max; j++)
                            {
                                if(FD_ISSET(j, &master))
                                {
                                    //tolgo il socket dal set e lo chiudo
                                    FD_CLR(j, &master);
                                    close(j);
                                    //mando offline l'utente
                                    logout_utente(&lista_utenti, i);
                                    client_connessi--;
                                    printf("Chiuso il socket %d\n", j);
                                    continue;
                                }
                            }
                            //libero lo spazio allocato per gli utenti
                            free_lista_utenti(lista_utenti);
                            //chiudo il socket di ascolto
                            close(sock_ascolto);
                            printf("Arresto del server completato\n");
                            exit(0);
                        }
                        else
                        {
                            //chiudo il socket ascolto e dealloco le strutture degli utenti
                            close(sock_ascolto);
                            free_lista_utenti(lista_utenti);
                            printf("Arresto del server completato\n");
                            exit(0);
                        }
                    }
                    else
                    {
                        //comando non riconosciuto
                        printf("Comando non riconosciuto\n");
                        comandi_server();
                    }
                    printf("\n");
                }
                else if(i == sock_ascolto)
                {
                    // richiesta di connessione
                    len = sizeof(client_addr);

                    // accetto la richiesta del client di connettersi
                    sock_comms = accept(sock_ascolto, (struct sockaddr*)&client_addr, (socklen_t*)&len);
                    if(sock_comms == -1)
                    {
                        perror("Errore nell'accept");
                        close(sock_comms);

                      continue;
                    }
                    printf("Connessione accettata sulla porta %d\n", ntohs(porta));

                    //tengo traccia dei client connessi, mi serve per il comando stop
                    client_connessi++;
                    
                    // Inserisco il nuovo socket nel set.
                    FD_SET(sock_comms, &master);
                    if(sock_comms > fd_max)
                    {

                       fd_max = sock_comms; // fd_max contiene ora il valore massimo dei descrittori di socket master.
                    }
                }
                else    //è il socket di comunicazione (sock_comms): è arrivato un messaggio da uno dei client
                {
                    // Un giocatore ha immesso un comando, lo identifico dato che è sempre su 5 caratteri.
                    ret = recv(i, comando, sizeof(comando), 0);
                    if(ret < 0 && start_time != 0 && rimanente <= 0)   
                    {
                        perror("Errore nella recv");
                        exit(1);
                    }
                    else if(ret == 0)
                    {
                        //comando end 
                        printf("Client %d chiuso, chiusura del socket in corso...", i);
                        close(i);
                        FD_CLR(i, &master);
                        client_connessi--;
                        if(start_time != 0 && rimanente <= 0) //partita iniziata ma tempo scaduto
                        {
                            continue;
                        }
                        printf("Socket %d chiuso correttamente\n", i);
                        if(primo_utente != -1) //se il primo utente era già connesso
                        {
                            //mando offline l'utente
                            logout_utente(&lista_utenti, i);

                            if(i == primo_utente) //è il primo utente a fare end
                            {
                                if(utenti_connessi > 1) //partita in corso
                                {
                                    utenti_connessi--;
                                    utenti_loggati--;
                                    memset(msg, 0, sizeof(msg));
                                    //invio il messaggio di logout all'assistente
                                    strcpy(msg, "logout");
                                    ret = send(secondo_utente, msg, strlen(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    //elimino la partita
                                    free_partita(game);
                                    game = NULL;
                                    printf("Informato l'assistente del fatto che il giocatore si è disconnesso\n\n\n");
                                }
                                else
                                {
                                    //l'assistente non è connesso --> ha già fatto "end" oppure non eravamo in partita
                                    printf("L'assistente ha già fatto end/non era connesso\n");
                                }
                            }
                            else if(i == secondo_utente)
                            {
                                //assistente disconnesso
                                if(utenti_connessi > 1) //partita in corso
                                {
                                    utenti_connessi--;
                                    utenti_loggati--;
                                    memset(msg, 0, sizeof(msg));
                                    //invio il messaggio di logout al giocatore
                                    strcpy(msg, "logout");
                                    ret = send(primo_utente, msg, sizeof(msg), 0); 
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    //elimino la partita
                                    free_partita(game);
                                    game = NULL;
                                    printf("Informato il giocatore del fatto che l'assistente si è disconnesso\n\n\n");
                                }    
                                else
                                {
                                    //il giocatore non è connesso --> ha già fatto "end" oppure non eravamo in partita
                                    printf("Il giocatore ha già fatto end/non era connesso\n");
                                }
                            }
                            else if(secondo_utente == -1)
                            {
                                //il primo utente aspetta il secondo che però non è connesso
                                strcpy(msg, "logout");
                                ret = send(primo_utente, msg, strlen(msg), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella send");
                                    exit(1);
                                }
                                printf("Informato il giocatore del fatto che l'altro client non è connesso\n\n\n");
                            }
                        }
                        if(start_time != 0) //se era in corso una partita azzero il timer (serve per il comando stop)
                        {
                            start_time = 0;
                        }
                        continue;
                    }
                    else if(ret > 0)
                    {
                        if(start_time != 0 && utenti_connessi > 1)  //partita in corso
                        {
                            //invio il tempo rimanente al giocatore
                            current_time = time(NULL);
                            rimanente = timer - (current_time - start_time);
                            network_msg = htons(rimanente);
                            ret = send(primo_utente, &network_msg, sizeof(network_msg), 0);
                            if(ret < 0)
                            {
                                perror("errore nella send");
                                exit(1);
                            }
                        }
                        //debug alla console del server 
                        printf( "Comando: %s\n"
                                "Stanza: %d\n"
                                "Socket: %d\n\n"
                                , comando, 
                                stanza != 0? stanza : -1,
                                i   ); 
                        if(strcmp(comando, "signup") == 0)
                        {
                            user = signup(i, &lista_utenti);
                            if(user < 0)
                            {
                                //impossibile registrare, email già utilizzata
                                continue;
                            }
                            //invio l'esito della registrazione
                            strcpy(msg, "Registrazione effettuata");
                            ret = send(i, msg, sizeof(msg), 0);
                            if(ret < 0)
                            {
                                perror("Errore nella send\n");
                                exit(1);
                            }
                            continue;
                        }
                        if(strcmp(comando, "login") == 0)
                        {
                            user = login(i, &lista_utenti);
                            if(user<1)
                            {
                                //email non registrata/psw errata/utente già online
                                continue;
                            } 
                            printf("Login effettuato\n");
                            utenti_loggati++;
                            if(utenti_loggati == 1)
                            {
                                //se primo utente loggato --> devo mostrare gli scenari --> invio stanza = 0
                                printf("Primo utente loggato\n");
                                stanza = 0;
                                ret = send(i, (void *)&stanza, sizeof(stanza), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella send");
                                    exit(1);
                                }
                                printf("Inviato l'elenco delle stanze al client\nAttendo la sua scelta\n");
                                ret = recv(i, (void *)&stanza, sizeof(stanza), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella recv");
                                    exit(1);
                                }
                                printf("Stanza selezionata dall'utente: %d\n", stanza);
                            }
                            else
                            {
                                printf("Secondo utente loggato, gli comunico la stanza\n");
                                printf("Stanza %d", stanza);
                                ret = send(i, (void *)&stanza, sizeof(stanza), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella send");
                                    exit(1);
                                }
                            }
                        }
                        //il login dell'utente viene fatto qualsiasi sia la stanza scelta per giocare
                        //da qui in poi i comandi di stanza in stanza possono variare quindi diversifico le stanze
                        if(stanza == 1)
                        {
                            //gestione comandi della stanza 1
                            if(strcmp(comando, "ready") == 0)
                            {
                                //l'utente è pronto a giocare
                                utenti_connessi ++;
                                if(utenti_connessi == 1)    //è il primo utente a connettersi per giocare
                                {
                                    if(client_connessi == 1)
                                    {
                                        printf("L'altro client è chiuso, chiusura del socket in corso...\n");
                                        strcpy(msg, "logout");
                                        ret = send(i, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                        close(i);
                                        FD_CLR(i, &master);
                                        logout_utente(&lista_utenti, i);
                                        client_connessi--;
                                        printf("Socket %d chiuso correttamente\n", i);
                                        continue;
                                    }
                                    //è il primo utente, mi salvo il suo sd per poi inviargli l'inizio partita
                                    primo_utente = i;
                                }
                                else if (utenti_connessi == 2) //entrambi gli utenti sono pronti a giocare
                                {
                                    printf("Inizializzazione strutture partita\n");                                    
                                    game = (partita*)malloc(sizeof(partita));  // Alloca la struttura principale
                                    if (game == NULL) 
                                    {
                                        perror("Errore nell'allocazione di game");
                                        return 1;
                                    }
                                    //inizializzo gli items dello scenario dove giocheranno
                                    inizializza_elementi_1(stanza, game);
                                    
                                    //comunico agli utenti l'inizio della partita
                                    strcpy(msg, "inizio");
                                    ret = send(primo_utente, msg, sizeof(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);

                                    }
                                    secondo_utente = i;
                                    ret = send(secondo_utente, msg, sizeof(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    printf("Partita iniziata\n");
                                    //invio il numero di game->token da raccogliere al primo utente
                                    network_msg = htons(game->token->oggetti_totali);
                                    ret = send(primo_utente, &network_msg, sizeof(network_msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    printf("Timer avviato\n");
                                    // Ottengo il tempo attuale e invio il tempo rimanente al giocatore
                                    start_time = time(NULL);
                                    current_time = time(NULL);
                                    rimanente = timer - (current_time - start_time);
                                    network_msg = htons(rimanente);
                                    ret = send(primo_utente, &network_msg, sizeof(network_msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                }
                                else
                                {
                                    printf("Errore nel numero di utenti connessi = %d\n", utenti_connessi);
                                }
                            }
                            if(strcmp(comando, "look") == 0)
                            {
                                printf("Comando look\n");
                                
                                //ricevo l'argomento, se il comando è look, è il primo utente 
                                ret = recv(primo_utente, arg1, sizeof(arg1), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella recv");
                                    exit(1);
                                }
                                
                                if(strcmp(arg1, "null") == 0)   //comando look senza argomenti
                                {   
                                    memset(msg, 0, sizeof(msg));
                                    strcpy(msg, "Sei nel laboratorio al centro c’è un ++tavolo++ con alcuni strumenti da laboratorio.\nNel ++deposito++ delle attrezzature vedi alcuni scaffali.\n\n");
                                    printf("%s\n", msg);
                                    ret = send(i, msg, strlen(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                }
                                else //comando look con l'argomento
                                {
                                    ret = 0;
                                    memset(msg, 0, sizeof(msg));
                                    //cerco tra le location
                                    for(j = 0;j < game->num_location; j++)
                                    {
                                        if(strcmp(arg1, game->loc[j].nome) == 0)
                                        {
                                            ret = 1;
                                            //se trovo la location salvo la descrizione per inviarla al client
                                            strcpy(msg, game->loc[j].descrizione);
                                            if(game->loc[j].sblocca_stato != NULL)
                                            {
                                                //sblocco eventuali oggetti presenti nella location
                                                unlock_state_loc(j, game->loc, game->objs);
                                            }
                                            break;
                                        }
                                    }
                                    if(ret == 1)    //ho trovato la location
                                    {
                                        ret = send(i, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                    }
                                    else   
                                    {
                                        //location non trovata, cerco tra gli oggetti
                                        j = cerca_oggetto(arg1, game->objs);
                                        if(j < 0)
                                        {
                                            //arg1 non era né location né oggetto
                                            strcpy(msg, "Oggetto/location non trovato/a\n");
                                            ret = send(i, msg, sizeof(msg), 0);
                                            if(ret < 0)
                                            {
                                                perror("Errore nella send");
                                                exit(1);
                                            }
                                        }
                                        else
                                        {
                                            //oggetto trovato
                                            strcpy(msg, game->objs[j].risposte[game->objs[j].stato]);   
                                            if(game->objs[j].stato > 0) 
                                            {
                                                //sblocco eventuali oggetti bloccati
                                                unlock_state_obj(j, game->objs, comando);
                                            }
                                            //invio la risposta attuale dell'oggetto
                                            ret = send(i, msg, sizeof(msg), 0);
                                            if(ret < 0)
                                            {
                                                perror("Errore nella send");
                                                exit(1);
                                            }
                                        }
                                    }
                                }
                                printf("Inviato l'esito al client\n");
                            }
                            if(strcmp(comando, "take") == 0)
                            {
                                printf("Comando take\n");
                                //ricevo l'argomento
                                ret = recv(primo_utente, arg1, sizeof(arg1), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella recv");
                                    exit(1);
                                }

                                memset(msg, 0, sizeof(msg));
                                j = cerca_oggetto(arg1, game->objs);
                                if(j < 0)
                                {
                                    strcpy(msg, "Oggetto non trovato\n");
                                    ret = send(primo_utente, msg, sizeof(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    //devo comunicare che non ha raccolto un token
                                    strcpy(msg, "non token");
                                    ret = send(primo_utente, msg, sizeof(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                }
                                else
                                {
                                    //j contiene l'indice dell'oggetto che l'utente vuole raccogliere
                                    if(game->objs[j].stato == 0) 
                                    {   
                                        //oggetto ancora da sbloccare
                                        strcpy(msg, game->objs[j].risposte[game->objs[j].stato]);
                                        ret = send(primo_utente, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                        //devo comunicare che non ha raccolto un token
                                        strcpy(msg, "non token");
                                        ret = send(primo_utente, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                    }
                                    else if(game->objs[j].massimo_stato < 3)
                                    {
                                        //l'oggetto non può essere raccolto
                                        strcpy(msg, "L'oggetto non può essere raccolto\n");
                                        ret = send(primo_utente, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                        //devo comunicare che non ha raccolto un token
                                        strcpy(msg, "non token");
                                        ret = send(primo_utente, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                    }
                                    else if(game->objs[j].stato > 2)
                                    {
                                        //oggetto già raccolto 
                                        strcpy(msg, game->objs[j].risposte[3]);
                                        ret = send(primo_utente, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                        //devo comunicare che non ha raccolto un token
                                        strcpy(msg, "non token");
                                        ret = send(primo_utente, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                    }
                                    else
                                    {
                                        //stato == 1
                                        //controllo se l'oggetto è provette e la centrifuga è ferma
                                        if(strcmp(game->objs[j].nome, "provette") == 0)
                                        {
                                            if(game->objs[0].moving == true)    //objs[0] = centrifuga
                                            {
                                                strcpy(msg, game->objs[j].risposte[1]); //comunico la risposta con  la centrifuga in movimento
                                                ret = send(primo_utente, msg, sizeof(msg), 0);
                                                if(ret < 0)
                                                {
                                                    perror("Errore nella send");
                                                    exit(1);
                                                }
                                                //devo comunicare che non ha raccolto un token
                                                strcpy(msg, "non token");
                                                ret = send(primo_utente, msg, sizeof(msg), 0);
                                                if(ret < 0)
                                                {
                                                    perror("Errore nella send");
                                                    exit(1);
                                                }
                                                continue;
                                            }
                                        }

                                        //invio la risposta al comando take
                                        strcpy(msg, game->objs[j].risposte[2]);
                                        ret = send(primo_utente, msg, sizeof(msg), 0);
                                        if(ret < 0)
                                        {
                                            perror("Errore nella send");
                                            exit(1);
                                        }
                                        //controllo se c'è un indovinello per raccoglierlo
                                        if(game->objs[j].indovinello != NULL)
                                        {
                                            //invio il quesito
                                            strcpy(msg, game->objs[j].indovinello->domanda);
                                            ret = send(primo_utente, msg, sizeof(msg), 0);
                                            if(ret < 0)
                                            {
                                                perror("Errore nella send");
                                                exit(1);
                                            }

                                            //aspetto la risposta
                                            ret = recv(primo_utente, msg, sizeof(msg), 0);
                                            if(ret < 0)
                                            {
                                                perror("Errore nella recv");
                                                exit(1);
                                            }
                                            if(strcmp(msg, game->objs[j].indovinello->risposta) == 0)
                                            {
                                                //procedo con incrementare lo stato dell'oggetto
                                                game->objs[j].stato = 3;
                                                game->token->nomi[game->token->numero_oggetti] = strdup(game->objs[j].nome);
                                                game->token->numero_oggetti++;
                                                strcpy(msg, "Risposta corretta, oggetto raccolto");
                                                ret = send(primo_utente, msg, sizeof(msg), 0);
                                                if(ret < 0)
                                                {
                                                    perror("Errore nella send");
                                                    exit(1);
                                                }
                                                //comunico che ha raccolto un token
                                                strcpy(msg, "token");
                                                ret = send(primo_utente, msg, sizeof(msg), 0);
                                                if(ret < 0)
                                                {
                                                    perror("Errore nella send");
                                                    exit(1);
                                                }

                                                if(strcmp(arg1, "provette") == 0)
                                                {
                                                    //cambio la risposta della centrifuga
                                                    game->objs[0].risposte[1] = strdup("La centrifuga è ferma, non vedi altro al suo interno\n");
                                                }

                                                if(strcmp(arg1, "campione") == 0)
                                                {
                                                    //cambio la risposta del microscopio
                                                    game->objs[3].risposte[1] = strdup("Il microscopio ha una luce LED che illumina un vetrino vuoto\n");
                                                }
                                            }
                                            else
                                            {
                                                strcpy(msg, "Risposta errata, non è possibile raccogliere l'oggetto");
                                                ret = send(primo_utente, msg, sizeof(msg), 0);
                                                if(ret < 0)
                                                {
                                                    perror("Errore nella send");
                                                    exit(1);
                                                }
                                                //devo comunicare che non ha raccolto un token
                                                strcpy(msg, "non token");
                                                ret = send(primo_utente, msg, sizeof(msg), 0);
                                                if(ret < 0)
                                                {
                                                    perror("Errore nella send");
                                                    exit(1);
                                                }
                                            }
                                        }
                                        else
                                        {
                                            //se non c'è l'indovinello ha fatto la take sulla presa, aumento lo stato e sblocco oggetti
                                            game->token->nomi[game->token->numero_oggetti] = strdup(game->objs[j].nome);
                                            game->token->numero_oggetti++;
                                            unlock_state_obj(j, game->objs, comando);
                                            game->objs[j].stato = 3;
                                            //comunico che ha raccolto un token
                                            strcpy(msg, "token");
                                            ret = send(primo_utente, msg, sizeof(msg), 0);
                                            if(ret < 0)
                                            {
                                                perror("Errore nella send");
                                                exit(1);
                                            }
                                        }  
                                    }    
                                }
                                printf("Inviato l'esito al client\n");
                            }
                            if(strcmp(comando, "use") == 0)
                            {
                                printf("Comando use\n");
                                //ricevo il numero di argomenti della use
                                ret = recv(primo_utente, msg, sizeof(msg), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella recv");
                                    exit(1);
                                }

                                if(strcmp(msg, "1") == 0)
                                {
                                    //devo ricevere un solo argomento
                                    ret = recv(primo_utente, arg1, sizeof(arg1), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella recv");
                                        exit(1);
                                    }

                                    //cerco tra gli oggetti esistenti
                                    j = cerca_oggetto(arg1, game->objs);
                                    if(j < 0)   //oggetto non esistente
                                    {
                                        strcpy(msg, "Oggetto non trovato\n");
                                    }
                                    else
                                    {
                                        //cerco tra gli oggetti raccolti 
                                        for(k = 0;k < game->token->numero_oggetti; k++)
                                        {
                                            if(strcmp(game->token->nomi[k], arg1) == 0)
                                            {
                                                //l'oggetto è stato raccolto quindi è utilizzabile
                                                break;
                                            }
                                        }

                                        if(k == game->token->numero_oggetti)
                                        {
                                            strcpy(msg, "Oggetto non ancora raccolto\n");
                                        }
                                        else
                                        {
                                            //oggetto raccolto e utilizzabile, in game->objs[j]
                                            //se è stato raccolto il suo stato è sicuramente >= 3
                                            //quindi controllo solo se è già stato usato
                                            if(game->objs[j].stato == game->objs[j].massimo_stato) //è già stato usato
                                            {
                                                strcpy(msg, game->objs[j].risposte[game->objs[j].massimo_stato]);
                                            }
                                            else if(game->objs[j].stato == 3)
                                            {
                                                strcpy(msg, game->objs[j].risposte[4]);
                                            } 
                                            else
                                            {
                                                //oggetto già usato, può essere campione o provette
                                                strcpy(msg, game->objs[j].risposte[5]);
                                            }
                                        }
                                    }
                                    //invio l'esito 
                                    ret = send(primo_utente, msg, sizeof(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    
                                }
                                else
                                {
                                    //msg == 2
                                    //devo ricevere due argomenti
                                    ret = recv(primo_utente, arg1, sizeof(arg1), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella recv");
                                        exit(1);
                                    }

                                    ret = recv(primo_utente, arg2, sizeof(arg2), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella recv");
                                        exit(1);
                                    }

                                    //cerco tra gli oggetti esistenti
                                    j = cerca_oggetto(arg1, game->objs);
                                    if(j < 0)
                                    {
                                        strcpy(msg, "Oggetto non trovato\n");
                                    }
                                    else
                                    {
                                        //cerco tra gli oggetti raccolti 
                                        for(k = 0;k < game->token->numero_oggetti; k++)
                                        {
                                            if(strcmp(game->token->nomi[k], arg1) == 0)
                                            {
                                                //l'oggetto è stato raccolto quindi è utilizzabile
                                                break;
                                            }
                                        }

                                        if(k == game->token->numero_oggetti)
                                        {
                                            strcpy(msg, "Oggetto non ancora raccolto\n");
                                        }
                                        else
                                        {
                                            //oggetto raccolto e utilizzabile, in game->objs[j]
                                            if(game->objs[j].stato == game->objs[j].massimo_stato)
                                            {
                                                strcpy(msg, game->objs[j].risposte[game->objs[j].massimo_stato]);

                                            }
                                            else if(strcmp(game->objs[j].nome, "campione") != 0 && strcmp(game->objs[j].nome, "provette") != 0)
                                            {
                                                //ha chiamato la use con presa, non c'è un utilizzo utile 
                                                strcpy(msg, "Non ci sono utilizzi con questi oggetti\n");
                                            }
                                            else
                                            {
                                                //devo controllare l'altro oggetto
                                                ret = j; //momentaneo
                                                j = cerca_oggetto(arg2, game->objs);
                                                if(j < 0)
                                                {
                                                    strcpy(msg, "Oggetto non trovato\n");
                                                }
                                                else
                                                {
                                                    //cerco tra gli oggetti raccolti 
                                                    for(k = 0;k < game->token->numero_oggetti; k++)
                                                    {
                                                        if(strcmp(game->token->nomi[k], arg2) == 0) 
                                                        {
                                                            //l'oggetto è stato raccolto quindi è utilizzabile
                                                            break;
                                                        }
                                                    }

                                                    if(k == game->token->numero_oggetti)
                                                    {
                                                        strcpy(msg, "Oggetto non ancora raccolto\n");
                                                    }
                                                    else
                                                    {
                                                        //oggetto raccolto e utilizzabile, in game->objs[j]
                                                        //devo controllare se game->objs[j] e game->objs[ret] corrispondono a campione e provette
                                                        if(game->objs[j].stato == game->objs[j].massimo_stato)
                                                        {
                                                            strcpy(msg, game->objs[j].risposte[game->objs[j].massimo_stato]);
                                                        }
                                                        else if( ((strcmp(game->objs[j].nome, "campione") == 0) && (strcmp(game->objs[ret].nome, "provette") == 0)) || ((strcmp(game->objs[j].nome, "provette") == 0) && (strcmp(game->objs[ret].nome, "campione") == 0)))
                                                        {
                                                            //soluzione trovata aggiorno gli stati
                                                            game->objs[j].stato = game->objs[j].massimo_stato;
                                                            game->objs[ret].stato = game->objs[ret].massimo_stato;
                                                            strcpy(msg, "Avete trovato l'antidoto, la formula dell’antidoto è: C6H1206\n");
                                                        }
                                                        else
                                                        {
                                                            strcpy(msg, "Non c'è un utilizzo per questi oggetti\n");
                                                        }
                                                    }
                                                }                                                        
                                            }
                                        }
                                    }
                                    //invio l'esito
                                    ret = send(primo_utente, msg, sizeof(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                }
                                printf("Inviato l'esito al client\n");
                            }
                            if(strcmp(comando, "objs") == 0)
                            {
                                printf("Comando objs\n");
                                sleep(1);
                                for(j=0;j<game->token->numero_oggetti;j++)  //per ogni oggetto raccolto invio il nome
                                {
                                    strcpy(msg, game->token->nomi[j]);
                                    ret = send(primo_utente, msg, strlen(msg), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    sleep(1);
                                }
                                memset(msg, 0, sizeof(msg));
                                strcpy(msg, "fine oggetti");    //comunico la fine del comando objs
                                ret = send(primo_utente, msg, strlen(msg), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella send");
                                    exit(1);
                                }
                                printf("Inviato l'esito al client\n");
                            }
                            if(strcmp(comando, "send") == 0)
                            {
                                printf("Comando send\n");
                                //ricevo il messaggio dal primo utente
                                ret = recv(i, msg, sizeof(msg), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella recv");
                                    exit(1);
                                }
                                //invio il messaggio al secondo utente
                                ret = send(secondo_utente, msg, sizeof(msg), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella send");
                                    exit(1);
                                }
                                printf("Inviato l'esito al client\n");
                            }
                            if(strcmp(comando, "try") == 0)
                            {
                                printf("Comando try\n");
                                //ricevo la soluzione proposta
                                ret = recv(i, arg1, sizeof(arg1), 0);
                                if(ret < 0)
                                {
                                    perror("Errore nella recv\n");
                                    exit(1);
                                }
                                printf("Soluzione proposta: %s\n", arg1);
                                if(strcmp(arg1, game->soluzione) == 0)
                                {
                                    //invio la fine dell'escape room
                                    strcpy(esito, "corretto");
                                    ret = send(primo_utente, esito, sizeof(esito), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    ret = send(secondo_utente, esito, sizeof(esito), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                    printf("\n\nEscape room completata\n\n");
                                }
                                else
                                {
                                    //comunico l'esito ai client
                                    strcpy(esito, "errato");
                                    ret = send(primo_utente, esito, sizeof(esito), 0);
                                    if(ret < 0)
                                    {

                                       perror("Errore nella send");
                                        exit(1);
                                    }
                                    ret = send(secondo_utente, esito, sizeof(esito), 0);
                                    if(ret < 0)
                                    {
                                        perror("Errore nella send");
                                        exit(1);
                                    }
                                }
                                printf("Inviato l'esito al client\n");
                            }

                        }
                        else
                        {
                            //gestione comandi delle altre stanze 
                        }
                    }
                }
            }
        }
        //controllo che non sia terminato il timer
        if(start_time != 0 && utenti_connessi > 1)
        {
            current_time = time(NULL);
            rimanente = timer - (current_time - start_time);
            if(rimanente <= 0)  //timer terminato
            {
                printf("\n\nTEMPO SCADUTO\n\n");
                strcpy(msg, "TIMEOUT");
                ret = send(primo_utente, msg, strlen(msg), 0);
                if(ret < 0)
                {
                    perror("errore nella send");
                    exit(1);
                }
                ret = send(secondo_utente, msg, strlen(msg), 0);
                if(ret < 0)
                {
                    perror("errore nella send");
                    exit(1);
                }
                start_time = 0;
            }
        }
    }
}