#include "Kitten.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

Kitten::Kitten(GameObject* player) : player(player), home(false) {
	stateMachine = new StateMachine();

	State* still = new State([&](float dt)->void {
		this->Wait(dt);
		});
}

void Kitten::Wait(float dt) {
	Vector3 curForce = GetPhysicsObject()->GetForce();
}

void Kitten::FollowPlayer(float dt) {

}