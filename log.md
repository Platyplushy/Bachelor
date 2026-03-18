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
