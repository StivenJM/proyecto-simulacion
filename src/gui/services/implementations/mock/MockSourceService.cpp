#include "MockSourceService.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace gui {
namespace {

GuiSource* findSource(GuiScenario& scenario, int sourceId)
{
    for (GuiSource& source : scenario.sources) {
        if (source.id == sourceId) {
            return &source;
        }
    }

    return nullptr;
}

float sanitizeSourceEnergy(float energy)
{
    if (!std::isfinite(energy)) {
        return 0.0f;
    }

    return std::clamp(energy, 0.0f, 1000.0f);
}

}  // namespace

void MockSourceService::addSource(GuiScenario& scenario, GuiSelection& selection)
{
    const int id = scenario.nextSourceId++;
    const float offset = static_cast<float>(scenario.sources.size()) * 0.35f;

    GuiSource source;
    source.id = id;
    source.name = "Source " + std::to_string(id);
    source.position = {-0.75f + offset, 0.0f, 0.0f};
    source.visible = true;
    source.color = {1.0f, 132.0f / 255.0f, 120.0f / 255.0f};
    scenario.sources.push_back(source);
    selection.selectSource(id);
}

bool MockSourceService::updateSourcePosition(GuiScenario& scenario, int sourceId, Vec3 position)
{
    if (GuiSource* source = findSource(scenario, sourceId)) {
        source->position = position;
        return true;
    }

    return false;
}

bool MockSourceService::updateSourceName(GuiScenario& scenario, int sourceId, const std::string& name)
{
    if (GuiSource* source = findSource(scenario, sourceId)) {
        source->name = name;
        return true;
    }

    return false;
}

bool MockSourceService::updateSourceEnergy(GuiScenario& scenario, int sourceId, float energy)
{
    if (GuiSource* source = findSource(scenario, sourceId)) {
        source->energy = sanitizeSourceEnergy(energy);
        return true;
    }

    return false;
}

bool MockSourceService::updateSourceVisibility(GuiScenario& scenario, int sourceId, bool visible)
{
    if (GuiSource* source = findSource(scenario, sourceId)) {
        source->visible = visible;
        return true;
    }

    return false;
}

void MockSourceService::moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta)
{
    if (GuiSource* source = findSource(scenario, selection.selectedSourceId())) {
        source->position.x += delta.x;
        source->position.y += delta.y;
        source->position.z += delta.z;
    }
}

}  // namespace gui
