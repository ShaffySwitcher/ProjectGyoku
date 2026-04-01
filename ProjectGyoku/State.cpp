#include "State.h"
#include "Supervisor.h"
#include "FPS.h"

StateManager gStateManager;

void StateManager::update()
{
	if (this->state) {
		this->state->update();
	}
}

void StateManager::render()
{
	if (gSupervisor.config.frameSkip == 0 || (gSupervisor.currentFrame % gSupervisor.config.frameSkip == 0)) {
		if (this->state) {
			this->state->render();
		}
	}
	FPS::render();
}

void StateManager::restore()
{
	if (this->state) {
		this->state->restore();
	}
}

std::shared_ptr<State> StateManager::setState(std::shared_ptr<State> state)
{
	auto oldState = this->state;
	this->state = state;
	if (this->state) {
		this->state->init();
	}
	return oldState;
}