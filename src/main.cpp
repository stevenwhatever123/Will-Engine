#include "pch.h"

#include "Managers/SystemManager.h"

#include <assimp/Importer.hpp>

int main(void)
{
    SystemManager* systemManager = new SystemManager();

    systemManager->init(1600, 900);

    while (!systemManager->shouldCloseWindow())
    {
        systemManager->update();
    }

    return 0;
}