//
// Created by Jonathan on 04.01.2022.
//

#ifndef AUDIOFY_PROJECTFILELISTCOMPONENT_H
#define AUDIOFY_PROJECTFILELISTCOMPONENT_H


#include <vector>
#include "IComponent.h"
class ProjectFileListComponent : public IComponent {

public:
    ProjectFileListComponent() = default;
    void Show() override;

private:
    int selectedItem = -1;
};


#endif //AUDIOFY_PROJECTFILELISTCOMPONENT_H
