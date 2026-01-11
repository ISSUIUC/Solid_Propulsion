use std::sync::mpsc::Receiver;
use std::sync::{Arc, RwLock};

use crate::fsm::State;

pub enum WriteMessages {
    SensorData { readings: [u8; 4] },
    Log(u8),
}

pub fn writer_thread_main(state: Arc<RwLock<State>>, write_queue: Receiver<WriteMessages>) {}
