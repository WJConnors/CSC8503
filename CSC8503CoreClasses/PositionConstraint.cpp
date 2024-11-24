#include "PositionConstraint.h"
//#include "../../Common/Vector3.h"
#include "GameObject.h"
#include "PhysicsObject.h"
//#include "Debug.h"



using namespace NCL;
using namespace Maths;
using namespace CSC8503;

PositionConstraint::PositionConstraint(GameObject* a, GameObject* b, float d)
{
	objectA		= a;
	objectB		= b;
	distance	= d;
}

PositionConstraint::~PositionConstraint()
{

}

//a simple constraint that stops objects from being more than <distance> away
//from each other...this would be all we need to simulate a rope, or a ragdoll
void PositionConstraint::UpdateConstraint(float dt)	{
    // Calculate the relative position between the objects
    Vector3 relativePos = objectA->GetTransform().GetPosition() -
        objectB->GetTransform().GetPosition();

    // Determine the current distance between the objects
    float currentDistance = Vector::Length(relativePos);

    // Calculate the offset from the desired distance
    float offset = distance - currentDistance;

    // Only proceed if there is a violation of the constraint
    if (abs(offset) > 0.0f) {
        // Normalize the relative position to get the offset direction
        Vector3 offsetDir = Vector::Normalise(relativePos);

        // Retrieve the physics objects of the two constrained game objects
        PhysicsObject* physA = objectA->GetPhysicsObject();
        PhysicsObject* physB = objectB->GetPhysicsObject();

        // Calculate the relative velocity between the objects
        Vector3 relativeVelocity = physA->GetLinearVelocity() -
            physB->GetLinearVelocity();

        // Compute the effective mass of the constraint
        float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

        if (constraintMass > 0.0f) {
            // Determine the relative velocity in the direction of the constraint
            float velocityDot = Vector::Dot(relativeVelocity, offsetDir);

            // Apply Baumgarte stabilization bias to help convergence
            float biasFactor = 0.01f;
            float bias = -(biasFactor / dt) * offset;

            // Compute the Lagrange multiplier for the constraint impulse
            float lambda = -(velocityDot + bias) / constraintMass;

            // Calculate the impulses to be applied to the objects
            Vector3 aImpulse = offsetDir * lambda;
            Vector3 bImpulse = -offsetDir * lambda;

            // Apply the impulses to the objects
            physA->ApplyLinearImpulse(aImpulse);
            physB->ApplyLinearImpulse(bImpulse);
        }
    }
}
