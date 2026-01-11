#![deny(clippy::unwrap_used)]

use rppal::spi::{Bus, Mode, Segment, SlaveSelect, Spi};
use std::process;
use std::sync::{mpsc, Arc, RwLock};
use std::thread;

use crate::fsm::Transitions;
use crate::peripherals::Rfm9x;
use crate::reader_thread::reader_thread_main;
use crate::writer_thread::{writer_thread_main, WriteMessages};

mod fsm;
mod peripherals;
mod pins;
mod reader_thread;
mod writer_thread;

fn main() {
    let state = Arc::new(RwLock::<fsm::State>::new(fsm::State::new()));

    let (reader_tx, rx) = mpsc::channel::<WriteMessages>();

    let lora_tx = reader_tx.clone();

    let reader_state = state.clone();
    let _reader_thread = thread::spawn(move || reader_thread_main(reader_state, reader_tx));

    let writer_state = state.clone();
    let _writer_thread = thread::spawn(move || writer_thread_main(writer_state, rx));

    let radio_spi =
        Spi::new(Bus::Spi1, SlaveSelect::Ss0, 5_000_000, Mode::Mode0).unwrap_or_else(|e| {
            eprintln!("Error: {e}");
            process::exit(1);
        });
    let mut radio = Rfm9x::new(radio_spi);

    loop {
        // block and wait for radio signal.
        let data = radio.receive_byte();

        if let Ok(transition) = Transitions::try_new(data) {
            // send ack on success
            let mut state_handle = state.write().unwrap();

            state_handle.transition(transition);

            let current_state = state_handle.as_byte();
            radio.send_byte(current_state);
            lora_tx.send(WriteMessages::Log(current_state)).unwrap();
        } else {
            // send nack on failure
            radio.send_byte(0x41);
        }
    }
}
