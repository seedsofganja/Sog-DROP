#pragma once
#include <Arduino.h>

// ---------------- Rete WiFi locale (niente internet) ----------------
// L'ESP32 crea questa rete: collegati col telefono e apri http://192.168.4.1
// Le credenziali vere vanno in include/secrets.h (escluso da git): copia
// include/secrets.example.h e cambia la password. Senza quel file valgono questi default.
#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef WIFI_SSID
#define WIFI_SSID     "SOG-Drop"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "cambiami123"   // almeno 8 caratteri: cambiala!
#endif

// ---------------- Fuso orario ----------------
// L'RTC conserva l'ora UTC; il firmware applica fuso e ora legale europea.
constexpr int32_t TZ_OFFSET_MIN = 60;   // Italia: UTC+1
constexpr bool    USE_EU_DST    = true;

// ---------------- Dimensioni impianto ----------------
constexpr uint8_t NUM_PLANTS = 3;
constexpr uint8_t MAX_FERT   = 5;
constexpr uint8_t MAX_WEEKS  = 30;

// ---------------- Pin (ESP32 DevKit V1) ----------------
constexpr uint8_t PIN_VALVE = 23;                        // elettrovalvola rubinetto
constexpr uint8_t PIN_MIX   = 19;                        // pompa di ricircolo
constexpr uint8_t PIN_PLANT[NUM_PLANTS] = {18, 17, 16};  // pompe piante
constexpr uint8_t PIN_FERT[MAX_FERT]    = {13, 4, 27, 26, 25};  // pompe peristaltiche
constexpr uint8_t PIN_HX711_DOUT = 34;
constexpr uint8_t PIN_HX711_SCK  = 32;
// Galleggiante di massimo tra il pin e GND, contatto CHIUSO a livello normale.
// Livello alto o filo staccato => pin HIGH => riempimento bloccato.
constexpr uint8_t PIN_FLOAT_MAX = 33;
constexpr uint8_t PIN_LED       = 2;
// RTC DS3231 su I2C: SDA = 21, SCL = 22

// Scheda relè 5V con jumper H/L impostati su H: il relè si attiva con il pin a HIGH.
// Così, durante avvio e reset (pin non ancora pilotati), i relè restano spenti.
// Con una scheda a trigger basso, impostare true.
constexpr bool RELAY_ACTIVE_LOW = false;

// ---------------- Vasca ----------------
constexpr float TANK_MAX_L     = 4.0f;  // volume massimo preparato in un lotto
constexpr float TANK_RESERVE_L = 0.3f;  // fondo sempre presente: le pompe non pescano aria

// ---------------- Sicurezze ----------------
constexpr uint32_t MAX_ON_VALVE_MS = 10UL * 60 * 1000;
constexpr uint32_t MAX_ON_FERT_MS  = 3UL * 60 * 1000;
constexpr uint32_t MAX_ON_PLANT_MS = 10UL * 60 * 1000;
constexpr uint32_t MAX_ON_MIX_MS   = 3UL * 60 * 1000;
constexpr uint32_t NO_PROGRESS_MS  = 30000;   // peso fermo per questo tempo => guasto
constexpr float    PROGRESS_MIN_G  = 20.0f;
constexpr uint32_t SETTLE_MS       = 2500;    // attesa per stabilizzare il peso
constexpr uint32_t FILL_RETRY_MS   = 30UL * 60 * 1000;
constexpr uint8_t  FILL_MAX_RETRIES = 6;
constexpr uint32_t WDT_TIMEOUT_S   = 30;
constexpr uint32_t LOG_MAX_BYTES   = 64UL * 1024;
