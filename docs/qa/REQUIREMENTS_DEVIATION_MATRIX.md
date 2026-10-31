# Requirements and design deviation matrix — reconciled baseline

**Issue #25:** documentation reconciliation, not final QA sign-off. Baseline Windows evidence remains tied to game source `7c6733d`. This documentation refresh is based on main at `3fcc586`. PR #60 audio commit `09bbf96` is merged. Owner-reported listening confirms collision-alert audibility, one-shot/no-loop behavior, and silence at SFX volume 0. Original Windows archive results are reported evidence; corrected pause/restart records in the revised archive remain unverified. Preserve the earlier matrix in [history](history/REQUIREMENTS_DEVIATION_MATRIX-pre-reconciliation.md).

## Sources and interpretation

Submitted `Team16_SnakeXtreme_Deliverables_Part1_SRS_SAD_STP.pdf`: SRS§5-6,Addendum§2/§4/§5,SAD§3.4/§3.9,STP§13/AppendixA. Baseline has43 requirements (29FR,7NFR,7SEC),44 cases and30 High cases. [Complete case matrix](STP_EXECUTION_MATRIX.md),[CSV](STP_EXECUTION_MATRIX.csv),[source/inventory hash](evidence/stp-inventory-validation.json).

**No deviation** below is limited to the behavior supported by source and executed automated evidence. It is not a claim of final100% requirement pass. **Pending verification** is an evidence gap,not automatically a defect. **Confirmed difference** records an actual baseline/implementation distinction; disposition and rationale must not be invented. Scope proposals are not silently approved.

## All43 requirements

| Requirement | Submitted behavior (short) / mapped cases | Current evidence | Deviation/finding |
|---|---|---|---|
| FR-001 | Grid and snake render on new game; TC-CORE-01 | Pending acceptance measurement; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-002 | Tick interval per difficulty; TC-CORE-02, TC-MODE-02 | Partial evidence; Automated scope passed; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-003 | Direction via arrows and WASD; TC-CORE-03, TC-CORE-04 | Partial evidence; Automated scope passed; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-004 | Wall collision at all four boundaries; TC-CORE-05 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-005 | Self collision and vacating-tail rule; TC-CORE-06 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-006 | Food on free cell and full board; TC-CORE-07 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-007 | Growth by exactly one; TC-CORE-08 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-008 | Mode selection; TC-MODE-01 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-009 | Difficulty sets speed in all nine combinations; TC-MODE-02 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-010 | Timed Challenge ends at zero; TC-MODE-03 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-011 | Obstacle placement and fatal collision; TC-MODE-04 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-012 | Power-up spawn interval; TC-PWR-01 | Automated scope passed; see linked case matrix | Spawn interval behavior supported; uncollected-item5 s expiry differs from Addendum relocation baseline (DV03). |
| FR-013 | Speed boost lasts five seconds; TC-PWR-02, TC-PWR-05 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-014 | Multiplier lasts ten seconds; TC-PWR-03, TC-PWR-05 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-015 | Shrink and minimum length; TC-PWR-04, TC-PWR-05 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-016 | Mode-specific score per food; TC-SCORE-01 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-017 | Real-time score HUD in every theme; TC-SCORE-02 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-018 | Persist Top10 with every field; TC-LB-01 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-019 | View ten-entry and empty leaderboard; TC-LB-02 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-020 | Insert only within Top10; TC-LB-03 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-021 | Missing/corrupt leaderboard fallback; TC-LB-04 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-022 | Main menu options and navigation; TC-UI-01 | Partial evidence; see linked case matrix | Confirmed label difference: Start Game versus specified New Game (DV05); functional navigation evidence exists. |
| FR-023 | Name entry boundaries; TC-UI-02 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| FR-024 | Pause/resume with active timer/effect; TC-UI-03 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-025 | Game Over restart and Main Menu; TC-UI-04 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-026 | Remap all movement keys; TC-SET-01 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-027 | Independent music and SFX volume; TC-SET-02 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-028 | Classic Neon Retro visual themes; TC-SET-03 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| FR-029 | Settings persist across restart; TC-SET-04 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| NFR-001 | Sustained minimum30FPS; TC-PERF-01 | Pending acceptance measurement; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| NFR-002 | Input acknowledgement within100ms; TC-PERF-02 | Pending acceptance measurement; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| NFR-003 | Missing/corrupt leaderboard fallback; TC-LB-04, TC-REL-01 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| NFR-004 | First game within three screens; TC-USE-01 | Pending human walkthrough; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| NFR-005 | Module separation; TC-MNT-01 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| NFR-006 | Windows 10 and11 clean build/run; TC-PORT-01 | Partial evidence; see linked case matrix | Windows 10reported build/play evidence; Windows 11NotTested,proposed omission not approved (DV06). |
| NFR-007 | Headless testability; TC-TEST-01 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| SEC-001 | Name sanitization; TC-SEC-01 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| SEC-002 | No PII stored; TC-SEC-02 | Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| SEC-003 | Missing/corrupt config fallback; TC-REL-01, TC-SEC-03 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| SEC-004 | Safe write under injected faults; TC-SEC-04 | Partial evidence; see linked case matrix | Atomic preservation evidence exists; cleanup promise differs (DV01). Native injection evidence must not be inferred from Linux. |
| SEC-005 | Untrusted data never executable; TC-SEC-05 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |
| SEC-006 | Name entry boundaries; TC-UI-02, TC-SEC-06 | Automated scope passed; Partial evidence; see linked case matrix | Pending verification — prescribed remaining observations/measurements listed per case; not automatically an implementation defect. |
| SEC-007 | Schema and size validation; TC-SEC-03 | Automated scope passed; see linked case matrix | No deviation — reviewed automated scope; final acceptance governed by case matrix. |

## Addendum constants and controls

| Item | Compared implementation | Finding |
|---|---|---|
| Grid/window | GameConfig30x20,30px,900x600; initial3segments,centre,right | No deviation in reviewed constants and initial-state tests; render-time acceptance separate. |
| Tick / timed mode | 200/120/70ms;60000ms | No deviation in reviewed constants/unit timing. |
| Food / effects | 10/10/15points; speed3/5for5s; multiplier2for10s; shrink1,floor3 | No deviation in reviewed source/tests. |
| Spawn schedule | 8-15s,one item; implementation adds5s uncollected lifetime | Schedule criterion supported; lifetime/relocation difference DV03. |
| Obstacles |8cells,min head Chebyshev distance3 | No deviation in reviewed placement/collision tests. |
| Name | trim,1-15characters,whitelist[ A-Za-z0-9_- ],physical15-byte input storage | No deviation in reviewed shared UI/pre-persistence validation. |
| Top10/ties/date | descending,strictly beats tenth when full,ties after existing,YYYY-MM-DD | No deviation in reviewed repository/corpus regressions. |
| Defaults / themes | arrows+permanentWASD,70/70,Classic; Classic/Neon/Retro | No deviation in reviewed constants/editor/repository tests; audible/visual checks separate. |
| File size / depth | <=65536bytes before parsing; bounded JSON; Settings retains depth8 | No deviation in reviewed cap/bounded-parse behavior; depth8 is implementation-specific,not a newly invented universal SRS value. |
| Queue | <=2pending turns,validated against last queued direction | No deviation in reviewed input/engine tests. |

## SAD component responsibilities (C1-C15)

| Component | Implementation evidence / realization | Finding |
|---|---|---|
| C1 Application/ScreenManager | AppManager+AppState enum/action transitions instead of an IScreen push/pop stack | Interface realization differs (DV02); navigation tests support behavior. |
| C2 UI Screens | Renderer,AppState,NameEntry,MenuSelection and hit tests | Responsibilities realized without independent IScreen classes; DV02. |
| C3 InputMapper | Settings movementForKey plus AppManager event dispatch | Mapping behavior supported; dedicated map(sf::Event)->Action class not implemented as drawn; DV02. |
| C4 Renderer/ThemeProvider | Renderer with Classic/Neon/Retro; model/UI tests | No deviation in reviewed responsibilities; final per-theme visual observations separate. |
| C5 AudioService | SFML sound buffers and independent gains, model/backend/UI tests | Existing model, backend and UI logs support the reviewed gain/routing scope. Owner-reported listening confirms collision-alert audibility, one-shot/no-loop behavior, and silence at SFX volume 0. Independent-channel listening, F5 behavior, platform-specific acceptance and other case-specific checks remain separate and pending. PR #60 waveform enhancement (`09bbf96`) is merged at `3fcc586`; this does not constitute overall audio or final QA acceptance. |
| C6 GameEngine | deterministic grid,queue,collision,food/growth; unit tests | No deviation in reviewed engine responsibility. |
| C7 ModeManager | mode strategies,difficulty,timer,obstacles | No deviation in reviewed responsibility/tests. |
| C8 PowerUpManager | schedules,pickup,replacement,effects | Core duties supported; uncollected lifetime difference DV03. |
| C9 ScoreService | per-mode food scoring and multiplier | No deviation in reviewed source/tests. |
| C10 InputValidator | shared whitelist/cap at UI and pre-serialization | No deviation in reviewed defense-in-depth integration. |
| C11 LeaderboardRepository | validation,Top10,date,recovery and optional warnings | No deviation in reviewed repository responsibilities; APIs use implementation result structures rather than assuming exact SAD pseudocode signatures. |
| C12 SettingsRepository | shared SecureFileStore,validation,defaults,depth8,warnings | No deviation in reviewed responsibility; preserve actual error/result interfaces. |
| C13 SecureFileStore | bounded JSON,safe paths,atomic replacement,sidecar locking | No deviation in reviewed integrity controls; unconditional startup-cleanup guarantee not met (DV01). |
| C14 Logger/Error handling | scoped warning codes through stderr and local result types; no separate rotating logger found in reviewed source | SAD size-limited log rotation/Logger facade not established (DV04); no fabricated logger implementation. |
| C15 Config/test seams | GameConfig,chrono-duration injection,seededmt19937,deterministic tests | Testability supported; exact IClock/IRandom facade realization differs (DV08). |

## Confirmed differences / proposals and reasons

| ID | Reference | Actual difference | Recorded reason / disposition |
|---|---|---|---|
| DV01 | SAD§3.9C13;TC-SEC-04 |24-hour,best-effort,temp/orphan cleanup instead of guaranteed removal at next startup | Source/report explicitly protects fresh files and active writers with age/locks. Document residual stale files; owner acceptance of this changed guarantee still needed. Not a claim that failed atomic writes corrupt destination. |
| DV02 | SAD§3.4C1-C3 | Finite-state AppState,shared renderer/model and settings/event mapping replace exact IScreen stack/InputMapper facade | Source confirms realization; functional/layering tests pass. Original design-change rationale/approval not recorded; team must confirm,not invent it. |
| DV03 | Addendum§4power-up behavior | Implementation despawns uncollected item after5s; baseline describes relocation at subsequent spawn | GameConfig and explicit PowerUpTests confirm5 s expiry. Original rationale/approval not recorded. Spawn8-15s criterion is a separate behavior and remains tested. |
| DV04 | SAD§3.9C14 | Application-managed size-limited log rotation not found | stderr warning/result handling exists; no evidence of rotating Logger. Decide whether to implement or explicitly accept a design deviation; do not assert no-deviation for all architecture. |
| DV05 | FR022/TC-UI-01 | Menu label Start Game instead of exactly New Game | Same navigation purpose; wording rationale not recorded. Record as minor naming difference for disposition,not an invented runtime failure. |
| DV06 | NFR006/TC-PORT-01 | Windows 11unavailable; owner proposes Windows 10only | Access limitation documented; Windows 10build/play report available. Proposed omission must be explicitly accepted and documented; Windows 11notPassed. |
| DV07 | C5 / owner enhancement | PR #60 changes the GameOver waveform following baseline acceptance. The enhancement is merged at `3fcc586`. | Owner-reported listening and original Windows archive results are scoped evidence; corrected pause/restart records in the revised archive have not been independently verified. This does not establish all audio, platform, or release acceptance. |
| DV08 | SAD C15 | Passed chrono steps and seeded RNG replace explicit IClock/IRandom interfaces | Deterministic tests support purpose; exact facade adaptation/rationale requires team review. |

These differences are not automatically8new product defects. Severity,requirement impact and acceptance require review. Do not perform an unrequested architecture rewrite just to make a table green.

## Remaining exit criteria

44-case mapping is complete,not44-case execution.100% High pass and43/43passing requirements are not certified. C6–C11 line coverage>=80%,sanitizers and cppcheck are supported by merged Linux reports. Target measurements,volunteers,full native evidence,privacy campaign,accepted deviations and final issue #25sign-off remain separate gates. No owner/instructor signatures,date,waiver,release tag or demo is invented.
