# Hardware

## Schema dell'impianto

```
Rubinetto ─► Filtro a Y ─► Elettrovalvola 12V NC ─┐
                                                  ▼
 Taniche concentrati ─► Pompe peristaltiche ─► ┌────────────────────┐ ◄─ 2 galleggianti di massimo
 (3, fino a 5)                                 │  Vasca 5 L         │
                                               │  su cella di carico│ ◄─ pompa di ricircolo
                                               └──┬──────┬──────┬───┘
                                                  ▼      ▼      ▼
                                              Pompa 1 Pompa 2 Pompa 3
                                                  ▼      ▼      ▼
                                              Pianta 1 Pianta 2 Pianta 3
```

## Lista della spesa

Le ricerche sono in inglese perché su AliExpress danno risultati migliori.

### Elettronica

| Componente | Qtà | Variante da scegliere | Ricerca AliExpress |
|---|---|---|---|
| ESP32 DevKit V1 (ESP-WROOM-32) | 1 | 30 pin, Type-C, CH340 o CP2102 | `ESP32 DevKit V1 30pin type-c` |
| Scheda di espansione con morsetti a vite | 1 | 30 pin | `ESP32 30pin expansion board screw terminal` |
| Scheda relè 8 canali **5V** con jumper H/L | 1 | 5V 8 canali | `8 channel relay module 5V high low level trigger optocoupler` |
| Modulo RTC DS3231 (AT24C32) | 1 | | `DS3231 AT24C32 RTC module` |
| Cella di carico **10 kg** + modulo HX711 | 1 | 10 kg | `load cell 10kg HX711` |
| Alimentatore 12V 5A | 1 | 12V 5A, spina EU | `12V 5A power adapter` |
| Step-down LM2596 (12V → 5V) | 1 | regolabile, va bene anche la confezione da 5 | `LM2596 DC-DC step down module` |
| Portafusibile in linea con fusibile | 1 | 5A | `inline blade fuse holder ATC` |
| Jack DC femmina 5,5×2,1 | 1 | facoltativo: in alternativa si taglia la spina dell'alimentatore | `DC power jack female 5.5x2.1` |
| Galleggiante a contatto | **2** | | `float switch side mount` |
| Cavo USB dati | 1 | adatto alle prese del PC | `USB C data cable` |

### Cavi

| Uso | Tipo | Qtà | Ricerca AliExpress |
|---|---|---|---|
| Segnali dentro la scatola | piattina rosso/nero in silicone, 22 AWG | 5 m | `2 pin red black silicone wire 22AWG` |
| Linea 12V dentro la scatola | piattina rosso/nero in silicone, 18 AWG | 2 m | `2 pin red black silicone wire 18AWG` |
| Dalla scatola a pompe, valvola e galleggianti | 2 poli in PVC, 20 AWG (meglio tondo con guaina) | 20 m | `2 core round cable 20AWG PVC sheathed` |
| Cella di carico → HX711 | schermato a 4 poli, 24 AWG | 1-2 m, un pezzo unico | `4 core shielded cable 24AWG` |
| Ponticelli | Dupont F-F | 1 confezione | `dupont jumper wire female female` |

### Pompe e idraulica

| Componente | Qtà | Variante | Ricerca AliExpress |
|---|---|---|---|
| Pompe peristaltiche 12V | 3 (fino a 5) | 12V | `12V peristaltic pump dosing` |
| Pompe a membrana 385, 12V | 4 | 12V (3 piante + 1 ricircolo) | `385 12V diaphragm pump` |
| Elettrovalvola **normalmente chiusa** | 1 | DC 12V, ½", NC | `12V solenoid valve 1/2 normally closed` |
| Filtro a Y in ottone | 1 | ½" femmina | `1/2 inch brass Y strainer` |
| Tubo in silicone alimentare | 10 m | misura delle peristaltiche (di solito 2×4 o 3×5) | `food grade silicone tube` |
| Tubo in silicone alimentare | 10 m | 8×10 o 8×12, per le pompe 385 | `food grade silicone tube` |
| Riduzioni portagomma in ottone | 3 | 8 → 4 mm | `brass barb reducer 8mm to 4mm` |
| Tubo per microirrigazione | 1 rotolo | 4/7 mm | `4/7mm drip irrigation hose` |
| Gocciolatori regolabili | 1 confezione | 1/4" | `adjustable dripper 1/4` |

### Scatola

| Componente | Qtà | Note | Ricerca AliExpress |
|---|---|---|---|
| Scatola stagna IP65 o IP67 | 1 | almeno 200×150×100 mm, meglio con coperchio trasparente | `IP65 waterproof junction box 200x150x100` |
| Pressacavi in nylon | 10 + 5 | PG7 (cavi da 3-6 mm) e PG9 (cavi da 4-8 mm) | `PG7 PG9 cable gland nylon` |

### Da comprare nei negozi

| Dove | Cosa |
|---|---|
| Supermercato | Batteria CR2032 per l'RTC |
| Materiale elettrico o ferramenta | 5 morsetti a leva Wago 221 a 5 posti |
| Ferramenta o negozio di irrigazione | Raccordo dal rubinetto al filtro da ½" (portare una foto del rubinetto), nastro in teflon |
| Casalinghi | Vasca da 5 L **opaca** per alimenti, 3-5 taniche o bottiglie opache con tappo |
| Facoltativo | Diodi 1N4007, uno per ogni pompa e per la valvola (vedi [Sicurezze](#sicurezze-da-rispettare-nel-cablaggio)) |

Serve anche un **multimetro**, per regolare lo step-down e controllare i collegamenti.

## Alimentazione

```
Alimentatore 12V ─► Fusibile 5A ─► Wago +12V ─┬─► COM dei relè (pompe e valvola)
                                              └─► LM2596 IN+ ─► OUT 5,0V ─┬─► ESP32 VIN (5V)
                                                                          └─► Scheda relè DC+
GND alimentatore ─► Wago GND ─► LM2596 IN−/OUT−, ESP32 GND, scheda relè DC−, negativo di pompe e valvola
```

- **Prima di collegare l'ESP32, regola l'LM2596.** Alimentalo a 12V, metti il multimetro sull'uscita e gira la vite del trimmer finché leggi 5,0V. Spesso arriva impostato a una tensione molto più alta, che brucerebbe l'ESP32.
- Tutti i GND (alimentatore, LM2596, ESP32, scheda relè) vanno collegati insieme.
- **RTC e HX711 si alimentano a 3,3V** dal pin 3V3 dell'ESP32, non a 5V. L'HX711 alimentato a 5V manderebbe 5V sul GPIO 34, e l'ESP32 non li sopporta. Il DS3231 alimentato a 5V ricarica la CR2032, che non è ricaricabile.

## Collegamenti all'ESP32

| Funzione | GPIO | Ingresso relè |
|---|---|---|
| Elettrovalvola | 23 | IN1 (passando dal secondo galleggiante, vedi sotto) |
| Pompa di ricircolo | 19 | IN2 |
| Pompa pianta 1 / 2 / 3 | 18 / 17 / 16 | IN3 / IN4 / IN5 |
| Fertilizzante 1 / 2 / 3 | 13 / 4 / 27 | IN6 / IN7 / IN8 |
| Fertilizzante 4 / 5 | 26 / 25 | servono altri 2 canali relè (es. una scheda a 2 canali dello stesso tipo) |
| HX711 DT (DOUT) | 34 | |
| HX711 SCK | 32 | |
| Galleggiante di massimo 1 | 33 | l'altro capo va a GND |
| RTC DS3231 SDA / SCL | 21 / 22 | |
| LED di stato | 2 | LED blu sulla scheda |

I pin sono definiti in [include/config.h](../../include/config.h).

### Scheda relè

- **Jumper H/L su H** per tutti i canali: il relè si attiva quando il pin dell'ESP32 è HIGH. Il firmware è impostato così (`RELAY_ACTIVE_LOW = false` in `config.h`). Con il trigger alto, durante l'avvio e i reset dell'ESP32 i relè restano spenti.
- La scheda deve essere **a 5V**. Nelle versioni a 12V il trigger alto richiede almeno 5V e i 3,3V dell'ESP32 non bastano.
- Pompe e valvola si collegano tra **COM** e **NO**: +12V su COM, NO al + del carico, il − del carico a GND. Senza comando, il contatto resta aperto e il carico spento.

### Cella di carico

Colori più comuni dei fili: rosso **E+**, nero **E−**, bianco **A−**, verde **A+**. Se il peso diminuisce quando aggiungi acqua, inverti A+ e A−, oppure rifai la calibrazione: il firmware accetta anche un fattore negativo. Per la prolunga verso l'HX711 usa il cavo schermato a 4 poli e collega la calza a GND solo dal lato dell'HX711.

## Sicurezze da rispettare nel cablaggio

1. **Secondo galleggiante in serie alla valvola.** Il primo galleggiante va all'ESP32 (GPIO 33). Il secondo si collega in serie al filo di comando della valvola: GPIO 23 → galleggiante 2 → IN1 della scheda relè. Quando la vasca è piena il contatto si apre, l'ingresso IN1 resta scollegato e il relè si spegne, anche se l'ESP32 è guasto e tiene il pin acceso. Su quel filo passano pochi milliampere, quindi il galleggiante non si rovina.
2. **Elettrovalvola normalmente chiusa (NC).** Se manca la corrente, l'acqua del rubinetto non passa.
3. **Galleggianti cablati "a sicurezza positiva".** Il contatto è chiuso a livello normale e si apre quando la vasca è piena. Se un filo si stacca, il sistema lo legge come vasca piena e non riempie. Molti galleggianti si possono girare per scegliere se il contatto è normalmente aperto o chiuso: verifica con il multimetro.
4. **Fusibile da 5A** sul positivo, subito dopo l'alimentatore.
5. **Diodi (consigliati).** Un diodo 1N4007 in parallelo a ogni pompa e alla valvola, con la striscia verso il +, riduce i disturbi che possono far riavviare l'ESP32 o falsare la bilancia.
6. **Umidità.** Tieni tutta l'elettronica nella scatola stagna, con tutti i pressacavi **sul lato sotto**. Fai fare al cavo una curva verso il basso prima del pressacavo, così l'acqua gocciola via. Monta la scatola più in alto della vasca e lontana dagli spruzzi.

## Montaggio della vasca

- La cella di carico a barra si monta **a Z** tra due piastre (legno o plexiglass): una vite da un lato verso la base, l'altra vite dal lato opposto verso il piano dove poggia la vasca. La freccia sulla cella indica la direzione del carico.
- La vasca deve poggiare **solo** sulla cella di carico, senza tubi rigidi che la tirano o la sostengono: usa tubi morbidi e lenti.
- I galleggianti si montano in alto sulla parete della vasca. I loro cavi devono essere morbidi e lenti, come i tubi.
- Le pompe delle piante e del ricircolo si fissano *fuori* dalla vasca e aspirano dal fondo con un tubo, così il loro peso non pesa sulla bilancia.
- I tubi dei fertilizzanti entrano dall'alto e non devono toccare il liquido, per evitare che sifonino.
- Il ricircolo aspira dal fondo e rimanda nella vasca stessa: basta per mescolare.
- Dalle pompe 385 esce tubo in silicone da 8 mm; con una riduzione 8 → 4 mm si passa al tubo 4/7 che va alle piante.
- Fai arrivare i tubi delle piante più in alto del livello della vasca, altrimenti si svuota per effetto sifone.
