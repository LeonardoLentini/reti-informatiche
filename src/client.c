#include "partita.h"
#include "include.h"

#define MAX_CLIENT 2    /* numero massimo di client che si possono connettere */
#define DIM_BUFFER 1024 /* grandezza buffer del client */
#define DIM_COMANDO 6   /* dimensione massima del comando  */
#define DIM_MSG 256     /* dimensione massima dei messaggi per la comunicazione client-server */

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


int main(int argc, char* argv[])
{
    int ret, sd, c, giocatore, i, scelta;
    uint8_t room = 0;
    uint16_t porta;
    char buf[DIM_BUFFER], comando[DIM_COMANDO], msg[DIM_MSG], email[50], psw[25];
    struct sockaddr_in server_addr;

    // Verifica sia presente un argomento valido per la porta
    if (argc != 2) 
    {
        printf("Inserire correttamente la porta.\n\n\tSintassi: ./client <porta>\n\n");
        exit(1);
    }
    porta = htons(4242);


    // Creazione socket del client
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0){
        perror("Errore socket non creato");
        exit(1);
    }
    printf("Socket creato\n");
    
    // Inizializzazione
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_port = porta;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    printf("Inizializzata struttura per l'indirizzo del server e socket del client\n");
    
    // Connessione al server
    for(i = 0; i < 10; i++){
        printf("Tentativo di connessione con il server...\n");
        ret = connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(ret == 0)
        {
            break;
        }
        sleep(3);
    }
    if(ret < 0){
        perror("Errore nella connect");
        exit(1);
    }
    printf("Connessione Stabilita con porta: %d\n\n", ntohs(porta));
    
    // Connessione effettuata, inizia lo scambio di informazioni con il server
    printf("Effettua il login oppure registrati:\n");

    while(true) 
    {
        printf("Per effettuare il LOGIN digita 1\nPer effettuare la REGISTRAZIONE digita 2\n...");
        
        // Controllo se scanf riesce a leggere un singolo intero
        if (scanf("%d", &scelta) != 1) 
        {
            printf("Il comando inserito non è valido. Si prega di inserire un comando valido.\n");
            printf("Inserire uno dei due numeri indicati e premere invio.\n\n");
            while (getchar() != '\n');  // Pulisce il buffer di input
            continue;
        }

        // Controlla se la scelta è valida
        if (scelta != 1 && scelta != 2) 
        {
            printf("Il comando inserito non è valido. Si prega di inserire un comando valido.\n");
            printf("Inserire uno dei due numeri indicati e premere invio.\n\n");
            continue;
        }

        if(scelta == 1)
        {
            //opzione login
            strcpy(comando, "login");
        }
        else 
        {
            //opzione signup
            strcpy(comando, "signup");
        }

        //invio il comando signup/login al server
        ret = send(sd, comando, sizeof(comando), 0);
        if(ret < 0)
        {
            perror("Errore nella send");
            exit(1);
        }

        memset(msg, 0, sizeof(msg));
        // aspetto la risposta del server di comando correttamente ricevuto
        ret = recv(sd, msg, sizeof(msg), 0);
        if(ret < 0)
        {
            perror("Errore nella recv");
            exit(1);
        }
        if(strcmp(msg, "Comando arrivato") != 0)
        {
            perror("Comando non arrivato correttamente\n");
            exit(1);
        }

        //richiedo la mail all'utente e la invio al server
        printf("Email:");
        scanf("%50s", email);
        ret = send(sd, email, sizeof(email), 0);
        if(ret < 0)
        {
            perror("Errore nella send");
            exit(1);
        }

        if(strcmp(comando, "signup") == 0)
        {
            //se la mail non era già registrata richiedo e invio la password
            ret = recv(sd, msg, sizeof(msg), 0);
            if(ret < 0)
            {
                perror("Errore nella recv");
                exit(1);
            }
            if(strcmp(msg, "Invio richiesta password") == 0)
            {
                printf("Password:");
                scanf("%25s", psw);
                ret = send(sd, psw, sizeof(psw), 0);
                if(ret < 0)
                {
                    perror("Errore nella send");
                    exit(1);
                }
            }
            else if(strcmp(msg, "La mail inserita è già in uso") == 0)
            {
                printf("%s\n", msg);
                continue;
            }
            
            //attendo l'esito della registrazione
            ret = recv(sd, msg, sizeof(msg), 0);
            if(ret < 0)
            {
                perror("Errore nella recv");
                exit(1);
            }
            printf("%s\n", msg);
            if(strcmp(msg, "Registrazione effettuata") == 0)
            {
                continue;
            }
            else
            {
                printf("Errore inatteso, registrazione non possibile\n");
                break;
            }
            
        }
        else
        {   
            //comando login, aspetto di sapere se la mail è riconosciuta o l'utente è già online
            memset(msg, 0, sizeof(msg));
            ret = recv(sd, msg, sizeof(msg), 0);
            if(ret < 0)
            {
                perror("Errore nella recv");
                exit(1);
            }


            if(strcmp(msg, "Utente già online") == 0)
            {
                printf("%s\n", msg);
                continue;
            }
            else if(strcmp(msg, "Email non registrata") == 0)
            {
                printf("%s\n\n", msg);
                continue;
            }
            else if(strcmp(msg, "Utente trovato") == 0)            
            {                
                //posso richiedere e inviare la psw
                printf("Password:");                
                scanf("%25s", psw);                
                ret = send(sd, psw, sizeof(psw), 0);                
                if(ret < 0)                
                {                    
                    perror("Errore nella send");                    
                    exit(1);                
                }                

                //aspetto l'esito dal server
                memset(msg, 0, sizeof(msg));                
                ret = recv(sd, msg, sizeof(msg), 0);                
                if(ret < 0)                
                {                    
                    perror("Errore nella recv");
                    exit(1);
                }

                if(strcmp(msg, "Password errata") == 0)
                {
                    printf("%s\n", msg);
                    continue;
                }
                else if(strcmp(msg, "Login effettuato con successo") == 0)
                {
                    printf("%s\n", msg);
                    break;
                }
                else    
                {
                    printf("Errore inatteso, si prega di riprovare\n");
                    continue;
                }
            }
        }
    }

    //login effettuato
    //Se è il primo utente mostro le stanze e faccio scegliere
    //Se è il secondo utente lo connetto alla stanza selezionata


    // Pulizia stdin perché può rimanere sporco
    while ((c = getchar()) != '\n' && c != EOF) ;

    //Aspetto che il server mi dica se devo scegliere la stanza oppure mi connetto a quella esistente
    ret = recv(sd, (void *)&room, sizeof(room), 0);
    if(ret < 0)
    {
        perror("Errore nella recv");
        exit(1);
    }
    //il server mi invia la stanza, se è 0 deve ancora essere scelta
    if(room == 0)
    {
        while (true)
        {
            rooms();    //mostro gli scenari disponibili
            //prelevo la scelta dall'stdin
            fgets(buf, sizeof(buf), stdin);
            sscanf(buf, "%5s %hhu", comando, &room);
            if((strcmp(comando, "start") == 0) && room == 1)
                break;
            if((strcmp(comando, "start") == 0) && room == 2){
                printf("Work in progress, per il momento scegliere un altro scenario\n");
                continue;
            }
            printf("Comando non valido, prova con\n\n\t start <room>\n\n");
        }
        printf("La stanza selezionata è la numero: %d\n", room);
        //invio la scelta al server
        ret = send(sd, (void *)&room, sizeof(room), 0);
        if(ret < 0)
        {
            perror("Errore nella send");
            exit(1);
        }
        //assegno il giocatore come principale
        giocatore = 1;
    }
    else
    {
        printf("Sei il secondo giocatore, benvenuto nell'escape room\n");
        //assegno il giocatore come secondario (assistente)
        giocatore = 2;
    }

    if(room == 1)
    {
        preparazione_partita(sd, giocatore);
        if(giocatore == 1)
        {
            inizio_partita();
            comandi_giocatore();
            partita_giocatore(sd);
        }
        else
        {
            inizio_partita_assistente();
            comandi_assistente();
            partita_assistente(sd);
        }
    }
    else
    {
        printf("Gestione altre escape rooms\n\n");
    }   
    close(sd); 
    exit(0);

}