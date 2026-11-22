#include "include.h"
#include "utente.h"

#define DIM_MSG 256     /* dimensione massima dei messaggi per la comunicazione client-server */

int creazione_sock_server(struct sockaddr_in *my_addr, int porta)
{
    int sd, ret;

    // Creazione socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1)
    {
        perror("Errore nella creazione del socket");
        exit(1);
    }
    printf("Socket creato\n");

    //permette il riavvio del server velocemente senza problemi di porta
    ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if(ret < 0) 
    {
        perror("ERR: Errore in setsockopt(...) - ");
        exit(1);
    }
    
    // Inizializzazione
    memset(my_addr, 0, sizeof(*my_addr));

    my_addr->sin_port = porta;
    my_addr->sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &my_addr->sin_addr);

    ret = bind(sd, (struct sockaddr*)my_addr, sizeof(*my_addr));
    if(ret == -1)
    {
        perror("Errore nella bind");
        exit(1);
    }
    printf("Assegnato indirizzo e porta al socket\n");

    return sd;
}

void comandi_server()
{
    printf("\n************************** CONSOLE DEL SERVER **************************\n\n");
    printf("Comandi disponibili:\n\n");
    printf("1)  start <port> --> Avvia un nuovo server di gioco\n");
    printf("2)  stop --> Ferma il server\n");
    printf("************************************************************************\n\n");
}

void stampaUtenti(utente* head) 
{
    utente* current = head;
    while (current != NULL) {
        printf("ID: %d\n", current->id);
        printf("Online: %s\n", current->online ? "Sì" : "No");
        printf("Email: %s\n", current->email);
        printf("Password: %s\n", current->password);
        printf("-------------------\n");
        current = current->next;
    }
}

struct utente* trovaUtente(utente* head, const char* email) 
{
    utente* corrente = head;
    while (corrente != NULL) {
        if (strcmp(corrente->email, email) == 0) {
            return corrente;
        }
        corrente = corrente->next;
    }
    return NULL;
}

int signup(int sd, struct utente** lista_utenti) 
{
    int ret, new_id;
    char email[50], psw[25], msg[DIM_MSG];
    struct utente* corrente = NULL;
    struct utente* nuovoUtente = NULL;
    //comunico che il comando è arrivato correttamente e si può procedere al signup
    strcpy(msg, "Comando arrivato");
    ret = send(sd, msg, strlen(msg), 0);
    if(ret < 0) {
        perror("Errore nella send");
        exit(1);
    }
    //ricevo l'email
    ret = recv(sd, email, sizeof(email), 0);
    if(ret < 0) {
        perror("Errore ricezione email\n");
        exit(1);
    }
    //cerco se l'utente fosse già presente 
    corrente = trovaUtente(*lista_utenti, email);
    if(corrente != NULL) 
    {
        strcpy(msg, "La mail inserita è già in uso");
        ret = send(sd, msg, sizeof(msg), 0);
        if(ret < 0) 
        {
            perror("Errore nella send");
            exit(1);
        }
        return -1;
    }
    //alloco lo spazio per il nuovo utente
    nuovoUtente = (struct utente*)malloc(sizeof(struct utente));
    if (nuovoUtente == NULL) 
    {
        perror("Errore nella allocazione della memoria per la struttura utente.\n");
        exit(1);
    }
    nuovoUtente->email = (char*)malloc(strlen(email) + 1);
    if (nuovoUtente->email == NULL) 
    {
        printf("Errore nella allocazione della memoria per email.\n");
        free(nuovoUtente);
        exit(1);
    }
    //richiedo la password
    strcpy(msg, "Invio richiesta password");
    ret = send(sd, msg, sizeof(msg), 0);
    if(ret < 0) 
    {
        perror("Errore nella send");
        exit(1);
    }
    ret = recv(sd, psw, sizeof(psw), 0);
    if(ret < 0) 
    {
        perror("Errore ricezione password\n");
        exit(1);
    }
    nuovoUtente->password = (char*)malloc(strlen(psw) + 1);
    if (nuovoUtente->password == NULL) {
        printf("Errore nella allocazione della memoria per password.\n");
        free(nuovoUtente->email);
        free(nuovoUtente);
        exit(1);
    }
    //aggiorno i campi del nuovo utente
    strcpy(nuovoUtente->email, email);
    strcpy(nuovoUtente->password, psw);
    nuovoUtente->socket_id = sd;
    nuovoUtente->next = NULL;
    new_id = 1;
    //inserisco l'utente in fondo alla lista utenti
    corrente = *lista_utenti;
    if (corrente == NULL) 
    {
        nuovoUtente->id = new_id;
        *lista_utenti = nuovoUtente;  
    } 
    else 
    {
        while (corrente->next != NULL) 
        {
            corrente = corrente->next;
            new_id++;
        }
        nuovoUtente->id = new_id + 1;
        corrente->next = nuovoUtente;
    }
    printf("Utente registrato con id %d\n", new_id);
    printf("\nUtenti attualmente registrati: \n\n");
    stampaUtenti(*lista_utenti);
    return new_id;
}

int login(int sd, struct utente** lista_utenti) 
{
    int ret;
    char email[50], psw[25], msg[DIM_MSG];
    struct utente* corrente = NULL;
    //comunico che il comando è arrivato correttamente e si può procede al login
    strcpy(msg, "Comando arrivato");
    ret = send(sd, msg, sizeof(msg), 0);
    if(ret < 0) 
    {
        perror("Errore nella send");
        exit(1);
    }
    //ricevo l'email
    ret = recv(sd, email, sizeof(email), 0);
    if(ret < 0) 
    {
        perror("Errore ricezione email\n");
        exit(1);
    }
    //controllo se l'email è registrata
    corrente = trovaUtente(*lista_utenti, email);
    if(corrente != NULL) 
    {
        if(corrente->online == true) 
        {
            strcpy(msg, "Utente già online");
            ret = send(sd, msg, strlen(msg), 0);
            if(ret < 0) 
            {
                perror("Errore nella send");
                exit(1);
            }
            return -1;
        } 
        else    //utente trovato e non online
        {
            memset(msg, 0, sizeof(msg));
            strcpy(msg, "Utente trovato");
            ret = send(sd, msg, sizeof(msg), 0);
            if(ret < 0) 
            {
                perror("Errore nella send");
                exit(1);
            }
            memset(psw, 0, sizeof(psw));
            //ricevo la password
            ret = recv(sd, psw, sizeof(psw), 0);
            if(ret < 0) 
            {
                perror("Errore ricezione password\n");
                exit(1);
            }
            //confronto con la password cell'utente
            if(strcmp(corrente->password, psw) == 0) 
            {
                strcpy(msg, "Login effettuato con successo");
                ret = send(sd, msg, sizeof(msg), 0);
                if(ret < 0) 
                {
                    perror("Errore nella send");
                    exit(1);
                }
                corrente->online = true;
                return corrente->id;
            } 
            else 
            {
                strcpy(msg, "Password errata");
                ret = send(sd, msg, sizeof(msg), 0);
                if(ret < 0) 
                {
                    perror("Errore nella send");
                    exit(1);
                }
                return 0;
            }
        }
    } 
    else 
    {
        strcpy(msg, "Email non registrata");
        ret = send(sd, msg, sizeof(msg), 0);
        if(ret < 0) 
        {
            perror("Errore nella send");
            exit(1);
        }
        return -2;
    }
}

void free_utente(utente* user)
{
    if (user != NULL) 
    {
        free(user->email);
        free(user->password);
        
        free(user);
    }
}

void free_lista_utenti(utente* head) 
{
    utente* current = head;
    utente* next;

    while (current != NULL) 
    {
        next = current->next; 
        free_utente(current); 
        current = next; 
    }
}

void logout_utente(struct utente **lista, int socket_id)
{
    struct utente* corrente = *lista;
    // Cerca l'account nella lista
    while (corrente != NULL) 
    {
        if (corrente->socket_id == socket_id) 
        {
            corrente->online = false;
            printf("L'utente %d è andato offline\n", socket_id);
        }
        corrente = corrente->next;
    }
}