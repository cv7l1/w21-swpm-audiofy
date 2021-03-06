//
// Created by Jonathan on 04.01.2022.
//

#ifndef AUDIOFY_PROJECTFILELISTCOMPONENT_H
#define AUDIOFY_PROJECTFILELISTCOMPONENT_H


#include <vector>
#include "IComponent.h"
#include "../../audio/AudioWorkspace.h"
#include "../../audio/AudioContext.h"
class ProjectFileListComponent : public IComponent {

public:
    ProjectFileListComponent(AudioContext* context) : _context(context) {};
    void Show() override;
    bool visible = true;

private:
    int selectedItem = -1;
    AudioContext* _context;
};


#endif //AUDIOFY_PROJECTFILELISTCOMPONENT_H
