# Issue #4 — SFML Presentation Framework & Window Lifecycle Setup

## Acceptance mapping

| Acceptance criterion | Implementation | Validation |
| --- | --- | --- |
| Presentation decoupled from engine logic | Dedicated `snake_presentation` target; SFML-free navigation library; no engine/game rules in renderer | Headless-only configure/build/test; inspect state target link dependencies |
| AppManager state flow initialized | MainMenu, Gameplay placeholder, Paused, terminal Exiting; explicit actions | State-flow tests cover valid transitions, invalid actions, quit from each state, terminal exit |
| Clean exit on window close | Closed event dispatches Quit and closes the window; loop breaks before update/draw/display | Native close-event smoke test plus manual Windows checks below |

## Requirement/design scope

- NFR-005: module boundaries initialized; future engine, modes, persistence and Catch2 targets are not implemented here.
- NFR-001: frame limit configured to 60 FPS. The required sustained ≥30 FPS performance test (TC-PERF-01) remains pending on the specified Windows hardware.
- SAD C1: application/window/main-loop scaffold.
- SAD C2/C4: placeholder screen rendering, not completed feature screens.
- The issue text calls AudioService “C4”; the submitted SAD labels Renderer C4 and AudioService C5. This story does not implement audio. Resolve that label discrepancy before later audio work.
- No FR-001 completion claim: there is no snake rendered yet, only a grid preview.
- No TC-UI/TC-CORE/TC-PERF completion claims: these require the later integrated product.

## Integration notes

The application owns `sf::RenderWindow`; RAII tears it down on return or exception. Events are polled every render frame. Application update uses a fixed 60 Hz accumulator with a 250 ms catch-up clamp. Paused/menu states do not invoke gameplay updates. Later engine movement must keep its own difficulty-based tick accumulator. Do not add sleep-based game ticks or poll input only at movement ticks.

The startup state goes directly from menu to the gameplay placeholder only to demonstrate navigation. Insert Name Entry and Mode/Difficulty screens in their own stories; do not treat this preview path as the final UX.

## Manual Windows smoke checklist (still required)

- [ ] Configure and compile on Windows 10 and Windows 11.
- [ ] Launch; client area is 900×600 and menu placeholder is shown.
- [ ] Enter opens grid preview, Esc pauses, Esc/Enter resumes.
- [ ] M returns to menu from preview and pause.
- [ ] Alt-tab from preview pauses; focus regain does not auto-resume.
- [ ] Window close button exits from menu, preview and pause without hanging.
- [ ] Esc from menu exits with return code 0.
- [ ] Repeat launch/close several times; no lingering process.

See `VALIDATION.md` for actual local execution results. Do not mark Windows checks or NFR-001 passed based only on Linux testing.
