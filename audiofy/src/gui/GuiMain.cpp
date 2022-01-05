//
// Created by Jonathan on 04.01.2022.
//

#include "GuiMain.h"
std::list<IComponent*> GuiMain::visibleComponents = std::list<IComponent*>();

void GuiMain::Show() {
    for(auto component : visibleComponents) {
        component->Show();
    }
}

void GuiMain::AddComponent(IComponent *component) {
    visibleComponents.push_back(component);
}

void GuiMain::RemoveComponent(IComponent *component) {
    visibleComponents.remove(component);
}
