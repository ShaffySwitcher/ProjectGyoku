#pragma once

#include <DxLib.h>
#include "Engine/State.h"

class Game : public State
{
public:
    virtual void init() override;
    virtual void destroy() override;
    virtual void update() override;
    virtual void render() override;
    virtual void restore() override;
};