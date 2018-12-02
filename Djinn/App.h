#pragma once

#include "Scene.h"

class App{
public:
    App();
    App(const App& other);
    ~App();
private:
    Scene scene;
};
