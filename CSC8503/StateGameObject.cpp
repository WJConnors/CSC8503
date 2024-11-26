#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
    counter = 0.0f;
    stateMachine = new StateMachine();

    // Define state A (move left)
    State* stateA = new State([&](float dt)->void {
        this->MoveLeft(dt);
        });

    // Define state B (move right)
    State* stateB = new State([&](float dt)->void {
        this->MoveRight(dt);
        });

    // Add states to the state machine
    stateMachine->AddState(stateA);
    stateMachine->AddState(stateB);

    // Define transitions
    stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool {
        return this->counter > 3.0f; // Transition to move right
        }));

    stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool {
        return this->counter < 0.0f; // Transition to move left
        }));
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
    stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
    GetPhysicsObject()->AddForce(Vector3(-10, 0, 0)); // Apply leftward force
    counter += dt; // Increment counter
}

void StateGameObject::MoveRight(float dt) {
    GetPhysicsObject()->AddForce(Vector3(10, 0, 0)); // Apply rightward force
    counter -= dt; // Decrement counter
}