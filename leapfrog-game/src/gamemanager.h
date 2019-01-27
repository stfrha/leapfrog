#pragma once

#include "oxygine-framework.h"
#include "mainactor.h"

class GameManager
{
private:
   MainActor * m_mainActor;

public:
   GameManager(MainActor* mainActor);

   void startGameManager(void);
};
