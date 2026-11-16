#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <algorithm>

#include "package.h"

package::package(Vector3 startingPos, Vector3 startingRotation) {
    position = startingPos;
    rotation = startingRotation;
}