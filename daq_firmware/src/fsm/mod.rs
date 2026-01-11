#[cfg(test)]
mod tests;

#[derive(Debug)]
pub enum State {
    Inert,
    Standby,
    Armed,
    Fire,
}

impl State {
    pub fn transition(&mut self, transition: Transitions) {
        // If current state sees transition that is valid, transition, else stay the same
        // match *self {

        // }
        match *self {
            State::Inert => match transition {
                Transitions::Activate => {
                    *self = State::Standby;
                }
                _ => *self = State::Inert,
            },
            State::Standby => match transition {
                Transitions::Arm => {
                    *self = State::Armed;
                }
                _ => *self = State::Standby,
            },
            State::Armed => match transition {
                Transitions::Disarm => {
                    *self = State::Standby;
                }
                Transitions::Begin => {
                    *self = State::Fire;
                }
                _ => *self = State::Armed,
            },
            State::Fire => match transition {
                Transitions::End => {
                    *self = State::Inert;
                }
                _ => *self = State::Fire,
            },
        }
    }
    pub fn new() -> State {
        return State::Inert;
    }

    pub fn as_byte(&self) -> u8 {
        match *self {
            State::Inert => 1,
            State::Standby => 2,
            State::Armed => 3,
            State::Fire => 4,
        }
    }
}

#[derive(Debug)]
pub enum Transitions {
    Activate,
    Arm,
    Disarm,
    Begin,
    End,
}

impl Transitions {
    pub fn try_new(number: u8) -> Result<Self, String> {
        if number == 1 {
            return Ok(Transitions::Activate);
        } else if number == 2 {
            return Ok(Transitions::Arm);
        } else if number == 3 {
            return Ok(Transitions::Disarm);
        } else if number == 4 {
            return Ok(Transitions::Begin);
        } else if number == 5 {
            return Ok(Transitions::End);
        }
        Err(String::from("Error".to_string()))
    }
}
