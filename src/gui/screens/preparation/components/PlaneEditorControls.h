#pragma once

#include "gui/input/CameraController.h"
#include "gui/input/InputState.h"

namespace gui {

struct PlaneEditorControls {
    static bool addPlanePressed(const InputState& input);
    static bool addRoomPressed(const InputState& input);
    static bool selectNextPressed(const InputState& input);
    static bool newDraftPressed(const InputState& input);
    static bool addDraftPointPressed(const InputState& input);
    static bool finalizeDraftPressed(const InputState& input);
    static bool editSelectedPressed(const InputState& input);
    static bool cancelDraftPressed(const InputState& input);
    static Vec3 movementDelta(const InputState& input, float deltaTime);
};

}  // namespace gui
