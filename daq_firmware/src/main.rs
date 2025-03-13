use std::{
    // fs::File,
    // io::{BufWriter, Write},
    thread,
    time::{Duration, Instant},
};

use rppal::spi::{Bus, Mode, Segment, SlaveSelect, Spi};

const TMP_READ_WORD: [u8; 2] = [0x01, 0x00];
const TMP_CONVERSION_FACTOR: f32 = 0.03125;
const BARO_READ_PRESSURE: [u8; 1] = [0b11101000];
// const ADC_READ: [u8; 8] = [0x08, 0x0, 0b00010000, 0x0, 0b00011000, 0x0, 0x0, 0x0];
// const ADC_VOLT_PER_LSB: f32 = 0.000805802;

fn main() {
    // let adc = Spi::new(Bus::Spi0, SlaveSelect::Ss2, 3200000, Mode::Mode0).unwrap();

    // TI TMP126
    let temp1 = Spi::new(Bus::Spi0, SlaveSelect::Ss0, 5000000, Mode::Mode0).unwrap();
    let temp2 = Spi::new(Bus::Spi0, SlaveSelect::Ss1, 5000000, Mode::Mode0).unwrap();

    // STM LPS25HBTR
    let barometer = Spi::new(Bus::Spi0, SlaveSelect::Ss3, 5000000, Mode::Mode0).unwrap();

    // barometer.set_bits_per_word(16).unwrap();

    // let csv = File::create("launch_stats.csv").unwrap();
    // let mut buf_writer = BufWriter::new(csv);

    let start = Instant::now();

    loop {
        let mut temp1_readings = [0u8, 0u8];
        let mut temp2_readings = [0u8, 0u8];
        let temp1_xfer = [
            Segment::with_write(&TMP_READ_WORD),
            Segment::with_read(&mut temp1_readings),
        ];
        let temp2_xfer = [
            Segment::with_write(&TMP_READ_WORD),
            Segment::with_read(&mut temp2_readings),
        ];

        let mut temp1_raw: f32 = 0.0;

        if let Ok(()) = temp1.transfer_segments(&temp1_xfer) {
            let temp = (((temp1_readings[0] as u16) << 8) | temp1_readings[1] as u16) >> 2;
            if temp & 0b0010000000000000 == 0 {
                temp1_raw = f32::try_from(temp).unwrap() * TMP_CONVERSION_FACTOR;
            } else {
                let temp = (0b1000000000000000 | (temp & 0b0001111111111111)) as i16;
                temp1_raw = f32::try_from(temp).unwrap() * TMP_CONVERSION_FACTOR;
            }
        }

        let mut temp2_raw: f32 = 0.0;

        if let Ok(()) = temp2.transfer_segments(&temp2_xfer) {
            let temp = (((temp2_readings[0] as u16) << 8) | temp2_readings[1] as u16) >> 2;
            if temp & 0b0010000000000000 == 0 {
                temp2_raw = f32::try_from(temp).unwrap() * TMP_CONVERSION_FACTOR;
            } else {
                let temp = (0b1000000000000000 | (temp & 0b0001111111111111)) as i16;
                temp2_raw = f32::try_from(temp).unwrap() * TMP_CONVERSION_FACTOR;
            }
        }

        let mut baro_readings = [0u8, 0u8, 0u8];

        let baro_xfer = [
            Segment::with_write(&BARO_READ_PRESSURE),
            Segment::with_read(&mut baro_readings),
        ];
        // keep clock going after initial word to keep reading
        let mut baro_out: i32 = 0;

        if let Ok(()) = barometer.transfer_segments(&baro_xfer) {
            baro_out = (baro_readings[2] as i32) << 16
                | (baro_readings[1] as i32) << 8
                | baro_readings[0] as i32;

            if baro_out & 32768 != 0 {
                baro_out |= 0b100000000111111111111111;
            }

            baro_out /= 4096;
        }
        //
        // let mut adc_readings = [0u8; 8];
        //
        // let mut thermo_raw: u16 = 0;
        // let mut pressure_raw: u16 = 0;
        // let mut thrust_raw: u16 = 0;
        //
        // let adc_xfer = [Segment::new(&mut adc_readings, &ADC_READ)];
        // if let Ok(()) = adc.transfer_segments(&adc_xfer) {
        //     // TODO: find magic values to convert stuff
        //     thermo_raw = (adc_readings[0] as u16) << 8 | adc_readings[1] as u16;
        //     pressure_raw = (adc_readings[2] as u16) << 8 | adc_readings[3] as u16;
        //     thrust_raw = (adc_readings[4] as u16) << 8 | adc_readings[5] as u16;
        //     let _battery_raw = (adc_readings[6] as u16) << 8 | adc_readings[7] as u16;
        //     println!(
        //         "Relative Temp: {}\nPressure: {}\nThrust: {}",
        //         thermo_raw, pressure_raw, thrust_raw
        //     );
        // }
        //
        // buf_writer
        //     .write(
        //         format!(
        //             "{}, {}, {}, {}, {}",
        //             start.elapsed().as_millis(),
        //             (temp1_raw + temp2_raw) / 2.0,
        //             baro_out,
        //             pressure_raw,
        //             thrust_raw
        //         )
        //         .as_bytes(),
        //     )
        //     .unwrap();
        //
        let timestamp = start.elapsed();

        println!(
            "Temperature is {}, pressure is {} hPA, Time: {}.{:03}",
            (temp1_raw + temp2_raw) / 2.0,
            baro_out,
            timestamp.as_secs(),
            timestamp.as_millis()
        );

        thread::sleep(Duration::from_millis(10));
    }
}
