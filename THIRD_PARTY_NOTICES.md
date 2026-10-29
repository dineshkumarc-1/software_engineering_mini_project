# Third-party runtime notices

The generated music and sound-effect PCM in `src/audio/AudioModel.cpp` is original
project code; no third-party audio recordings or compositions are bundled.

## OpenAL Soft

The Windows build copies the unmodified `openal32.dll` supplied with the pinned
SFML 2.5.1 sources. Its embedded version is OpenAL Soft **1.18.1**. It is a
separate dynamically linked library under the GNU Library General Public License
version 2 (or later); see `assets/licenses/OpenAL-Soft-COPYING.txt`.

Corresponding upstream source, including build instructions:
https://github.com/kcat/openal-soft/tree/openal-soft-1.18.1

Source archive:
https://github.com/kcat/openal-soft/archive/refs/tags/openal-soft-1.18.1.tar.gz

SFML's unmodified packaged runtime:
https://github.com/SFML/SFML/tree/2.5.1/extlibs/bin

Users may replace/relink this dynamic library with a compatible modified version;
this project's distribution imposes no restriction on modifying that library or
reverse-engineering for debugging those modifications. Preserve these notices
and the library's licence when redistributing it. The repository does not vendor
the DLL: CMake copies it from SFML's fetched dependency into the build artifact.
Linux/macOS builds use their installed OpenAL runtime instead.

## Other existing dependencies

SFML 2.5.1: https://github.com/SFML/SFML/blob/2.5.1/license.md (zlib/png).
Catch2 v3.8.1 (tests): https://github.com/catchorg/Catch2/blob/v3.8.1/LICENSE.txt
(Boost Software License 1.0).
nlohmann/json v3.11.3: https://github.com/nlohmann/json/blob/v3.11.3/LICENSE.MIT
(MIT).
DejaVu Sans: the font's bundled `assets/fonts/dejavu-sans-ttf-2.37/LICENSE`.
