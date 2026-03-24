# Endringslogg

Dette dokumentet inneholder en oversikt over endringer utført i prosjektet, inkludert hvilke filer som ble endret og en kort beskrivelse av endringen.

Alle fremtidige kodeendringer skal loggføres her med tidspunkt, filer, hva som ble endret, hvorfor det ble endret, og teststatus.

| Dato | Fil | Endring |
| :--- | :--- | :--- |
| 2026-03-02 | log.md | Opprettet loggfil for sporing av endringer. |
| 2026-03-02 | agents.md | Oppdatert med nye regler for rolle, scope, arbeidsflyt og koderelasjoner. |
| 2026-03-02 | Core/Src/pwm_control.c, Core/Inc/pwm_control.h | Opprettet nye filer for PWM-styring og modulert PWM-funksjonalitet. |
| 2026-03-02 | Core/Src/main.c | Oppdatert til å bruke pwm_control-modulen. |
| 2026-03-04 | Core/Src/pwm_control.c, Core/Inc/pwm_control.h, Core/Src/main.c | Lagt til init/startfunksjoner for begge motorer, dead-time-konfig og testsekvens for duty. |
| 2026-03-11 | Core/Src/myapp.c, Core/Inc/myapp.h, Core/Src/main.c | Refaktorert initialisering til myapp-modulen. Planlagt motoroppsett. |
| 2026-03-11 | Core/Src/pwm_control.c, Core/Inc/pwm_control.h, Core/Src/myapp.c | Implementert maskinvaretest: 3-fase PWM (20 kHz, 50 % duty, 120 grader faseforskyvning) med deadtime (1 us). |
| 2026-03-02 | Core/Src/* | Utført gjennomgang av koden for å forstå prosjektets struktur og funksjonalitet. |

### Endring 2026-03-18 15:28
- **Files changed:** Bachelor/AGENTS.md, Bachelor/agents.md, Bachelor/log.md
- **What/Why:** Oppdatert agent-instruksene slik at de eksplisitt gjelder som faste instrukser for hele repoet. Strammet inn arbeidsflytkrav slik at alle kodeendringer alltid skal føres i `log.md`, også små endringer. La også til `AGENTS.md` som peker til den vedlikeholdte instruksjonsfilen for bedre kompatibilitet med agentverktøy som forventer dette filnavnet.
- **Build/Test result:** Ikke kjørt. `git status` kunne ikke kjøres på grunn av Git `dubious ownership` for dette repoet i nåværende miljø.

### Endring 2026-03-18 15:46
- **Files changed:** Bachelor/Core/Inc/pwm_control.h, Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Src/myapp.c, Bachelor/log.md
- **What/Why:** Erstattet TIM1-maskinvaretesten i `App_Init()` med en konkret runtime-konfigurasjon for 3-fase komplementær PWM. TIM1 settes opp med center-aligned mode 1, PWM mode 1 på CH1/CH2/CH3, komplementære utganger CH1N/CH2N/CH3N, 20 kHz og 400 ns dead-time. Init-kallet er lagt i `myapp.c` som ønsket.
- **Build/Test result:** Ikke kjørt. `git status` kunne fortsatt ikke kjøres på grunn av Git `dubious ownership` i dette miljøet.

### Endring 2026-03-18 16:02
- **Files changed:** Bachelor/Core/Inc/pwm_control.h, Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Src/myapp.c, Bachelor/log.md
- **What/Why:** La til separat duty-styring for TIM1 fase A/B/C og en enkel 3-fase åpen-loop modulasjon i `App_Run()`. Dette gir 120 graders faseforskyvning mellom fase-referansene til CH1, CH2 og CH3, mens TIM1 fortsatt kjører felles center-aligned carrier med komplementære utganger og dead-time.
- **Build/Test result:** Ikke kjørt. `git status` kunne fortsatt ikke kjøres på grunn av Git `dubious ownership` i dette miljøet.

### Endring 2026-03-11 11:15
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Inc/pwm_control.h, Bachelor/Core/Src/myapp.c
- **What/Why:** Implementert `PWM_HardwareTest_3Phase` for å verifisere gatedrivere. Bruker Asymmetric PWM mode på TIM1 for å generere tre 50 % duty-signaler med 120 graders faseforskyvning. Deadtime er satt til 1 us.
- **Build/Test result:** Ikke kjørt.

### Endring 2026-03-11 10:45
- **Files changed:** Bachelor/Core/Src/myapp.c, Bachelor/Core/Inc/myapp.h, Bachelor/Core/Src/main.c
- **What/Why:** Flyttet applikasjonsnivå-initialisering (BSP, PWM, COM) fra `main.c` til `myapp.c` for å holde `main.c` ren og modulær. Introduserte `App_Init()` og `App_Run()` som hovedinngangspunkter for applikasjonen.
- **Build/Test result:** Ikke kjørt.

### Endring 2026-03-04 14:42
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Inc/pwm_control.h, Bachelor/Core/Src/main.c
- **What/Why:** Lagt til `PWM_Init`, startfunksjoner for TIM1/TIM8 med CHx+CHxN, dead-time-konfig (400 ns) og funksjoner for å sette duty på alle tre faser per motor. Oppdatert `main.c` til å initialisere PWM og kjøre enkel testsekvens (10 % i 2 sek, deretter 20 %).
- **Build/Test result:** Ikke kjørt.

### Endring 2026-03-19
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Oppdatert GPIO-basert 3-fasetest slik at både high-side (`PA8/PA9/PA10`) og low-side (`PB13/PB14/PB15`) skrives i hver testtilstand. Low-side er nå eksplisitt komplementær til high-side igjen i softwaretesten.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 2
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Byttet fase B og C i 3-fase testsekvensen for å korrigere fasefølgen relativt til fase A. Dette retter 120 graders forskyvningen når signalene ellers har riktig duty og komplementære low-side-signaler.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 3
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Gjenopprettet den opprinnelige 6-tilstandssekvensen for 50 % duty og 120 graders faseforskyvning. Denne gir stigende flanker ved 0, 1/3 og 2/3 av perioden relativt til fase A.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 4
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Inverterte kun fase B i den GPIO-baserte 3-fasetesten for å korrigere at fase B gikk lav der den skulle gå høy. Fase A ble beholdt uendret for isolert feilsøking av B.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 5
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Justerte kun fase C i testsekvensen slik at den følger samme bølgeform som fase B, men forskjøvet ytterligere 120 grader. Fase A og B ble beholdt uendret.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 6
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Inverterte kun fase B i den GPIO-baserte testsekvensen etter verifikasjon på PCB. Fase B hadde riktig 120 graders tidsplassering, men motsatt polaritet.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 7
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Inverterte kun fase C i den GPIO-baserte testsekvensen etter PCB-verifikasjon. Fase A og B var riktige, mens fase C hadde motsatt polaritet.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 8
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Reduserte duty cycle i den GPIO-baserte 3-fasetesten fra 50 % til 20 % for alle faser. Testsekvensen ble utvidet til 15 tilstander for å beholde 120 graders faseforskyvning samtidig som hver fase bare er høy i 3/15 av perioden.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 9
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Endret den GPIO-baserte 3-fasetesten til 80 % duty på alle faser, med samme 120 graders faseforskyvning.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.

### Endring 2026-03-19 10
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Justerte den GPIO-baserte 3-fasetesten fra 80 % til 66,7 % duty. Ved 120 graders faseforskyvning gir 80 % wrap-around og oppdelte pulser; 66,7 % er høyeste sammenhengende duty med ren fasefølge i denne testen.
- **Build/Test result:** Ikke kjørt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljøet.
### Endring 2026-03-20 12:33
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Gjenopprettet den GPIO-baserte 3-fasetesten til 50 % duty pÃ¥ alle faser. Testtabellen ble redusert fra 15 til 6 tilstander, og oppdateringsfrekvensen ble justert til 120 kHz slik at fasefÃ¸lgen fortsatt gir 20 kHz grunnfrekvens med 120 graders faseforskyvning.
- **Build/Test result:** Ikke kjÃ¸rt. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljÃ¸et.
### Endring 2026-03-20 13:00
- **Files changed:** Bachelor/Core/Inc/stm32g4xx_it.h, Bachelor/Core/Src/stm32g4xx_it.c, Bachelor/log.md
- **What/Why:** Gjenopprettet `TIM1_UP_TIM16_IRQHandler()` for den GPIO-baserte 3-fasetesten. IRQ-prototypen og handleren ble lagt i CubeMX sine `USER CODE`-blokker slik at de ikke blir slettet ved neste regenerering etter endringer i `.ioc`.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 19
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Gjorde sektorforskyvningen i kommuteringen eksplisitt med `MOTOR_COMMUTATION_SECTOR_OFFSET`, og satte ny testkandidat til `0`. UART-debug viser naa baade filtrert hall-sektor og kommando-sektor (`hall`/`cmd`) slik at videre tuning av kommuteringstabellen kan gjores systematisk. Hypotesen er at motoren laaser i neste steg fordi elektrisk kommuteringssektor ligger feil relativt til rotorposisjonen, ikke fordi hall-filteret stopper.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 20
- **Files changed:** Bachelor/Core/Src/app_freertos.c, Bachelor/Core/Src/hall_state_filter.c, Bachelor/log.md
- **What/Why:** Rettet timing rundt hall-basert kommutering. `defaultTask` kjorte tidligere bare hver `10 ms`, mens hall-filteret krevde `3` stabile samples, som i praksis betydde rundt `30 ms` stabil hall-kode før ny sektor ble godtatt. Det er for tregt og forklarer godt hvorfor motoren bare rykker eller laaser. Kontroellsløyfen kjores naa hver `1 ms`, ra `hall_debug`/`hall_probe` er skrudd av under motorstyring for aa unngaa UART-flom, og hall-filteret er justert til `1 ms` sampleperiode og `2` stabile samples.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 21
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Satt TIM1-kommuterings-PWM til `1 kHz` for testing av mykere/lavere switchingfrekvens under hall-basert motorstyring. Dette krevde baade ny periode og ny prescaler i `PWM_TIM1_Configure3PhaseComplementary()`, siden `1 kHz` med center-aligned PWM ikke faar plass i en 16-bit timer med `PSC=0`.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 22
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Reverterte TIM1-kommuterings-PWM fra `1 kHz` tilbake til `20 kHz` etter test uten endring i motoroppforsel. Hensikten er aa holde switchingfrekvensen tilbake paa opprinnelig nivaa og heller fokusere videre feilsoking paa kommuteringstabell, sektoroffset og fasekobling.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 23
- **Files changed:** Bachelor/flytskjema.md, Bachelor/log.md
- **What/Why:** Opprettet et arbeidsdokument med flytskjema for dagens kontrollflyt i prosjektet. Dokumentet kobler sammen `main`, FreeRTOS-tasken, hall-filtrering, kommutering og relevante debugspor, og oppsummerer hva `hall_log.md.txt` antyder om neste steg i feilsokingen.
- **Build/Test result:** Ikke kjort. Dokumentasjonsendring.
### Endring 2026-03-23 18
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Gjorde fase-mappingen i kommuteringsmodulen eksplisitt og byttet fra antatt `U=CH1, V=CH2, W=CH3` til testmapping `U=CH3, V=CH2, W=CH1`. Dette er neste naturlige tuning-steg naar endring av kommuteringsretning alene ikke ga synlig effekt, og hypotesen er at fasekoblingen mellom `U/V/W` og TIM1-kanalene ikke stemte med motorens faktiske ledningsrekkefolge.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 17
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Justerte kommuteringen slik at den bruker nabosektor i motsatt retning i forhold til filtrert hall-sektor. Dette er et vanlig neste steg naar motoren rykker og prover aa gaa feil vei, fordi statorfeltet da sannsynligvis laa paa feil side av rotorposisjonen. Retningsvalget er samlet i `MOTOR_COMMUTATION_REVERSED_DIRECTION` for enkel videre tuning.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 16
- **Files changed:** Bachelor/Core/Inc/pwm_control.h, Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Inc/motor_commutation.h, Bachelor/Core/Src/motor_commutation.c, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** La til en egen `motor_commutation`-modul og utvidet PWM-laget slik at hver TIM1-fase kan settes til `HIGH`, `LOW` eller `FLOAT`. Kommuteringstabellen bruker filtrert hall-sektor `1..6` og anvender en 6-step sekvens paa `U/V/W`, med forelopig mapping `U->TIM1_CH1`, `V->TIM1_CH2`, `W->TIM1_CH3`. Modulen holder duty separat og skriver ut valgt kommuteringstilstand via UART for verifikasjon. `defaultTask`-stacken er fortsatt `512 * 4` bytes fordi hall-filter og kommuteringsdebug bruker mer stack enn standardoppsettet.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 15
- **Files changed:** Bachelor/Core/Inc/hall_state_filter.h, Bachelor/Core/Src/hall_state_filter.c, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** La til en egen `hall_state_filter`-modul som leser Hall 1 som `U=HALL_1_C`, `V=HALL_1_B`, `W=HALL_1_A`, filtrerer bort ugyldige koder (`000`/`111`), krever stabilitet over flere samples, og bare godtar gyldige nabotransisjoner i 6-stegssekvensen. Modulen skriver ut akseptert sektor `1..6` og ren `UVW`-kode, og er ment som mellomledd før kommuteringstabellen kobles på motorstyringen.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 14
- **Files changed:** Bachelor/pins.md, Bachelor/Core/Src/hall_debug.c, Bachelor/Core/Src/hall_probe.c, Bachelor/Core/Inc/stm32g4xx_it.h, Bachelor/Core/Src/stm32g4xx_it.c, Bachelor/Core/Src/gpio.c, Bachelor/log.md
- **What/Why:** Flyttet `HALL_1_A` fra `PC0` til `PC9` i dokumentasjon og hall-debug/probe-koden. La ogsaa til `EXTI9_5_IRQHandler()` og NVIC-enable for `EXTI9_5_IRQn`, slik at `PC9` faktisk kan trigge interrupt sammen med `PC7/PC8`.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 13
- **Files changed:** Bachelor/Core/Inc/hall_probe.h, Bachelor/Core/Src/hall_probe.c, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** La til en egen `hall_probe`-modul som pollet leser `HALL_1_A/B/C` paa `PC0/PC1/PC7` hvert `50 ms` og skriver ut ved nivaaendring. Dette brukes for aa skille mellom et reelt EXTI-problem og et pin-/wiringproblem paa `PC0`, siden `Hall 1 A` fortsatt ikke ga interrupts selv etter bytte av sensor.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 12
- **Files changed:** Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** Oekte stack-storrelsen til `defaultTask` fra `128 * 4` til `512 * 4` bytes. Hall-debugen bruker `MyPrint_Printf()`/`vsnprintf()` i task-kontekst, og den opprinnelige stacken var sannsynligvis for liten naar mange hall-events og UART-prints kom tett ved oppstart.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 11
- **Files changed:** Bachelor/Core/Src/hall_debug.c, Bachelor/log.md
- **What/Why:** Fjernet runtime-rekonfigureringen av Hall-pinnene i `hall_debug`. Modulen leser fortsatt snapshots ved EXTI, men overstyrer ikke lenger `gpio.c` sin oppsettsekvens ved oppstart. Dette ble gjort fordi den nye pin-rekonfigureringen sannsynligvis gjorde oppstarten ustabil i praksis.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 10
- **Files changed:** Bachelor/Core/Src/hall_debug.c, Bachelor/log.md
- **What/Why:** Forbedret hall-debug for motor 1 ved aa la `hall_debug`-modulen re-konfigurere Hall 1-pinnene med `GPIO_PULLUP` og `GPIO_MODE_IT_RISING_FALLING`. I tillegg tas snapshot av `H1_A/B/C` idet EXTI inntreffer, slik at UART-print viser faktisk pinntilstand ved interrupt i stedet for et senere task-les. Dette skal gi mer paalitelige observasjoner, spesielt for `HALL_1_A`.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 9
- **Files changed:** Bachelor/Core/Src/hall_debug.c, Bachelor/log.md
- **What/Why:** Deaktiverte Hall 2-debug i `hall_debug`-modulen mens bare motor 1 er koblet opp. Dette hindrer UART-prints fra ikke-tilkoblet/flytende `PC2/PC3/PC8` i aa forstyrre testing av `HALL_1_A/B/C`. Hall 2 kan aktiveres igjen senere ved aa sette `HALL_DEBUG_ENABLE_MOTOR2` til `1`.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 8
- **Files changed:** Bachelor/Core/Inc/hall_debug.h, Bachelor/Core/Src/hall_debug.c, Bachelor/Core/Inc/myapp.h, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** Flyttet hall-debuglogikken ut av `myapp.c` og inn i en egen `hall_debug`-modul. `myapp` er naa tilbake til aa vaere hovedsakelig init/orchestrering, mens EXTI-callback, eventflagg og UART-debug for hall-signaler ligger samlet i nye filer. De eksisterende systemfilene beholdes bare som tynne hook-punkter der det er nodvendig.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 7
- **Files changed:** Bachelor/pins.md, Bachelor/Core/Inc/myapp.h, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/Core/Inc/stm32g4xx_it.h, Bachelor/Core/Src/stm32g4xx_it.c, Bachelor/Core/Src/gpio.c, Bachelor/log.md
- **What/Why:** Oppdatert pindokumentasjonen med `HALL_1_C = PC7` og `HALL_2_C = PC8`. La til `EXTI9_5_IRQHandler()` og NVIC-enable for `PC7/PC8`, samt debugflyt for hall-signaler: EXTI-callbacken setter eventflagg, og `defaultTask` skriver ut hvilken hall-kanal som trigget og aktuelle `A/B/C`-nivaaer via UART. Dette gir hall-debug uten aa blokkere inne i ISR.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 6
- **Files changed:** Bachelor/Core/Inc/myprint.h, Bachelor/Core/Src/myprint.c, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/Core/Src/main.c, Bachelor/log.md
- **What/Why:** La til en enkel `myprint`-modul for UART-terminal via Nucleo `COM1` / `LPUART1` paa `PA2` (TX) og `PA3` (RX). Modulen kan skrive tekst og polle/echo mottatte tegn. Samtidig ble motorsekvensen satt paa pause for ren UART-testing, `defaultTask` ble endret til aa prosessere terminalinput og sende periodiske statusmeldinger, og dobbel `BSP_COM_Init()` i `main.c` ble fjernet slik at UART bare initialiseres ett sted.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 5
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** Endret startfrekvensen for soft-start fra `100 Hz` til `1000 Hz`. Duty cycle i den GPIO-baserte 3-fasetesten ble beholdt paa `25 %` med samme 12-stegs fasesekvens.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 4
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** Endret startfrekvensen for soft-start til `100 Hz`. For aa faa `25 %` duty i den GPIO-baserte 3-fasetesten ble tilstandstabellen utvidet fra `6` til `12` steg, slik at hver fase er high i `3 av 12` steg med `120 grader` faseforskyvning.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 3
- **Files changed:** Bachelor/Core/Inc/myapp.h, Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** Justerte soft-startprofilen slik at lave frekvenser holdes lenger og rampen blir mer aggressiv etter hvert. Delay mellom steg er naa progressiv i stedet for fast, og prosentvis frekvensoekning oeker med frekvensen. Flyttet ogsaa opprettelsen av blink-tasken til `App_StartTasks()` i `myapp`, som kalles fra FreeRTOS-init etter at kernel er initialisert.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23
- **Files changed:** Bachelor/Core/Inc/pwm_control.h, Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Src/myapp.c, Bachelor/log.md
- **What/Why:** La til runtime-styring av frekvens for den GPIO-baserte 3-fasetesten og en myk oppstartsrampe i `App_Init()`. Oppstarten begynner naa paa `10 Hz` og oeker gradvis mot `20 kHz` med `50 ms` mellom hvert trinn. Rampa bruker en enkel prosentvis oekning per steg for aa gi roligere oppstart ved lave frekvenser enn store lineare hopp.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 2
- **Files changed:** Bachelor/Core/Src/myapp.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** Flyttet frekvensrampen ut av `App_Init()` og inn i `defaultTask` i FreeRTOS. `App_Init()` starter naa kun den GPIO-baserte 3-fasetesten paa `10 Hz`, mens `defaultTask` oeker frekvensen videre hvert `50 ms`. Dette gjoer at rampen skjer mens systemet faktisk kjorer, i stedet for aa bli fullfort under init.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23
- **Files changed:** Bachelor/Core/Inc/pwm_control.h, Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Src/myapp.c, Bachelor/log.md
- **What/Why:** La til runtime-styring av frekvens for den GPIO-baserte 3-fasetesten og en myk oppstartsrampe i `App_Init()`. Oppstarten begynner naa paa `10 Hz` og oeker gradvis mot `20 kHz` med `50 ms` mellom hvert trinn. Rampa bruker en enkel prosentvis oekning per steg for aa gi roligere oppstart ved lave frekvenser enn store lineare hopp.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:10
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Endret den GPIO-baserte 3-fasetesten fra 20 kHz til 2 kHz ved aa redusere testens step-frekvens fra 120 kHz til 12 kHz. `TIM1.Prescaler` i CubeMX hadde ikke effekt paa denne testen fordi `PWM_HardwareTest_3Phase()` rekonfigurerer TIM1 direkte ved runtime.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:15
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Endret den GPIO-baserte 3-fasetesten fra 2 kHz til 1.2 kHz ved aa redusere testens step-frekvens fra 12 kHz til 7.2 kHz. Med 6 tilstander per elektrisk periode gir dette 1.2 kHz grunnfrekvens.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:22
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Justerte kun fase C i den GPIO-baserte 3-fasetesten. C high-side var komplementaer til fase B (`110001` mot `001110`) i stedet for aa vaere 120 grader forskjoevet. Sekvensen for C ble endret til `100011` slik at A og B beholdes uendret mens C foelger samme 50 % boelgeform med riktig faseforskyvning.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:27
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Endret kun frekvensen i den GPIO-baserte 3-fasetesten fra 1.2 kHz til 800 Hz. Step-frekvensen ble redusert fra 7.2 kHz til 4.8 kHz, mens fasesekvens og duty cycle ble beholdt uendret.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:30
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Endret kun frekvensen i den GPIO-baserte 3-fasetesten fra 800 Hz til 10 Hz. Step-frekvensen ble redusert fra 4.8 kHz til 60 Hz, mens duty cycle og faseforskyvning ble beholdt uendret.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:36
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Rettet 10 Hz-oppsettet for den GPIO-baserte 3-fasetesten. Aarsaken til at 10 Hz ga ca. 1.8 kHz var at TIM1 er 16-bit, mens forrige oppsett lot `ARR` bli for stor med `Prescaler=0`. La til en egen test-prescaler (`PSC=44`) og beholdt step-frekvensen paa 60 Hz, som gir reell 10 Hz grunnfrekvens med uendret duty cycle og faseforskyvning.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:40
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Endret den GPIO-baserte 3-fasetesten tilbake til 800 Hz. Step-frekvensen ble satt til 4.8 kHz, og test-prescaleren ble satt tilbake til `0` siden denne frekvensen passer innen 16-bit timerperioden uten ekstra preskalering. Duty cycle og faseforskyvning ble beholdt uendret.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:42
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Endret kun frekvensen i den GPIO-baserte 3-fasetesten fra 800 Hz til 100 Hz. Step-frekvensen ble redusert fra 4.8 kHz til 600 Hz, mens duty cycle, fasesekvens og faseforskyvning ble beholdt uendret.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-20 13:47
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Forenklet frekvensoppsettet i den GPIO-baserte 3-fasetesten slik at frekvensen naa styres av én konstant: `PWM_TEST_OUTPUT_FREQUENCY_HZ`. Step-frekvensen beregnes automatisk fra antall testtilstander, og en fast test-prescaler brukes for aa unngaa 16-bit begrensningen ved lave frekvenser.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 23
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Gjorde den systematiske kommuteringstestingen eksplisitt i kode ved aa samle alle seks fase-permutasjonene (`UVW->CH321`, `CH132`, `CH123`, `CH312`, `CH231`, `CH213`) i en tabell og styre valgt kandidat med `MOTOR_COMMUTATION_PHASE_MAP`. `MOTOR_COMMUTATION_SECTOR_OFFSET` beholdes som separat testparameter `0..5`. UART viser naa aktiv konfigurasjon (`cfg`) og offset i baade oppstart og `COMM`-prints, slik at hver iterasjon kan testes og dokumenteres uten tvetydighet.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-23 24
- **Files changed:** Bachelor/flytskjema.md, Bachelor/log.md
- **What/Why:** Oppdaterte arbeidsdokumentet for videre systemdesign. Flytskjemaet beskriver naa ønsket retning med IRQ-basert Hall-lesing som primærflyt, polling-basert Hall som feilsøkingsspor, joystick-polling hver `50 ms`, og display-oppdatering for hastighet og batteri/ladning. Mermaid-flyten ble erstattet med mer SDL-lignende tekstlige prosessbeskrivelser som er enklere å videreutvikle som spesifikasjon.
- **Build/Test result:** Ikke kjort. Dokumentasjonsendring.
### Endring 2026-03-23 25
- **Files changed:** Bachelor/flytskjema.md, Bachelor/log.md
- **What/Why:** La til dødmansknapp som eksplisitt sikkerhetskrav i systembeskrivelsen. Dokumentet beskriver nå at motoren bare kan være aktiv når dødmansknappen holdes inne, samt egen SDL-flyt for arming, release og umiddelbar sikker stopp.
- **Build/Test result:** Ikke kjort. Dokumentasjonsendring.
### Endring 2026-03-23 26
- **Files changed:** Bachelor/flytskjema.md, Bachelor/log.md
- **What/Why:** La til en første tegnet flytskjema-skisse i dokumentet med ASCII-bokser og piler. Skissen viser oppstart, runtime, hall/kommutering, joystick, dødmansknapp, display og batteri i en mer visuell form som kan videreutvikles sammen.
- **Build/Test result:** Ikke kjort. Dokumentasjonsendring.
### Endring 2026-03-24 1
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Etter at alle sektoroffsettene ga samme resultat, ble kommuteringstesten utvidet med en eksplisitt polaritetsvariant via `MOTOR_COMMUTATION_INVERT_POLARITY`. Dette bytter `HIGH` og `LOW` i hele 6-step-tabellen uten aa endre hall-lesing eller fase-permutasjon. Hensikten er aa teste om selve kraftretningen i tabellen er feil, selv om hall-sekvens og sektorbytter ser riktige ut. UART viser naa ogsaa `inv=0/1` sammen med `cfg` og `off`.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 2
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Endret TIM1-fasestyringen til en enklere 6-step-modell inspirert av BLDC-eksempelet: `HIGH` betyr naa kun PWM paa high-side-utgangen, `LOW` betyr kun low-side-utgangen aktiv, og `FLOAT` betyr begge av. Tidligere brukte `HIGH`/`LOW` begge TIM1-utgangene samtidig som et komplementaert par, noe som kan ha gitt riktig PWM-bilde paa scope men feil kraftvei i inverteren. Denne endringen tester hypotesen om at selve realiseringen av kommuteringstilstandene var feil, ikke hall-sekvensen.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 3
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/log.md
- **What/Why:** Reverterte den alternative 6-step-realiseringen etter test paa target. Varianten med eksplisitt high-side-PWM og separat low-side-utgang ga verken rykk eller stroemtrekk, noe som tyder paa at det aktuelle drivertrinnet deres ikke kan drives paa den maaten med dagens TIM1-oppsett. `PWM_PHASE_STATE_HIGH/LOW/FLOAT` er derfor satt tilbake til den opprinnelige komplementaere TIM1-logikken.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 4
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** La til en eksplisitt manuell faseprobe-modus i kommuteringsmodulen. Med `MOTOR_COMMUTATION_MODE_MANUAL_PROBE` holdes ett fast fasepar aktivt (`U+V-`, `U+W-`, `V+W-`, `V+U-`, `W+U-`, `W+V-`) valgt via `MOTOR_COMMUTATION_MANUAL_STEP`, uten hall-oppdateringer. Hensikten er aa identifisere den faktiske elektriske fasekoblingen og se hvilke kombinasjoner som gir laasing/moment direkte, siden hall-basert kommutering hittil ikke har skilt kandidatene tydelig.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 5
- **Files changed:** Bachelor/Core/Src/pwm_control.c, Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Forenklet den manuelle faseprobetesten ytterligere ved aa sette TIM1-kommuterings-PWM til `10 kHz` og starte paa `5 %` duty med automatisk opptrapping til `25 %`. I manuell probe-modus oekes duty med `2.5 %` hvert `400 ms` mens samme fasepar holdes aktivt. Hensikten er aa se om motoren bygger moment gradvis i en enklere og mykere test enn et fast hopp til hoey duty.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 6
- **Files changed:** Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** Oekte `defaultTask`-stacken tilbake til `512 * 4` bytes. Den hadde falt tilbake til `128 * 4`, og med `MyPrint_Printf()` i manuell probe/ramp-modus er det sannsynligvis for lite. Dette passer godt med symptomet der UART-prints og blink-task stoppet rett etter oppstart.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 7
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Byttet fra fast manuell faseprobe til en enkel open-loop 6-step testmodus. Dette er fortsatt uavhengig av hall-kommutering, men i stedet for aa holde ett fast fasepar aktivt gaar den naa sakte gjennom step `1..6` med `150 ms` mellom trinnene. Duty starter fortsatt paa `5 %` og rampes opp til `25 %`. Hensikten er aa skille mellom "motoren laaser seg fordi ett felt holdes fast" og "motoren klarer ikke aa foelge en enkel elektrisk sekvens i det hele tatt".
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 8
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Brukte open-loop-loggen som grunnlag for en eksplisitt hall-til-kommuteringsmapping og byttet tilbake til hall-basert modus. Siden motoren faktisk gikk rundt i open-loop med `UVW->CH321`, men veldig hakkete, er neste naturlige steg aa kommutere synkront med hall-sektorene i stedet for med fast tidsbase. Den nye tabellen mapper hallsektor `1..6` til den open-loop-stepen som faktisk saa ut til aa dra rotoren videre i loggen, i stedet for aa bruke enkel sektoroffset alene.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 9
- **Files changed:** Bachelor/Core/Src/gpio.c, Bachelor/Core/Src/motor_commutation.c, Bachelor/pins.md, Bachelor/log.md
- **What/Why:** La til `PB12` som poll-basert start/stopp for motorstyringen. Pinnen er konfigurert som `GPIO_INPUT` med `PULLUP`, og kommuteringen holdes helt av ved oppstart til knappen trykkes. Et nytt trykk stopper motoren og setter alle tre faser til `FLOAT`. Dette er gjort for aa unngaa at motor/PWM starter automatisk mens dere programmerer eller feilsoker paa stoyete PCB. UART oppgir ogsaa naa at startknappen er `PB12` med aktiv lav antakelse.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 10
- **Files changed:** Bachelor/Core/Inc/motor_commutation.h, Bachelor/Core/Src/motor_commutation.c, Bachelor/Core/Src/app_freertos.c, Bachelor/log.md
- **What/Why:** Strammet inn `PB12`-start/stopp-flyten slik at hall-filteret ikke prosesseres eller printer noe foer motoren faktisk er aktivert. `defaultTask` poller naa start/stopp via `MotorCommutation_Process()` foerst, og kaller bare `HallStateFilter_Process()` naar motoren er enablet. Dette reduserer risikoen for oppstartsustabilitet/UART-flom paa stoyete PCB foer brukeren trykker start.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 11
- **Files changed:** Bachelor/Core/Src/hall_state_filter.c, Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Slo av de loepende `HALL FILTER`- og `COMM`-printf-ene under drift via egne debug-flagg. Hvis MCU-en klikker idet foerste hall-overgang kommer inn, er UART-belastning og `vsnprintf`/blocking transmit i tettsittende overgangsloeper en sannsynlig software-aarsak. Endringen lar oss teste samme hall- og kommuteringslogikk uten runtime-printspam.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 12
- **Files changed:** Bachelor/Core/Src/stm32g4xx_it.c, Bachelor/log.md
- **What/Why:** Endret `HardFault_Handler()` til aa disable interrupts og trigge `NVIC_SystemReset()` i stedet for aa bli staaende i en uendelig loekke. Hensikten er aa gjore kortet mer brukbart under feilsoking naar en hardfault utloeses av stoy eller feil i motorstyringen.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 13
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Skrev om hall-modusen til en eksplisitt seksjonsmodell med `switch (hall_sector)` i stedet for indirekte hall-til-step-oppslag. Hver gyldig hallsektor `1..6` velger naa direkte hvilket fasepar som skal vaere aktivt, med kommentarer som viser tilsiktet elektrisk seksjon (`U+V-`, `U+W-`, `V+W-`, osv.). Dette gjoer kommuteringslogikken lettere aa lese, verifisere og tune naar fokus er "i denne seksjonen skal disse transistorene vaere paa".
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 14
- **Files changed:** Bachelor/Core/Src/hall_state_filter.c, Bachelor/log.md
- **What/Why:** Slo paa igjen runtime-print for aksepterte hall-tilstander i `hall_state_filter`. Disse ble tidligere skrudd av for aa redusere UART-belastning under feilsoking av oppstartsproblemer, men brukeren trenger naa igjen a se hvilke hallsektorer som faktisk blir akseptert av filteret.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 15
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Gjorde hall-basert oppstart mildere ved aa bare latch-e foerste gyldige hallsektor etter start og vente paa neste gyldige sektorovergang foer kommutering faktisk aktiveres. Hensikten er aa unngaa at krafttrinnet slaas paa momentant i samme oeyeblikk som foerste hall-lesing kommer inn, siden det var akkurat der systemet klikket.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 16
- **Files changed:** Bachelor/Core/Src/gpio.c, Bachelor/log.md
- **What/Why:** Fjernet EXTI-modus fra hall-pinnene og satte dem til rene `GPIO_MODE_INPUT` under videre feilsoking. Hallfiltreringen bruker allerede polling i `hall_state_filter`, sa EXTI paa `PC1/PC7/PC9` var ikke nodvendig for motorstyringen og kunne bidra til IRQ-stoy eller ustabilitet naar hallsignalene begynte aa svinge. EXTI0/1/2/3-NVIC-enable for disse linjene ble ogsaa tatt ut fra `gpio.c`.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 17
- **Files changed:** Bachelor/Core/Inc/reset_diag.h, Bachelor/Core/Src/reset_diag.c, Bachelor/Core/Src/myapp.c, Bachelor/log.md
- **What/Why:** La til enkel reset-diagnostikk som skriver resetkilden ved oppstart (`POR/PDR`, `BOR`, `PINRST`, `SFTRST`, `IWDG`, `WWDG`, `LPWR`). Dette er gjort fordi kortet ser ut til aa restarte uten a gaa via `HardFault_Handler()`, og vi trenger aa skille mellom software reset, watchdog, brown-out og ekstern reset/stoy paa resetlinjen.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 18
- **Files changed:** Bachelor/Core/Src/reset_diag.c, Bachelor/log.md
- **What/Why:** Rettet reset-diagnostikken til STM32G4-flaggenes faktiske navn. `RCC_CSR_PORRSTF` finnes ikke paa STM32G431, sa utskriften ble endret til aa bruke `OBLRSTF`, `BORRSTF`, `PINRSTF`, `SFTRSTF`, `IWDGRSTF`, `WWDGRSTF` og `LPWRRSTF`.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 19
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** Forenklet `PB12`-styringen fra toggle start/stopp med debounce og intern state-maskin til en ren "hold for aa kjoere"-gate. Naar `PB12` holdes lav, resettes runtime-state og motorstyringen er enablet; naar knappen slippes, disable-es utgangene umiddelbart. Hensikten er aa eliminere ekstra knappelogikk som mulig kilde til ustabilitet etter at brukeren observerte at problemene oppstod etter knappintegrasjonen.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 20
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/Core/Inc/motor_commutation.h, Bachelor/Core/Src/app_freertos.c, Bachelor/Core/Src/gpio.c, Bachelor/pins.md, Bachelor/log.md
- **What/Why:** Rullet tilbake motorstyringen til siste kjente stadie foer knappintegrasjon og de senere hall-start/gate-endringene begynte aa gi reset-problemer. Dette setter kommuteringen tilbake til den enklere open-loop-varianten ved `10 kHz` med duty-ramp og uten `PB12`-gate. `app_freertos` kaller igjen hallfilter og kommutering uten knappavhengig gating, og `PB12` er tatt ut av GPIO-oppsett og pindokumentasjon. Reset-diagnostikken ble beholdt fordi den er passiv og nyttig under videre feilsoking.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 21
- **Files changed:** Bachelor/Core/Src/motor_commutation.c, Bachelor/log.md
- **What/Why:** La inn en ren isolasjonstest ved aa sette `MOTOR_COMMUTATION_MODE` til `DISABLED`. Da prosesseres hallfilteret fortsatt og kan skrive gyldige hallsektorer, men motorstyringen skriver ikke nye kommuteringstrinn under drift. Hensikten er aa skille mellom "reset ved hall-lesing" og "reset idet motorstyringen faktisk skriver fasekommandoer".
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 22
- **Files changed:** Bachelor/Core/Src/app_freertos.c, Bachelor/Core/Src/hall_state_filter.c, Bachelor/log.md
- **What/Why:** Rettet sannsynlig oppstartsreset ved aa oeke `defaultTask`-stacken tilbake til `512 * 4` bytes og samtidig skru av loepende `HALL FILTER`-printf ved runtime. Endringsloggen dokumenterer allerede at `128 * 4` tidligere ga symptomer som stopp i UART/blink rett etter oppstart, og at `MyPrint_Printf()`/`vsnprintf()` i task-kontekst var en sannsynlig utløsende faktor. Denne endringen reduserer baade stackpress og blokkende UART-arbeid i oppstartsloopen.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
### Endring 2026-03-24 23
- **Files changed:** Bachelor/Core/Src/hall_state_filter.c, Bachelor/Core/Src/motor_commutation.c, Bachelor/Core/Src/gpio.c, Bachelor/pins.md, Bachelor/log.md
- **What/Why:** Slo paa igjen UART-utskrift av aksepterte hallsektorer, satte motorstyringen tilbake til hall-modus, og la til `PB12` som poll-basert toggle-knapp for motoren. `PB12` er konfigurert som `GPIO_INPUT` med intern `PULLUP`; et trykk toggler mellom motor av/paa, og naar motoren er av settes alle tre faser til `FLOAT`. Dette gir hall-observasjoner igjen uten aa starte motoren automatisk ved oppstart.
- **Build/Test result:** Ikke kjort. Lokal ARM-toolchain/make er ikke tilgjengelig i dette miljoet.
