# Agent Instructions

This repository keeps its persistent agent instructions in `agents.md`.

Read and follow:
- `agents.md`
- `log.md`

Additional logging requirement:
- When prompts are used as part of the development process, log them in `log.md` for later reporting.
- For each such entry, include:
  - the prompt used
  - a very short description of what the answer/result was

STM/Cube system files are holy:
- Do not modify STM32Cube/ST-generated system files unless the user explicitly asks for it.
- If system-level behavior must be extended, prefer adding new project files and calling them from existing user code hooks instead of editing the generated file directly.
- In generated system files, only small call sites in `USER CODE` blocks are acceptable by default. Do not place new function implementations or larger logic bodies inside those files unless explicitly requested.
- Examples of files to treat as holy unless explicitly approved: `main.c`, `main.h`, `gpio.c`, `tim.c`, `adc.c`, `dma.c`, `app_freertos.c`, `stm32g4xx_it.c`, generated startup files, and similar Cube-generated sources/headers.
