use std::{
    fs::File,
    io::{BufWriter, Write},
    sync::{
        atomic::{AtomicBool, Ordering},
        Arc,
    },
    thread,
    time::{Duration, Instant},
};

use chrono;
use ctrlc;
use rppal::gpio::Gpio;
use rppal::spi::{Bus, Mode, Segment, SlaveSelect, Spi};

const TMP_READ_WORD: [u8; 2] = [0x01, 0x00];
const TMP_CONVERSION_FACTOR: f32 = 0.03125;
const BARO_READ_PRESSURE: [u8; 1] = [0b11101000];
const ADC_MAIN_READ: [u8; 8] = [0x08, 0x0, 0b00010000, 0x0, 0b00011000, 0x0, 0x0, 0x0];

fn main() {
    let gpios = Gpio::new().expect("Should be able to get GPIO access");

    // U3 MUX507
    let mux1_enable = gpios.get(22).expect("Should be able to get pin 22");
    // U6 MUX507
    let mux2_enable = gpios.get(23).expect("Should be able to get pin 23");
    let mux_select0 = gpios.get(24).expect("Should be able to get pin 24");
    let mux_select1 = gpios.get(25).expect("Should be able to get pin 25");
    let mux_select2 = gpios.get(26).expect("Should be able to get pin 26");

    // TI ADC124S01
    let adc = Spi::new(Bus::Spi0, SlaveSelect::Ss2, 12000000, Mode::Mode0).unwrap();

    // TI TMP126
    let temp1 = Spi::new(Bus::Spi0, SlaveSelect::Ss0, 5000000, Mode::Mode0).unwrap();
    let temp2 = Spi::new(Bus::Spi0, SlaveSelect::Ss1, 5000000, Mode::Mode0).unwrap();

    // STM LPS25HBTR
    let barometer = Spi::new(Bus::Spi0, SlaveSelect::Ss3, 5000000, Mode::Mode0).unwrap();

    let csv = File::create(format!(
        "launches/launch_stats_{}.csv",
        chrono::offset::Local::now().format("%Y-%m-%d-%H:%M:%S")
    ))
    .unwrap();
    let mut buf_writer = BufWriter::new(csv);

    writeln!(buf_writer, "Temp1, Pressure, Timestamp").expect("Cannot write to file");

    let mut mux1_enable = mux1_enable.into_output_high();
    let mut mux2_enable = mux2_enable.into_output_low();
    let mut mux_select0 = mux_select0.into_output_high();
    let _mux_select1 = mux_select1.into_output_high();
    let _mux_select2 = mux_select2.into_output_high();

    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();

    ctrlc::set_handler(move || {
        r.store(false, Ordering::SeqCst);
        println!("Finishing");
    })
    .expect("Should be able to set handler");

    let start = Instant::now();

    while running.load(Ordering::SeqCst) {
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

        dbg!(temp1_raw, temp2_raw);

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

        dbg!(baro_out);

        let mut main_adc_readings = [0u8; 8];

        // On 8 of U3
        let mut thermo1_final = 0.0;
        let mut pressure_final = 0.0;
        let mut thrust_final = 0.0;
        let adc_xfer = [Segment::new(&mut main_adc_readings, &ADC_MAIN_READ)];

        if let Ok(()) = adc.transfer_segments(&adc_xfer) {
            // TODO: find magic values to convert stuff
            // (ADC_voltage/4096.0 * 3.3v) / 101
            let thermo_raw = (main_adc_readings[0] as u16) << 8 | main_adc_readings[1] as u16;
            let pressure_raw = (main_adc_readings[2] as u16) << 8 | main_adc_readings[3] as u16;
            let thrust_raw = (main_adc_readings[4] as u16) << 8 | main_adc_readings[5] as u16;
            let _battery_raw = (main_adc_readings[6] as u16) << 8 | main_adc_readings[7] as u16;
            // println!(
            //     "Relative Temp: {}\nPressure: {}\nThrust: {}",
            //     thermo_raw, pressure_raw, thrust_raw
            // );

            dbg!(main_adc_readings);

            let thermo_voltage =
                ((f32::try_from(thermo_raw).unwrap() / 4095.0 * 3.3) / 101.0) * 1.0E6;
            dbg!(thermo_voltage);
            // Replace with any equation that takes microvolts as an input
            thermo1_final = 2.508355E-2 * thermo_voltage + 7.860106E-8 * thermo_voltage.powi(2)
                - 2.503131E-10 * thermo_voltage.powi(3)
                + 8.315270E-14 * thermo_voltage.powi(4)
                - 1.228034E-17 * thermo_voltage.powi(5)
                + 9.804036E-22 * thermo_voltage.powi(6)
                - 4.413030E-26 * thermo_voltage.powi(7)
                + 1.057734E-30 * thermo_voltage.powi(8)
                - 1.052755E-35 * thermo_voltage.powi(9);

            dbg!(thermo1_final);

            let pressure_voltage = f64::try_from(pressure_raw).unwrap() / 4095.0 * 3.3;
            dbg!(pressure_voltage);
            // Find range using equation
            let current = (pressure_voltage / 165.0) * 1.0E3;
            pressure_final = (current - 4.0) / 16.0 * 1500.0;
            dbg!(pressure_final);

            dbg!(thrust_raw);
            let thrust_voltage = f64::try_from(thrust_raw).unwrap() / 4095.0 * 3.3 / 137.98630137;
            dbg!(thrust_voltage);
            thrust_final = thrust_voltage / 24.6E-3 * 1500.0;
        }

        mux1_enable.set_low();
        mux2_enable.set_high();
        thread::sleep(Duration::from_millis(1));

        let mut thermo2_final = 0.0;
        let mut thermo2_readings = [0u8, 0u8];
        let adc_xfer = [Segment::new(&mut thermo2_readings, &[0u8, 0u8])];

        if let Ok(()) = adc.transfer_segments(&adc_xfer) {
            let thermo_raw = (thermo2_readings[0] as u16) << 8 | thermo2_readings[1] as u16;
            let thermo_voltage =
                ((f32::try_from(thermo_raw).unwrap() / 4095.0 * 3.3) / 101.0) * 1.0E6;

            thermo2_final = 2.508355E-2 * thermo_voltage + 7.860106E-8 * thermo_voltage.powi(2)
                - 2.503131E-10 * thermo_voltage.powi(3)
                + 8.315270E-14 * thermo_voltage.powi(4)
                - 1.228034E-17 * thermo_voltage.powi(5)
                + 9.804036E-22 * thermo_voltage.powi(6)
                - 4.413030E-26 * thermo_voltage.powi(7)
                + 1.057734E-30 * thermo_voltage.powi(8)
                - 1.052755E-35 * thermo_voltage.powi(9);
        }

        mux_select0.set_low();
        thread::sleep(Duration::from_millis(1));

        let mut thermo3_final = 0.0;
        let mut thermo3_readings = [0u8, 0u8];
        let adc_xfer = [Segment::new(&mut thermo3_readings, &[0u8, 0u8])];

        if let Ok(()) = adc.transfer_segments(&adc_xfer) {
            let thermo_raw = (thermo3_readings[0] as u16) << 8 | thermo3_readings[1] as u16;
            let thermo_voltage =
                ((f32::try_from(thermo_raw).unwrap() / 4095.0 * 3.3) / 101.0) * 1.0E6;

            thermo3_final = 2.508355E-2 * thermo_voltage + 7.860106E-8 * thermo_voltage.powi(2)
                - 2.503131E-10 * thermo_voltage.powi(3)
                + 8.315270E-14 * thermo_voltage.powi(4)
                - 1.228034E-17 * thermo_voltage.powi(5)
                + 9.804036E-22 * thermo_voltage.powi(6)
                - 4.413030E-26 * thermo_voltage.powi(7)
                + 1.057734E-30 * thermo_voltage.powi(8)
                - 1.052755E-35 * thermo_voltage.powi(9);
        }

        mux2_enable.set_low();
        mux1_enable.set_high();
        mux_select0.set_high();

        thread::sleep(Duration::from_millis(1));
        let timestamp = start.elapsed();

        // println!(
        //     "Temp1 is {:.4}, Temp2 is {:.4},Temp3 is {:.4}, Thrust is {:.4}, Pressure is {:.4} hPA",
        //     (temp1_raw + temp2_raw) / 2.0 + thermo1_final,
        //     (temp1_raw + temp2_raw) / 2.0 + thermo2_final,
        //     (temp1_raw + temp2_raw) / 2.0 + thermo3_final,
        //     f64::try_from(baro_out).unwrap() + pressure_final,
        // );

        if let Err(e) = writeln!(
            buf_writer,
            "{:.4}, {:.4}, {:.4}, {:.4}, {:.4} {}.{}",
            (temp1_raw + temp2_raw) / 2.0 + thermo1_final,
            (temp1_raw + temp2_raw) / 2.0 + thermo2_final,
            (temp1_raw + temp2_raw) / 2.0 + thermo3_final,
            f64::try_from(baro_out).unwrap() + pressure_final,
            thrust_final,
            timestamp.as_secs(),
            timestamp.subsec_millis()
        ) {
            eprintln!("Couldn't write to file: {}", e);
        }

        thread::sleep(Duration::from_millis(7));
    }

    if let Err(e) = buf_writer.flush() {
        eprintln!("Couldn't flush final contents to file: {}", e);
    }

    println!("Finished");
}
