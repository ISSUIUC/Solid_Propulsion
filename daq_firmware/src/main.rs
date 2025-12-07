use rppal::spi::{Bus, Mode, Segment, SlaveSelect, Spi};

enum State {
    Inert,
    Standby,
    Armed,
    Firing,
}

impl State {
    fn new() -> Self {
        State::Inert
    }
}

fn main() {
    let lora_spi=Spi::new(Bus::Spi0,SlaveSelect::Ss2,5000000,Mode::Mode0).unwrap();
    let mut test=0b00001000;
    let mut rec=0b00000000;
    let check_reg=lora_spi.transfer(test,rec).unwrap(); //Reads from address x10, should recieve xFF, Needs to be tested
    // println!(check_reg);
    lora_spi.write(0b1000000100001000); //Writes to set to sleep mode Write Format: 1 (For write mode) 7 bits (For address) 8 bits (For Data)
    let mut state = State::new();
}

