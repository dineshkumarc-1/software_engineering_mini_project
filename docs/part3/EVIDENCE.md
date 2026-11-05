# Part 3 execution evidence

Source commit `04d782a345464349da10c91d6796b3ed99fc7b96`; 5 November 2026 UTC. Linux runs only, not Windows or human acceptance. The CTest and GUI suites overlap. Capability-dropping was used for filesystem tests and UI scripts. Debug line/branch coverage used GCC 11.5, gcovr 8.8 and clean counters; Release desktop used cached SFML 2.5.1, Catch2 and JSON; audio output was null. Fresh executable SHA-256 `b5c5c2cc8cb059e44e0cbf4ab881c2e33b639b16e89e3df4ad97dbcca1d62d8c`. A first desktop configure failed due to missing UDev development files; after installing them the unchanged build was successfully configured and tested. Initial coverage collection failed on compiler-ID source lookup and an overly broad filter; the final scoped collection succeeded without altering tests or counters.

## Instrumented headless CTest

```text
Test project /data/part3-build
      Start  1: app_state_flow
 1/21 Test  #1: app_state_flow .................................   Passed    0.01 sec
      Start  2: score_service
 2/21 Test  #2: score_service ..................................   Passed    0.00 sec
      Start  3: input_validator_tests
 3/21 Test  #3: input_validator_tests ..........................   Passed    0.00 sec
      Start  4: secure_file_store_tests
 4/21 Test  #4: secure_file_store_tests ........................   Passed    0.08 sec
      Start  5: leaderboard_repository_tests
 5/21 Test  #5: leaderboard_repository_tests ...................   Passed    0.05 sec
      Start  6: leaderboard_model_tests
 6/21 Test  #6: leaderboard_model_tests ........................   Passed    0.01 sec
      Start  7: security_corpus_tests
 7/21 Test  #7: security_corpus_tests ..........................   Passed    0.10 sec
      Start  8: settings_corpus_tests
 8/21 Test  #8: settings_corpus_tests ..........................   Passed    0.02 sec
      Start  9: secure_write_fault_tests
 9/21 Test  #9: secure_write_fault_tests .......................   Passed    0.08 sec
      Start 10: game_session
10/21 Test #10: game_session ...................................   Passed    0.00 sec
      Start 11: game_engine_tests
11/21 Test #11: game_engine_tests ..............................   Passed    0.18 sec
      Start 12: game_mode_tests
12/21 Test #12: game_mode_tests ................................   Passed    2.61 sec
      Start 13: powerup_tests
13/21 Test #13: powerup_tests ..................................   Passed    0.37 sec
      Start 14: gameplay_tests
14/21 Test #14: gameplay_tests .................................   Passed    0.13 sec
      Start 15: settings_tests
15/21 Test #15: settings_tests .................................   Passed    0.03 sec
      Start 16: audio_model_tests
16/21 Test #16: audio_model_tests ..............................   Passed    0.02 sec
      Start 17: performance_probe_tests
17/21 Test #17: performance_probe_tests ........................   Passed    0.01 sec
      Start 18: layering_core_is_sfml_free
18/21 Test #18: layering_core_is_sfml_free .....................   Passed    0.01 sec
      Start 19: security_banned_api_scan
19/21 Test #19: security_banned_api_scan .......................   Passed    0.21 sec
      Start 20: security_banned_api_scan_finds_planted_calls
20/21 Test #20: security_banned_api_scan_finds_planted_calls ...   Passed    0.00 sec
      Start 21: security_banned_api_scan_ignores_lookalikes
21/21 Test #21: security_banned_api_scan_ignores_lookalikes ....   Passed    0.00 sec

100% tests passed out of 21

Total Test time (real) =   3.96 sec
```

## Python tooling tests

```text
test_existing_directory_is_not_overwritten (test_acceptance_bundle.AcceptanceBundleTests.test_existing_directory_is_not_overwritten) ... ok
test_foreign_or_untrusted_artifact_url_is_rejected (test_acceptance_bundle.AcceptanceBundleTests.test_foreign_or_untrusted_artifact_url_is_rejected) ... ok
test_hashes_identify_bytes_without_claiming_tested_source (test_acceptance_bundle.AcceptanceBundleTests.test_hashes_identify_bytes_without_claiming_tested_source) ... ok
test_invalid_commit_never_creates_output (test_acceptance_bundle.AcceptanceBundleTests.test_invalid_commit_never_creates_output) ... ok
test_missing_or_empty_file_is_rejected (test_acceptance_bundle.AcceptanceBundleTests.test_missing_or_empty_file_is_rejected) ... ok
test_packaged_dependency_hashes_are_included (test_acceptance_bundle.AcceptanceBundleTests.test_packaged_dependency_hashes_are_included) ... ok
test_private_absolute_path_is_not_saved (test_acceptance_bundle.AcceptanceBundleTests.test_private_absolute_path_is_not_saved) ... ok
test_size_cap_and_control_characters_are_rejected (test_acceptance_bundle.AcceptanceBundleTests.test_size_cap_and_control_characters_are_rejected) ... ok
test_template_does_not_award_passes (test_acceptance_bundle.AcceptanceBundleTests.test_template_does_not_award_passes) ... ok
test_existing_output_is_not_overwritten (test_collect_launch.CollectLaunchTests.test_existing_output_is_not_overwritten) ... ok
test_invalid_schema_fails (test_collect_launch.CollectLaunchTests.test_invalid_schema_fails) ... ok
test_malformed_json_fails (test_collect_launch.CollectLaunchTests.test_malformed_json_fails) ... ok
test_marker_without_report_fails_cli_and_records_reason (test_collect_launch.CollectLaunchTests.test_marker_without_report_fails_cli_and_records_reason) ... ok
test_missing_first_frame_fails (test_collect_launch.CollectLaunchTests.test_missing_first_frame_fails) ... ok
test_nonzero_exit_fails_even_with_valid_report (test_collect_launch.CollectLaunchTests.test_nonzero_exit_fails_even_with_valid_report) ... ok
test_overlapping_windows_fail_shared_validation (test_collect_launch.CollectLaunchTests.test_overlapping_windows_fail_shared_validation) ... ok
test_timeout_fails_even_with_valid_report (test_collect_launch.CollectLaunchTests.test_timeout_fails_even_with_valid_report) ... ok
test_valid_report_marker_and_exit_succeed_without_acceptance (test_collect_launch.CollectLaunchTests.test_valid_report_marker_and_exit_succeed_without_acceptance) ... ok
test_valid_report_without_marker_fails (test_collect_launch.CollectLaunchTests.test_valid_report_without_marker_fails) ... ok
test_300_good_windows_still_require_target_hardware_and_powerups (test_performance_tools.PerformanceToolsTests.test_300_good_windows_still_require_target_hardware_and_powerups) ... ok
test_drops_are_flagged_and_no_report_grants_acceptance (test_performance_tools.PerformanceToolsTests.test_drops_are_flagged_and_no_report_grants_acceptance) ... ok
test_duplicate_json_keys_are_invalid (test_performance_tools.PerformanceToolsTests.test_duplicate_json_keys_are_invalid) ... ok
test_invalid_timestamps_cannot_become_small_latencies (test_performance_tools.PerformanceToolsTests.test_invalid_timestamps_cannot_become_small_latencies) ... ok
test_latency_boundary_and_rejections_are_separate (test_performance_tools.PerformanceToolsTests.test_latency_boundary_and_rejections_are_separate) ... ok
test_malformed_root_and_sample_objects_are_rejected (test_performance_tools.PerformanceToolsTests.test_malformed_root_and_sample_objects_are_rejected) ... ok
test_mixed_context_and_session_gaps_cannot_form_a_300_second_run (test_performance_tools.PerformanceToolsTests.test_mixed_context_and_session_gaps_cannot_form_a_300_second_run) ... ok
test_nonzero_anchor_boundary_and_aligned_gap (test_performance_tools.PerformanceToolsTests.test_nonzero_anchor_boundary_and_aligned_gap) ... ok
test_off_grid_and_pre_first_frame_windows_are_rejected (test_performance_tools.PerformanceToolsTests.test_off_grid_and_pre_first_frame_windows_are_rejected) ... ok
test_overlapping_duplicate_and_reversed_windows_are_rejected (test_performance_tools.PerformanceToolsTests.test_overlapping_duplicate_and_reversed_windows_are_rejected) ... ok
test_rejected_input_with_display_timestamp_is_invalid (test_performance_tools.PerformanceToolsTests.test_rejected_input_with_display_timestamp_is_invalid) ... ok
test_windows_without_first_frame_are_rejected (test_performance_tools.PerformanceToolsTests.test_windows_without_first_frame_are_rejected) ... ok
test_wrong_types_and_non_one_second_windows_are_invalid (test_performance_tools.PerformanceToolsTests.test_wrong_types_and_non_one_second_windows_are_invalid) ... ok
test_zero_fps_is_not_dropped_and_partial_windows_are_not_counted (test_performance_tools.PerformanceToolsTests.test_zero_fps_is_not_dropped_and_partial_windows_are_not_counted) ... ok

----------------------------------------------------------------------
Ran 33 tests in 0.020s

OK
```

## Desktop Release CTest

```text
Test project /data/part3-app
      Start  1: app_state_flow
 1/21 Test  #1: app_state_flow .................................   Passed    0.00 sec
      Start  2: score_service
 2/21 Test  #2: score_service ..................................   Passed    0.00 sec
      Start  3: input_validator_tests
 3/21 Test  #3: input_validator_tests ..........................   Passed    0.00 sec
      Start  4: secure_file_store_tests
 4/21 Test  #4: secure_file_store_tests ........................   Passed    0.04 sec
      Start  5: leaderboard_repository_tests
 5/21 Test  #5: leaderboard_repository_tests ...................   Passed    0.03 sec
      Start  6: leaderboard_model_tests
 6/21 Test  #6: leaderboard_model_tests ........................   Passed    0.00 sec
      Start  7: security_corpus_tests
 7/21 Test  #7: security_corpus_tests ..........................   Passed    0.05 sec
      Start  8: settings_corpus_tests
 8/21 Test  #8: settings_corpus_tests ..........................   Passed    0.01 sec
      Start  9: secure_write_fault_tests
 9/21 Test  #9: secure_write_fault_tests .......................   Passed    0.06 sec
      Start 10: game_session
10/21 Test #10: game_session ...................................   Passed    0.00 sec
      Start 11: game_engine_tests
11/21 Test #11: game_engine_tests ..............................   Passed    0.01 sec
      Start 12: game_mode_tests
12/21 Test #12: game_mode_tests ................................   Passed    0.21 sec
      Start 13: powerup_tests
13/21 Test #13: powerup_tests ..................................   Passed    0.02 sec
      Start 14: gameplay_tests
14/21 Test #14: gameplay_tests .................................   Passed    0.01 sec
      Start 15: settings_tests
15/21 Test #15: settings_tests .................................   Passed    0.02 sec
      Start 16: audio_model_tests
16/21 Test #16: audio_model_tests ..............................   Passed    0.01 sec
      Start 17: performance_probe_tests
17/21 Test #17: performance_probe_tests ........................   Passed    0.00 sec
      Start 18: layering_core_is_sfml_free
18/21 Test #18: layering_core_is_sfml_free .....................   Passed    0.01 sec
      Start 19: security_banned_api_scan
19/21 Test #19: security_banned_api_scan .......................   Passed    0.18 sec
      Start 20: security_banned_api_scan_finds_planted_calls
20/21 Test #20: security_banned_api_scan_finds_planted_calls ...   Passed    0.01 sec
      Start 21: security_banned_api_scan_ignores_lookalikes
21/21 Test #21: security_banned_api_scan_ignores_lookalikes ....   Passed    0.00 sec

100% tests passed out of 21

Total Test time (real) =   0.69 sec
```

## Sanitizer CTest

```text
Test project /data/part3-san
      Start  1: app_state_flow
 1/21 Test  #1: app_state_flow .................................   Passed    0.06 sec
      Start  2: score_service
 2/21 Test  #2: score_service ..................................   Passed    0.02 sec
      Start  3: input_validator_tests
 3/21 Test  #3: input_validator_tests ..........................   Passed    0.02 sec
      Start  4: secure_file_store_tests
 4/21 Test  #4: secure_file_store_tests ........................   Passed    0.39 sec
      Start  5: leaderboard_repository_tests
 5/21 Test  #5: leaderboard_repository_tests ...................   Passed    0.26 sec
      Start  6: leaderboard_model_tests
 6/21 Test  #6: leaderboard_model_tests ........................   Passed    0.05 sec
      Start  7: security_corpus_tests
 7/21 Test  #7: security_corpus_tests ..........................   Passed    0.66 sec
      Start  8: settings_corpus_tests
 8/21 Test  #8: settings_corpus_tests ..........................   Passed    0.15 sec
      Start  9: secure_write_fault_tests
 9/21 Test  #9: secure_write_fault_tests .......................   Passed    0.69 sec
      Start 10: game_session
10/21 Test #10: game_session ...................................   Passed    0.02 sec
      Start 11: game_engine_tests
11/21 Test #11: game_engine_tests ..............................   Passed    1.34 sec
      Start 12: game_mode_tests
12/21 Test #12: game_mode_tests ................................   Passed   27.14 sec
      Start 13: powerup_tests
13/21 Test #13: powerup_tests ..................................   Passed    2.83 sec
      Start 14: gameplay_tests
14/21 Test #14: gameplay_tests .................................   Passed    1.17 sec
      Start 15: settings_tests
15/21 Test #15: settings_tests .................................   Passed    0.12 sec
      Start 16: audio_model_tests
16/21 Test #16: audio_model_tests ..............................   Passed    0.09 sec
      Start 17: performance_probe_tests
17/21 Test #17: performance_probe_tests ........................   Passed    0.05 sec
      Start 18: layering_core_is_sfml_free
18/21 Test #18: layering_core_is_sfml_free .....................   Passed    0.02 sec
      Start 19: security_banned_api_scan
19/21 Test #19: security_banned_api_scan .......................   Passed    0.38 sec
      Start 20: security_banned_api_scan_finds_planted_calls
20/21 Test #20: security_banned_api_scan_finds_planted_calls ...   Passed    0.01 sec
      Start 21: security_banned_api_scan_ignores_lookalikes
21/21 Test #21: security_banned_api_scan_ignores_lookalikes ....   Passed    0.01 sec

100% tests passed out of 21

Total Test time (real) =  35.48 sec
```

## Static analysis

```text
-- Running Cppcheck 2.13.0
-- cppcheck: no findings in src/, include/ and tests/
```

## Audio backend

```text
Restored backend gains: music=25 sfx=80
7 real SFML/OpenAL audio backend checks passed
```

## Navigation

```text
PASS: blank name cannot advance by mouse
PASS: whitespace-only name cannot advance
PASS: 16th character is rejected at runtime
PASS: invalid character rejected without inserting it
PASS: mouse Back returns from name entry
PASS: three confirmation-click launch, mouse Resume and Main menu
PASS: leaderboard table has working mouse Back
PASS: mouse launch/pause/menu: Classic/Easy
PASS: mouse launch/pause/menu: Classic/Medium
PASS: mouse launch/pause/menu: Classic/Hard
PASS: mouse launch/pause/menu: Timed/Easy
PASS: mouse launch/pause/menu: Timed/Medium
PASS: mouse launch/pause/menu: Timed/Hard
PASS: mouse launch/pause/menu: Obstacle/Easy
PASS: mouse launch/pause/menu: Obstacle/Medium
PASS: mouse launch/pause/menu: Obstacle/Hard
PASS: mouse restart after actual wall collision
PASS: gap click does not activate key capture
PASS: duplicate movement binding rejected in real UI
PASS: drag beyond slider track clamps without changing other channel
PASS: mouse Save persists clamped independent volumes
PASS: clean default-mode exit
PASS: default mode produces no probe files
23 desktop navigation/settings checks passed; target Windows/manual acceptance pending
```

## Settings

```text
PASS: missing config generates 70/70 defaults
PASS: Settings opens without changing a row
PASS: desktop independent music 0 / SFX 100 preview
PASS: Save persists independent channel volumes
PASS: desktop independent music 100 / SFX 0 preview
PASS: Cancel does not overwrite saved configuration
PASS: reopen restores saved volume draft after Cancel
PASS: theme selection persists
PASS: desktop remap to I/K/J/L persists SFML key codes
PASS: failed Save stays on Settings with visible error
PASS: failed Save preserves existing target directory
PASS: failed Save removes only its owned temporary file
PASS: Cancel after failed Save restores previously active values
PASS: recovered fixture retains all saved settings
PASS: normal menu exit returns 0
PASS: real application relaunch restores settings
PASS: normal menu exit returns 0
PASS: real startup regenerates corrupt config defaults
PASS: corrupt-file warning emitted
PASS: normal menu exit returns 0
PASS: no audio backend leaves Settings usable (silent fallback)
PASS: normal menu exit returns 0
22 Linux/Xvfb desktop Settings checks passed; null output, no audible or Windows acceptance.
```

## Leaderboard

```text
PASS: Top-10 table exposes rank name and descending score for every row
PASS: all ten rows expose mode difficulty and ISO date, covering all nine combinations
PASS: maximum-width name and score plus leap-day date survive presentation
PASS: viewing the six-column table does not rewrite persisted entries
PASS: Escape returns to menu
PASS: mouse Back returns to menu
PASS: clean app exit
PASS: completed session saves normalized player name
PASS: completed session persists an ISO date without a timestamp
PASS: completed session persists the active mode and difficulty
PASS: Game Over frames do not resubmit score
PASS: new score appears in the real UI table
PASS: clean app exit
PASS: saved leaderboard survives process restart
PASS: clean app exit
PASS: zero-byte board surfaces D6 warning
PASS: zero-byte board is not rewritten merely by viewing
PASS: clean app exit
PASS: whitespace-only board surfaces D6 warning
PASS: whitespace-only board is not rewritten merely by viewing
PASS: clean app exit
PASS: missing board has no false warning
PASS: missing board remains unchanged on viewing
PASS: clean app exit
PASS: valid empty board has no false warning
PASS: valid empty board remains unchanged on viewing
PASS: clean app exit
PASS: corrupt board displays empty table and warning without crashing
PASS: clean app exit
29 leaderboard integration checks passed; Windows/manual acceptance pending
```

## Security UI

```text
PASS main menu shown - SnakeXtreme | Main menu | Up/Down: select, Enter: confirm, Esc: exit
PASS New Game opens name entry - SnakeXtreme | Name:  |
PASS TC-SEC-06: memory stays stable while 10,000 characters are typed - RSS 100024 kB -> 100024 kB
PASS TC-SEC-06: 10,000 typed characters leave exactly 15 in the field - name='AAAAAAAAAAAAAAA', 9.4 s, title: SnakeXtreme | Name: AAAAAAAAAAAAAAA | Maximum 15 characters.
PASS TC-SEC-06: the 16th and later characters are refused with a message - SnakeXtreme | Name: AAAAAAAAAAAAAAA | Maximum 15 characters.
PASS TC-SEC-01: 'Ab,cd': each refused character shows the message; field keeps 'Abcd' - name='Abcd', refused 1/1 with message
PASS TC-SEC-01: 'a"b': each refused character shows the message; field keeps 'ab' - name='ab', refused 1/1 with message
PASS TC-SEC-01: '../x': each refused character shows the message; field keeps 'x' - name='x', refused 3/3 with message
PASS TC-SEC-01: '{"x":1}': each refused character shows the message; field keeps 'x1' - name='x1', refused 5/5 with message
PASS TC-SEC-01: 'C:\\x': each refused character shows the message; field keeps 'Cx' - name='Cx', refused 2/2 with message
PASS TC-SEC-01: 'a;b|c': each refused character shows the message; field keeps 'abc' - name='abc', refused 2/2 with message
PASS TC-SEC-01: 'España': each refused character shows the message; field keeps 'Espaa' - name='Espaa', refused 1/1 with message
PASS TC-SEC-01: 'emoji😀': each refused character shows the message; field keeps 'emoji' - name='emoji', refused 1/1 with message
PASS TC-SEC-01: Tab inserts nothing - name=''
PASS TC-SEC-01: an empty name cannot be confirmed - SnakeXtreme | Name:  | Enter a valid name (1-15 characters), not only spaces.
PASS TC-SEC-01: the valid name A_b-1 x is accepted - SnakeXtreme | Menu | Classic / Medium | Left/Right: mode, Up/Down: difficulty, Enter: start, Esc: exit
PASS game starts - SnakeXtreme | Playing | Classic / Medium | Score 0 | High 0 | Esc: pause, M: menu
PASS game over reached - SnakeXtreme | Game over: You hit the wall | Classic / Medium | Score 0 | High 0 | Score saved: rank 1 | Enter: play agai
PASS clean exit - exit code 0
files written: ['config.json', 'leaderboard.json', 'runtime.log']
PASS TC-SEC-02: config.json has no user name, host name or absolute path - 153 bytes
PASS TC-SEC-02: leaderboard.json has no user name, host name or absolute path - 196 bytes
PASS TC-SEC-02: runtime.log has no user name, host name or absolute path - 95 bytes
PASS TC-SEC-02: config.json holds only the Appendix B fields - ['key_down', 'key_left', 'key_right', 'key_up', 'music_volume', 'sfx_volume', 'theme', 'version']
runtime log: Setting vertical sync not supported | [WARN] SettingsRepository: E_FILE_MISSING: defaults loaded
SUMMARY 23/23 passed
```

## Power-up runtime

```text
PASS main menu ready - SnakeXtreme | Main menu | Up/Down: select, Enter: confirm, Esc: exit
PASS name entered - SnakeXtreme | Name: Tejas |
PASS mode menu ready - SnakeXtreme | Menu | Classic / Medium | Left/Right: mode, Up/Down: difficulty, Enter: start, Esc: exit
PASS Classic / Medium game started - SnakeXtreme | Playing | Classic / Medium | Score 0 | High 0 | Esc: pause, M: menu
process exit code: 0
PASS clean exit - exit code 0
files written: ['config.json', 'runtime.log']
played 150.8 s, 1310 moves, state Playing
appearances: [('8.6', (19, 1), 'Speed'), ('18.5', (12, 1), 'Multiplier'), ('33.5', (6, 3), 'Shrink'), ('45.7', (28, 5), 'Multiplier'), ('55.0', (17, 19), 'Shrink'), ('63.1', (0, 9), 'Shrink'), ('76.5', (15, 19), 'Speed'), ('89.0', (21, 7), 'Multiplier'), ('100.4', (9, 3), 'Speed'), ('112.0', (2, 3), 'Multiplier'), ('124.2', (15, 4), 'Multiplier'), ('137.1', (22, 0), 'Multiplier')]
collected: {'type': 'Speed', 'time': 10.912665128707886, 'length_before': 8, 'length_after': 8, 'move_index': 92, 'effect_title': ('Speed', 5.0), 'boost_ms': 71.87118201420226, 'end': 15.982272386550903, 'end_index': 162, 'after_ms': 120.51770687103271}
collected: {'type': 'Multiplier', 'time': 21.38871169090271, 'length_before': 15, 'length_after': 15, 'move_index': 207, 'effect_title': ('Multiplier', 10.0), 'pause': (10.0, 'Paused', 10.0)}
collected: {'type': 'Multiplier', 'time': 49.205267906188965, 'length_before': 23, 'length_after': 23, 'move_index': 414, 'effect_title': ('Multiplier', 10.0)}
collected: {'type': 'Shrink', 'time': 56.27950167655945, 'length_before': 24, 'length_after': 23, 'move_index': 473}
collected: {'type': 'Shrink', 'time': 66.22492742538452, 'length_before': 26, 'length_after': 25, 'move_index': 556}
collected: {'type': 'Speed', 'time': 77.62109804153442, 'length_before': 25, 'length_after': 25, 'move_index': 651, 'effect_title': ('Speed', 5.0), 'boost_ms': 72.98566555154734, 'end': 82.71892380714417, 'end_index': 721, 'after_ms': 120.39661407470703}
collected: {'type': 'Multiplier', 'time': 90.64392137527466, 'length_before': 25, 'length_after': 25, 'move_index': 787, 'effect_title': ('Multiplier', 10.0)}
collected: {'type': 'Speed', 'time': 103.58681464195251, 'length_before': 25, 'length_after': 25, 'move_index': 895, 'effect_title': ('Speed', 5.0), 'boost_ms': 72.5450926813586, 'end': 108.69623351097107, 'end_index': 965, 'after_ms': 120.31967639923096}
collected: {'type': 'Multiplier', 'time': 114.32166695594788, 'length_before': 25, 'length_after': 25, 'move_index': 1012, 'effect_title': ('Multiplier', 10.0)}
collected: {'type': 'Multiplier', 'time': 125.36787724494934, 'length_before': 25, 'length_after': 25, 'move_index': 1104, 'effect_title': ('Multiplier', 10.0)}
collected: {'type': 'Multiplier', 'time': 138.7034661769867, 'length_before': 25, 'length_after': 25, 'move_index': 1215, 'effect_title': ('Multiplier', 10.0)}
food with x2: [20, 20, 20, 20, 20, 20, 20] food without: [10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10]
log: Shrink at (6, 3) gone without pick-up at 37.5 s
log: Shrink at (6, 3) gone without pick-up at 37.8 s
log: Shrink at (6, 3) gone without pick-up at 38.0 s
PASS power-ups keep appearing during play - 12 seen in 151 s (0 restarts)
gaps between seen appearances (s): 9.9, 14.9, 12.2, 9.2, 8.2, 13.4, 12.4, 11.4, 11.7, 12.1, 13.0
PASS Speed: title shows "Speed boost" with seconds left - ('Speed', 5.0)
PASS Speed: moves every ~72 ms during the boost - 71.9 ms over 29 moves
PASS Speed: boost ends after ~5 s - 5.07 s
PASS Speed: back to ~120 ms afterwards - 120.5 ms
PASS Multiplier: title shows "Score x2" with seconds left - ('Multiplier', 10.0)
PASS Multiplier: pause freezes the effect time - before 10.0, paused Paused 10.0
PASS food during x2 scores +20 - [20, 20, 20, 20, 20, 20, 20]
PASS food without x2 scores +10 - [10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10]
PASS Shrink: length 24 -> 23 - measured 23
PASS Shrink: length 26 -> 25 - measured 25
collected types: ['Multiplier', 'Shrink', 'Speed']
SUMMARY 16/16 passed
```

## Line coverage detail

```text
------------------------------------------------------------------------------
                           GCC Code Coverage Report
Directory: .
------------------------------------------------------------------------------
File                                         Lines     Exec  Cover   Missing
------------------------------------------------------------------------------
include/snakextreme/AppState.hpp                 1        1   100%
include/snakextreme/AudioModel.hpp               2        2   100%
include/snakextreme/DifficultyProfile.hpp
                                                11       10    90%   31
include/snakextreme/GameEngine.hpp               5        5   100%
include/snakextreme/GameMode.hpp                28       26    92%   43,55
include/snakextreme/GameSession.hpp             19       19   100%
include/snakextreme/LeaderboardModel.hpp
                                                 5        5   100%
include/snakextreme/LeaderboardTable.hpp
                                                 9        9   100%
include/snakextreme/MenuSelection.hpp           33       33   100%
include/snakextreme/ModeManager.hpp              1        1   100%
include/snakextreme/ModeStrategy.hpp             4        4   100%
include/snakextreme/NameEntry.hpp               31       29    93%   44-45
include/snakextreme/PerformanceProbe.hpp
                                                 6        6   100%
include/snakextreme/PowerUp.hpp                 15       14    93%   39
include/snakextreme/PowerUpManager.hpp           2        2   100%
include/snakextreme/Settings.hpp                 5        5   100%
include/snakextreme/SettingsRepository.hpp
                                                 1        1   100%
include/snakextreme/UiHitTest.hpp               32       28    87%   30-33
src/app/AppState.cpp                            32       31    96%   35
src/audio/AudioModel.cpp                        34       32    94%   29,48
src/diagnostics/PerformanceProbe.cpp            83       78    94%   12,67,89,97-98
src/engine/GameEngine.cpp                      208      204    98%   91,93,196,198
src/gameplay/GameplayController.cpp            143      140    97%   250-251,253
src/modes/ModeManager.cpp                       49       47    95%   43-44
src/modes/ModeStrategy.cpp                     119      117    98%   60,251
src/persistence/LeaderboardModel.cpp            21       14    66%   15,19-20,25-28
src/persistence/LeaderboardRepository.cpp
                                               196      180    91%   33,53,103,114-116,214,226-228,329-330,338-339,345,380
src/persistence/SecureFileStore.cpp            365      316    86%   117-120,157,180,193,223-225,287,295,304,314,327,332,366,370,373,377-378,401,406,445-446,455,530,543,559,630,635,659,665,678,680,682,685,690,725,777,828,840,854-856,871,894,929-930
src/persistence/Settings.cpp                    60       56    93%   28-31
src/persistence/SettingsRepository.cpp          50       49    98%   69
src/powerups/PowerUpEffects.cpp                 60       56    93%   76,95-97
src/powerups/PowerUpManager.cpp                106      106   100%
src/scoring/ScoreService.cpp                    25       24    96%   18
src/validation/InputValidator.cpp               37       36    97%   67
------------------------------------------------------------------------------
TOTAL                                         1798     1686    93%
------------------------------------------------------------------------------
```

## Coverage summary JSON

```text
{"root": "../part3-source", "gcovr/summary_format_version": "0.6", "files": [{"filename": "include/snakextreme/AppState.hpp", "line_total": 1, "line_covered": 1, "line_percent": 100.0, "function_total": 1, "function_covered": 1, "function_percent": 100.0, "branch_total": 0, "branch_covered": 0, "branch_percent": null}, {"filename": "include/snakextreme/AudioModel.hpp", "line_total": 2, "line_covered": 2, "line_percent": 100.0, "function_total": 2, "function_covered": 2, "function_percent": 100.0, "branch_total": 0, "branch_covered": 0, "branch_percent": null}, {"filename": "include/snakextreme/DifficultyProfile.hpp", "line_total": 11, "line_covered": 10, "line_percent": 90.9, "function_total": 2, "function_covered": 2, "function_percent": 100.0, "branch_total": 4, "branch_covered": 3, "branch_percent": 75.0}, {"filename": "include/snakextreme/GameEngine.hpp", "line_total": 5, "line_covered": 5, "line_percent": 100.0, "function_total": 3, "function_covered": 3, "function_percent": 100.0, "branch_total": 4, "branch_covered": 4, "branch_percent": 100.0}, {"filename": "include/snakextreme/GameMode.hpp", "line_total": 28, "line_covered": 26, "line_percent": 92.9, "function_total": 4, "function_covered": 4, "function_percent": 100.0, "branch_total": 16, "branch_covered": 14, "branch_percent": 87.5}, {"filename": "include/snakextreme/GameSession.hpp", "line_total": 19, "line_covered": 19, "line_percent": 100.0, "function_total": 6, "function_covered": 6, "function_percent": 100.0, "branch_total": 2, "branch_covered": 2, "branch_percent": 100.0}, {"filename": "include/snakextreme/LeaderboardModel.hpp", "line_total": 5, "line_covered": 5, "line_percent": 100.0, "function_total": 4, "function_covered": 4, "function_percent": 100.0, "branch_total": 2, "branch_covered": 1, "branch_percent": 50.0}, {"filename": "include/snakextreme/LeaderboardTable.hpp", "line_total": 9, "line_covered": 9, "line_percent": 100.0, "function_total": 2, "function_covered": 2, "function_percent": 100.0, "branch_total": 30, "branch_covered": 15, "branch_percent": 50.0}, {"filename": "include/snakextreme/MenuSelection.hpp", "line_total": 33, "line_covered": 33, "line_percent": 100.0, "function_total": 3, "function_covered": 3, "function_percent": 100.0, "branch_total": 17, "branch_covered": 15, "branch_percent": 88.2}, {"filename": "include/snakextreme/ModeManager.hpp", "line_total": 1, "line_covered": 1, "line_percent": 100.0, "function_total": 0, "function_covered": 0, "function_percent": null, "branch_total": 0, "branch_covered": 0, "branch_percent": null}, {"filename": "include/snakextreme/ModeStrategy.hpp", "line_total": 4, "line_covered": 4, "line_percent": 100.0, "function_total": 4, "function_covered": 4, "function_percent": 100.0, "branch_total": 0, "branch_covered": 0, "branch_percent": null}, {"filename": "include/snakextreme/NameEntry.hpp", "line_total": 31, "line_covered": 29, "line_percent": 93.5, "function_total": 4, "function_covered": 4, "function_percent": 100.0, "branch_total": 36, "branch_covered": 26, "branch_percent": 72.2}, {"filename": "include/snakextreme/PerformanceProbe.hpp", "line_total": 6, "line_covered": 6, "line_percent": 100.0, "function_total": 6, "function_covered": 6, "function_percent": 100.0, "branch_total": 0, "branch_covered": 0, "branch_percent": null}, {"filename": "include/snakextreme/PowerUp.hpp", "line_total": 15, "line_covered": 14, "line_percent": 93.3, "function_total": 5, "function_covered": 5, "function_percent": 100.0, "branch_total": 4, "branch_covered": 3, "branch_percent": 75.0}, {"filename": "include/snakextreme/PowerUpManager.hpp", "line_total": 2, "line_covered": 2, "line_percent": 100.0, "function_total": 0, "function_covered": 0, "function_percent": null, "branch_total": 0, "branch_covered": 0, "branch_percent": null}, {"filename": "include/snakextreme/Settings.hpp", "line_total": 5, "line_covered": 5, "line_percent": 100.0, "function_total": 5, "function_covered": 5, "function_percent": 100.0, "branch_total": 0, "branch_covered": 0, "branch_percent": null}, {"filename": "include/snakextreme/SettingsRepository.hpp", "line_total": 1, "line_covered": 1, "line_percent": 100.0, "function_total": 1, "function_covered": 1, "function_percent": 100.0, "branch_total": 0, "branch_covered": 0, "branch_percent": null}, {"filename": "include/snakextreme/UiHitTest.hpp", "line_total": 32, "line_covered": 28, "line_percent": 87.5, "function_total": 5, "function_covered": 5, "function_percent": 100.0, "branch_total": 54, "branch_covered": 41, "branch_percent": 75.9}, {"filename": "src/app/AppState.cpp", "line_total": 32, "line_covered": 31, "line_percent": 96.9, "function_total": 1, "function_covered": 1, "function_percent": 100.0, "branch_total": 39, "branch_covered": 35, "branch_percent": 89.7}, {"filename": "src/audio/AudioModel.cpp", "line_total": 34, "line_covered": 32, "line_percent": 94.1, "function_total": 4, "function_covered": 4, "function_percent": 100.0, "branch_total": 32, "branch_covered": 25, "branch_percent": 78.1}, {"filename": "src/diagnostics/PerformanceProbe.cpp", "line_total": 83, "line_covered": 78, "line_percent": 94.0, "function_total": 10, "function_covered": 9, "function_percent": 90.0, "branch_total": 462, "branch_covered": 242, "branch_percent": 52.4}, {"filename": "src/engine/GameEngine.cpp", "line_total": 208, "line_covered": 204, "line_percent": 98.1, "function_total": 24, "function_covered": 24, "function_percent": 100.0, "branch_total": 217, "branch_covered": 159, "branch_percent": 73.3}, {"filename": "src/gameplay/GameplayController.cpp", "line_total": 143, "line_covered": 140, "line_percent": 97.9, "function_total": 28, "function_covered": 28, "function_percent": 100.0, "branch_total": 141, "branch_covered": 106, "branch_percent": 75.2}, {"filename": "src/modes/ModeManager.cpp", "line_total": 49, "line_covered": 47, "line_percent": 95.9, "function_total": 16, "function_covered": 15, "function_percent": 93.8, "branch_total": 24, "branch_covered": 19, "branch_percent": 79.2}, {"filename": "src/modes/ModeStrategy.cpp", "line_total": 119, "line_covered": 117, "line_percent": 98.3, "function_total": 23, "function_covered": 23, "function_percent": 100.0, "branch_total": 110, "branch_covered": 90, "branch_percent": 81.8}, {"filename": "src/persistence/LeaderboardModel.cpp", "line_total": 21, "line_covered": 14, "line_percent": 66.7, "function_total": 2, "function_covered": 2, "function_percent": 100.0, "branch_total": 32, "branch_covered": 10, "branch_percent": 31.2}, {"filename": "src/persistence/LeaderboardRepository.cpp", "line_total": 196, "line_covered": 180, "line_percent": 91.8, "function_total": 13, "function_covered": 13, "function_percent": 100.0, "branch_total": 364, "branch_covered": 245, "branch_percent": 67.3}, {"filename": "src/persistence/SecureFileStore.cpp", "line_total": 365, "line_covered": 316, "line_percent": 86.6, "function_total": 21, "function_covered": 20, "function_percent": 95.2, "branch_total": 504, "branch_covered": 306, "branch_percent": 60.7}, {"filename": "src/persistence/Settings.cpp", "line_total": 60, "line_covered": 56, "line_percent": 93.3, "function_total": 13, "function_covered": 12, "function_percent": 92.3, "branch_total": 117, "branch_covered": 80, "branch_percent": 68.4}, {"filename": "src/persistence/SettingsRepository.cpp", "line_total": 50, "line_covered": 49, "line_percent": 98.0, "function_total": 4, "function_covered": 4, "function_percent": 100.0, "branch_total": 182, "branch_covered": 117, "branch_percent": 64.3}, {"filename": "src/powerups/PowerUpEffects.cpp", "line_total": 60, "line_covered": 56, "line_percent": 93.3, "function_total": 15, "function_covered": 15, "function_percent": 100.0, "branch_total": 44, "branch_covered": 26, "branch_percent": 59.1}, {"filename": "src/powerups/PowerUpManager.cpp", "line_total": 106, "line_covered": 106, "line_percent": 100.0, "function_total": 16, "function_covered": 16, "function_percent": 100.0, "branch_total": 110, "branch_covered": 79, "branch_percent": 71.8}, {"filename": "src/scoring/ScoreService.cpp", "line_total": 25, "line_covered": 24, "line_percent": 96.0, "function_total": 7, "function_covered": 7, "function_percent": 100.0, "branch_total": 5, "branch_covered": 4, "branch_percent": 80.0}, {"filename": "src/validation/InputValidator.cpp", "line_total": 37, "line_covered": 36, "line_percent": 97.3, "function_total": 4, "function_covered": 4, "function_percent": 100.0, "branch_total": 48, "branch_covered": 39, "branch_percent": 81.2}], "line_total": 1798, "line_covered": 1686, "line_percent": 93.8, "function_total": 258, "function_covered": 254, "function_percent": 98.4, "branch_total": 2600, "branch_covered": 1706, "branch_percent": 65.6}
```
