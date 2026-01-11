use crate::fsm::{State, Transitions};

#[test]
fn fsm_inits_correctly() {
    assert!(matches!(State::new(), State::Inert));
}

#[test]
fn fsm_inert_to_standby() {
    let mut state = State::new();

    state.transition(Transitions::Activate);

    assert!(matches!(state, State::Standby));
}

#[test]
fn fsm_ignores_bad_transitions_inert() {
    let mut state = State::new();
    state.transition(Transitions::Arm);
    assert!(matches!(state, State::Inert));

    state.transition(Transitions::Disarm);
    assert!(matches!(state, State::Inert));

    state.transition(Transitions::Begin);
    assert!(matches!(state, State::Inert));

    state.transition(Transitions::End);
    assert!(matches!(state, State::Inert));
}

#[test]
fn fsm_standby_to_armed() {
    let mut state = State::Standby;

    state.transition(Transitions::Arm);

    assert!(matches!(state, State::Armed));
}

#[test]
fn fsm_ignores_bad_transitions_standby() {
    let mut state = State::Standby;
    state.transition(Transitions::Activate);
    assert!(matches!(state, State::Standby));

    state.transition(Transitions::Disarm);
    assert!(matches!(state, State::Standby));

    state.transition(Transitions::Begin);
    assert!(matches!(state, State::Standby));

    state.transition(Transitions::End);
    assert!(matches!(state, State::Standby));
}

#[test]
fn fsm_armed_to_standby() {
    let mut state = State::Armed;

    state.transition(Transitions::Disarm);

    assert!(matches!(state, State::Standby));
}

#[test]
fn fsm_armed_to_firing() {
    let mut state = State::Armed;

    state.transition(Transitions::Begin);

    assert!(matches!(state, State::Fire));
}

#[test]
fn fsm_ignores_bad_transitions_armed() {
    let mut state = State::Armed;

    state.transition(Transitions::Activate);
    assert!(matches!(state, State::Armed));

    state.transition(Transitions::End);
    assert!(matches!(state, State::Armed));

    state.transition(Transitions::Arm);
    assert!(matches!(state, State::Armed));
}

#[test]
fn fsm_fire_to_inert() {
    let mut state = State::Fire;

    state.transition(Transitions::End);

    assert!(matches!(state, State::Inert));
}

#[test]
fn fsm_ignores_bad_transitions_fire() {
    let mut state = State::Fire;

    state.transition(Transitions::Activate);
    assert!(matches!(state, State::Fire));

    state.transition(Transitions::Arm);
    assert!(matches!(state, State::Fire));

    state.transition(Transitions::Disarm);
    assert!(matches!(state, State::Fire));

    state.transition(Transitions::Begin);
    assert!(matches!(state, State::Fire));
}

