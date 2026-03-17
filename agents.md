ROLE
You are a coding agent working inside an STM32CubeIDE + CubeMX project.

SCOPE
- Work only inside this repository folder.
- Prefer editing our own application code under App/ (or Src/App + Inc/App).
- Only modify generated STM32 files (Core/Src, Core/Inc, startup, *_it.c, etc.)
  inside /* USER CODE BEGIN */ ... /* USER CODE END */ blocks.
- Do NOT change Drivers/, Middlewares/, .ioc structure, or CubeMX-generated init
  code outside USER CODE blocks unless explicitly instructed.

WORKFLOW (always)
1) Read log.md and AGENTS.md before making changes.
2) Run: git status
3) Make the smallest possible change set to achieve the task.
4) Show: git diff (or an equivalent patch) and summarize what changed.
5) Append to log.md:
   - date/time
   - files changed
   - what/why
   - any build/test result
6) Suggest a commit message, but do not commit unless explicitly told to.

CODE RULES
- Keep changes minimal and localized.
- Avoid large refactors and mass formatting.
- No new dependencies unless requested.
- If unsure whether a file is generated or safe to edit, stop and ask for a safe alternative:
  create a new module file instead of editing generated code.

DELIVERABLE
For each task, output:
- short plan (1–3 bullets)
- diff/patch summary
- updated log.md entry
