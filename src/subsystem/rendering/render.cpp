#include "render.hpp"

void Renderer::init(string windowName="Engine") {

    eventManager->registerEvent("QuitGameEvent", new QuitGameEventHandler());
    eventManager->registerEvent("ChangeTicEvent", new ChangeTicEventHandler());
    eventManager->registerEvent("GamePauseEvent", new GamePauseEventHandler());
    eventManager->registerEvent("DefaultCollisionEvent", new DefaultCollisionEventHandler());
    eventManager->registerEvent("DefaultPhysicsEvent", new DefaultPhysicsEventHandler());
    eventManager->registerEvent("RecordEvent", new RecordEventHandler());

    addParametersEvent();

    app->quit = false;
    app->replay = false;
    app->record = false;

    int rendererFlags, windowFlags;
    rendererFlags = SDL_RENDERER_ACCELERATED;
    windowFlags = SDL_WINDOW_RESIZABLE;
    // handle unsuccessful initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
        exit(1);
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        exit(1);
    }

    // attempt to create the window
    app->window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
    // If the window pointer is null, we have an error
    if (!app->window) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        exit(1);
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    // attempt to create the renderer
    app->renderer = SDL_CreateRenderer(app->window, -1, rendererFlags);

    // if the renderer pointer is null, we have an error
    if (!app->renderer) {
        cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        exit(1);
    }

}

void Renderer::getWindowSize(int *window_width, int *window_height){
    SDL_GetWindowSize(app->window, window_width, window_height);
}

// Function to prepare the scene by clearing the screen and drawing an Entity
// attach timer here
void Renderer::prepareScene() {
    SDL_SetRenderDrawColor(app->renderer, BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, BACKGROUND_COLOR_O);  // Background color

    app->font = TTF_OpenFont("src/utils/Minecraft.ttf", 12);
    if(!app->font){
        cout<<"Font is null"<<endl;
    }
    // SDL_Color textColor = {0, 0, 0, 255}; // White color
    // SDL_Surface* textSurface = TTF_RenderText_Solid(app->font, "Hello, SDL2!", textColor);
    // SDL_Texture* textTexture = SDL_CreateTextureFromSurface(app->renderer, textSurface);



    // SDL_FreeSurface(textSurface); // Free the surface once we have the texture
    // SDL_Rect textRect;
    // textRect.x = 100;
    // textRect.y = 100;
    // textRect.w = 100;
    // textRect.h = 100;

    // SDL_DestroyTexture(textTexture);
    // TTF_CloseFont(font);
    SDL_RenderClear(app->renderer);

    // SDL_RenderCopy(app->renderer, textTexture, NULL, &textRect);
}

void writeEntityMapToFile(const std::unordered_map<int, std::vector<Entity *>> &entity_map, const std::string &filename, int client_id) {
    Serializer render_serializer;
    std::ofstream file(filename, std::ios::app); // Open in append mode
    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << filename << std::endl;
        return;
    }
    static int iteration = 1; 
    file << "Entity Map Iteration: " << iteration++ << "\n";
    for (const auto &pair : entity_map) {
        file << "Key: " << pair.first << "\n";
        for (Entity *entity : pair.second) {
            if(pair.first == client_id){
                if(entity->renderingHandler != nullptr){
                    file << "  " << render_serializer.serializeEntity(entity) << "\n";
                }
            }else file << "  " << render_serializer.serializeEntity(entity) << "\n";
            
        }
    }
    file << "-----------------------------\n";
    file.close();
}

std::vector<std::unordered_map<int, std::vector<Entity *>>> parseEntityMaps(const std::string &filename) {
    Serializer render_serializer;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file: " + filename);
    }

    std::vector<std::unordered_map<int, std::vector<Entity *>>> entityMaps;
    std::unordered_map<int, std::vector<Entity *>> currentMap;
    int currentKey = -1;

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("Entity Map Iteration:", 0) == 0) {
            // Save the current map and start a new one
            if (!currentMap.empty()) {
                entityMaps.push_back(currentMap);
                currentMap.clear();
            }
        } else if (line.rfind("Key:", 0) == 0) {
            // Parse key
            currentKey = std::stoi(line.substr(5));
        } else if (!line.empty() && line[0] == ' ') {
            // Parse serialized entity
            Entity *entity = render_serializer.deserializeEntity(line.substr(2));
            currentMap[currentKey].push_back(entity);
        }
    }
    // Save the last map
    if (!currentMap.empty()) {
        entityMaps.push_back(currentMap);
    }
    file.close();
    return entityMaps;
}

// Function to present the scene
// attach timer here
void Renderer::presentScene(const unordered_map<int, std::vector<Entity *>> &entity_map, int client_id, bool use_custom_renderer) {
    // Draw the entities from the entity_map

    // Record entities
    if(app->record){
        writeEntityMapToFile(entity_map, "record.txt", client_id);
    }

    if(app->replay){
        // render the recorded events.
        std::vector<std::unordered_map<int, std::vector<Entity *>>> em = parseEntityMaps("record.txt");
        if(app->replayIndex < em.size()){
            for (const auto pair : em[app->replayIndex]) {
                std::vector<Entity *> entities = pair.second;
                for (Entity *entity : entities) {
                    entity->draw(app->renderer);
                }
            }
            app->replayIndex++;
        }else{
            app->replay = false;
            app->replayIndex = 0;
        }

    }

    if(use_custom_renderer || app->replay){
        SDL_RenderPresent(app->renderer);
        return;
    }

    for (const auto pair : entity_map) {
        std::vector<Entity *> entities = pair.second;
        for (Entity *entity : entities) {
            if(pair.first == client_id){
                if(entity->renderingHandler != nullptr){
                    entity->renderingHandler->renderEntity(entity);
                }
            }
            else entity->draw(app->renderer);
        }
    }
    SDL_RenderPresent(app->renderer);  // Present the final rendered scene
}

void Renderer::cleanup() {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
    TTF_Quit();
}

ModularRenderer::ModularRenderer() {
    this->rendererTimeline = new Timeline(gameTimeline, 1);
}

void DefaultRenderer::renderEntity(Entity *entity) {
    entity->draw(app->renderer);
}

Renderer *renderer;