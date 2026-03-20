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
