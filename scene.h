#ifndef SCENE_H
#define SCENE_H

class Scene {
public:
    virtual void Init() { };
    virtual void Update() { };
    virtual void Draw() { };
};

#endif
