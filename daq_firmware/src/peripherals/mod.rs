use rppal::spi::Spi;

pub struct Rfm9x {
    spi_slave: Spi,
    // will also need pins corresponding to IRQ pins
}

impl Rfm9x {
    pub fn new(spi_slave: Spi) -> Self {
        Self { spi_slave }
    }

    pub fn read_register(&mut self, address: u8, buffer: &mut [u8]) {
        todo!();
    }

    pub fn write_register(&mut self, address: u8, buffer: &mut [u8]) {
        todo!();
    }

    pub fn receive_byte(&mut self) -> u8 {
        todo!();
    }

    pub fn send_byte(&mut self, input: u8) {
        todo!();
    }
}
