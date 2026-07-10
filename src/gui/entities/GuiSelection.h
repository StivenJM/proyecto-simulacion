#pragma once

namespace gui {

enum class GuiObjectType {
    None,
    General,
    Plane,
    Source,
    Receiver,
    Triangle,
};

struct GuiSelection {
    GuiObjectType type = GuiObjectType::None;
    int id = 0;
    int selectedTriangleIdValue = 0;

    bool isGeneralSelected() const { return type == GuiObjectType::General; }
    bool isPlaneSelected() const { return type == GuiObjectType::Plane && id != 0; }
    bool isSourceSelected() const { return type == GuiObjectType::Source && id != 0; }
    bool isReceiverSelected() const { return type == GuiObjectType::Receiver && id != 0; }
    bool isTriangleSelected() const { return selectedTriangleIdValue != 0; }
    int selectedPlaneId() const { return isPlaneSelected() ? id : 0; }
    int selectedSourceId() const { return isSourceSelected() ? id : 0; }
    int selectedReceiverId() const { return isReceiverSelected() ? id : 0; }
    int selectedTriangleId() const { return selectedTriangleIdValue; }
    void selectGeneral() { type = GuiObjectType::General; id = 0; selectedTriangleIdValue = 0; }
    void selectPlane(int planeId) { type = planeId == 0 ? GuiObjectType::None : GuiObjectType::Plane; id = planeId; selectedTriangleIdValue = 0; }
    void selectSource(int sourceId) { type = sourceId == 0 ? GuiObjectType::None : GuiObjectType::Source; id = sourceId; selectedTriangleIdValue = 0; }
    void selectReceiver(int receiverId) { type = receiverId == 0 ? GuiObjectType::None : GuiObjectType::Receiver; id = receiverId; selectedTriangleIdValue = 0; }
    void selectTriangle(int triangleId) { selectedTriangleIdValue = triangleId; }
    void clearTriangle() { selectedTriangleIdValue = 0; }
    void clear() { type = GuiObjectType::None; id = 0; selectedTriangleIdValue = 0; }
};

}  // namespace gui
