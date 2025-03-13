# Raspberry Pi Zero 2 W Firmware

This firmware is designed to run on the Raspberry Pi Zero 2 W and interfaces with the ADC128, TMP126, and LPS25HBTR sensors. It is written in Rust, leveraging its safety and performance benefits for embedded systems.

## Features

- **ADC128 Integration**: Utilizes the ADC128 for analog-to-digital conversion, enabling the reading of analog signals.
- **TMP126 Temperature Sensor**: Reads temperature data with high accuracy and low power consumption.
- **LPS25HBTR Pressure Sensor**: Monitors barometric pressure and temperature, providing environmental data.
- **Efficient Resource Management**: Optimized for the Raspberry Pi Zero 2 W, ensuring minimal resource usage.

## Prerequisites

- Raspberry Pi Zero 2 W
- Rust toolchain installed on your development machine
- `cross` tool for cross-compiling Rust applications
- SSH and SCP tools for remote development and deployment
- Docker (required for using `cross`)

## Building the Firmware

To build the firmware, it is preferable to use the `cross` tool for cross-compiling. This ensures that the binary is optimized for the Raspberry Pi Zero 2 W architecture.

1. **Install `cross`**:
   ```bash
   cargo install cross
   ```

   Note that Docker is required to run `cross`.

2. **Build the Project**:
   ```bash
   cross build --target aarch64-unknown-linux-gnu
   ```

   This command will generate a binary named `daq_firmware` compatible with the Raspberry Pi Zero 2 W.

## Deployment

The preferred method for developing and deploying the firmware is to use SSH and SCP. This allows for efficient transfer of the binary to the Raspberry Pi and remote execution.

1. **Transfer the Binary**:
   ```bash
   scp target/aarch64-unknown-linux-gnu/release/daq_firmware daq@sets3.local:/home/daq/
   ```

2. **Run the Binary**:
   ```bash
   ssh daq@sets3.local
   chmod +x ./home/daq/daq_firmware
   ./home/daq/daq_firmware
   ```

## Development Workflow

1. **Edit the Code**: Make changes to the Rust source files on your development machine.
2. **Build the Project**: Use `cross` to build the project for the Raspberry Pi Zero 2 W.
3. **Transfer the Binary**: Use SCP to transfer the compiled binary to the Raspberry Pi.
4. **Run and Test**: Execute the binary on the Raspberry Pi using SSH and monitor the output.

## License

This project is licensed under the University of Illinois/NCSA Open Source License. See the LICENSE file for more details.
