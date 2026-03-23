# Flytskjema

Dette dokumentet er et arbeidsutkast for videre systemdesign. Fra nå av beskriver det ønsket flyt, ikke bare dagens implementasjon.

## Retning videre

Vi legger til grunn denne arkitekturen:

- Hall-lesing skal være IRQ-basert i normal drift.
- Hall-polling beholdes kun som feilsøkingsspor.
- Joystick skal polles hvert `50 ms`.
- Dødmansknapp må holdes inne for at motorene skal få lov til å levere moment.
- Display skal vise minst hastighet og ladning.
- Dokumentet bruker SDL-lignende tekstlig flyt med beskrivelser per prosess.

## Systemoversikt

```text
SYSTEM BachelorMotorControl

  STARTUP
    -> init clock
    -> init GPIO / DMA / TIM / ADC / UART / display
    -> init PWM / motor control / hall / joystick / battery measurement
    -> start scheduler

  RUNTIME
    -> hall IRQ driver kommutering
    -> joystick task oppdaterer ønsket hastighet hver 50 ms
    -> deadman input avgjør om motoren er tillatt aktiv
    -> display task oppdaterer UI
    -> battery task måler og filtrerer ladning
    -> debug polling kan brukes ved feilsøking
```

## Tegnet flytskjema

Dette er en første visuell skisse av systemet slik jeg ser det for meg nå.

```text
+------------------+
|      START       |
+------------------+
          |
          v
+------------------+
| Init maskinvare  |
| Clock / GPIO     |
| DMA / TIM / ADC  |
| UART / Display   |
+------------------+
          |
          v
+------------------+
| Init moduler     |
| PWM              |
| Hall             |
| Joystick         |
| Battery          |
| Display          |
+------------------+
          |
          v
+------------------+
| Start FreeRTOS   |
| og tasks         |
+------------------+
          |
          v
+------------------------------------------------------+
|                    RUNTIME                           |
+------------------------------------------------------+
    |                    |                  | 
    |                    |                  |
    v                    v                  v
+-----------+     +-------------+    +---------------+
| Hall IRQ  |     | Joystick    |    | Battery task  |
| fra EXTI  |     | poll 50 ms  |    | ADC + filter  |
+-----------+     +-------------+    +---------------+
    |                    |                  |
    v                    v                  v
+-----------+     +-------------+    +---------------+
| Snapshot  |     | Les akser   |    | Beregn        |
| Hall A/B/C|     | + knapper   |    | ladning       |
+-----------+     +-------------+    +---------------+
    |                    |                  |
    v                    v                  v
+-----------+     +-------------+    +---------------+
| Gyldig    |     | Lag ønsket  |    | Publiser      |
| hallkode? |     | hastighet   |    | batteristatus |
+-----------+     +-------------+    +---------------+
    |                    |                  |
    | nei                |                  |
    v                    |                  |
+-----------+            |                  |
| Forkast / |            |                  |
| debug-logg|            |                  |
+-----------+            |                  |
    |                    |                  |
    +--------------------+--------+---------+
                                 |
                                 v
                     +-------------------------+
                     |    Motor Supervisor     |
                     | hastighet + battery +   |
                     | feil + deadman-status   |
                     +-------------------------+
                                 |
                                 v
                     +-------------------------+
                     | Er deadman aktiv?       |
                     | Er systemet klart?      |
                     +-------------------------+
                          | ja           | nei
                          |              |
                          v              v
                +----------------+   +------------------+
                | Tillat moment  |   | Safe off         |
                | / duty-grense  |   | PWM av / stopp   |
                +----------------+   +------------------+
                          |
                          v
                +-------------------------+
                | Hall-basert             |
                | kommutering             |
                | U/V/W -> PWM TIM1       |
                +-------------------------+
                          |
                          v
                +-------------------------+
                | Motor går / oppdaterer  |
                | hastighetstilstand      |
                +-------------------------+


Parallelt i runtime:

+------------------+        +----------------------+
| Deadman knapp    | -----> | DeadmanSafety        |
| press / release  |        | Armed / NotArmed     |
+------------------+        | EmergencyStop        |
                            +----------------------+
                                       |
                                       v
                            +----------------------+
                            | Motor Supervisor     |
                            +----------------------+

+------------------+        +----------------------+
| Display task     | <----- | Hastighet / ladning  |
| periodisk        |        | status / deadman     |
+------------------+        +----------------------+

+------------------+        +----------------------+
| Hall polling     | -----> | Kun debug /          |
| feilsøking       |        | verifikasjon         |
+------------------+        +----------------------+
```

## Enkel operativ flyt

Hvis vi tegner det helt enkelt, er hovedideen denne:

```text
Joystick ----+
             |
Deadman -----+----> Motor Supervisor ----> Tillat / ikke tillat moment
             |                                   |
Battery -----+                                   v
                        Hall IRQ -----------> Kommutering ----------> PWM
                                                                   |
                                                                   v
                                                                 Motor

Motor + Battery + Deadman status ------> Display
```

## Hvordan jeg ser for meg ansvaret

- `Hall IRQ` bestemmer når kommuteringen må oppdateres.
- `Joystick` bestemmer ønsket hastighet.
- `Deadman` bestemmer om hastighet i det hele tatt får lov til å bli brukt.
- `Motor Supervisor` er beslutningspunktet.
- `Commutation` gjør beslutningen om til faktiske PWM-signaler.
- `Display` viser resultatet og tilstanden til systemet.

## SDL-flyt for oppstart

```text
PROCESS Startup
  STATE Reset
    ACTION HAL_Init
    ACTION SystemClock_Config
    ACTION MX_GPIO_Init
    ACTION MX_DMA_Init
    ACTION MX_TIM1_Init
    ACTION MX_TIM8_Init
    ACTION MX_ADC1_Init
    ACTION MX_ADC2_Init
    NEXTSTATE DriversReady

  STATE DriversReady
    ACTION App_Init
    DESCRIPTION Initialiserer applikasjonsmoduler og gjør systemet klart før scheduler starter.
    NEXTSTATE RTOSReady

  STATE RTOSReady
    ACTION osKernelInitialize
    ACTION MX_FREERTOS_Init
    ACTION osKernelStart
    NEXTSTATE Running
ENDPROCESS
```

## SDL-flyt for hall og kommutering

Dette er hovedflyten vi bør styre etter videre.

```text
PROCESS HallAndCommutation
  STATE WaitForHallEdge
    INPUT Hall IRQ from EXTI
    DESCRIPTION En hall-endring skal være det primære signalet som driver motorens elektriske sektor.
    NEXTSTATE CaptureHallState

  STATE CaptureHallState
    ACTION Les Hall A/B/C umiddelbart
    ACTION Lagre timestamp og rå hall-kode
    DESCRIPTION Snapshot tas i eller tett ved IRQ for å unngå at nivåene endrer seg før de behandles.
    NEXTSTATE ValidateHallState

  STATE ValidateHallState
    DECISION Er hall-koden gyldig?
    IF invalid
      ACTION forkast kode
      ACTION eventuelt logg debug
      NEXTSTATE WaitForHallEdge
    IF valid
      NEXTSTATE DetermineSector

  STATE DetermineSector
    ACTION Map hall-kode til sektor 1..6
    ACTION Kontroller at overgang er lovlig naboovergang
    DESCRIPTION Hindrer hopp mellom ikke-nabo-sektorer.
    NEXTSTATE ComputeCommand

  STATE ComputeCommand
    ACTION Finn command_sector ut fra hall_sector og sektoroffset
    ACTION Slå opp fase-tabell og fasekart
    DESCRIPTION Her testes fortsatt offset og fasekobling systematisk.
    NEXTSTATE ApplyPWM

  STATE ApplyPWM
    ACTION Sett U/V/W til HIGH, LOW eller FLOAT
    ACTION Oppdater TIM1-kanaler
    DESCRIPTION Dette er selve 6-step-kommuteringen.
    NEXTSTATE WaitForHallEdge
ENDPROCESS
```

## SDL-flyt for hall-feilsøking

Polling beholdes, men skal ikke være primær styring i sluttløsningen.

```text
PROCESS HallDebugPolling
  STATE Idle
    TIMER every 50 ms or slower
    DESCRIPTION Brukes kun når IRQ-flyten må verifiseres mot faktiske pinnivåer.
    NEXTSTATE SamplePins

  STATE SamplePins
    ACTION Les Hall A/B/C direkte fra GPIO
    ACTION Sammenlign med forrige verdi
    IF changed
      ACTION skriv debug-logg
    NEXTSTATE Idle
ENDPROCESS
```

## SDL-flyt for joystick

Joystick er tenkt som operatørens kommandokilde.

```text
PROCESS JoystickControl
  STATE Idle
    TIMER every 50 ms
    NEXTSTATE ReadJoystick

  STATE ReadJoystick
    ACTION Les joystick-akse(r) og eventuelle knapper
    ACTION Filtrer eller glatte input ved behov
    DESCRIPTION Polling hvert 50 ms er tregt nok til å være stabilt og raskt nok for brukerinput.
    NEXTSTATE InterpretCommand

  STATE InterpretCommand
    ACTION Konverter joystick-posisjon til ønsket hastighet og eventuell retning
    ACTION Påfør grenser, deadband og sikkerhetsregler
    DESCRIPTION Denne prosessen skal ikke kommutere direkte, bare oppdatere ønsket settpunkt når sikkerhetsbetingelsene tillater det.
    NEXTSTATE PublishSetpoint

  STATE PublishSetpoint
    ACTION Oppdater delt variabel eller kø med ønsket hastighet
    ACTION Varsle motorstyring hvis nødvendig
    NEXTSTATE Idle
ENDPROCESS
```

## SDL-flyt for dødmansknapp

Dødmansknappen er et eksplisitt sikkerhetskrav og bør behandles som en egen prosess.

```text
PROCESS DeadmanSafety
  STATE NotArmed
    INPUT deadman button released
    ACTION sett motor_tillatelse = false
    ACTION sett ønsket hastighet = 0
    DESCRIPTION Uten aktiv dødmansknapp skal motoren ikke levere moment.
    NEXTSTATE WaitForPress

  STATE WaitForPress
    INPUT deadman button pressed
    ACTION verifiser at systemet er klart og uten feil
    IF not ready
      NEXTSTATE NotArmed
    IF ready
      NEXTSTATE Armed

  STATE Armed
    ACTION sett motor_tillatelse = true
    DESCRIPTION Så lenge knappen holdes inne kan joystick få lov til å be om hastighet.
    INPUT deadman button released
    NEXTSTATE EmergencyStop

  STATE EmergencyStop
    ACTION sett ønsket hastighet = 0
    ACTION deaktiver moment umiddelbart
    ACTION sett PWM til sikker av eller frihjul etter valgt strategi
    ACTION oppdater status for display og feillogg
    NEXTSTATE NotArmed
ENDPROCESS
```

## SDL-flyt for motorstyring på høyt nivå

```text
PROCESS MotorSupervisor
  STATE Idle
    INPUT ønsket hastighet fra joystick
    DESCRIPTION Holder oversikt over om motoren skal stå stille, starte, akselerere eller stoppe.
    NEXTSTATE EvaluateRequest

  STATE EvaluateRequest
    DECISION Er deadman aktiv og ønsket hastighet forskjellig fra 0?
    IF no
      ACTION sett PWM til sikker av
      NEXTSTATE Idle
    IF yes
      ACTION sett duty-grense og ønsket retning
      NEXTSTATE Run

  STATE Run
    DESCRIPTION Selve kommuteringen drives av Hall IRQ, men supervisor bestemmer om motoren får lov til å levere moment.
    ACTION Oppdater duty eller momentgrenser
    ACTION Overvåk feil, batteri, deadman og stoppsignal
    NEXTSTATE Idle or Run
ENDPROCESS
```

## SDL-flyt for display

Displayet er en egen prosess og bør ikke være koblet tett mot ISR.

```text
PROCESS DisplayUpdate
  STATE Wait
    TIMER every 100 ms
    NEXTSTATE CollectData

  STATE CollectData
    ACTION Les siste beregnede hastighet
    ACTION Les siste batterinivå / ladning
    ACTION Les eventuell status som feil, retning, driftstilstand og deadman-status
    NEXTSTATE Render

  STATE Render
    ACTION Oppdater display
    DESCRIPTION UI skal vise brukerrelevant informasjon, ikke rå debug-data som standard.
    NEXTSTATE Wait
ENDPROCESS
```

## SDL-flyt for batteri og ladning

```text
PROCESS BatteryMonitor
  STATE Wait
    TIMER every 100 ms to 500 ms
    NEXTSTATE SampleBattery

  STATE SampleBattery
    ACTION Les ADC-verdi for batteri
    ACTION Filtrer målingen
    ACTION Regn om til spenning og prosent ladning
    NEXTSTATE PublishBattery

  STATE PublishBattery
    ACTION Lagre siste batteriverdi for display og sikkerhetslogikk
    ACTION Sett lavspenningsvarsel ved behov
    NEXTSTATE Wait
ENDPROCESS
```

## Beskrivelse av ansvar per delsystem

- `Hall IRQ`
  Primær kilde for rotorposisjon i normal drift.
- `Hall polling`
  Sekundær mekanisme for feilsøking og verifikasjon.
- `Joystick`
  Setter ønsket hastighet og eventuelt retning, men styrer ikke fasene direkte.
- `Deadman safety`
  Må gi eksplisitt tillatelse før motoren får levere moment.
- `Motor supervisor`
  Binder sammen joystick, batteri, feil og tillatt moment/duty.
- `Commutation`
  Gjør om hall-sektor og tillatt moment til faktiske PWM-tilstander.
- `Display`
  Viser hastighet, ladning og status på en rolig og periodisk måte.
- `Battery monitor`
  Leverer filtrert batteritilstand til display og sikkerhetslogikk.

## Kommentar om dagens kodebase

Dagens kode er ikke helt i denne målarkitekturen ennå:

- `HallStateFilter_Process()` er i dag polling-basert.
- `HallDebug_OnExti()` finnes allerede som IRQ-spor.
- `MotorCommutation_Process()` kjører i task-kontekst.

Det betyr at neste naturlige steg i koden blir å flytte hall-kjeden nærmere denne modellen:

1. EXTI fanger hall-endring.
2. Hall-state snapshot lagres raskt.
3. Task-kontekst behandler snapshot sikkert.
4. Kommutering oppdateres fra den IRQ-drevne hendelsen.

## Neste beskrivelser vi kan legge til

- En egen SDL-seksjon for nødstopp og feiltilstander.
- En tydelig policy for hva `safe off` betyr: brems, frihjul eller definert stopprampe.
- En datamodell for delte variabler mellom ISR, motor-task og display-task.
- En tabell for `hall_code -> hall_sector -> command_sector -> U/V/W`.
- En konkret task-plan for FreeRTOS med foreslåtte perioder og prioriteringer.
