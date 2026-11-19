# Verification Record

## Host-Side Control Checks

`App/Src/app_control.c` was compiled with Apple Clang using hardware stubs and `-Wall -Wextra -Werror`.

Verified:

- Temperature above the threshold enables the fan.
- Low humidity or a high soil ADC value enables the pump.
- A high light ADC value enables the grow light under the current sensor-polarity assumption.
- Temperature and humidity offsets trigger the critical alarm only after crossing the configured boundaries.

## EEPROM Regression Checks

Two related 8-bit address issues were identified:

1. `g_write_addr >= EEPROM_SIZE` was always false when `g_write_addr` was `uint8_t`, so the write pointer could wrap into reserved addresses instead of resetting to `EEPROM_DATA_START`.
2. The dump-loop address could wrap beyond the 40-record region and scan reserved addresses.

Both addresses are now 16-bit. EEPROM read/write functions continue to receive 8-bit addresses only after the wider counters have been bounds-checked.

A host-side stub test compiled with `-Wall -Wextra -Werror` verifies:

- Writing 41 records wraps safely instead of writing addresses `0`–`15`.
- Dumping reads exactly 40 records (`240` byte reads).
- Dumping never reads reserved addresses.
- The output contains the expected header/footer and 40 records.

## Firmware and Hardware Evidence

- The cleaned candidate has not been cross-compiled because `arm-none-eabi-gcc` is unavailable in the current environment.
- No STM32 board or sensor hardware was connected during cleanup, so this is not a new on-device test of the cleaned firmware.
- The original team defense presentation contains hardware photographs and a 3:06 demonstration of the dashboard and actuator behavior. Selected photographs and a compressed copy are included in `docs/` with team permission.
- The video records the original implementation. It does not replace the host-side EEPROM regression checks or independently verify the later address-width fix.
- The original machine-specific ELF and build outputs are not redistributed.

A future commit should add a fresh cross-build result when the ARM toolchain is available, but the project now includes real hardware evidence rather than only a proposed design.
