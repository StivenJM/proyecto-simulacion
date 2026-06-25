#pragma once

namespace gui {

enum class GuiObjectType {
    None,
    Plane,
    Source,
    Receiver,
    Triangle,
};

struct GuiSelection {
    GuiObjectType type = GuiObjectType::None;
    int id = 0;

    bool isPlaneSelected() const { return type == GuiObjectType::Plane && id != 0; }
    int selectedPlaneId() const { return isPlaneSelected() ? id : 0; }
    void selectPlane(int planeId) { type = planeId == 0 ? GuiObjectType::None : GuiObjectType::Plane; id = planeId; }
    void clear() { type = GuiObjectType::None; id = 0; }
};

}  // namespace gui
