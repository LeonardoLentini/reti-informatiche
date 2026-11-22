# Il virus del Caos – Escape Room

## 📋 Descrizione del Progetto
Questa applicazione è un'**escape room testuale cooperativa** con architettura **client-server**, sviluppata come progetto per il corso di **Reti Informatiche**. Due giocatori devono collaborare per uscire da un laboratorio di biotecnologie sigillato prima che il tempo scada.

Il sistema offre:
* **Server di Gioco:** Gestisce registrazione e login degli utenti, la sincronizzazione dei due giocatori, lo stato della partita e il timer di 5 minuti.
* **Due Ruoli Distinti:**
    * **Giocatore principale:** Esplora le location del laboratorio, raccoglie oggetti, risolve gli indovinelli e sintetizza l'antidoto.
    * **Assistente di laboratorio:** Riceve la formula dell'antidoto dal compagno e la verifica nel sistema del laboratorio per sbloccare le porte.
* **Comunicazione TCP:** Messaggi testuali a dimensione fissa (256 byte) per comandi e risposte, e messaggi binari (`uint16_t`) per il tempo rimanente.
* **I/O Multiplexing:** Il server gestisce con `select()` lo standard input, il socket di ascolto e i due client; i client monitorano allo stesso modo tastiera e socket del server.
* **Thin Client:** Tutta la logica di gioco risiede nel server, il client si limita a inviare comandi e mostrare le risposte.

## 🛠️ Stack Tecnologico
* **Linguaggio:** C.
* **Rete:** Socket TCP (API POSIX/Berkeley).
* **Build:** GCC e Make.
* **Sistema Operativo:** Linux (testato anche su WSL).

---

## 🚀 Guida all'Installazione e all'Avvio

### 1. Compilazione
Dalla cartella principale della repository:
```bash
make
```
Vengono generati tre eseguibili nella cartella corrente: `server`, `client` (giocatore principale) e `other` (assistente).

### 2. Avvio
Servono **tre terminali** aperti nella cartella della repository. Avviare prima i due client, che tentano di connettersi al server ogni 3 secondi per circa 30 secondi:

| Terminale | Comando | Ruolo |
|-----------|---------|-------|
| A | `./client 6000` | Primo client |
| B | `./other 6100` | Secondo client |
| C | `./server` | Server |

Nel terminale del server digitare:
```
start 4242
```
Entro pochi secondi i due client si connettono.

> ℹ️ **Nota:** Come previsto dalla specifica del corso, i numeri di porta passati da linea di comando non vengono utilizzati: la porta del server si sceglie con il comando `start`, mentre i client contattano sempre il server sulla porta `4242`, inserita staticamente nel codice.

In alternativa, su un sistema con `gnome-terminal`, lo script del corso compila e apre automaticamente le tre finestre:
```bash
./exec2024.sh
```
(oppure `make run`). Nella finestra del server va comunque digitato `start 4242`.

### 3. Registrazione e Login
In **entrambi** i client:
1.  Digitare `2` per registrarsi e inserire email e password.
2.  Digitare `1` per effettuare il login con le stesse credenziali.

### 4. Inizio della Partita
Il primo client che effettua il login diventa il **giocatore principale** e sceglie lo scenario con:
```
start 1
```
Il secondo client diventa automaticamente l'**assistente**.

> ⚠️ Il secondo client deve aver **già effettuato il login** prima che il giocatore principale scelga lo scenario, altrimenti il server chiude la partita con il messaggio *"Il secondo giocatore non è connesso"*.

### 5. Arresto del Server
A partita conclusa, digitare nel terminale del server:
```
stop
```

---

## 🎮 Comandi di Gioco

### Server
| Comando | Descrizione |
|---------|-------------|
| `start <porta>` | Avvia il server sulla porta indicata |
| `stop` | Arresta il server (solo se non ci sono partite in corso) |

### Giocatore principale
| Comando | Descrizione |
|---------|-------------|
| `look` | Descrive la stanza |
| `look <location \| oggetto>` | Descrive una location o un oggetto |
| `take <oggetto>` | Raccoglie un oggetto (alcuni sono bloccati da un indovinello) |
| `use <oggetto1> [oggetto2]` | Usa un oggetto o combina due oggetti |
| `objs` | Mostra gli oggetti raccolti |
| `send` | Invia la formula dell'antidoto all'assistente |
| `end` | Abbandona la partita |

### Assistente
| Comando | Descrizione |
|---------|-------------|
| `try <soluzione>` | Prova una formula per l'antidoto |
| `end` | Abbandona la partita |

---

## 🧩 Soluzione
<details>
<summary>Mostra la soluzione completa (spoiler)</summary>

**Giocatore principale:**
```
look
look tavolo
look centrifuga
look presa
take presa
take provette          → indovinello, risposta: sodio
look deposito
look microscopio
take campione          → indovinello, risposta: 7
use provette campione
send                   → formula: C6H12O6
```

**Assistente:**
```
try C6H12O6
```

> ⚠️ Il messaggio di gioco mostra la formula come `C6H1206` (con lo **zero**), ma la soluzione accettata dal server è `C6H12O6` (con la **lettera O**).

</details>

---

## 📂 Struttura della Repository
```text
/
├── docs/
│   └── Documentazione.pdf   # Relazione del progetto
├── src/
│   ├── server.c             # Server: console, I/O multiplexing e logica di gioco
│   ├── client.c             # Client: connessione, registrazione/login e scelta dello scenario
│   ├── partita.c/.h         # Gestione della partita lato client per i due ruoli
│   ├── utente.c/.h          # Registrazione, login e lista utenti; socket di ascolto del server
│   ├── item.c/.h            # Strutture dello scenario (location, oggetti, enigmi, token)
│   └── include.h            # Header di sistema comuni
├── exec2024.sh              # Script di compilazione ed esecuzione fornito dal corso
└── makefile
```
