#pragma once

#include "RenderTypes.h"

#include <vector>

namespace gui {

enum class PlaneOrientation {
    Horizontal,
    VerticalX,
    VerticalZ,
};

struct EditablePlane {
    int id;
    Vec3 center;
    float width;
    float height;
    PlaneOrientation orientation;
};

class SceneEditor {
public:
    SceneEditor();

    void addPlane();
    void addRoom();
    void selectNext();
    void moveSelected(Vec3 delta);

    int selectedPlaneId() const;
    const std::vector<EditablePlane>& planes() const;
    std::vector<LineVertex> buildLineVertices() const;

private:
    void addPlane(Vec3 center, float width, float height, PlaneOrientation orientation);

    int nextId_ = 1;
    int selectedPlaneId_ = 0;
    std::vector<EditablePlane> planes_;
};

}  // namespace gui
