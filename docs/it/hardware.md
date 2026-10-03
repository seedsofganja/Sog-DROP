# Hardware

## Schema dell'impianto

```
Rubinetto ─► Filtro ─► Elettrovalvola 12V NC ─┐
                                              ▼
 Taniche concentrati ─► Pompe peristaltiche ─► ┌───────────────────┐ ◄─ galleggiante di massimo
 (fino a 5)                                    │  Vasca 5 L        │
                                               │  su cella di carico│ ◄─ pompa di ricircolo
                                               └──┬──────┬──────┬──┘
                                                  ▼      ▼      ▼
                                              Pompa 1 Pompa 2 Pompa 3
                                                  ▼      ▼      ▼
                                              Pianta 1 Pianta 2 Pianta 3
```

## Lista della spesa

| Componente | Qtà | Note | Costo indicativo |
|---|---|---|---|
| ESP32 DevKit V1 (ESP-WROOM-32) | 1 | | 8 € |
| Modulo RTC DS3231 con batteria CR2032 | 1 | Mantiene l'ora anche senza corrente | 3 € |
| Cella di carico 5 kg + modulo HX711 | 1 | Pesa la vasca | 8 € |
| Elettrovalvola 12V normalmente chiusa, ½" | 1 | Plastica, per uso alimentare | 8 € |
| Filtro a Y o a retina, ½" | 1 | Protegge la valvola dal calcare | 5 € |
| Galleggiante a contatto (NC a livello basso) | 1 | Sicurezza anti-trabocco | 3 € |
| Pompe peristaltiche 12V | 3 (fino a 5) | Kamoer NKP o simili | 10-15 € cad. |
| Pompe a membrana 12V (tipo R385 o mini-pompe da 3-6 W) | 4 | 3 piante + 1 ricircolo | 5-8 € cad. |
| Scheda relè 16 canali, 12V, optoisolata | 1 | Oppure 2 schede da 8 | 12 € |
| Alimentatore 12V 5A | 1 | | 12 € |
| Step-down 12V → 5V (es. LM2596 o MP1584) | 1 | Alimenta l'ESP32 dal pin VIN | 2 € |
| Vasca o tanica alimentare da 5 L | 1 | Meglio opaca: la luce fa crescere le alghe | 5 € |
| Taniche o bottiglie per i concentrati | 3-5 | Opache, con tappo forato per il tubo | |
| Tubo in silicone, tubo da irrigazione 4/6 mm, gocciolatori o anelli | | | 10 € |
| Scatola stagna IP65, pressacavi, morsetti | 1 | | 10-15 € |

## Collegamenti all'ESP32

| Funzione | GPIO | Note |
|---|---|---|
| Elettrovalvola | 23 | via relè |
| Pompa ricircolo | 19 | via relè |
| Pompa pianta 1 / 2 / 3 | 18 / 17 / 16 | via relè |
| Fertilizzante 1 / 2 / 3 / 4 / 5 | 13 / 4 / 27 / 26 / 25 | via relè |
| HX711 DT (DOUT) | 34 | |
| HX711 SCK | 32 | |
| Galleggiante di massimo | 33 | l'altro capo va a GND |
| RTC DS3231 SDA / SCL | 21 / 22 | alimentazione 3.3V |
| LED di stato | 2 | LED blu sulla scheda |

I pin sono definiti in [include/config.h](../../include/config.h).

## Sicurezze da rispettare nel cablaggio

1. **Galleggiante in serie alla valvola.** Oltre che all'ESP32, collega un secondo contatto del galleggiante in serie all'alimentazione dell'elettrovalvola (oppure usa un relè comandato dal galleggiante). In questo modo la vasca non può traboccare neanche se l'ESP32 si guasta con la valvola aperta.
2. **Elettrovalvola normalmente chiusa (NC).** Se manca la corrente, l'acqua del rubinetto non passa.
3. **Galleggiante cablato "a sicurezza positiva".** Il contatto è chiuso a livello normale. Se il filo si stacca, il sistema lo legge come vasca piena e non riempie.
4. **Relè e ESP32.** Molte schede relè a 5V non scattano bene con i 3.3V dell'ESP32. Scegli una scheda con ingressi optoisolati compatibili con 3.3V, oppure alimenta la parte optoisolatore (VCC) a 3.3V e la parte bobine (JD-VCC) a 12V o 5V secondo il modello. Se il relè si attiva con il pin a HIGH, imposta `RELAY_ACTIVE_LOW = false` in `config.h`.
5. **Diodi.** Se al posto dei relè usi moduli MOSFET, metti un diodo di ricircolo (1N4007 o simile) in parallelo a ogni pompa e alla valvola.
6. **Umidità.** Tieni tutta l'elettronica nella scatola IP65, con i pressacavi verso il basso. Se possibile, applica una vernice protettiva (conformal coating) sulle schede.

## Montaggio della vasca

- La vasca deve poggiare **solo** sulla cella di carico, senza tubi rigidi che la tirano o la sostengono: tubi morbidi e lenti.
- Le pompe delle piante e del ricircolo si fissano *fuori* dalla vasca (aspirano dal fondo con un tubo), così il loro peso non pesa sulla bilancia.
- I tubi dei fertilizzanti entrano dall'alto e non devono toccare il liquido, per evitare che sifonino.
- Il ricircolo aspira dal fondo e rimanda nella vasca stessa: basta per mescolare.
- Fai arrivare i tubi delle piante più in alto del livello della vasca, altrimenti si svuota per effetto sifone.
