#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <algorithm>

#include "package.h"

package::package(Vector3 startingPos, Vector3 startingRotation) {
    position = startingPos;
    rotation = startingRotation;
}

void package::followDrone(Vector3 dronePos, Vector3 droneYaw) {
    position = dronePos;
    position.y -= 0.5f;

    rotation = droneYaw;
}

void package::fall() {
    position.y -= 1.0f;
    if (position.y < 0.0f) position.y = 0.0f;
}