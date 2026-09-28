---
name: lct-2026-report-repo
description: Где лежит git-репозиторий отчёта хакатона ЛЦТ 2026 (usb_token) и как он соотносится с Ghidra-проектом
metadata: 
  node_type: memory
  type: project
  originSessionId: 62c092a8-4c4f-492b-9407-fa1ae57c1caf
  modified: 2026-09-20T14:56:01.387Z
---

Git-репозиторий отчёта команды по кейсу ЛЦТ 2026 (реверс usb_token, трек Positive Technologies) — [[lct-2026-usb-token-reverse]].

- Remote: `git@github.com:HarisGOD1/case-12-reverse-engineering-pcb-team-3.git`, ветка `main`
- Актуальная раскладка каталогов описана в `README.md`; ранее отдельное исследование гаммы объединено с `attack3/`

Рабочая директория сессии — это Ghidra-проект (`*.rep`, локальный чекаут shared-проекта Ghidra), а НЕ репозиторий отчёта: git-репо отчёта живёт отдельным каталогом. Конкретные локальные пути этой машины — в приватном `CLAUDE.local.md` (вне git), не здесь. Новые векторы атаки оформляются как `attackN/` рядом с существующими.
