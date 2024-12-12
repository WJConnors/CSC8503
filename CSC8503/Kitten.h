#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class Kitten : public GameObject {
		public:
			Kitten(GameObject* player);
			~Kitten();

			virtual void Update(float dt);
		protected:
			void Wait(float dt);
			void FollowPlayer(float dt);

			StateMachine* stateMachine;
			GameObject* player;

			bool home;
		};
	}
}