#ifndef SCENE_H
#define SCENE_H

class Scene {
public:
    virtual Color GetClearColor() { Color{225,225,225,255}; }

    virtual void Update() { };
    virtual void Draw() { };
};

#endif
