#ifndef SCENE_H
#define SCENE_H

class Scene {
public:
    virtual void Update() { };
    virtual void Draw() { };

    virtual std::pair<float, float> GetAngleDistance() { return std::pair<float, float>(25.0f, 20.0f); }
    virtual Color GetClearColor() { return Color{225,225,225,255}; }
    virtual std::pair<int, int> GetTiles() {
        return std::pair<int, int>(8, 6);
    }
    virtual std::pair<int, int> GetTileResolution() {
        //return std::pair<int, int>(126, 168);
        //return std::pair<int, int>(168, 224);
        return std::pair<int, int>(315, 420);
        //return std::pair<int, int>(420, 560);
    }
};

#endif
