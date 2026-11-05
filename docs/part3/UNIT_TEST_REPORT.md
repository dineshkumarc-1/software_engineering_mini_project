# Unit test report — Part 3

**Tested source:** `04d782a345464349da10c91d6796b3ed99fc7b96` (tree `e7f5e8ef5f943d067375af0b375420bf81b421f3`), clean detached checkout of main.  
**Run:** 5 November 2026 UTC, automated Linux headless Debug coverage build; not a Windows or human acceptance test.  
**Status:** In progress; no final QA sign-off.

## Results

| Run | Result | Scope |
|---|---|---|
| GCC-instrumented CTest suite | 21/21 entries passed; 0 failed | Headless tests plus security and layering gates; includes integration, not exclusively unit tests |
| GCC Release desktop CTest | 21/21 entries passed | Linux desktop-enabled build; not Windows |
| Clang ASan/UBSan CTest | 21/21 entries passed; no sanitizer output | Linux headless, leak detection enabled |
| cppcheck 2.13 | No findings | Project src, include and tests |
| Python unittest discovery | 33/33 tests passed | Project test tooling, separate from STP cases |

[Execution evidence](EVIDENCE.md) lists the CTest entries, Python tests and static-analysis output. Do not add these counts as independent STP case passes. No native Windows execution was performed in this Part 3 run.

## Environment and reproduction

Amazon Linux 2023 x86_64; GCC 11.5.0; CMake/CTest 4.4.4; Catch2 3.8.1; cached Catch2 and nlohmann/json sources. GCC Debug `--coverage -O0`; warnings-as-errors; app disabled. All inherited Linux capabilities removed for tests to make filesystem permission checks meaningful. The fresh Linux desktop executable SHA-256 is `b5c5c2cc8cb059e44e0cbf4ab881c2e33b639b16e89e3df4ad97dbcca1d62d8c`. Configure and build output are summarized in [execution evidence](EVIDENCE.md). Fresh build directory and cleared `.gcda` files before running CTest.

```sh
cmake -S . -B <build> -DSNAKE_BUILD_APP=OFF -DCMAKE_BUILD_TYPE=Debug -DSNAKE_WARNINGS_AS_ERRORS=ON -DFETCHCONTENT_SOURCE_DIR_CATCH2=<catch2-source> -DFETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON=<json-source> '-DCMAKE_CXX_FLAGS=--coverage -O0' '-DCMAKE_EXE_LINKER_FLAGS=--coverage'
cmake --build <build> --parallel 3
find <build> -name '*.gcda' -delete
setpriv --bounding-set=-all --inh-caps=-all --ambient-caps=-all ctest --test-dir <build> --output-on-failure
setpriv --bounding-set=-all --inh-caps=-all --ambient-caps=-all python3 -m unittest discover -s tests -p 'test_*.py' -v
```
Use normal unprivileged execution where capabilities are already absent.
