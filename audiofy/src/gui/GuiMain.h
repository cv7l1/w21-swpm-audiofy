//
// Created by Jonathan on 04.01.2022.
//

#ifndef AUDIOFY_GUIMAIN_H
#define AUDIOFY_GUIMAIN_H


#include <list>
#include "components/IComponent.h"

class GuiMain {
public:
    static void Show();
    static void AddComponent(IComponent* component);

    static void RemoveComponent(IComponent* component);
private:
    static std::list<IComponent*> visibleComponents;
};

#endif //AUDIOFY_GUIMAIN_H
