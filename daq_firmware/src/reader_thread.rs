use std::sync::mpsc::Sender;
use std::sync::{Arc, RwLock};

use crate::fsm::State;
use crate::WriteMessages;

pub fn reader_thread_main(state: Arc<RwLock<State>>, writer_handle: Sender<WriteMessages>) {
    todo!();
}
