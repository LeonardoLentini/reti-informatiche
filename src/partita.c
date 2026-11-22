#include "include.h"
#include "partita.h"

#define DIM_MSG 256     /* dimensione massima dei messaggi per la comunicazione client-server */


void rooms()
{
    printf("\n************************** ESCAPE ROOM *******************************\n\n");
    printf("Seleziona uno scenario con il comando 'start <room>'\n\n");
    printf("1)  Il virus del Caos\n");
    printf("2)  Coming soon...\n");
    printf("************************************************************************\n\n");
}

int ricevi_tempo(int sd)
{
    int ret;
    int16_t rimanente;
    uint16_t network_msg;

    ret = recv(sd, &network_msg, sizeof(network_msg), 0);
    if(ret < 0)
    {
        perror("Errore nella recv");
        exit(1);
    }
    rimanente = (int)ntohs(network_msg);
    return rimanente;
}

void preparazione_partita(int sd, int giocatore)
{
    char msg[256];
    int ret;

    memset(msg, 0, sizeof(msg));
    if(giocatore == 1)
    {
        //notifico al server che il giocatore principale è pronto alla partita
        strcpy(msg, "ready");
        ret = send(sd, msg, strlen(msg), 0);
        if(ret < 0)
        {
            perror("errore nella send");
            exit(1);
        }

        while (true)
        {
            printf("Attendi che il secondo giocatore si connetta...\n");
            //se ricevo il messaggio di inizio dal server, significa che l'assistente è pronto a giocare e la partita ha inizio
            ret = recv(sd, msg, sizeof(msg), 0);
            if(ret < 0)
            {
                perror("Errore nella recv");
                exit(1);
            }
            else if(ret == 0)
            {
                printf("Il server è stato chiuso\n");
                exit(1);
            }
            if(strcmp(msg, "inizio") == 0)
            {
                memset(msg, 0, sizeof(msg));
                break;
            }
            else if(strcmp(msg, "logout") == 0)
            {
                printf("Il secondo giocatore non è connesso\nChiusura escape room\n");
                exit(1);
            }
            else
            {
                printf("inbox = %s\n", msg);
                continue;
            }
        }
    }
    else
    {
        strcpy(msg, "ready");
        //notifico al server che l'assistente è pronto alla partita
        ret = send(sd, msg, strlen(msg), 0);
        if(ret < 0)
        {
            perror("errore nella send");
            exit(1);
        }

        printf("Attendi l'inizio della partita...\n");
        ret = recv(sd, msg, sizeof(msg), 0);
        if(ret < 0)
        {
            perror("Errore nella recv");
            exit(1);
        } 
        
        //aspetto che il server dia inizio alla partita
        if(strcmp(msg, "inizio") == 0)
        {
            printf("Inizio partita\n");
        }
        else
        {
            printf("Errore inatteso\n");
            exit(1);
        }
                
    }
    printf("\n\nLA PARTITA HA INZIO GIOCATORE %d!\n\n", giocatore);
}

void inizio_partita()
{
    printf("************************************************************************\n"
            "Il laboratorio è stato sigillato!\n"
            "Un potente virus, conosciuto come 'Caos', è stato rilasciato accidentalmente in un laboratorio di biotecnologie avanzate. \n"
            "Hai solo 5 minuti prima che il laboratorio venga completamente irradiato per contenere l’espansione del virus.\n");
    printf("Devi riuscire a sintetizzare l'antidoto, con le istruzioni nascoste da uno scienziato pazzo \n"
            "che ha lasciato indizi nei sistemi del laboratorio. \n"
            "Per uscire devi poi comunicare la formula dell’antidoto alla sala di controllo (giocatore 2).\n");
}

void inizio_partita_assistente()
{
    printf("Partecipi all’escape room.\n"
            "Attendi che il giocatore 1 ti invii la formula per sintetizzare l’antidoto, \n"
            "se il software (il server) ritiene che la formula sia corretta, l’escape room termina altrimenti continua.\n"
            "se vuoi puoi provare ad indovinare la formula da solo\n");
}

void comandi_giocatore()
{
    printf("\nI tuoi comandi per questa escape room sono:\n"
            "look [location | object]\n"
            "take [object]\n"
            "use [object1] [object2]\n"
            "objs\n"
            "send\n"
            "end\n************************************************************************\n\n");
}

void comandi_assistente()
{
    printf("\nI tuoi comandi per questa escape room sono:\n"
            "try [solution]\n"
            "end\n");
}

void partita_giocatore(int sd) 
{
    int ret, max_fd, tempo_rimanente, minuti, secondi;
    char msg[DIM_MSG], msg_stdin[DIM_MSG], risposta[20], comando[6], arg1[15], arg2[15];
    int token_totali, token_raccolti = 0, token_rimanenti;
    uint16_t network_msg;
    fd_set read_fds;
        
    //aspetto di ricevere il numero di token totali da raccogliere
    ret = recv(sd, &network_msg, sizeof(network_msg), 0);
    if(ret < 0)
    {
        perror("Errore nella recv");
        exit(1);
    }
    token_totali = ntohs(network_msg);
    token_rimanenti = token_totali;

    //aspetto di ricevere il tempo totale della partita
    ret = recv(sd, &network_msg, sizeof(network_msg), 0);
    if(ret < 0)
    {
        perror("Errore nella recv");
        exit(1);
    }
    tempo_rimanente = ntohs(network_msg);
    minuti = tempo_rimanente/60;
    secondi = tempo_rimanente%60;

    //entro nel ciclo della partita 
    while (true) 
    {
        memset(msg, 0, sizeof(msg));
        memset(msg_stdin, 0, sizeof(msg_stdin));
        memset(comando, 0, sizeof(comando));
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));
        memset(risposta, 0, sizeof(risposta));
        // Imposto fd_set per select()
        FD_ZERO(&read_fds);
        FD_SET(sd, &read_fds);              // Aggiungo il socket del server al set
        FD_SET(STDIN_FILENO, &read_fds);    // Aggiungo stdin al set
        if(sd > STDIN_FILENO)
        {
            max_fd = sd;
        }
        else
        {
            max_fd = STDIN_FILENO;
        }
        
        ret = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if(ret == -1) 
        {
            perror("Errore nella select");
            exit(1);
        }

        // Controlla se il server ha inviato un messaggio
        memset(msg, 0, sizeof(msg));
        ret = recv(sd, msg, sizeof(msg), MSG_DONTWAIT);             
        if(ret == -1) 
        {
            if(errno != EWOULDBLOCK) 
            {
                perror("Errore nella ricezione del msg\n");
                exit(1);
            }
        } 
        else if(ret > 0) 
        {
            if(strcmp(msg, "corretto") == 0) //l'assitente ha inserito la soluzione corretta per l'antidoto
            {
                printf("La soluzione proposta dall'assistente è corretta!\nLe porte del laboratorio si aprono\nEscape room completata\n");
                break;
            }
            if(strcmp(msg, "TIMEOUT") == 0) //è scaduto il tempo
            {
                printf("TEMPO SCADUTO\nEscape room fallita!\n");
                sleep(3);
                break;
            }
            if(strcmp(msg, "errato") == 0) //l'assistente ha inserito una soluzione errata per l'antidoto
            {
                printf("La soluzione proposta dall'assistente è errata\n");
            }
            if(strcmp(msg, "logout") == 0) //l'assistente si è disconnesso, il giocatore non può vincere da solo
            {
                printf("Purtroppo il tuo assistente ti ha abbandonato\nEscape room fallita\n");
                break;
            }
        } 
        else //ret == 0, server chiuso
        {
            close(sd);
            printf("Il server è stato chiuso, arrivederci\n");
            exit(0);
        }

        // Controlla se l'utente ha inserito un comando in stdin
        if(FD_ISSET(STDIN_FILENO, &read_fds)) 
        {
            if(fgets(msg_stdin, sizeof(msg_stdin), stdin) == NULL) 
            {
                printf("Errore durante la lettura da stdin\n");
                exit(1);
            }

            //prelevo il comando dell'utente
            ret = sscanf(msg_stdin, "%s %s %s", comando, arg1, arg2);
            // Eseguo il comando in base a quanto inserito dall'utente
            if((ret == 1 || ret == 2) && strcmp(comando, "look") == 0) 
            {
                //comando look
                if(ret == 1) //nessun argomento
                {   
                    //invio il comando al server
                    ret = send(sd, comando, sizeof(comando), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }

                    //ricevo il tempo mancante
                    tempo_rimanente = ricevi_tempo(sd);

                    //invio l'argomento null 
                    strcpy(arg1, "null");
                    ret = send(sd, arg1, sizeof(arg1), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }
                    memset(msg, 0, sizeof(msg));
                    //ricevo la risposta
                    ret = recv(sd, msg, sizeof(msg), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella recv"); 
                        exit(1); 
                    }
                    printf("%s\n", msg);
                } 
                else //argomento (oggetto o location) inserito 
                {
                    //invio il comando al server
                    ret = send(sd, comando, sizeof(comando), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }
                    //ricevo il tempo mancante
                    tempo_rimanente = ricevi_tempo(sd);
                    
                    //invio l'argomento del comando look
                    ret = send(sd, arg1, sizeof(arg1), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }
                    //ricevo la risposta
                    ret = recv(sd, msg, sizeof(msg), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella recv"); 
                        exit(1); 
                    }
                    printf("%s\n", msg);
                }
            }
            else if(ret == 2 && strcmp(comando, "take") == 0) 
            {
                //comando take, invio il comando al server
                ret = send(sd, comando, sizeof(comando), 0);
                if(ret < 0) 
                { 
                    perror("Errore nella send"); 
                    exit(1); 
                }
                
                //ricevo il tempo rimanente
                tempo_rimanente = ricevi_tempo(sd);
                
                //invio l'argomento del comando take
                ret = send(sd, arg1, sizeof(arg1), 0);
                if(ret < 0) 
                { 
                    perror("Errore nella send"); 
                    exit(1); 
                }
                //ricevo la risposta dal server
                ret = recv(sd, msg, sizeof(msg), 0);
                if(ret < 0) 
                { 
                    perror("Errore nella recv"); 
                    exit(1);
                }
                //controllo se c'è l'enigma
                printf("%s\n", msg);
                if(strcmp(msg, "L'OGGETTO È BLOCCATO, RISOLVI L'ENIGMA\n") == 0) 
                {
                    //ricevo la domanda dell'enigma
                    ret = recv(sd, msg, sizeof(msg), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella recv"); 
                        exit(1); 
                    }
                    printf("%s\n", msg);
                    printf("Risposta...");
                    scanf("%s", msg);
                    //invio la risposta proposta all'enigma
                    ret = send(sd, msg, sizeof(msg), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }
                    //aspetto l'esito dell'indovinello
                    ret = recv(sd, msg, sizeof(msg), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella recv"); 
                        exit(1); 
                    }
                    printf("%s\n", msg);
                }
                //aspetto di sapere se ha raccolto un token
                ret = recv(sd, msg, sizeof(msg), 0);
                if(ret < 0)
                {
                    perror("Errore nella recv");
                    exit(1);
                }
                else if(strcmp(msg, "token") == 0)
                {
                    token_raccolti ++;
                    printf("Hai raccolto un token!\n");
                }
                 
            }
            else if((ret == 2 || ret == 3) && strcmp(comando, "use") == 0) 
            {
                //comando use
                if(ret == 2) //use con un solo oggetto
                {
                    //invio il comando
                    ret = send(sd, comando, sizeof(comando), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }

                    //ricevo il tempo rimanente
                    tempo_rimanente = ricevi_tempo(sd);
                    
                    //invio il numero di argomenti
                    strcpy(msg, "1");
                    ret = send(sd, msg, strlen(msg), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }
                    
                    //invio l'argomento
                    ret = send(sd, arg1, strlen(arg1), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }
                } 
                else 
                {
                    //invio il comando
                    ret = send(sd, comando, sizeof(comando), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }

                    //ricevo il tempo rimanente
                    tempo_rimanente = ricevi_tempo(sd);

                    //invio il numero di argomenti            
                    strcpy(msg, "2");
                    ret = send(sd, msg, strlen(msg), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }
                    
                    //invio il primo argomento
                    ret = send(sd, arg1, sizeof(arg1), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }

                    //invio il secondo argomento
                    ret = send(sd, arg2, sizeof(arg2), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella send"); 
                        exit(1); 
                    }
                }
                //ricevo l'esito della use
                ret = recv(sd, msg, sizeof(msg), 0);
                if(ret < 0) 
                { 
                    perror("Errore nella recv"); 
                    exit(1); 
                }
                printf("%s\n", msg);
                 
            }
            else if(ret == 1 && strcmp(comando, "objs") == 0) 
            {
                //comando objs, invio il comando
                ret = send(sd, comando, sizeof(comando), 0);
                if(ret < 0) 
                { 
                    perror("Errore nella send"); 
                    exit(1); 
                }

                //ricevo il tempo rimanente
                tempo_rimanente = ricevi_tempo(sd);
                
                //ciclo finché ci sono oggetti 
                while(true) 
                {
                    memset(msg, 0, sizeof(msg));
                    //ricevo l'oggetto
                    ret = recv(sd, msg, sizeof(msg), 0);
                    if(ret < 0) 
                    { 
                        perror("Errore nella recv"); 
                        exit(1); 
                    }
                    if(strcmp(msg, "fine oggetti") == 0) //oggetti finiti interrompo il ciclo
                        break;
                    printf("%s\n", msg); //stampo l'oggetto ricevuto
                }
                printf("\n");
                
            }
            else if(ret == 1 && strcmp(comando, "send") == 0) 
            {
                //comando send, invio il comando 
                ret = send(sd, comando, sizeof(comando), 0);
                if(ret < 0) 
                { 
                    perror("Errore nella send"); 
                    exit(1); 
                }

                //ricevo il tempo rimanente
                tempo_rimanente = ricevi_tempo(sd);
                
                printf("Digita qui la formula dell'antidoto, verrà inviata al tuo collega fuori dal laboratorio\n...");
                scanf("%s", msg);
                //invio la formula proposta dal giocatore
                ret = send(sd, msg, sizeof(msg), 0);
                if(ret < 0) 
                { 
                    perror("Errore nella send"); 
                    exit(1); 
                }
                printf("Inviato\n");
                 
            }
            else if(ret == 1 && strcmp(comando, "end") == 0) 
            {
                //comando end
                printf("Grazie per aver giocato, arrivederci!\n");
                break;
            }
            else if(ret > 0)
            {
                //comando non riconosciuto
                printf("Attenzione, il comando inserito non è stato riconosciuto, si prega di riprovare\n");
                comandi_giocatore();
                continue;
            }
            //aggiorno i token rimanenti e stampo le info della partita
            token_rimanenti = token_totali - token_raccolti;
            minuti = tempo_rimanente/60;
            secondi = tempo_rimanente%60;
            printf("\n************************************************************************\n"
                "INFO PARTITA:\n"
                "TEMPO RIMANENTE: %d minuti %d secondi\n"
                "TOKEN RACCOLTI: %d\tTOKEN RIMANENTI: %d\n\n", (minuti >= 0)? minuti:0, (secondi >= 0)? secondi:0, token_raccolti, token_rimanenti);
        }   
    }
}

void partita_assistente(int sd)
{
    int ret;
    char msg[256], msg_stdin[256], esito[20], comando[6], arg1[10];
    fd_set read_fds;

    while(true)
    {
        // Inizializzazione dei buffer
        memset(msg, 0, sizeof(msg));
        memset(msg_stdin, 0, sizeof(msg_stdin));
        memset(comando, 0, sizeof(comando));
        memset(arg1, 0, sizeof(arg1));
        memset(esito, 0, sizeof(esito));

        // Configurazione dei descrittori di file per la select
        FD_ZERO(&read_fds);
        FD_SET(sd, &read_fds);              // Descrittore del socket
        FD_SET(STDIN_FILENO, &read_fds);    // Descrittore di input da tastiera

        ret = select(sd + 1, &read_fds, NULL, NULL, NULL);
        if (ret < 0)
        {
            perror("Errore nella select");
            exit(1);
        }

        // Controllo se ci sono messaggi dal server
        if (FD_ISSET(sd, &read_fds))
        {
            ret = recv(sd, msg, sizeof(msg), 0);
            if (ret > 0)
            {
                printf("Inbox: %s\n", msg);
                if (strcmp(msg, "logout") == 0) //il giocatore ha fatto end
                {
                    printf("Il giocatore si è arreso, non riesce a trovare un antidoto\nEscape room fallita\n");
                    break;
                }
                if (strcmp(msg, "TIMEOUT") == 0)    //tempo scaduto
                {
                    printf("\nTempo scaduto\nEscape room fallita\n");
                    sleep(3);
                    break;
                }
                if (strcmp(msg, "errato") == 0) //soluzione dell'antidoto proposta errata
                {
                    printf("\nLa soluzione proposta è errata\n");
                }
            }
            else if (ret == 0)  //server chiuso
            {
                close(sd);
                printf("Il server è stato chiuso, arrivederci\n");
                exit(0);
            }
            else
            {
                perror("Errore nella ricezione del msg");
                exit(1);
            }
        }

        // Controllo se c'è input da tastiera
        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            if (fgets(msg_stdin, sizeof(msg_stdin), stdin) == NULL)
            {
                printf("Errore durante la lettura da stdin\n");
                exit(1);
            }

            //prelevo il comando dell'assistente
            ret = sscanf(msg_stdin, "%s %s", comando, arg1);
            if (ret == 2 && strcmp(comando, "try") == 0)
            {
                //comando try, invio il comando al server
                ret = send(sd, comando, sizeof(comando), 0);
                if (ret < 0)
                {
                    perror("Errore nella send");
                    exit(1);
                }

                //invio la soluzione proposta
                ret = send(sd, arg1, sizeof(arg1), 0);
                if (ret < 0)
                {
                    perror("Errore nella send");
                    exit(1);
                }

                //ricevo l'esito della soluzione
                ret = recv(sd, esito, sizeof(esito), 0);
                if (ret < 0)
                {
                    perror("Errore nella recv");
                    exit(1);
                }
                if (strcmp(esito, "corretto") == 0) //escape room completata
                {
                    printf("Avete trovato l'antidoto!\nLe porte del laboratorio si aprono\nEscape room completata\n");
                    break;
                }
                if (strcmp(esito, "errato") == 0)   //soluzione errata
                {
                    printf("La soluzione proposta è errata\n");
                }
            }
            else if (ret == 1 && strcmp(comando, "end") == 0)
            {
                // Comando di fine sessione
                printf("Grazie per aver giocato, arrivederci!\n");
                break;
            }
            else
            {
                // Comando non valido
                printf("Attenzione, il comando inserito non è stato riconosciuto, si prega di riprovare\n");
                comandi_assistente();
            }
        }
    }
}





