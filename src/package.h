#pragma once
#include <raylib.h>

class package {
    private: 
    Vector3 position;
    Vector3 rotation;
    bool attached = false;

    public:
    package(Vector3 startingPos, Vector3 startingRotation);
};
