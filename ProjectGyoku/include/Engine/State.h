#pragma once
#include <memory>

class State
{
public:
	virtual ~State() = default;
	virtual void init() = 0;
	virtual void destroy() {};
	virtual void update() = 0;
	virtual void render() = 0;
	virtual void restore() = 0;
};

class StateManager {
public:
	void update();
	void render();
	void restore();
	
	std::shared_ptr<State> setState(std::shared_ptr<State> state);
	std::shared_ptr<State> getState() const { return this->state; }

private:
	std::shared_ptr<State> state = nullptr;
	std::shared_ptr<State> pendingState = nullptr;
};

extern StateManager gStateManager;
