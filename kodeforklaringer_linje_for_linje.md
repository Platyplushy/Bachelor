# Linje-for-linje forklaringer av kodefiler

Sist oppdatert: 2026-05-11

Dette dokumentet samler linje-for-linje forklaringer av prosjektets egne kodefiler. Hensikten er aa kunne bruke forklaringene som grunnlag for pseudokode, blokkdiagram og flytskjema.

## app_default_task

`Core/Src/app_default_task.c` inneholder hovedloekken for FreeRTOS sin default task. Den samler periodiske bakgrunnsoppgaver som terminal, joystick, temperatursensor, UART-status, temperaturlogg, motorsikkerhet og LCD.

## Core/Inc/app_default_task.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#ifndef INC_APP_DEFAULT_TASK_H_` | Starter include guard, slik at headeren ikke inkluderes flere ganger i samme kompilering. |
| 2 | `#define INC_APP_DEFAULT_TASK_H_` | Definerer guard-navnet som brukes av linje 1. |
| 3 | Tom linje | Brukes kun for lesbarhet. |
| 4 | `void AppDefaultTask_Run(void *argument);` | Deklarerer funksjonen som kjoerer default task-logikken. `app_freertos.c` bruker denne deklarasjonen for aa kalle funksjonen. |
| 5 | Tom linje | Brukes kun for lesbarhet. |
| 6 | `#endif /* INC_APP_DEFAULT_TASK_H_ */` | Avslutter include guard. |

## Core/Src/app_default_task.c

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#include "app_default_task.h"` | Inkluderer egen header, slik at `.c`-filen bruker samme offentlige funksjonsdeklarasjon som resten av prosjektet. |
| 2 | Tom linje | Brukes kun for lesbarhet. |
| 3 | `#include "cmsis_os.h"` | Inkluderer CMSIS-OS/FreeRTOS API. Brukes til `osKernelGetTickCount()` og `osThreadYield()`. |
| 4 | `#include "hall_debug.h"` | Inkluderer API for hall-debug. Koden brukes bare hvis hall-debug aktiveres med makroen paa linje 13. |
| 5 | `#include "hall_probe.h"` | Inkluderer API for hall-probe. Brukes sammen med hall-debug. |
| 6 | `#include "joystick.h"` | Inkluderer joystick-modulen, slik at tasken kan lese og behandle joystick-input. |
| 7 | `#include "lcd_status.h"` | Inkluderer LCD-statusmodulen, som bygger og skriver tekst til displayet. |
| 8 | `#include "motor_commutation.h"` | Inkluderer motor-commutation-modulen, brukt for motorstatus og hastighet. |
| 9 | `#include "motor_temperature_safety.h"` | Inkluderer temperatursikkerheten, som stopper motorene ved for hoey temperatur. |
| 10 | `#include "myprint.h"` | Inkluderer UART/print-modulen for terminalmeldinger. |
| 11 | `#include "temperature_sensor.h"` | Inkluderer temperatursensor-modulen for temperatur- og ADC-data. |
| 12 | Tom linje | Brukes kun for lesbarhet. |
| 13 | `#define APP_DEFAULT_TASK_ENABLE_RAW_HALL_DEBUG 0U` | Setter raa hall-debug av. Hvis verdien endres til `1U`, kompileres hall-debug-kallene inn. |
| 14 | `#define APP_DEFAULT_TASK_LCD_UPDATE_INTERVAL_MS 500U` | Bestemmer at LCD/status oppdateres hvert 500 ms. |
| 15 | `#define APP_DEFAULT_TASK_TEMPERATURE_PRINT_INTERVAL_MS 500U` | Bestemmer at temperaturstatus skrives til UART hvert 500 ms. |
| 16 | Tom linje | Brukes kun for lesbarhet. |
| 17 | `static void AppDefaultTask_PrintTemperatureStatus(float motor1_temp_c, float motor2_temp_c)` | Starter en privat hjelpefunksjon for aa skrive temperaturstatus til UART. `static` betyr at funksjonen bare kan brukes i denne filen. |
| 18 | `{` | Aapner funksjonsblokken. |
| 19 | `const uint16_t motor1_raw_adc = TemperatureSensor_GetMotor1RawAdc();` | Leser raa ADC-verdi for motor 1 fra temperatursensor-modulen. |
| 20 | `const uint16_t motor2_raw_adc = TemperatureSensor_GetMotor2RawAdc();` | Leser raa ADC-verdi for motor 2. |
| 21 | `const int motor1_valid = (motor1_temp_c > -200.0f) ? 1 : 0;` | Sjekker om motor 1-temperaturen er gyldig. Verdier under eller lik `-200.0f` tolkes som ugyldig maaling. |
| 22 | `const int motor2_valid = (motor2_temp_c > -200.0f) ? 1 : 0;` | Sjekker om motor 2-temperaturen er gyldig paa samme maate. |
| 23 | `const int motor1_temp_x10 = motor1_valid ? (int)((motor1_temp_c * 10.0f) + 0.5f) : 0;` | Konverterer motor 1-temperaturen til tidels grader. Eksempel: `23.4 C` blir `234`. Hvis temperaturen er ugyldig, brukes `0`. |
| 24 | `const int motor2_temp_x10 = motor2_valid ? (int)((motor2_temp_c * 10.0f) + 0.5f) : 0;` | Gjoer samme konvertering for motor 2. |
| 25 | Tom linje | Brukes kun for lesbarhet. |
| 26 | `if ((motor1_valid != 0) && (motor2_valid != 0)) {` | Hvis begge temperaturene er gyldige, skrives normal temperaturstatus. |
| 27 | `MyPrint_Printf("TEMP M1: adc=%u temp=%u.%u C | M2: adc=%u temp=%u.%u C\r\n",` | Starter en formatert UART-melding med ADC og temperatur for begge motorer. |
| 28 | `(unsigned)motor1_raw_adc,` | Sender ADC-verdien for motor 1 inn i formatstrengen. Cast til `unsigned` matcher `%u`. |
| 29 | `(unsigned)(motor1_temp_x10 / 10),` | Sender heltallsdelen av motor 1-temperaturen. |
| 30 | `(unsigned)(motor1_temp_x10 % 10),` | Sender desimaldelen av motor 1-temperaturen. |
| 31 | `(unsigned)motor2_raw_adc,` | Sender ADC-verdien for motor 2. |
| 32 | `(unsigned)(motor2_temp_x10 / 10),` | Sender heltallsdelen av motor 2-temperaturen. |
| 33 | `(unsigned)(motor2_temp_x10 % 10));` | Sender desimaldelen av motor 2-temperaturen og avslutter print-kallet. |
| 34 | `} else {` | Hvis minst en temperatur er ugyldig, brukes alternativ print. |
| 35 | `MyPrint_Printf("TEMP M1: adc=%u temp=invalid | M2: adc=%u temp=invalid\r\n",` | Starter UART-melding som viser at temperaturverdiene er ugyldige. |
| 36 | `(unsigned)motor1_raw_adc,` | Sender ADC-verdien for motor 1. |
| 37 | `(unsigned)motor2_raw_adc);` | Sender ADC-verdien for motor 2 og avslutter print-kallet. |
| 38 | `}` | Avslutter `else`-blokken. |
| 39 | `}` | Avslutter hjelpefunksjonen. |
| 40 | Tom linje | Brukes kun for lesbarhet. |
| 41 | `void AppDefaultTask_Run(void *argument)` | Starter hovedfunksjonen for default task. Denne kalles fra `StartDefaultTask()` i `app_freertos.c`. |
| 42 | `{` | Aapner funksjonsblokken. |
| 43 | `uint32_t last_status_tick = osKernelGetTickCount();` | Lagrer naavaerende OS-tick for siste UART alive-melding. |
| 44 | `uint32_t last_lcd_speed_tick = osKernelGetTickCount();` | Lagrer naavaerende OS-tick for siste LCD/status-oppdatering. |
| 45 | `uint32_t last_temperature_print_tick = osKernelGetTickCount();` | Lagrer naavaerende OS-tick for siste temperaturprint. |
| 46 | Tom linje | Brukes kun for lesbarhet. |
| 47 | `(void)argument;` | Marker argumentet som bevisst ubrukt, slik at kompilatoren ikke gir unused parameter-warning. |
| 48 | Tom linje | Brukes kun for lesbarhet. |
| 49 | `for (;;) {` | Starter en evig loekke. FreeRTOS tasks skal normalt kjoere kontinuerlig og ikke returnere. |
| 50 | `MyPrint_ProcessTerminal();` | Behandler terminal/kommandoer over UART. |
| 51 | `Joystick_Process();` | Behandler joystick-input. |
| 52 | `TemperatureSensor_Process();` | Oppdaterer temperatursensor-modulen, for eksempel nye ADC-data, filtrering eller temperaturberegning. |
| 53 | Tom linje | Brukes kun for lesbarhet. |
| 54 | `#if APP_DEFAULT_TASK_ENABLE_RAW_HALL_DEBUG` | Starter betinget kompilering av hall-debug. Koden under finnes bare i bygget hvis makroen paa linje 13 er aktiv. |
| 55 | `HallDebug_Process();` | Kjoerer hall-debug-prosessering hvis debug er aktivert. |
| 56 | `HallProbe_Process();` | Kjoerer hall-probe-prosessering hvis debug er aktivert. |
| 57 | `#endif` | Avslutter betinget kompilering. |
| 58 | Tom linje | Brukes kun for lesbarhet. |
| 59 | `if ((osKernelGetTickCount() - last_status_tick) >= 5000U) {` | Sjekker om det har gaatt minst 5000 ms siden forrige UART alive-melding. Tick-differanse brukes slik at koden ogsaa fungerer naar tick-telleren ruller over. |
| 60 | `last_status_tick = osKernelGetTickCount();` | Oppdaterer tidspunktet for siste alive-melding. |
| 61 | `MyPrint_Print("UART alive: RX=PA3 TX=PA2\r\n");` | Skriver et enkelt livstegn til UART-terminalen. |
| 62 | `}` | Avslutter 5-sekundersblokken. |
| 63 | Tom linje | Brukes kun for lesbarhet. |
| 64 | `if ((osKernelGetTickCount() - last_temperature_print_tick) >=` | Starter sjekk av om nok tid har gaatt siden forrige temperaturprint. |
| 65 | `APP_DEFAULT_TASK_TEMPERATURE_PRINT_INTERVAL_MS) {` | Fullfoerer sjekken med intervallet fra linje 15, altsaa 500 ms. |
| 66 | `const float motor1_temp_c = TemperatureSensor_GetMotor1TemperatureC();` | Leser beregnet temperatur for motor 1 i grader Celsius. |
| 67 | `const float motor2_temp_c = TemperatureSensor_GetMotor2TemperatureC();` | Leser beregnet temperatur for motor 2 i grader Celsius. |
| 68 | Tom linje | Brukes kun for lesbarhet. |
| 69 | `last_temperature_print_tick = osKernelGetTickCount();` | Oppdaterer tidspunktet for siste temperaturprint. |
| 70 | `AppDefaultTask_PrintTemperatureStatus(motor1_temp_c, motor2_temp_c);` | Skriver temperaturstatus til UART med hjelpefunksjonen. |
| 71 | `}` | Avslutter temperaturprint-blokken. |
| 72 | Tom linje | Brukes kun for lesbarhet. |
| 73 | `if ((osKernelGetTickCount() - last_lcd_speed_tick) >=` | Starter sjekk av om nok tid har gaatt siden forrige LCD/status-oppdatering. |
| 74 | `APP_DEFAULT_TASK_LCD_UPDATE_INTERVAL_MS) {` | Fullfoerer sjekken med intervallet fra linje 14, altsaa 500 ms. |
| 75 | `const float motor1_temp_c = TemperatureSensor_GetMotor1TemperatureC();` | Leser motor 1-temperatur paa nytt for LCD og sikkerhetslogikk. |
| 76 | `const float motor2_temp_c = TemperatureSensor_GetMotor2TemperatureC();` | Leser motor 2-temperatur paa nytt for LCD og sikkerhetslogikk. |
| 77 | `const int vehicle_speed_x10 = (int)(MotorCommutation_GetVehicleSpeedKmh() * 10.0f);` | Leser kjoeretoeyhastighet i km/t og ganger med 10, slik at en desimal kan beholdes som heltall. |
| 78 | Tom linje | Brukes kun for lesbarhet. |
| 79 | `last_lcd_speed_tick = osKernelGetTickCount();` | Oppdaterer tidspunktet for siste LCD/status-oppdatering. |
| 80 | `MotorTemperatureSafety_Process(motor1_temp_c, motor2_temp_c);` | Kjoerer temperatursikkerheten. Hvis en motor er for varm, stopper denne modulen motorene. |
| 81 | `LcdStatus_Update(vehicle_speed_x10,` | Starter oppdatering av LCD-status og sender inn hastigheten. |
| 82 | `MotorCommutation_IsEnabled(),` | Sender inn om motorene er aktivert. Dette brukes til `ON` eller `OFF` helt til hoeyre paa displayets linje 1. |
| 83 | `motor1_temp_c,` | Sender inn motor 1-temperatur til LCD-modulen. |
| 84 | `motor2_temp_c,` | Sender inn motor 2-temperatur til LCD-modulen. |
| 85 | `MotorTemperatureSafety_GetFaults());` | Sender inn aktiv temperaturfeilstatus, slik at displayet kan vise `TEMP FAULT M1`, `TEMP FAULT M2` eller `TEMP FAULT M1+M2`. |
| 86 | `}` | Avslutter LCD/status-blokken. |
| 87 | Tom linje | Brukes kun for lesbarhet. |
| 88 | `osThreadYield();` | Gir CPU-tid frivillig tilbake til FreeRTOS, slik at andre tasks kan kjoere. |
| 89 | `}` | Avslutter en runde i evigloekken. Programmet hopper deretter tilbake til linje 49. |
| 90 | `}` | Avslutter `AppDefaultTask_Run()`. I praksis naas ikke denne linjen fordi loekken er evig. |

## Forenklet flyt

```text
Start AppDefaultTask_Run
|
|-- Lagre starttidspunkt for UART alive, LCD og temperaturprint
|
`-- Evig loop
    |
    |-- Behandle terminal
    |-- Behandle joystick
    |-- Oppdater temperatursensor
    |
    |-- Hvis hall-debug er aktivert:
    |   |-- Behandle hall-debug
    |   `-- Behandle hall-probe
    |
    |-- Hvert 5 sekund:
    |   `-- Skriv UART alive
    |
    |-- Hvert 500 ms:
    |   `-- Skriv temperaturstatus til UART
    |
    |-- Hvert 500 ms:
    |   |-- Les temperaturer
    |   |-- Les hastighet
    |   |-- Kjoer overtemperatur-sikkerhet
    |   `-- Oppdater LCD
    |
    `-- Yield til FreeRTOS
```

## hall_debug

`Core/Src/hall_debug.c` brukes til aa logge hall-sensorendringer. Selve avbruddet skal gjoere minst mulig arbeid, saa interrupt-callbacken setter bare flagg og lagrer et snapshot. Utskrift skjer senere fra `HallDebug_Process()`, som kalles fra default task naar hall-debug er aktivert.

## Core/Inc/hall_debug.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#ifndef INC_HALL_DEBUG_H_` | Starter include guard, slik at headeren ikke inkluderes flere ganger i samme kompilering. |
| 2 | `#define INC_HALL_DEBUG_H_` | Definerer guard-navnet som brukes av linje 1. |
| 3 | Tom linje | Brukes kun for lesbarhet. |
| 4 | `#include "main.h"` | Inkluderer STM32/HAL-definisjoner fra prosjektet. Trengs blant annet for `uint16_t`, GPIO-typer og pin-definisjoner. |
| 5 | Tom linje | Brukes kun for lesbarhet. |
| 6 | `void HallDebug_Init(void);` | Deklarerer init-funksjonen for hall-debug. |
| 7 | `void HallDebug_Process(void);` | Deklarerer prosessfunksjonen som skriver ut registrerte hall-hendelser. |
| 8 | `void HallDebug_OnExti(uint16_t gpio_pin);` | Deklarerer funksjonen som kalles naar et GPIO EXTI-avbrudd skjer. |
| 9 | Tom linje | Brukes kun for lesbarhet. |
| 10 | `#endif /* INC_HALL_DEBUG_H_ */` | Avslutter include guard. |

## Core/Src/hall_debug.c

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#include "hall_debug.h"` | Inkluderer egen header, slik at `.c`-filen matcher de offentlige funksjonsdeklarasjonene. |
| 2 | Tom linje | Brukes kun for lesbarhet. |
| 3 | `#include "hall_state_filter.h"` | Inkluderer hall-filtermodulen. Den faar samme EXTI-callback som debug-koden. |
| 4 | `#include "myprint.h"` | Inkluderer UART/print-modulen som brukes til debug-utskrift. |
| 5 | Tom linje | Brukes kun for lesbarhet. |
| 6 | `#define HALL_DEBUG_ENABLE_MOTOR2 0U` | Slukker debug-utskrift for motor 2. Settes denne til `1U`, kan motor 2-hendelser ogsaa logges. |
| 7 | Tom linje | Brukes kun for lesbarhet. |
| 8 | `#define HALL_EVENT_H1_A (1U << 0)` | Definerer bitflagget for hall-sensor A paa motor 1. |
| 9 | `#define HALL_EVENT_H1_B (1U << 1)` | Definerer bitflagget for hall-sensor B paa motor 1. |
| 10 | `#define HALL_EVENT_H1_C (1U << 2)` | Definerer bitflagget for hall-sensor C paa motor 1. |
| 11 | `#define HALL_EVENT_H2_A (1U << 3)` | Definerer bitflagget for hall-sensor A paa motor 2. |
| 12 | `#define HALL_EVENT_H2_B (1U << 4)` | Definerer bitflagget for hall-sensor B paa motor 2. |
| 13 | `#define HALL_EVENT_H2_C (1U << 5)` | Definerer bitflagget for hall-sensor C paa motor 2. |
| 14 | Tom linje | Brukes kun for lesbarhet. |
| 15 | `static volatile uint32_t s_hallEventFlags = 0U;` | Lagrer hvilke hall-hendelser som har skjedd. `static` gjoer variabelen privat for filen, og `volatile` er viktig fordi den endres baade fra interrupt og vanlig task-kode. |
| 16 | `static volatile uint8_t s_hall1SnapshotA = 0U;` | Lagrer siste avleste verdi for motor 1 hall A ved interrupt. |
| 17 | `static volatile uint8_t s_hall1SnapshotB = 0U;` | Lagrer siste avleste verdi for motor 1 hall B ved interrupt. |
| 18 | `static volatile uint8_t s_hall1SnapshotC = 0U;` | Lagrer siste avleste verdi for motor 1 hall C ved interrupt. |
| 19 | `static volatile uint8_t s_hall2SnapshotA = 0U;` | Lagrer siste avleste verdi for motor 2 hall A ved interrupt. |
| 20 | `static volatile uint8_t s_hall2SnapshotB = 0U;` | Lagrer siste avleste verdi for motor 2 hall B ved interrupt. |
| 21 | `static volatile uint8_t s_hall2SnapshotC = 0U;` | Lagrer siste avleste verdi for motor 2 hall C ved interrupt. |
| 22 | Tom linje | Brukes kun for lesbarhet. |
| 23 | `static uint8_t HallDebug_ReadPin(GPIO_TypeDef *port, uint16_t pin)` | Starter en privat hjelpefunksjon for aa lese en GPIO-pin som `0` eller `1`. |
| 24 | `{` | Aapner funksjonsblokken. |
| 25 | `return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? 1U : 0U;` | Leser pinnen med HAL. Hvis pinnen er hoey returneres `1`, ellers returneres `0`. |
| 26 | `}` | Avslutter hjelpefunksjonen. |
| 27 | Tom linje | Brukes kun for lesbarhet. |
| 28 | `static void HallDebug_CaptureSnapshots(void)` | Starter en privat hjelpefunksjon som lagrer et samlet snapshot av alle hall-inngangene. |
| 29 | `{` | Aapner funksjonsblokken. |
| 30 | `s_hall1SnapshotA = HallDebug_ReadPin(GPIOC, GPIO_PIN_9);` | Leser motor 1 hall A fra `PC9` og lagrer verdien. |
| 31 | `s_hall1SnapshotB = HallDebug_ReadPin(GPIOC, GPIO_PIN_1);` | Leser motor 1 hall B fra `PC1` og lagrer verdien. |
| 32 | `s_hall1SnapshotC = HallDebug_ReadPin(GPIOC, GPIO_PIN_7);` | Leser motor 1 hall C fra `PC7` og lagrer verdien. |
| 33 | `s_hall2SnapshotA = HallDebug_ReadPin(GPIOC, GPIO_PIN_2);` | Leser motor 2 hall A fra `PC2` og lagrer verdien. |
| 34 | `s_hall2SnapshotB = HallDebug_ReadPin(GPIOC, GPIO_PIN_3);` | Leser motor 2 hall B fra `PC3` og lagrer verdien. |
| 35 | `s_hall2SnapshotC = HallDebug_ReadPin(GPIOC, GPIO_PIN_8);` | Leser motor 2 hall C fra `PC8` og lagrer verdien. |
| 36 | `}` | Avslutter snapshot-funksjonen. |
| 37 | Tom linje | Brukes kun for lesbarhet. |
| 38 | `static void HallDebug_PrintHall1State(const char *source)` | Starter en privat hjelpefunksjon som skriver ut motor 1 hall-status. `source` forteller hvilken pin som trigget interrupt. |
| 39 | `{` | Aapner funksjonsblokken. |
| 40 | `MyPrint_Printf("%s IRQ: H1_A=%u H1_B=%u H1_C=%u\r\n",` | Starter formatert UART-utskrift av motor 1 hall-status. |
| 41 | `source,` | Sender inn tekst om hvilken EXTI-kilde som trigget utskriften. |
| 42 | `s_hall1SnapshotA,` | Sender snapshot-verdien for motor 1 hall A. |
| 43 | `s_hall1SnapshotB,` | Sender snapshot-verdien for motor 1 hall B. |
| 44 | `s_hall1SnapshotC);` | Sender snapshot-verdien for motor 1 hall C og avslutter print-kallet. |
| 45 | `}` | Avslutter motor 1 print-funksjonen. |
| 46 | Tom linje | Brukes kun for lesbarhet. |
| 47 | `static void HallDebug_PrintHall2State(const char *source)` | Starter en privat hjelpefunksjon som skriver ut motor 2 hall-status. |
| 48 | `{` | Aapner funksjonsblokken. |
| 49 | `MyPrint_Printf("%s IRQ: H2_A=%u H2_B=%u H2_C=%u\r\n",` | Starter formatert UART-utskrift av motor 2 hall-status. |
| 50 | `source,` | Sender inn tekst om hvilken EXTI-kilde som trigget utskriften. |
| 51 | `HallDebug_ReadPin(GPIOC, GPIO_PIN_2),` | Leser motor 2 hall A direkte fra `PC2` for utskrift. |
| 52 | `HallDebug_ReadPin(GPIOC, GPIO_PIN_3),` | Leser motor 2 hall B direkte fra `PC3` for utskrift. |
| 53 | `HallDebug_ReadPin(GPIOC, GPIO_PIN_8));` | Leser motor 2 hall C direkte fra `PC8` og avslutter print-kallet. |
| 54 | `}` | Avslutter motor 2 print-funksjonen. |
| 55 | Tom linje | Brukes kun for lesbarhet. |
| 56 | `void HallDebug_Init(void)` | Starter init-funksjonen for hall-debug. |
| 57 | `{` | Aapner funksjonsblokken. |
| 58 | `MyPrint_Print("Hall debug ready: H1=PC9/PC1/PC7\r\n");` | Skriver en oppstartsmelding til UART som viser hvilke pinner motor 1 hall-debug bruker. |
| 59 | `}` | Avslutter init-funksjonen. |
| 60 | Tom linje | Brukes kun for lesbarhet. |
| 61 | `void HallDebug_Process(void)` | Starter prosessfunksjonen som skal kalles fra task-kontekst, ikke direkte fra interrupt. |
| 62 | `{` | Aapner funksjonsblokken. |
| 63 | `uint32_t event_flags;` | Lager en lokal variabel for aa kopiere og behandle hall-hendelsesflaggene. |
| 64 | Tom linje | Brukes kun for lesbarhet. |
| 65 | `__disable_irq();` | Deaktiverer interrupts kortvarig for aa lese og nullstille flaggene atomisk. |
| 66 | `event_flags = s_hallEventFlags;` | Kopierer globale hendelsesflagg til lokal variabel. |
| 67 | `s_hallEventFlags = 0U;` | Nullstiller globale hendelsesflagg slik at samme hendelse ikke skrives ut flere ganger. |
| 68 | `__enable_irq();` | Aktiverer interrupts igjen. Den kritiske seksjonen er ferdig. |
| 69 | Tom linje | Brukes kun for lesbarhet. |
| 70 | `if (event_flags & HALL_EVENT_H1_A) {` | Sjekker om motor 1 hall A har trigget siden forrige prosessering. |
| 71 | `HallDebug_PrintHall1State("EXTI HALL_1_A");` | Skriver ut motor 1 hall-status og markerer at kilden var hall A. |
| 72 | `}` | Avslutter H1_A-sjekken. |
| 73 | `if (event_flags & HALL_EVENT_H1_B) {` | Sjekker om motor 1 hall B har trigget. |
| 74 | `HallDebug_PrintHall1State("EXTI HALL_1_B");` | Skriver ut motor 1 hall-status og markerer at kilden var hall B. |
| 75 | `}` | Avslutter H1_B-sjekken. |
| 76 | `if (event_flags & HALL_EVENT_H1_C) {` | Sjekker om motor 1 hall C har trigget. |
| 77 | `HallDebug_PrintHall1State("EXTI HALL_1_C");` | Skriver ut motor 1 hall-status og markerer at kilden var hall C. |
| 78 | `}` | Avslutter H1_C-sjekken. |
| 79 | `if ((HALL_DEBUG_ENABLE_MOTOR2 != 0U) && ((event_flags & HALL_EVENT_H2_A) != 0U)) {` | Sjekker om motor 2-debug er aktivert og om motor 2 hall A har trigget. |
| 80 | `HallDebug_PrintHall2State("EXTI HALL_2_A");` | Skriver ut motor 2 hall-status og markerer at kilden var hall A. |
| 81 | `}` | Avslutter H2_A-sjekken. |
| 82 | `if ((HALL_DEBUG_ENABLE_MOTOR2 != 0U) && ((event_flags & HALL_EVENT_H2_B) != 0U)) {` | Sjekker om motor 2-debug er aktivert og om motor 2 hall B har trigget. |
| 83 | `HallDebug_PrintHall2State("EXTI HALL_2_B");` | Skriver ut motor 2 hall-status og markerer at kilden var hall B. |
| 84 | `}` | Avslutter H2_B-sjekken. |
| 85 | `if ((HALL_DEBUG_ENABLE_MOTOR2 != 0U) && ((event_flags & HALL_EVENT_H2_C) != 0U)) {` | Sjekker om motor 2-debug er aktivert og om motor 2 hall C har trigget. |
| 86 | `HallDebug_PrintHall2State("EXTI HALL_2_C");` | Skriver ut motor 2 hall-status og markerer at kilden var hall C. |
| 87 | `}` | Avslutter H2_C-sjekken. |
| 88 | `}` | Avslutter `HallDebug_Process()`. |
| 89 | Tom linje | Brukes kun for lesbarhet. |
| 90 | `void HallDebug_OnExti(uint16_t gpio_pin)` | Starter funksjonen som kalles naar en EXTI-hendelse skjer paa en GPIO-pin. |
| 91 | `{` | Aapner funksjonsblokken. |
| 92 | `HallDebug_CaptureSnapshots();` | Leser og lagrer alle hall-verdiene med en gang interruptet skjer. |
| 93 | Tom linje | Brukes kun for lesbarhet. |
| 94 | `switch (gpio_pin) {` | Velger handling basert paa hvilken GPIO-pin som trigget EXTI. |
| 95 | `case GPIO_PIN_9:` | Treffer hvis interruptet kom fra `PC9`, motor 1 hall A. |
| 96 | `s_hallEventFlags |= HALL_EVENT_H1_A;` | Setter hendelsesflagget for motor 1 hall A. |
| 97 | `break;` | Avslutter denne `case`-grenen. |
| 98 | `case GPIO_PIN_1:` | Treffer hvis interruptet kom fra `PC1`, motor 1 hall B. |
| 99 | `s_hallEventFlags |= HALL_EVENT_H1_B;` | Setter hendelsesflagget for motor 1 hall B. |
| 100 | `break;` | Avslutter denne `case`-grenen. |
| 101 | `case GPIO_PIN_7:` | Treffer hvis interruptet kom fra `PC7`, motor 1 hall C. |
| 102 | `s_hallEventFlags |= HALL_EVENT_H1_C;` | Setter hendelsesflagget for motor 1 hall C. |
| 103 | `break;` | Avslutter denne `case`-grenen. |
| 104 | `case GPIO_PIN_2:` | Treffer hvis interruptet kom fra `PC2`, motor 2 hall A. |
| 105 | `if (HALL_DEBUG_ENABLE_MOTOR2 != 0U) {` | Sjekker om motor 2-debug er aktivert foer flagget settes. |
| 106 | `s_hallEventFlags |= HALL_EVENT_H2_A;` | Setter hendelsesflagget for motor 2 hall A. |
| 107 | `}` | Avslutter motor 2-debug-sjekken. |
| 108 | `break;` | Avslutter denne `case`-grenen. |
| 109 | `case GPIO_PIN_3:` | Treffer hvis interruptet kom fra `PC3`, motor 2 hall B. |
| 110 | `if (HALL_DEBUG_ENABLE_MOTOR2 != 0U) {` | Sjekker om motor 2-debug er aktivert. |
| 111 | `s_hallEventFlags |= HALL_EVENT_H2_B;` | Setter hendelsesflagget for motor 2 hall B. |
| 112 | `}` | Avslutter motor 2-debug-sjekken. |
| 113 | `break;` | Avslutter denne `case`-grenen. |
| 114 | `case GPIO_PIN_8:` | Treffer hvis interruptet kom fra `PC8`, motor 2 hall C. |
| 115 | `if (HALL_DEBUG_ENABLE_MOTOR2 != 0U) {` | Sjekker om motor 2-debug er aktivert. |
| 116 | `s_hallEventFlags |= HALL_EVENT_H2_C;` | Setter hendelsesflagget for motor 2 hall C. |
| 117 | `}` | Avslutter motor 2-debug-sjekken. |
| 118 | `break;` | Avslutter denne `case`-grenen. |
| 119 | `default:` | Treffer hvis pinnen ikke er en av hall-pinnene som debug-koden kjenner. |
| 120 | `break;` | Gjoer ingenting for ukjente pinner. |
| 121 | `}` | Avslutter `switch`-blokken. |
| 122 | `}` | Avslutter `HallDebug_OnExti()`. |
| 123 | Tom linje | Brukes kun for lesbarhet. |
| 124 | `void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)` | Definerer HAL-callbacken som kalles naar et GPIO EXTI-avbrudd er ferdig behandlet av HAL. |
| 125 | `{` | Aapner funksjonsblokken. |
| 126 | `HallStateFilter_OnExti(GPIO_Pin);` | Sender EXTI-hendelsen videre til hall-filteret, som brukes av motorstyringen. |
| 127 | `HallDebug_OnExti(GPIO_Pin);` | Sender samme EXTI-hendelse videre til hall-debug, som setter debug-flagg og lagrer snapshot. |
| 128 | `}` | Avslutter HAL-callbacken. |

## Forenklet flyt for hall_debug

```text
GPIO EXTI interrupt
|
`-- HAL_GPIO_EXTI_Callback(GPIO_Pin)
    |
    |-- Send pin til HallStateFilter_OnExti
    |
    `-- Send pin til HallDebug_OnExti
        |
        |-- Les snapshot av alle hall-pinner
        |
        `-- Sett event-flagg for riktig hall-sensor

Senere i default task:
|
`-- HallDebug_Process
    |
    |-- Kopier og nullstill event-flagg mens interrupt er av
    |
    |-- Hvis H1_A/H1_B/H1_C flagg er satt:
    |   `-- Skriv motor 1 hall-status til UART
    |
    `-- Hvis motor 2-debug er aktivert og H2-flagg er satt:
        `-- Skriv motor 2 hall-status til UART
```

## hall_probe

`Core/Src/hall_probe.c` brukes til aa sjekke hall-sensorene ved polling i stedet for interrupt. Den leser motor 1 sine hall-pinner med fast intervall og skriver til UART bare naar verdiene har endret seg. Dette er nyttig som enkel feilsoking for aa se om hall-sensorene faktisk skifter mellom `0` og `1`.

## Core/Inc/hall_probe.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#ifndef INC_HALL_PROBE_H_` | Starter include guard, slik at headeren ikke inkluderes flere ganger i samme kompilering. |
| 2 | `#define INC_HALL_PROBE_H_` | Definerer guard-navnet som brukes av linje 1. |
| 3 | Tom linje | Brukes kun for lesbarhet. |
| 4 | `#include "main.h"` | Inkluderer STM32/HAL-definisjoner fra prosjektet. Trengs for GPIO-typer, pin-definisjoner og standard heltallstyper. |
| 5 | Tom linje | Brukes kun for lesbarhet. |
| 6 | `void HallProbe_Init(void);` | Deklarerer init-funksjonen for hall-probe. |
| 7 | `void HallProbe_Process(void);` | Deklarerer prosessfunksjonen som poller hall-pinnene og skriver endringer til UART. |
| 8 | Tom linje | Brukes kun for lesbarhet. |
| 9 | `#endif /* INC_HALL_PROBE_H_ */` | Avslutter include guard. |

## Core/Src/hall_probe.c

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#include "hall_probe.h"` | Inkluderer egen header, slik at `.c`-filen matcher de offentlige funksjonsdeklarasjonene. |
| 2 | Tom linje | Brukes kun for lesbarhet. |
| 3 | `#include "myprint.h"` | Inkluderer UART/print-modulen som brukes til aa skrive hall-status til terminal. |
| 4 | Tom linje | Brukes kun for lesbarhet. |
| 5 | `static uint8_t s_lastHall1A = 0xFFU;` | Lagrer forrige avleste verdi for motor 1 hall A. Startverdien `0xFFU` er bevisst ugyldig, slik at foerste ekte avlesning alltid regnes som en endring. |
| 6 | `static uint8_t s_lastHall1B = 0xFFU;` | Lagrer forrige avleste verdi for motor 1 hall B, med samme ugyldige startverdi. |
| 7 | `static uint8_t s_lastHall1C = 0xFFU;` | Lagrer forrige avleste verdi for motor 1 hall C, med samme ugyldige startverdi. |
| 8 | `static uint32_t s_lastPollTick = 0U;` | Lagrer tidspunktet for forrige polling, slik at modulen kan vente 50 ms mellom hver lesing. |
| 9 | Tom linje | Brukes kun for lesbarhet. |
| 10 | `static uint8_t HallProbe_Read(GPIO_TypeDef *port, uint16_t pin)` | Starter en privat hjelpefunksjon for aa lese en GPIO-pin som `0` eller `1`. |
| 11 | `{` | Aapner funksjonsblokken. |
| 12 | `return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? 1U : 0U;` | Leser pinnen med HAL. Hvis pinnen er hoey returneres `1`, ellers returneres `0`. |
| 13 | `}` | Avslutter hjelpefunksjonen. |
| 14 | Tom linje | Brukes kun for lesbarhet. |
| 15 | `void HallProbe_Init(void)` | Starter init-funksjonen for hall-probe. |
| 16 | `{` | Aapner funksjonsblokken. |
| 17 | `s_lastHall1A = 0xFFU;` | Nullstiller forrige verdi for hall A til ugyldig startverdi. |
| 18 | `s_lastHall1B = 0xFFU;` | Nullstiller forrige verdi for hall B til ugyldig startverdi. |
| 19 | `s_lastHall1C = 0xFFU;` | Nullstiller forrige verdi for hall C til ugyldig startverdi. |
| 20 | `s_lastPollTick = HAL_GetTick();` | Lagrer naavaerende HAL-tick som startpunkt for polling-intervallet. |
| 21 | `MyPrint_Print("Hall probe ready: polling H1_A/H1_B/H1_C\r\n");` | Skriver en oppstartsmelding til UART som viser at hall-probe er klar og poller motor 1 sine hall-pinner. |
| 22 | `}` | Avslutter init-funksjonen. |
| 23 | Tom linje | Brukes kun for lesbarhet. |
| 24 | `void HallProbe_Process(void)` | Starter prosessfunksjonen. Denne skal kalles jevnlig fra en task, for eksempel default task. |
| 25 | `{` | Aapner funksjonsblokken. |
| 26 | `uint32_t now = HAL_GetTick();` | Leser naavaerende HAL-tick, altsaa systemtid i millisekunder. |
| 27 | `uint8_t hall1a;` | Lager lokal variabel for ny verdi fra motor 1 hall A. |
| 28 | `uint8_t hall1b;` | Lager lokal variabel for ny verdi fra motor 1 hall B. |
| 29 | `uint8_t hall1c;` | Lager lokal variabel for ny verdi fra motor 1 hall C. |
| 30 | Tom linje | Brukes kun for lesbarhet. |
| 31 | `if ((now - s_lastPollTick) < 50U) {` | Sjekker om det har gaatt mindre enn 50 ms siden forrige polling. Tick-differanse brukes slik at koden taaler tick-overflyt. |
| 32 | `return;` | Avslutter funksjonen tidlig hvis det ikke er tid for ny polling enda. |
| 33 | `}` | Avslutter 50 ms-sjekken. |
| 34 | Tom linje | Brukes kun for lesbarhet. |
| 35 | `s_lastPollTick = now;` | Oppdaterer tidspunktet for siste polling. |
| 36 | Tom linje | Brukes kun for lesbarhet. |
| 37 | `hall1a = HallProbe_Read(GPIOC, GPIO_PIN_9);` | Leser motor 1 hall A fra `PC9`. |
| 38 | `hall1b = HallProbe_Read(GPIOC, GPIO_PIN_1);` | Leser motor 1 hall B fra `PC1`. |
| 39 | `hall1c = HallProbe_Read(GPIOC, GPIO_PIN_7);` | Leser motor 1 hall C fra `PC7`. |
| 40 | Tom linje | Brukes kun for lesbarhet. |
| 41 | `if ((hall1a != s_lastHall1A) ||` | Starter sjekk av om hall A har endret seg siden forrige lagrede verdi. |
| 42 | `(hall1b != s_lastHall1B) ||` | Sjekker om hall B har endret seg. |
| 43 | `(hall1c != s_lastHall1C)) {` | Sjekker om hall C har endret seg og fullfoerer samlet endringssjekk. |
| 44 | `s_lastHall1A = hall1a;` | Lagrer ny verdi for hall A. |
| 45 | `s_lastHall1B = hall1b;` | Lagrer ny verdi for hall B. |
| 46 | `s_lastHall1C = hall1c;` | Lagrer ny verdi for hall C. |
| 47 | Tom linje | Brukes kun for lesbarhet. |
| 48 | `MyPrint_Printf("POLL H1: A=%u B=%u C=%u\r\n", hall1a, hall1b, hall1c);` | Skriver de nye hall-verdiene til UART. Dette skjer bare naar minst en verdi har endret seg. |
| 49 | `}` | Avslutter endringssjekken. |
| 50 | `}` | Avslutter `HallProbe_Process()`. |

## Forenklet flyt for hall_probe

```text
HallProbe_Init
|
|-- Sett forrige hall-verdier til ugyldig startverdi
|-- Lagre starttidspunkt
`-- Skriv "Hall probe ready" til UART

HallProbe_Process
|
|-- Les systemtid
|
|-- Hvis det har gaatt mindre enn 50 ms:
|   `-- Returner uten aa gjoere noe
|
|-- Oppdater siste poll-tidspunkt
|
|-- Les H1_A, H1_B og H1_C
|
|-- Hvis en av verdiene har endret seg:
|   |-- Lagre de nye verdiene
|   `-- Skriv ny hall-status til UART
|
`-- Ferdig til neste kall
```

## hall_state_filter

`Core/Src/hall_state_filter.c` leser hall-sensorene, oversetter kombinasjonen `UVW` til en gyldig sektor `1-6`, filtrerer bort ugyldige/ulogiske overganger, og sender godkjente hall-tilstander videre til `motor_commutation`. Modulen kan brukes baade fra polling (`HallStateFilter_Process`) og direkte fra EXTI-avbrudd (`HallStateFilter_OnExti`).

## Core/Inc/hall_state_filter.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#ifndef INC_HALL_STATE_FILTER_H_` | Starter include guard. |
| 2 | `#define INC_HALL_STATE_FILTER_H_` | Definerer include guard-navnet. |
| 3 | Tom linje | Lesbarhet. |
| 4 | `#include "main.h"` | Henter STM32/HAL-typer og pin-definisjoner. |
| 5 | Tom linje | Lesbarhet. |
| 6-15 | `typedef struct { ... } HallStateFilter_State;` | Definerer datastrukturen som beskriver en akseptert hall-tilstand: U/V/W-bit, samlet hall-kode, sektor, syklusteller, millisekund-tick og overgangsnummer. |
| 17-21 | `typedef enum { ... } HallStateFilter_MotorIndex;` | Definerer indeksene for motor 1, motor 2 og antall motorer. |
| 23 | `void HallStateFilter_Init(void);` | Initierer filterets runtime-data. |
| 24 | `void HallStateFilter_Process(void);` | Poller begge motorenes hall-pinner og forsoker aa akseptere nye tilstander. |
| 25 | `void HallStateFilter_OnExti(uint16_t gpio_pin);` | Kalles fra GPIO EXTI-callback naar en hall-pin endrer seg. |
| 26-27 | `HallStateFilter_GetLatestState(...)` | Returnerer siste aksepterte hall-tilstand for valgt motor hvis den finnes. |
| 28-29 | `HallStateFilter_GetCurrentState(...)` | Leser hall-pinnene akkurat naa og fyller en state uten aa bruke filterhistorikk. |
| 30 | Tom linje | Lesbarhet. |
| 31 | `#endif /* INC_HALL_STATE_FILTER_H_ */` | Avslutter include guard. |

## Core/Src/hall_state_filter.c

| Linje(r) | Kode/blokk | Forklaring |
|---:|---|---|
| 1 | `#include "hall_state_filter.h"` | Inkluderer egen header. |
| 3 | `#include "motor_commutation.h"` | Trengs fordi filteret varsler motorstyringen naar en hall-tilstand er akseptert. |
| 4 | `#include "myprint.h"` | Brukes til eventuell debug-utskrift. |
| 6 | `HALL_FILTER_SAMPLE_PERIOD_MS` | Setter minimumstid mellom pollinger. `0U` betyr at polling ikke begrenses av denne verdien. |
| 7 | `HALL_FILTER_STABLE_COUNT` | Antall like maalinger som kreves foer en kandidat aksepteres. `1U` betyr at en gyldig overgang kan aksepteres med en gang. |
| 8 | `HALL_FILTER_INVALID_CODE` | Spesialverdi for ugyldig hall-kode. |
| 9 | `HALL_FILTER_ENABLE_RUNTIME_PRINTS` | Slukker/tenner runtime debug-utskrift. |
| 10 | `HALL_FILTER_MIN_TRANSITION_MS` | Minimumstid mellom aksepterte overganger. `0U` betyr ingen ekstra tidsbegrensning. |
| 12-15 | `HallCodeSectorMap` | Kobler en hall-kode til en elektrisk sektor. |
| 17-25 | `HallFilter_MotorPins` | Samler GPIO-port/pin for U, V og W for en motor, pluss et navn. |
| 27-35 | `HallFilter_Runtime` | Holder filterets interne tilstand per motor: siste state, kandidatkode, stabilitetsteller, siste tick og overgangsteller. |
| 37-44 | `kHallSequence` | Definerer gyldig sekvens av hall-koder og sektorene de tilsvarer. Bare disse kodene regnes som gyldige. |
| 46-49 | `kHallFilterMotorPins` | Binder motor 1 til `PC7/PC1/PC9` og motor 2 til `PC2/PC3/PC8`. |
| 51 | `s_hallFilterRuntime` | Runtime-array med en tilstand per motor. |
| 52-55 | `HallStateFilter_ReadPin` | Leser en GPIO-pin med HAL og returnerer `1` ved `GPIO_PIN_SET`, ellers `0`. |
| 57-65 | `HallStateFilter_ReadHallCode` | Leser U, V og W for valgt motor og bygger en 3-bit hall-kode: `U` flyttes til bit 2, `V` til bit 1, `W` til bit 0. |
| 67-78 | `HallStateFilter_FindSequenceIndex` | Leter i `kHallSequence` etter en hall-kode. Returnerer indeks hvis funnet, ellers `HALL_FILTER_INVALID_CODE`. |
| 80-101 | `HallStateFilter_IsAdjacent` | Sjekker om to gyldige hall-koder ligger ved siden av hverandre i sekvensen. Dette filtrerer bort hopp som ikke skal skje i normal rotasjon. |
| 103-119 | `HallStateFilter_FillState` | Validerer hall-koden, splitter den til U/V/W-bit, finner sektor og fyller en `HallStateFilter_State`. |
| 121-124 | `HallStateFilter_GetCycleCounter` | Returnerer CPU-syklustelleren `DWT->CYCCNT`, brukt til mer presis RPM-beregning i motorstyringen. |
| 126-157 | `HallStateFilter_AcceptState` | Lager en ferdig state, oeker overgangstelleren, legger inn tid/sykluser, lagrer state som siste gyldige, og kaller `MotorCommutation_OnHallStateAccepted`. |
| 146-156 | Runtime print-blokk | Kompileres bare inn hvis `HALL_FILTER_ENABLE_RUNTIME_PRINTS` er aktivert. Brukes til aa skrive hall-status for motor 2. |
| 159-188 | `HallStateFilter_TryAcceptState` | Hovedfilteret: avviser ugyldig kode, samme kode som sist, for raske overganger og ikke-nabo-overganger. Hvis alt er OK, aksepteres ny state. |
| 190-218 | `HallStateFilter_ProcessMotor` | Poller en motor. Leser hall-kode, krever stabil kandidat, og sender den til `TryAcceptState` naar stabilitetskravet er oppfylt. |
| 220-234 | `HallStateFilter_Init` | Nullstiller runtime for begge motorer og skriver oppstartsmelding til UART. |
| 236-242 | `HallStateFilter_Process` | Leser systemtid og prosesserer motor 1 og motor 2. |
| 244-262 | `HallStateFilter_OnExti` | Kalles ved GPIO EXTI. Velger motor basert paa pinnen som trigget, leser aktuell hall-kode, og forsoker aa akseptere den direkte. |
| 264-282 | `HallStateFilter_GetLatestState` | Returnerer siste aksepterte state. Bruker kort interrupt-lukking for aa kopiere runtime-data trygt. |
| 284-304 | `HallStateFilter_GetCurrentState` | Leser hall-pinnene direkte, fyller en state hvis koden er gyldig, og legger paa naa-tid/syklusteller. |

## Forenklet flyt for hall_state_filter

```text
Init
|
`-- Nullstill runtime for M1 og M2

Ved polling:
|
`-- HallStateFilter_Process
    |
    |-- ProcessMotor M1
    `-- ProcessMotor M2
        |
        |-- Les hall-kode
        |-- Sjekk stabil kandidat
        `-- TryAcceptState

Ved EXTI:
|
`-- HallStateFilter_OnExti(GPIO_Pin)
    |
    |-- Finn hvilken motor pinnen tilhoerer
    |-- Les hall-kode for den motoren
    `-- TryAcceptState

TryAcceptState
|
|-- Avvis ugyldig kode
|-- Avvis samme kode som sist
|-- Avvis for rask overgang hvis min-tid er satt
|-- Avvis ikke-nabo i hall-sekvens
`-- AcceptState og varsle MotorCommutation
```

## hardfault_diag

`Core/Src/hardfault_diag.c` lagrer viktige Cortex-M feilmeldingsregistre naar en hardfault skjer, og resetter deretter mikrokontrolleren. Verdiene ligger i globale `volatile` variabler slik at de kan inspiseres i debugger etter feil.

## Core/Inc/hardfault_diag.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#ifndef HARDFLT_DIAG_H` | Starter include guard. |
| 2 | `#define HARDFLT_DIAG_H` | Definerer include guard-navnet. |
| 3 | Tom linje | Lesbarhet. |
| 4 | `#include <stdint.h>` | Henter standard heltallstyper som `uint32_t`. |
| 5 | Tom linje | Lesbarhet. |
| 6 | `extern volatile uint32_t g_hardfault_hfsr;` | Deklarerer global variabel for HardFault Status Register. |
| 7 | `extern volatile uint32_t g_hardfault_cfsr;` | Deklarerer global variabel for Configurable Fault Status Register. |
| 8 | `extern volatile uint32_t g_hardfault_mmfar;` | Deklarerer global variabel for Memory Management Fault Address Register. |
| 9 | `extern volatile uint32_t g_hardfault_bfar;` | Deklarerer global variabel for Bus Fault Address Register. |
| 10 | Tom linje | Lesbarhet. |
| 11 | `void HardFaultDiag_Handle(void);` | Deklarerer funksjonen som skal kalles ved hardfault. |
| 12 | Tom linje | Lesbarhet. |
| 13 | `#endif /* HARDFLT_DIAG_H */` | Avslutter include guard. |

## Core/Src/hardfault_diag.c

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#include "hardfault_diag.h"` | Inkluderer egen header. |
| 2 | Tom linje | Lesbarhet. |
| 3 | `#include "main.h"` | Henter CMSIS/HAL-definisjoner for `SCB` og `NVIC_SystemReset`. |
| 4 | Tom linje | Lesbarhet. |
| 5 | `volatile uint32_t g_hardfault_hfsr = 0U;` | Lager global lagringsplass for `HFSR`. |
| 6 | `volatile uint32_t g_hardfault_cfsr = 0U;` | Lager global lagringsplass for `CFSR`. |
| 7 | `volatile uint32_t g_hardfault_mmfar = 0U;` | Lager global lagringsplass for `MMFAR`. |
| 8 | `volatile uint32_t g_hardfault_bfar = 0U;` | Lager global lagringsplass for `BFAR`. |
| 9 | Tom linje | Lesbarhet. |
| 10 | `void HardFaultDiag_Handle(void)` | Starter hardfault-handlerens diagnosefunksjon. |
| 11 | `{` | Aapner funksjonsblokken. |
| 12 | `g_hardfault_hfsr = SCB->HFSR;` | Leser og lagrer hardfault-statusregisteret. |
| 13 | `g_hardfault_cfsr = SCB->CFSR;` | Leser og lagrer konfigurerbart fault-statusregister. |
| 14 | `g_hardfault_mmfar = SCB->MMFAR;` | Leser og lagrer adresse for memory-management fault. |
| 15 | `g_hardfault_bfar = SCB->BFAR;` | Leser og lagrer adresse for bus fault. |
| 16 | `NVIC_SystemReset();` | Resetter mikrokontrolleren etter at registerverdiene er lagret. |
| 17 | `}` | Avslutter funksjonen. |

## joystick

`Core/Src/joystick.c` leser joystick via ADC, filtrerer/debouncer knappetrykk, regner om raaverdier til kommandoer fra `-100` til `100`, mikser drive/turn til venstre og hoeyre motor, og sender kommandoene videre til `motor_commutation`.

## Core/Inc/joystick.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1-2 | Include guard | Hindrer dobbel inkludering av headeren. |
| 4 | `#include <stdint.h>` | Henter `uint8_t`, `uint16_t` og `int16_t`. |
| 6-10 | `Joystick_MotorCommand` | Beskriver en motor-kommando med `enabled`, retning `forward` og duty i prosent. |
| 12 | `Joystick_Init` | Initierer joystickens interne tilstand. |
| 13 | `Joystick_Process` | Leser ADC, oppdaterer knapp og sender motor-kommandoer. |
| 14-15 | `Joystick_GetRawX/Y` | Returnerer siste raaverdi fra ADC. |
| 16-20 | Kommando-gettere | Returnerer beregnede kommandoer for speed, drive, turn, left og right. |
| 21 | `Joystick_IsButtonActive` | Returnerer om knappen er aktiv. |
| 22-23 | MotorCommand-gettere | Returnerer siste beregnede venstre/hoeyre motor-kommando. |
| 25 | `#endif` | Avslutter include guard. |

## Core/Src/joystick.c

| Linje(r) | Kode/blokk | Forklaring |
|---:|---|---|
| 1 | `#include "joystick.h"` | Inkluderer egen header. |
| 3 | `#include "adc.h"` | Gir tilgang til `hadc1` og ADC-konfigurasjon. |
| 4 | `#include "motor_commutation.h"` | Trengs for aa sende hjulkommandoer og toggle motor on/off. |
| 5 | `#include "myprint.h"` | Brukes til valgfri debug-utskrift. |
| 7-14 | Senter/deadband/min/max | Kalibreringsverdier for X- og Y-aksen. De bestemmer midtpunkt, doedsone og omraade for skalering. |
| 15-16 | Knappeterskel/debounce | Definerer ADC-terskel for knapp og hvor lenge signalet maa vaere stabilt. |
| 17-24 | Output/duty/timing/debug | Definerer output-deadband, duty-omraade, poll-intervall, print-intervall og debug-brytere. |
| 26-44 | Static runtime-variabler | Holder siste ADC-verdier, beregnede kommandoer, motor-kommandoer, knappestatus og tidsstempler. |
| 46-56 | `Joystick_Clamp100` | Begrenser en verdi til intervallet `-100` til `100`. |
| 58-92 | `Joystick_ComputeAxisCommand` | Regner om en ADC-raaverdi til kommando. Returnerer `0` i deadband, positiv verdi over senter og negativ verdi under senter. |
| 94 | `#if JOYSTICK_RAW_ADC_ONLY == 0U` | Kompilerer motorstyringslogikken bare naar raw ADC-only ikke er aktiv. |
| 95-115 | `Joystick_UpdateMixedCommands` | Mikser drive og turn: venstre = drive + turn, hoeyre = drive - turn. Deretter clamps og deadband-filtreres begge. |
| 117-142 | `Joystick_ComputeMotorCommand` | Oversetter en kommando til `enabled`, retning og duty. `0` gir motor av; ellers skaleres duty mellom minimum og maksimum. |
| 145-170 | `Joystick_Init` | Nullstiller alle runtime-variabler og setter starttidspunkt for knapp, polling og printing. |
| 172-183 | Start av `Joystick_Process` | Leser tid og returnerer tidlig hvis det ikke har gaatt 50 ms siden forrige polling. |
| 184-199 | Foerste ADC-lesing | Starter ADC, venter paa konvertering, leser `s_joystickRawX`, og bruker den til drive-kommando med Y-kalibrering. |
| 200-210 | Bytte ADC-kanal | Konfigurerer ADC til kanal 2 for neste lesing. Stopper/returnerer ved feil. |
| 212-222 | Andre ADC-lesing | Starter ADC igjen, venter, leser `s_joystickRawY`, og stopper ADC. |
| 224-231 | Tilbakestill ADC-kanal | Setter ADC tilbake til kanal 1 slik at neste runde starter riktig. |
| 233-245 | Knapp og debounce | Regner knapp aktiv fra ADC-terskel, registrerer endringer, venter debounce-tid, og toggler motorene naar et stabilt knappetrykk oppdages. |
| 247-258 | Turn-kommando | Regner X/turn-kommando fra `s_joystickRawY`. Naar knappen holdes aktiv, laases siste turn-verdi. |
| 260-277 | Raw-only eller motorstyring | I raw-only nulles motorstyringen. Ellers mikses kommandoer, motor-kommandoer beregnes, og `MotorCommutation_SetWheelCommands` kalles. |
| 279-295 | Print-rate og endringsfilter | Returnerer hvis printintervallet ikke er naadd, eller hvis X/Y ikke har endret seg nok siden sist. |
| 297-319 | Debug-utskrift | Kompileres bare inn hvis `JOYSTICK_ENABLE_PRINTS` er aktivert. Skriver enten raw ADC eller ferdige motor-kommandoer. |
| 320-324 | Ikke-print fallback | Marker variabler som brukt naar prints er deaktivert, for aa unngaa compiler-warnings. |
| 327-375 | Getter-funksjoner | Returnerer siste lagrede raw-verdier, kommandoer, knappestatus og motor-kommandoer. |

## Forenklet flyt for joystick

```text
Joystick_Process
|
|-- Hvis 50 ms ikke har gaatt: returner
|-- Les ADC-verdi 1
|-- Beregn drive-kommando
|-- Bytt ADC-kanal
|-- Les ADC-verdi 2
|-- Gjenopprett ADC-kanal
|-- Oppdater knapp med debounce
|-- Ved stabilt knappetrykk: toggle motor on/off
|-- Beregn turn-kommando
|-- Miks drive/turn til venstre og hoeyre
|-- Send hjulkommandoer til MotorCommutation
`-- Eventuell debug-print
```

## lcd_status

`Core/Src/lcd_status.c` bygger to tekstlinjer paa 16 tegn og skriver dem til RGB LCD-displayet. Linje 1 viser hastighet og motorstatus `ON/OFF`. Linje 2 viser enten temperatur for motor 1 og 2, eller feilmelding ved overtemperatur.

## Core/Inc/lcd_status.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#ifndef INC_LCD_STATUS_H_` | Starter include guard. |
| 2 | `#define INC_LCD_STATUS_H_` | Definerer include guard-navnet. |
| 3 | Tom linje | Lesbarhet. |
| 4 | `#include "main.h"` | Henter STM32/HAL-typer som `uint8_t`. |
| 5 | Tom linje | Lesbarhet. |
| 6-10 | `void LcdStatus_Update(...)` | Deklarerer funksjonen som oppdaterer LCD med hastighet, motorstatus, temperaturer og temperaturfeil. |
| 11 | Tom linje | Lesbarhet. |
| 12 | `#endif /* INC_LCD_STATUS_H_ */` | Avslutter include guard. |

## Core/Src/lcd_status.c

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#include "lcd_status.h"` | Inkluderer egen header. |
| 2 | Tom linje | Lesbarhet. |
| 3 | `#include "motor_temperature_safety.h"` | Henter bitmaskene for temperaturfeil. |
| 4 | `#include "rgb_lcd1602.h"` | Henter LCD-driveren som faktisk skriver til displayet. |
| 5 | Tom linje | Lesbarhet. |
| 6 | Prototype for `LcdStatus_FormatVehicleLine` | Forteller kompilatoren at hjelpefunksjonen finnes senere i filen. |
| 7 | Prototype for `LcdStatus_FormatTemperatureLine` | Deklarerer hjelpefunksjonen for normal temperaturlinje. |
| 8 | Prototype for `LcdStatus_FormatOvertemperatureLine` | Deklarerer hjelpefunksjonen for feilmelding. |
| 9 | Prototype for `LcdStatus_FormatTemperatureField` | Deklarerer hjelpefunksjonen som formaterer en temperaturverdi. |
| 10 | Tom linje | Lesbarhet. |
| 11-15 | `LcdStatus_Update(...)` signatur | Starter offentlig oppdateringsfunksjon med hastighet, motorstatus, temperaturer og feilflagg. |
| 16 | `{` | Aapner funksjonen. |
| 17 | `char lcd_line_1[17];` | Lager buffer for LCD-linje 1: 16 tegn pluss nullterminering. |
| 18 | `char lcd_line_2[17];` | Lager buffer for LCD-linje 2. |
| 19 | Tom linje | Lesbarhet. |
| 20-22 | Negativ hastighet | Gjoer hastigheten positiv hvis den er negativ. |
| 23-25 | Maks hastighet | Begrenser visning til `99.9` km/t fordi formatet bruker tre siffer med en desimal. |
| 27 | `LcdStatus_FormatVehicleLine(...)` | Bygger linje 1 med hastighet og ON/OFF. |
| 29-33 | Feil eller temperatur | Ved temperaturfeil bygges feillinje, ellers bygges normal temperaturlinje. |
| 35-36 | Skriv linje 1 | Setter cursor til kolonne 0, rad 0 og skriver linje 1. |
| 38-39 | Skriv linje 2 | Setter cursor til kolonne 0, rad 1 og skriver linje 2. |
| 40 | `}` | Avslutter `LcdStatus_Update`. |
| 42-47 | Start `LcdStatus_FormatVehicleLine` | Lager heltallsdel, desimaldel og loop-variabel. |
| 48-50 | Fyll med mellomrom | Setter alle 16 tegn til blanke, slik at gamle tegn paa displayet overskrives. |
| 52-59 | Hastighetstekst | Skriver `NN.Nkm/t` fra venstre side av linjen. |
| 61-68 | Motorstatus | Skriver `ON` paa posisjon 14-15 eller `OFF` paa posisjon 13-15, helt til hoeyre. |
| 70 | Nullterminering | Setter `\0` etter tegn 16 slik at stringen kan printes trygt. |
| 71 | `}` | Avslutter vehicle line-funksjonen. |
| 73-86 | `LcdStatus_FormatTemperatureLine` | Bygger normal temperaturlinje: `T1 xx.x T2 yy.y`. Hver temperatur skrives via `LcdStatus_FormatTemperatureField`. |
| 88-95 | Start feillinje | Lager loop-variabel og fyller linjen med mellomrom. |
| 96-106 | `TEMP FAULT ` | Skriver fast tekst for temperaturfeil. |
| 108-114 | Begge motorer feil | Hvis begge fault-bit er satt, skrives `M1+M2`. |
| 115-117 | Motor 1 feil | Hvis bare M1-feil er satt, skrives `M1`. |
| 118-120 | Motor 2 feil | Hvis bare M2-feil er satt, skrives `M2`. |
| 123 | Nullterminering | Avslutter stringen. |
| 124 | `}` | Avslutter feillinje-funksjonen. |
| 126-129 | Start temperaturfelt | Starter formattering av ett temperaturfelt paa fire tegn. |
| 130-136 | Ugyldig omraade | Temperatur under `0.0` eller over `99.9` vises som `--.-`. |
| 138-141 | Skalering og maks | Skalerer temperatur til tidels grader og begrenser til `999`. |
| 143-147 | Skriv felt | Skriver hundrer-/tier-/enerposisjon som `NN.N`. |

## Forenklet flyt for lcd_status

```text
LcdStatus_Update
|
|-- Begrens hastighet til 0..999 tidels km/t
|-- Formater linje 1: hastighet + ON/OFF
|
|-- Hvis temperaturfeil:
|   `-- Formater TEMP FAULT-linje
|-- Ellers:
|   `-- Formater T1/T2-temperaturer
|
|-- Skriv linje 1 til LCD rad 0
`-- Skriv linje 2 til LCD rad 1
```

## motor_commutation

`Core/Src/motor_commutation.c` er motorstyringens hovedmodul. Den holder runtime-tilstand for begge motorer, tar imot hjulkommandoer fra joystick, bruker hall-tilstander til aa velge kommuteringssektor, setter PWM-fasene, beregner RPM/hastighet, og kan stoppe alle motorutganger ved feil.

## Core/Inc/motor_commutation.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1-2 | Include guard | Hindrer dobbel inkludering. |
| 4 | `#include "hall_state_filter.h"` | Trengs for `HallStateFilter_MotorIndex` og `HallStateFilter_State`. |
| 5 | `#include "main.h"` | Henter STM32/HAL-typer og standard prosjektdefinisjoner. |
| 7 | `MotorCommutation_Init` | Initierer PWM, runtime og DWT-syklusteller. |
| 8 | `MotorCommutation_Process` | Periodisk prosessfunksjon som haandterer toggle, knapp, sweep og motorprosessering. |
| 9 | `MotorCommutation_SetDuty` | Setter duty direkte for begge motorer. |
| 10 | `MotorCommutation_SetWheelCommands` | Setter oenskede hjulkommandoer i prosent for motor 1 og motor 2. |
| 11 | `MotorCommutation_ToggleEnabled` | Ber modulen toggle motor on/off. Selve togglingen skjer i process-kontekst. |
| 12 | `MotorCommutation_ForceStop` | Tvinger motorene av og nullstiller kommando/runtime ved feil. |
| 13-14 | `MotorCommutation_OnHallStateAccepted` | Callback fra hall-filteret naar en ny hall-tilstand er godkjent. |
| 15-17 | Hastighetsgettere | Returnerer hastighet for motor 1, motor 2 og gjennomsnittlig kjoeretoeyhastighet. |
| 18 | `MotorCommutation_IsEnabled` | Returnerer om motorstyringen er aktiv. |
| 19 | `MotorCommutation_UsesHallInputs` | Returnerer om modulen bruker hall-modus. |
| 21 | `#endif` | Avslutter include guard. |

## Core/Src/motor_commutation.c

| Linje(r) | Kode/blokk | Forklaring |
|---:|---|---|
| 1-5 | Includes | Henter egen API, hall-filter, UART-print og PWM-kontroll. |
| 7-25 | Duty/RPM/kommando-konstanter | Definerer start-duty, motor-spesifikke biasverdier, referanse-RPM, referansehastighet, target, maks duty, RPM-kontrollintervall, slew-steg og timeout. |
| 27-47 | Sektor/modus/button-konstanter | Definerer sektoroffset for retninger, polaritet, hall/sweep-modus, runtime-print, hvilke motorer som er aktivert, og PB12-knappeparametre. |
| 48-52 | `MotorCommutation_MotorIndex` | Intern motorindeks for motor 1, motor 2 og antall motorer. |
| 54-57 | `MotorCommutation_Direction` | Intern enum for forover/revers. |
| 59-60 | Motorretninger | Setter normal retning for M1 og M2. |
| 62-63 | Funksjonspekere | Definerer typer for PWM-konfigurasjon og fasekontroll. |
| 65-69 | `MotorCommutation_PhasePattern` | Beskriver U/V/W-fasenes tilstand for ett kommuteringstrinn. |
| 71-76 | `MotorCommutation_MotorConfig` | Kobler motorens navn, hall-indeks og PWM-funksjoner sammen. |
| 78-98 | `MotorCommutation_Runtime` | All intern tilstand per motor: duty, RPM, hallhistorikk, sweep, kommando, target RPM, retning og slew-timing. |
| 100-106 | `MotorCommutation_SweepVariant` | Beskriver test/sweep-varianter for kanalrekkefolge og stepretning. |
| 108-115 | `kMotorCommutationTable` | Seks-trinns kommuteringstabell: hver sektor bestemmer hvilke faser som er high, low eller flytende. |
| 117-120 | `kMotorCommutationMotorConfig` | Kobler M1 til TIM1/PWM og M2 til TIM8/PWM. |
| 122-125 | `kMotorCommutationMotorEnabled` | Forteller om motor 1 og motor 2 er kompilert aktivert. |
| 127-140 | `kMotorCommutationSweepVariants` | Testtabell for aa prove ulike kanalrekkefolger og retninger i sweep-modus. |
| 142-148 | `kMotorCommutationHallVariant` | Standard kanalvariant for hall-basert kommutering. |
| 150-156 | Globale runtime-variabler | Holder per-motor runtime, global enabled-status, toggle-request og PB12-knappetilstand. |
| 158-169 | Private prototyper | Deklarerer hjelpefunksjoner som brukes foer de er definert. |
| 171-181 | `MotorCommutation_ClampCommand` | Begrenser en kommando til `-100..100`. |
| 183-186 | `MotorCommutation_HasOppositeSign` | Sjekker om to kommandoer har motsatt fortegn, brukt for aa rampe via null ved retningsbytte. |
| 188-206 | `MotorCommutation_StepToward` | Flytter gjeldende kommando ett slew-steg mot target uten aa passere target. |
| 208-223 | `MotorCommutation_ApplyPolarityVariant` | Inverterer high/low hvis polaritetsinvertering er aktivert. |
| 225-232 | `MotorCommutation_DisableMotorOutputs` | Setter alle tre PWM-faser for en motor til flytende og duty `0`. |
| 234-241 | `MotorCommutation_DisableAllOutputs` | Kaller disable for begge motorer. |
| 243-260 | `MotorCommutation_TryGetMotorIndexFromHallIndex` | Oversetter hall-filterets motorindeks til motor_commutation sin interne motorindeks. |
| 262-291 | `MotorCommutation_ResetRuntimeState` | Setter en motors runtime tilbake til trygg starttilstand, inkludert duty, RPM, hallhistorikk, kommandoer og sweep-data. |
| 293-330 | `MotorCommutation_ApplyAppliedCommand` | Tar en faktisk kommando, setter retning/target RPM, starter duty hvis motoren nettopp ble aktivert, eller nullstiller runtime ved kommando `0`. |
| 332-352 | `MotorCommutation_ProcessCommandSlew` | Ramper `command_percent` mot `requested_percent` hvert 50 ms. Ved retningsbytte tvinges target foerst til `0`. |
| 354-415 | `MotorCommutation_ProcessRpmControl` | Enkel RPM-regulator: timeouter RPM hvis hall ikke oppdateres, beregner feedforward-duty, legger paa P-ledd, bias og slew-begrensning, og returnerer om duty endret seg. |
| 417-428 | `MotorCommutation_GetStartDutyPercent` | Returnerer start-duty. M2 og M1 revers har egne startverdier. |
| 430-441 | `MotorCommutation_GetDutyBiasPercent` | Returnerer duty-bias, forelopig bare M2 har bias. |
| 443-450 | `MotorCommutation_GetRpmDutySlewStepPercent` | Returnerer hvor raskt duty kan endres i RPM-reguleringen. M1 revers har tregere steg. |
| 452-467 | `MotorCommutation_GetSectorOffset` | Returnerer sektoroffset basert paa motor og kommando-retning. Dette bestemmer hvilken kommuteringssektor som skal brukes relativt til hall-sektoren. |
| 469-488 | `MotorCommutation_ProcessSweepRamp` | I sweep-modus rampes duty opp mot target med fast intervall. Returnerer om duty endret seg. |
| 490-499 | `MotorCommutation_GetCommandSector` | Regner hall-sektor + offset om til kommando-sektor `1..6`. Ugyldig hall-sektor gir `0`. |
| 501-511 | `MotorCommutation_ApplyPattern` | Wrapper som bruker standard hall-variant naar et fasemoenster skal brukes. |
| 513-540 | `MotorCommutation_ApplyPatternWithVariant` | Setter U/V/W-fasene via PWM-funksjonen for valgt motor, kanalvariant og duty. Kan skrive runtime debug hvis aktivert. |
| 542-553 | `MotorCommutation_ApplySector` | Validerer sektor. Ugyldig sektor slukker motorutganger, gyldig sektor henter moenster fra kommuteringstabellen. |
| 555-611 | `MotorCommutation_ProcessMotor1Sweep` | Testmodus for motor 1: slukker M2, steger gjennom sweep-varianter, setter fast duty og skriver variant/steg til UART. |
| 613-653 | `MotorCommutation_SetEnabled` | Slur motorstyring av eller paa. Ved av: slukker alle utganger. Ved paa: resetter runtime og starter enten sweep-modus eller hall-modus. |
| 655-688 | `MotorCommutation_ProcessButton` | Kompileres bare hvis PB12-knappen er aktivert. Leser/debouncer PB12 og toggler motorene ved godkjent trykk. |
| 690-733 | `MotorCommutation_ProcessMotor` | Periodisk motorprosess: sjekker om motor er aktiv, prosesserer command slew, slukker ved ingen kommando, henter siste hall-state, oppdaterer RPM-duty og re-appliserer sektor ved duty-endring. |
| 735-779 | `MotorCommutation_OnHallStateAccepted` | Kalles fra hall-filter ved ny hall-tilstand. Oppdaterer hall/RPM-data, beregner kommando-sektor og setter nytt PWM-moenster. |
| 781-817 | `MotorCommutation_Init` | Aktiverer DWT-syklusteller, konfigurerer PWM for begge motorer, slukker utganger, resetter runtime og skriver oppstartsinfo. |
| 819-845 | `MotorCommutation_Process` | Global prosessfunksjon: haandterer toggle-request, eventuell PB12-knapp, returnerer hvis disabled, ellers kjoerer sweep eller prosesserer begge motorer. |
| 847-861 | `MotorCommutation_SetDuty` | Setter samme duty for begge motorer etter begrensning til `0..100`. |
| 863-875 | `MotorCommutation_SetWheelCommands` | Setter oensket kommando for motor 1 og 2, eller `0` hvis motoren er deaktivert i konfigurasjonen. |
| 877-880 | `MotorCommutation_ToggleEnabled` | Setter bare et flagg. Faktisk toggle skjer senere i `MotorCommutation_Process`, som er tryggere enn aa toggle direkte fra knapp/joystick-kontekst. |
| 882-903 | `MotorCommutation_ForceStop` | Tvinger motorstyringen av, nullstiller alle kommandoer/RPM/duty/halltiming og slukker alle PWM-utganger. Brukes ved overtemperatur. |
| 905-910 | `MotorCommutation_GetMotor1SpeedKmh` | Regner motor 1 RPM om til km/t basert paa referansehastighet og referanse-RPM. |
| 912-917 | `MotorCommutation_GetMotor2SpeedKmh` | Regner motor 2 RPM om til km/t. |
| 919-941 | `MotorCommutation_GetVehicleSpeedKmh` | Henter begge motorhastigheter, setter fortegn etter retning, nuller inaktive motorer og returnerer gjennomsnitt. |
| 943-946 | `MotorCommutation_IsEnabled` | Returnerer global enabled-status. |
| 948-951 | `MotorCommutation_UsesHallInputs` | Returnerer `1` hvis kommuteringen er satt til hall-modus. |

## Forenklet flyt for motor_commutation

```text
Joystick
|
`-- MotorCommutation_SetWheelCommands(M1, M2)
    |
    `-- Lagre requested_percent per motor

Periodisk:
|
`-- MotorCommutation_Process
    |
    |-- Hvis toggle requested: bytt enabled on/off
    |-- Hvis disabled: returner
    |
    |-- Hvis sweep-modus:
    |   `-- ProcessMotor1Sweep
    |
    `-- Hall-modus:
        |-- ProcessMotor M1
        `-- ProcessMotor M2

Ved ny hall-state:
|
`-- MotorCommutation_OnHallStateAccepted
    |
    |-- Sjekk at hall-modus, enabled og kommando er aktiv
    |-- Beregn RPM fra syklustid mellom hall-overganger
    |-- Regn hall-sektor + offset til kommando-sektor
    `-- Sett PWM-fasemoenster for sektoren

Ved overtemperatur:
|
`-- MotorCommutation_ForceStop
    |
    |-- Disable global enabled
    |-- Nullstill kommandoer, duty, RPM og halltiming
    `-- Slukk alle PWM-utganger
```

## motor_temperature_safety

`Core/Src/motor_temperature_safety.c` passer paa motortemperaturene. Modulen setter feilflagg hvis en motor naar 80 C eller mer, stopper motorstyringen ved aktiv feil, og slipper feilen igjen naar temperaturen synker til 70 C eller lavere. Dette gir hysterese, slik at feilen ikke flapper av og paa rundt samme temperaturgrense.

## Core/Inc/motor_temperature_safety.h

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#ifndef INC_MOTOR_TEMPERATURE_SAFETY_H_` | Starter include guard, slik at headeren ikke inkluderes flere ganger i samme kompilering. |
| 2 | `#define INC_MOTOR_TEMPERATURE_SAFETY_H_` | Definerer guard-navnet som linje 1 sjekker. |
| 3 | Tom linje | Brukes kun for lesbarhet. |
| 4 | `#include "main.h"` | Henter STM32/HAL-typer, blant annet `uint8_t`. |
| 5 | Tom linje | Brukes kun for lesbarhet. |
| 6 | `#define MOTOR_TEMPERATURE_FAULT_M1 (1U << 0)` | Definerer bitflagget for overtemperaturfeil paa motor 1. |
| 7 | `#define MOTOR_TEMPERATURE_FAULT_M2 (1U << 1)` | Definerer bitflagget for overtemperaturfeil paa motor 2. |
| 8 | Tom linje | Brukes kun for lesbarhet. |
| 9 | `void MotorTemperatureSafety_Process(float motor1_temp_c, float motor2_temp_c);` | Deklarerer prosessfunksjonen som sjekker temperaturene og stopper motorene ved feil. |
| 10 | `uint8_t MotorTemperatureSafety_GetFaults(void);` | Deklarerer getter for aktive temperaturfeilflagg. LCD-status bruker dette til aa vise riktig feilmelding. |
| 11 | Tom linje | Brukes kun for lesbarhet. |
| 12 | `#endif /* INC_MOTOR_TEMPERATURE_SAFETY_H_ */` | Avslutter include guard. |

## Core/Src/motor_temperature_safety.c

| Linje | Kode | Forklaring |
|---:|---|---|
| 1 | `#include "motor_temperature_safety.h"` | Inkluderer egen header, slik at kildefilen matcher den offentlige API-en. |
| 2 | Tom linje | Brukes kun for lesbarhet. |
| 3 | `#include "motor_commutation.h"` | Henter `MotorCommutation_ForceStop()`, som brukes for aa stoppe motorutgangene ved overtemperatur. |
| 4 | `#include "myprint.h"` | Henter UART-printfunksjonen som skriver feilmelding naar en ny temperaturfeil oppstaar. |
| 5 | Tom linje | Brukes kun for lesbarhet. |
| 6 | `#define MOTOR_TEMPERATURE_LIMIT_C 80.0f` | Setter grensen for naar overtemperaturfeil skal aktiveres. |
| 7 | `#define MOTOR_TEMPERATURE_CLEAR_C 70.0f` | Setter grensen for naar en aktiv temperaturfeil kan fjernes igjen. |
| 8 | Tom linje | Brukes kun for lesbarhet. |
| 9 | `static uint8_t s_motorTemperatureFaults = 0U;` | Lagrer aktive feilflagg privat i modulen. Bit 0 er motor 1, bit 1 er motor 2. |
| 10 | Tom linje | Brukes kun for lesbarhet. |
| 11 | `static uint8_t MotorTemperatureSafety_IsTemperatureValid(float temperature_c)` | Starter en privat hjelpefunksjon som vurderer om en temperaturverdi er gyldig. |
| 12 | `{` | Aapner funksjonsblokken. |
| 13 | `return (temperature_c > -200.0f) ? 1U : 0U;` | Returnerer `1` for gyldig temperatur. Verdier paa `-200 C` eller lavere tolkes som ugyldige maalinger. |
| 14 | `}` | Avslutter hjelpefunksjonen. |
| 15 | Tom linje | Brukes kun for lesbarhet. |
| 16 | `void MotorTemperatureSafety_Process(float motor1_temp_c, float motor2_temp_c)` | Starter prosessfunksjonen som kalles periodisk fra default task. |
| 17 | `{` | Aapner funksjonsblokken. |
| 18 | `uint8_t new_faults = s_motorTemperatureFaults;` | Kopierer eksisterende feilstatus til en lokal variabel som kan oppdateres. |
| 19 | Tom linje | Brukes kun for lesbarhet. |
| 20 | `if (MotorTemperatureSafety_IsTemperatureValid(motor1_temp_c) != 0U) {` | Sjekker motor 1 bare hvis temperaturverdien er gyldig. |
| 21 | `if (motor1_temp_c >= MOTOR_TEMPERATURE_LIMIT_C) {` | Sjekker om motor 1 har naadd eller passert 80 C. |
| 22 | `new_faults |= MOTOR_TEMPERATURE_FAULT_M1;` | Setter feilflagget for motor 1 uten aa endre andre feilflagg. |
| 23 | `} else if (motor1_temp_c <= MOTOR_TEMPERATURE_CLEAR_C) {` | Hvis motor 1 er nede paa 70 C eller lavere, kan M1-feilen fjernes. |
| 24 | `new_faults &= (uint8_t)~MOTOR_TEMPERATURE_FAULT_M1;` | Fjerner feilflagget for motor 1. Casten holder resultatet som `uint8_t`. |
| 25 | `}` | Avslutter temperaturgrense-sjekken for motor 1. |
| 26 | `}` | Avslutter gyldighetssjekken for motor 1. |
| 27 | Tom linje | Brukes kun for lesbarhet. |
| 28 | `if (MotorTemperatureSafety_IsTemperatureValid(motor2_temp_c) != 0U) {` | Sjekker motor 2 bare hvis temperaturverdien er gyldig. |
| 29 | `if (motor2_temp_c >= MOTOR_TEMPERATURE_LIMIT_C) {` | Sjekker om motor 2 har naadd eller passert 80 C. |
| 30 | `new_faults |= MOTOR_TEMPERATURE_FAULT_M2;` | Setter feilflagget for motor 2 uten aa endre andre feilflagg. |
| 31 | `} else if (motor2_temp_c <= MOTOR_TEMPERATURE_CLEAR_C) {` | Hvis motor 2 er nede paa 70 C eller lavere, kan M2-feilen fjernes. |
| 32 | `new_faults &= (uint8_t)~MOTOR_TEMPERATURE_FAULT_M2;` | Fjerner feilflagget for motor 2. |
| 33 | `}` | Avslutter temperaturgrense-sjekken for motor 2. |
| 34 | `}` | Avslutter gyldighetssjekken for motor 2. |
| 35 | Tom linje | Brukes kun for lesbarhet. |
| 36 | `if (new_faults != 0U) {` | Hvis minst ett feilflagg er aktivt, skal motorstyringen stoppes. |
| 37 | `if (s_motorTemperatureFaults == 0U) {` | Sjekker om dette er overgangen fra ingen feil til aktiv feil. |
| 38 | `MyPrint_Print("Motor stop: overtemperature fault\r\n");` | Skriver en UART-melding bare naar feilen foerst oppstaar, ikke hver gang prosessfunksjonen kalles. |
| 39 | `}` | Avslutter sjekken for ny feil. |
| 40 | `MotorCommutation_ForceStop();` | Tvinger motorstyringen av og slukker PWM-utgangene. |
| 41 | `}` | Avslutter aktiv-feil-blokken. |
| 42 | Tom linje | Brukes kun for lesbarhet. |
| 43 | `s_motorTemperatureFaults = new_faults;` | Lagrer oppdatert feilstatus tilbake i modulens statiske variabel. |
| 44 | `}` | Avslutter `MotorTemperatureSafety_Process()`. |
| 45 | Tom linje | Brukes kun for lesbarhet. |
| 46 | `uint8_t MotorTemperatureSafety_GetFaults(void)` | Starter getter-funksjonen for aktive temperaturfeil. |
| 47 | `{` | Aapner funksjonsblokken. |
| 48 | `return s_motorTemperatureFaults;` | Returnerer bitmasken med aktive temperaturfeil. |
| 49 | `}` | Avslutter getter-funksjonen. |

## Forenklet flyt for motor_temperature_safety

```text
MotorTemperatureSafety_Process
|
|-- Start med forrige feilstatus
|
|-- Hvis motor 1 temperatur er gyldig:
|   |-- Hvis >= 80 C: sett M1-feil
|   `-- Hvis <= 70 C: fjern M1-feil
|
|-- Hvis motor 2 temperatur er gyldig:
|   |-- Hvis >= 80 C: sett M2-feil
|   `-- Hvis <= 70 C: fjern M2-feil
|
|-- Hvis minst en feil er aktiv:
|   |-- Skriv UART-feil hvis dette er ny feil
|   `-- Tving motorstyring av
|
`-- Lagre ny feilstatus
```
