# daq4_firmware

STM32H745IIT6 (dual-core, LQFP176) DAQ firmware. Sensor drivers run on the Cortex-M7. The Cortex-M4 project is unchanged.

The CM7 drivers were first written in `DAQfirmware3/` and ported here. This project's CubeMX output was kept as generated. The driver code lives in its own files and inside `USER CODE` blocks.

**Status: nothing has run on hardware, and the project will not build until the pin map is decided** (see [Before first build](#before-first-build)).

## Board

| Part | Role |
| --- | --- |
| STM32H745IIT6 | Microcontroller |
| 8 thermocouple amplifiers | Feed the thermocouple ADC |
| TI ADS7066IYBHT (SPI1) | Thermocouple ADC: 8 channels, 16 bit |
| TI ADS7066IYBHT (SPI2) | Pressure transducer / load cell ADC ("ADC 2"): 8 channels, 16 bit |
| Op amp, precision resistors | Analog front end |
| 3.3 V external supply | Powers the ADCs. Both use their internal 2.5 V reference, so the input range is 0-2.5 V |
| SD card | Logging (no firmware yet) |

Requirements: 3.3 V logic, at least 500 Hz per channel, target 1 kHz per channel.

## Drivers

| Files (`CM7/Core/`) | What it does |
| --- | --- |
| `Inc/daq_board.h` | Pin map, SPI instances and ADC reference settings. **All placeholders** (`TODO(pin map)`) |
| `Src/spi_bus.c`, `Inc/spi_bus.h` | Two SPI buses, one ADC each, software chip selects, 16 MHz SCLK, mode 0 |
| `Src/ads7066.c`, `Inc/ads7066.h` | ADS7066: reset, presence and data-path checks, offset calibration, auto-sequence of AIN0-AIN7, reads all 8 channels |
| `Src/daq_timing.c`, `Inc/daq_timing.h` | Microsecond delays and the 1 kHz schedule, from the DWT cycle counter |

The ADS7066 driver was written from the TI datasheet (SBAS928C, revised September 2023), and the comments cite its sections:
- **Init:** a software reset, then a check that `SYSTEM_STATUS` reads `0x81`. That catches a missing part or a floating MISO line. Next it reads the `0xA5A5` test pattern, which checks the data path, and calibrates the offset. Every configuration write is read back to confirm it.
- **Reads:** each read is 9 frames of 24 bits. The first frame holds a stale result and is discarded. The next 8 hold one conversion of each channel. Each result carries the 4-bit channel ID, and the driver checks it, so results can't be stored under the wrong channel.
- **Timing:** there is at least 4 µs (one 250 kSPS cycle) between frames. That lets each conversion (3.2 µs) and the next sample (0.8 µs) finish.
- **SPI clock:** SCLK is 16 MHz, because 32 MHz would break the 16 ns SDO timing (`tD_CKDO`) in mode 0.

`main.c` reads both ADCs at 1 kHz into `daq_sample` (`tc_raw[8]`, `pt_raw[8]`) for viewing in a debugger. It also tracks `error_count`, `overrun_count` and `read_us_max`, the longest read of both ADCs. An ADC that fails init is skipped, and its status is in `tc_init_status` / `pt_init_status`.

All functions block until the transfer finishes (polling, no DMA or interrupts). They return `HAL_StatusTypeDef` and leave their outputs unchanged on failure.

## Before first build

1. Decide the pin map, then set every `TODO(pin map)` in `CM7/Core/Inc/daq_board.h`: both SPI buses, both chip selects, and `DAQ_BOARD_GPIO_CLK_ENABLE()`. SPI1 on PA4-PA7 comes from `DAQfirmware3.ioc`, and the SPI2 pins (PB12-PB15) are guesses. `daq4_firmware.ioc` has no SPI pins yet.
2. Uncomment `DAQ_BOARD_PINS_CONFIRMED`. Until then the build stops with an `#error` on purpose.
3. Build with `make` from `Makefile/CM7` (needs `arm-none-eabi-gcc`, e.g. `brew install --cask gcc-arm-embedded`).

## Changes to existing files

| File | Change | Why |
| --- | --- | --- |
| `CM7/Core/Inc/stm32h7xx_hal_conf.h` | Enabled `HAL_SPI_MODULE_ENABLED` | SPI types were not available |
| `Makefile/CM7/Makefile` | Added the driver sources and the HAL SPI sources | They were not being built |
| `CM7/Core/Src/stm32h7xx_hal_msp.c` | `HAL_SPI_MspInit` for both buses (in `USER CODE BEGIN 0`), kernel clock CKPER (HSI, 64 MHz) | The default source is PLL1Q, but the PLLs are off |
| `CM7/Core/Src/main.c` | Start-up calls and a 1 kHz sampling loop, inside `USER CODE` blocks | |

## Not done yet

- SD card logging. `daq_sample` is only visible through a debugger.
- Converting counts to temperature, pressure and force. This needs the amplifier gains, references and sensor calibrations. `ads7066_to_volts()` converts counts to volts.
- Moving the CPU off the 64 MHz HSI. The PLLs are off. This is enough for 1 kHz, but it is not a precise clock.
- The SPI peripherals and their pins are not in `daq4_firmware.ioc`.

## Check on hardware or against the schematic

- **`read_us_max`** should be well under 1000 µs. The estimate is about 150 µs for both ADCs, but HAL overhead has not been measured.
- **MCU power (not decided yet):** `SystemClock_Config` uses `PWR_DIRECT_SMPS_SUPPLY` (CubeMX setting). Change it in CubeMX if the board uses the LDO. A wrong setting can stop the MCU from starting.
- **Analog range:** every amplifier output must stay within 0-2.5 V, the internal reference range. Anything above reads as full scale. Also, the REF pins must have only a 1-10 µF capacitor, with no external reference.
- **Input drive:** the datasheet (section 8.2) recommends an amplifier and an RC charge-kickback filter in front of each input.

## Regenerating from CubeMX

`daq4_firmware.ioc` was not changed. It has no SPI peripherals or pins, so there is no `MX_GPIO_Init()`. That's fine: `spi_bus_init()` sets up the chip select pins, and `HAL_SPI_MspInit()` sets up the SCK/MISO/MOSI pins and clocks, all from `daq_board.h`. Regenerating will keep the code inside `USER CODE` blocks. It will probably overwrite the Makefile sources and the SPI module setting in `stm32h7xx_hal_conf.h`. To prevent that, enable SPI1 and SPI2 in CubeMX once the pin map is decided, and set the chip select pins as plain GPIO outputs. If CubeMX then generates its own `HAL_SPI_MspInit`, delete the one in `USER CODE BEGIN 0` of `stm32h7xx_hal_msp.c`.

## Testing so far

- All CM7 sources (and `stm32h7xx_hal_spi.c`) pass a clang type-check for Cortex-M7 against this project's HAL headers, with `-Wall -Wextra -Wconversion` and `DAQ_BOARD_PINS_CONFIRMED` defined, and produce no warnings. Without that define, the build stops with the intended `#error`.
- `ads7066.c` passed host tests (in `DAQfirmware3`) against a simulated ADS7066 that models the register protocol, reset, the test pattern, auto-sequencing and channel IDs. The file is identical here. The tests are not in the repo.
- **Not tested:** a full `arm-none-eabi-gcc` build and link, `spi_bus.c`, the `main.c` loop timing, and anything on hardware.
