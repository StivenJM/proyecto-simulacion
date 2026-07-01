#include "MockReceiverService.h"

#include <string>

namespace gui {
namespace {

GuiReceiver* findReceiver(GuiScenario& scenario, int receiverId)
{
    for (GuiReceiver& receiver : scenario.receivers) {
        if (receiver.id == receiverId) {
            return &receiver;
        }
    }

    return nullptr;
}

}  // namespace

void MockReceiverService::addReceiver(GuiScenario& scenario, GuiSelection& selection)
{
    const int id = scenario.nextReceiverId++;
    const float offset = static_cast<float>(scenario.receivers.size()) * 0.35f;

    GuiReceiver receiver;
    receiver.id = id;
    receiver.name = "Receiver " + std::to_string(id);
    receiver.position = {0.75f - offset, 0.0f, 0.0f};
    receiver.visible = true;
    receiver.color = {125.0f / 255.0f, 251.0f / 255.0f, 126.0f / 255.0f};
    scenario.receivers.push_back(receiver);
    selection.selectReceiver(id);
}

bool MockReceiverService::updateReceiverPosition(GuiScenario& scenario, int receiverId, Vec3 position)
{
    if (GuiReceiver* receiver = findReceiver(scenario, receiverId)) {
        receiver->position = position;
        return true;
    }

    return false;
}

bool MockReceiverService::updateReceiverName(GuiScenario& scenario, int receiverId, const std::string& name)
{
    if (GuiReceiver* receiver = findReceiver(scenario, receiverId)) {
        receiver->name = name;
        return true;
    }

    return false;
}

bool MockReceiverService::updateReceiverVisibility(GuiScenario& scenario, int receiverId, bool visible)
{
    if (GuiReceiver* receiver = findReceiver(scenario, receiverId)) {
        receiver->visible = visible;
        return true;
    }

    return false;
}

void MockReceiverService::moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta)
{
    if (GuiReceiver* receiver = findReceiver(scenario, selection.selectedReceiverId())) {
        receiver->position.x += delta.x;
        receiver->position.y += delta.y;
        receiver->position.z += delta.z;
    }
}

}  // namespace gui
