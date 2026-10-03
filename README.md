# SOG-Drop

🇮🇹 Italiano · 🇬🇧 [English](docs/en/README.md) *(traduzione in arrivo)*

Sistema automatico di fertirrigazione per 3 piante in vaso, basato su ESP32. Funziona **senza internet e senza supervisione**: lo programmi col telefono quando sei in serra e poi va avanti da solo per tutto il ciclo di coltivazione.

## Come funziona

Si imposta un **piano settimanale**: per ogni settimana i litri per pianta, i giorni di irrigazione e la dose di ogni fertilizzante (ml/L). A ogni irrigazione il sistema:

1. riempie la vasca dal rubinetto con la quantità d'acqua necessaria, misurandola a peso
2. dosa i fertilizzanti uno alla volta con le pompe peristaltiche, mescolando dopo ognuno
3. distribuisce la soluzione alle piante, controllando a peso quanto riceve ciascuna
4. scrive tutto nel registro

Se il volume supera la capienza della vasca, il ciclo si ripete a lotti. La concentrazione della soluzione avanzata in vasca viene tenuta in conto nel lotto successivo.

## Comportamento senza supervisione

| Situazione | Cosa fa il sistema |
|---|---|
| Interruzione di corrente | Al ritorno riprende il ciclo dal punto in cui era. Un fertilizzante interrotto a metà viene saltato, per non dosarlo due volte. L'ora resta corretta grazie alla batteria dell'RTC. |
| Manca acqua dal rubinetto | Riprova ogni 30 minuti, fino a 6 volte, poi rinuncia per quel giorno |
| Un fertilizzante sembra finito | Lo registra come avviso e irriga comunque |
| Una pompa delle piante è bloccata | Esclude quella pianta dal ciclo e continua con le altre |
| Vasca troppo piena | Il galleggiante chiude la valvola, sia via software sia via hardware |
| Il programma si blocca | Il watchdog riavvia l'ESP32 entro 30 secondi |
| Qualsiasi uscita accesa troppo a lungo | Viene spenta automaticamente da un limite di sicurezza |

Il LED sulla scheda indica lo stato: **lampeggio lento** = tutto ok, **lampeggio veloce** = ci sono avvisi da leggere oppure l'ora non è impostata, **acceso fisso** = irrigazione in corso.

## Uso

1. Collegati col telefono alla rete WiFi **SOG-Drop** (nome e password in `include/secrets.h`, vedi [Compilare e caricare](#compilare-e-caricare)) e apri **http://192.168.4.1**
2. **Prima volta:**
   - *Stato* → **Sincronizza ora dal telefono**
   - *Manuale* → tara e calibrazione della bilancia
   - *Manuale* → calibrazione di ogni pompa peristaltica
   - *Manuale* → prova ogni pompa e la valvola per qualche secondo
3. *Piano* → imposta data di inizio, ora, fertilizzanti e settimane, spunta **Attivo** e premi **Salva**
4. Nel *Riepilogo* trovi quanto concentrato serve per tutto il piano: lascia nelle taniche questa quantità più una scorta
5. *Stato* → controlla la prossima irrigazione. Con **Irriga ora** puoi fare un ciclo di prova.

Al ritorno in serra, apri *Stato* per gli avvisi e *Registro* per lo storico completo.

## Sviluppo

- Hardware, collegamenti e lista della spesa: [docs/it/hardware.md](docs/it/hardware.md)
- Documentazione divisa per lingua: [docs/it/](docs/it/) (italiano), [docs/en/](docs/en/) (inglese, in arrivo)
- Firmware: C++ (Arduino) con [PlatformIO](https://platformio.org/)

### Compilare e caricare

1. Installa l'estensione **PlatformIO IDE** in VS Code
2. Apri la cartella del progetto
3. Copia `include/secrets.example.h` come `include/secrets.h` e scegli nome e password del WiFi (il file è escluso da git)
4. Collega l'ESP32 via USB e premi **Upload** (freccia nella barra in basso)
5. **Serial Monitor** (115200 baud) mostra il registro in tempo reale

### Struttura

| File | Contenuto |
|---|---|
| `include/config.h` | Pin, WiFi, fuso orario, limiti di sicurezza |
| `include/secrets.h` | Nome e password del WiFi (locale, escluso da git) |
| `src/main.cpp` | Avvio e loop principale |
| `src/cycle.*` | Ciclo di fertirrigazione (macchina a stati) |
| `src/schedule.*` | Calendario settimanale |
| `src/storage.*` | Piano, calibrazioni e stato salvati in memoria (LittleFS) |
| `src/scale.*` | Cella di carico e galleggiante |
| `src/outputs.*` | Relè con limiti di tempo di sicurezza |
| `src/clock.*` | RTC DS3231, fuso e ora legale |
| `src/log.*` | Registro su file e avvisi |
| `src/web.*`, `src/web_ui.h` | Rete WiFi locale, API e pagina web |
