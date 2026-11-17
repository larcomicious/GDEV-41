#ifndef SCENE_MANAGER
#define SCENE_MANAGER

#include <raylib.h>

#include <iostream>
#include <string>
#include <unordered_map>

class SceneManager;

// Base class that all scenes inherit
class Scene {
    // Reference to the scene manager.
    // In practice, you would want to make this private (or protected)
    // and set this via the constructor.
    SceneManager* scene_manager;
public:
    // Begins the scene. This is where you can load resources
    virtual void Begin() = 0;

    // Ends the scene. This is where you can unload resources
    virtual void End() = 0;

    // Updates scene's state (physics, input, etc. can be added here)
    virtual void Update() = 0;

    // Draws the scene's current state
    virtual void Draw() = 0;

    void SetSceneManager(SceneManager* scene_manager) {
        this->scene_manager = scene_manager;
    }

    SceneManager* GetSceneManager() {
        return scene_manager;
    }
};


class SceneManager {
    // Mapping between a scene ID and a reference to the scene
    std::unordered_map<int, Scene*> scenes;

     // Current active scene
    Scene* active_scene = nullptr;

public:
    // Adds the specified scene to the scene manager, and assigns it
    // to the specified scene ID
    void RegisterScene(Scene* scene, int scene_id) {
        scenes[scene_id] = scene;
    }

    // Removes the scene identified by the specified scene ID
    // from the scene manager
    void UnregisterScene(int scene_id) {
        scenes.erase(scene_id);
    }

    // Switches to the scene identified by the specified scene ID.
    void SwitchScene(int scene_id) {
        // If the scene ID does not exist in our records,
        // don't do anything (or you can print an error message).
        if (scenes.find(scene_id) == scenes.end()) {
            std::cout << "Scene ID not found" << std::endl;
            return;
        }

        // If there is already an active scene, end it first
        if (active_scene != nullptr) {
            active_scene->End();
        }

        std::cout << "Moved to Scene " << scene_id << std::endl;

        active_scene = scenes[scene_id];

        active_scene->Begin();
    }

    // Gets the active scene
    Scene* GetActiveScene() {
        return active_scene;
    }
};

// Resource manager implemented as a singleton
class ResourceManager {
    std::unordered_map<std::string, Texture> textures;

    ResourceManager() {}

public:
    // Delete copy constructor and copy operator (=)
    // Ensures there will only be one instance of the resource manager
    ResourceManager(const ResourceManager&) = delete;
    void operator=(const ResourceManager&) = delete;

    static ResourceManager* GetInstance() {
        static ResourceManager insance;
        return &insance;
    }

    Texture GetTexture(const std::string& path) {
        // If the texture does not exist yet in our records,
        // load it and store it in memory.
        if (textures.find(path) == textures.end()) {
            std::cout << "Loaded " << path << " from Disk" << std::endl;
            textures[path] = LoadTexture(path.c_str());
        }
        else {
            std::cout << "Resource Already Loaded" << std::endl;
        }

        return textures[path];
    }

    // Used for unloading all the textures when the game is closed.
    void UnloadAllTextures() {
        for (auto it : textures) {
            UnloadTexture(it.second);
        }

        textures.clear();
    }
};

#endif