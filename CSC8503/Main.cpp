#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();

	int data = 0;

	// State A definition
	State* A = new State([&](float dt)->void {
		std::cout << "I'm in state A!\n";
		data++;
		});

	// State B definition
	State* B = new State([&](float dt)->void {
		std::cout << "I'm in state B!\n";
		data--;
		});

	// State transition from A to B
	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool {
		return data > 10;
		});

	// State transition from B to A
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
		return data < 0;
		});

	// Add states and transitions to the state machine
	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);

	// Run the state machine for 100 iterations
	for (int i = 0; i < 100; ++i) {
		testMachine->Update(1.0f);
	}
}

vector<Vector3> testNodes;
void TestPathfinding() {
	NavigationGrid grid("TestGrid1.txt");

	NavigationPath outPath;

	Vector3 startPos(80, 0, 10);
	Vector3 endPos(80, 0, 80);

	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while(outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}

void TestBehaviourTree() {
	float behaviourTimer;
	float distanceToTarget;
	BehaviourAction* findKey = new BehaviourAction("Find Key",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for a key!\n";
				behaviourTimer = rand() % 100;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				behaviourTimer -= dt;
				if (behaviourTimer <= 0.0f) {
					std::cout << "Found a key!\n";
					return Success;
				}
			}
			return state;
		}
		);
	BehaviourAction* goToRoom = new BehaviourAction("Go To Room",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Going to the loot room!\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				distanceToTarget -= dt;
				if (distanceToTarget <= 0.0f) {
					std::cout << "Reached room!\n";
					return Success;
				}
			}
			return state;
		}
		);
	BehaviourAction* openDoor = new BehaviourAction("open Door",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Opening Door!\n";
				return Success;
			}
			return state;
		}
		);
	BehaviourAction* lookForTreasure = new BehaviourAction("Look For Treasure",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for treasure!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some treasure!\n";
					return Success;
				}
				std::cout << "No treasure in here...\n";
				return Failure;
			}
			return state;
		}
		);
	BehaviourAction* lookForItems = new BehaviourAction("Look For Items",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for items!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some items!\n";
					return Success;
				}
				std::cout << "No items in here...\n";
				return Failure;
			}
			return state;
		}
		);

	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);

	for (int i = 0; i < 5; ++i) {
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		std::cout << "We're going on adventure!\n";
		while (state == Ongoing) {
			state = rootSequence->Execute(1.0f);
		}
		if (state == Success) {
			std::cout << "What a successful adventure\n";
		}
		else if (state == Failure) {
			std::cout << "What a waste of time!\n";
		}
	}
	std::cout << "All done\n";
}

Window* w;

int highScore = -1;
int score = -1;
class TestPacketReceiver : public PacketReceiver {
public:
	TestPacketReceiver(std::string name) {
		this->name = name;
	}
	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;
			std::string msg = realPacket->GetStringFromData();
			int curScore = std::stoi(msg);
			if (curScore > highScore) {
				std::cout << "new highscore: " << curScore << std::endl;
				highScore = curScore;
			}
		}
	}
protected:
	std::string name;
};

void Client(int score) {
	NetworkBase::Initialise();
	TestPacketReceiver clientReceiver("Client");
	int port = NetworkBase::GetDefaultPort();
	GameClient* client = new GameClient();
	client->RegisterPacketHandler(String_Message, &clientReceiver);
	bool canConnect = client->Connect(127, 0, 0, 1, port);
	if (!canConnect) {
		std::cout << "Failed to connect to the server!" << std::endl;
		return;
	}
	for (int i = 0; i < 100; i++) {
		StringPacket clientPacket(std::to_string(score));
		client->SendPacket(clientPacket);
		client->UpdateClient();
	}
	NetworkBase::Destroy();
}

void Server() {
	NetworkBase::Initialise();
	TestPacketReceiver serverReceiver("Server");
	int port = NetworkBase::GetDefaultPort();
	GameServer* server = new GameServer(port, 1);
	server->RegisterPacketHandler(String_Message, &serverReceiver);

	while (true) {
		StringPacket serverPacket("Server says hello!");
		server->SendGlobalPacket(serverPacket);

		server->UpdateServer();
	}
	NetworkBase::Destroy();
}

void TestNetworking() {
	NetworkBase::Initialise();

	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver("Client");

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);

	for (int i = 0; i < 100; ++i) {
		StringPacket serverPacket("Server says hello! " + std::to_string(i));
		server->SendGlobalPacket(serverPacket);

		StringPacket clientPacket(("Client says hello! " + std::to_string(i)));
		client->SendPacket(clientPacket);

		server->UpdateServer();
		client->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	NetworkBase::Destroy();
}

class GameScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
			float dt = w->GetTimer().GetTimeDeltaSeconds();
			if (dt > 0.1f) {
				std::cout << "Skipping large time delta" << std::endl;
				continue; //must have hit a breakpoint or something to have a 1 second frame time!
			}
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
				w->ShowConsole(true);
			}
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
				w->ShowConsole(false);
			}

			if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
				w->SetWindowPosition(0, 0);
			}

			w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
			DisplayPathfinding();

			g->UpdateGame(dt);
		}
		Client(g->score);
		delete g;
		return PushdownResult::Pop;
	};
	void OnAwake() override {
		w->ShowOSPointer(false);
		w->LockMouseToWindow(true);
		g = new TutorialGame();
		w->GetTimer().GetTimeDeltaSeconds();
	}
	TutorialGame* g;
};

class IntroScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			*newState = new GameScreen();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};
	void OnAwake() override {
		std::cout << "Press space to start or escape to end" << std::endl;

	}
};

void RunPushdownAutomata() {
	PushdownMachine machine(new IntroScreen());
	while (w->UpdateWindow()) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (!machine.Update(dt)) {
			return;
		}
	}
}






/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
	//TestNetworking();
	//TestStateMachine();
	//TestBehaviourTree();
	WindowInitialisation initInfo;
	initInfo.width = 1280;
	initInfo.height = 720;
	initInfo.windowTitle = "CSC8503 Game technology!";

	std::cout << "s or c" << std::endl;
	char type;
	std::cin >> type;

	if (type == 's') Server();

	w = Window::CreateGameWindow(initInfo);

	if (!w->HasInitialised()) {
		return -1;
	}

	RunPushdownAutomata();



	//TestPathfinding();

	Window::DestroyGameWindow();
}