## Summary

Describe the change and why it is needed.

## How to test

- [ ] `cmake --build` with a documented preset
- [ ] `ctest --output-on-failure` (enable `BAJA_BUILD_TESTS` on Windows)
- [ ] On Linux, sanitizer ctest when the change touches parsers, physics, or ECS
- [ ] Gameplay smoke (`game --map hideout` and `game --map hospital`) when behavior or assets might be affected

## Notes

- New logic should come with Catch2 tests when it can run without a window.
- Do not add secrets, machine-specific `C:\\Users` paths, or large binaries.
- Source license is MIT; asset terms are in `assets/CREDITS.md`.
