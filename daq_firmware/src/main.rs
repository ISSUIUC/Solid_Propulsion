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
    let mut state = State::new();
}

