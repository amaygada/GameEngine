#include "main.hpp"
#include <memory>
#include <vector>

int main(int argc, char *argv[]) {

    // The connection type of this program (either server or client)
    ServerClient connection;

    // If there is no command-line argument or the argument is not "server", default to a client connection
    if (argc != 2 || strcmp(argv[1], "server") != 0) {

        const char *type = "";
        connection = ServerClient(type);

    }
    // If the command-line argument is "server", set this connection to be the server
    else {

        connection = ServerClient("server");

    }

    // Global vector to hold all entities, including server-side entities
    std::vector<Entity *> E = {};
    // Vector to hold this client's entities
    std::vector<Entity *> myEntities = {};

    // If this connection is a client, intitialize the application
    if (connection.isClient()) {

        renderer->init("Game");

    }

    if (connection.isClient()) {

        // Create a controllable entity
        SDL_Color shapeColor5 = {0, 0, 255, 255};  // Blue color
        Entity *shape5 = new Entity(300, 300, 100, 100, shapeColor5);
        // shape5.inputHandler = new DefaultEntityInputHandler();
        shape5->physicsHandler = new DefaultMovementPhysicsHandler(true);

        // Push the entity to the vector of all entities
        E.push_back(shape5);
        // Push the entity to the vector of only this client's entities
        myEntities.push_back(shape5);
        

    }
    else {

        SDL_Color shapeColor = {255, 0, 0, 255};  // Red color
        Entity *shape1 = new Entity(1000, 0, 100, 500, shapeColor);
        shape1->physicsHandler = new DefaultGravityPhysicsHandler(false);

        // Push the entity to the vector of all entities
        E.push_back(shape1);

        // Initialize the pattern-following shape
        SDL_Color shapeColor3 = {255, 255, 0, 255};  // Yellow color
        Entity *shape3 = new Entity(10, 10, 100, 100, shapeColor3);
        std::vector<SDL_Rect> shape3Path = {};
        shape3Path.push_back({SCREEN_WIDTH/3, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/2, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH-100,10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/2, 10, 1, 1});
        shape3Path.push_back({SCREEN_WIDTH/3, 10, 1, 1});
        shape3Path.push_back({10, 10, 1, 1});
        shape3->patternHandler = new DefaultPatternHandler(shape3Path);

        // Push the entity to the vector of all entities
        E.push_back(shape3);

        // Ensure that the server's entities are reflected in its ServerClient component
        connection.serverAddEntities(E);

    }

    int64_t last_render_time = globalTimeline->getTime();

    while(true){

        int64_t frame_delta = globalTimeline->getTime() - last_render_time;
        
        if(frame_delta < (1e9/RENDER_FPS - (2*1e6))) SDL_Delay((1e9/RENDER_FPS - frame_delta)*1e-6);

        last_render_time = globalTimeline->getTime();

        // If this connection is a client, prepare the renderer and check user input
        if (connection.isClient()) {

            renderer->prepareScene();

            inputSubsystem->doInput(E);

        }

        // New, updated entities returned from the server
        std::vector<Entity *> updatedEntities = connection.update(E);

        // Number of entities on the server-side (this would be the total amount of entities in the game)
        const int numEntities = updatedEntities.size();

        if (connection.isClient()) {

            // This client's (unique) ID
            const int clientID = connection.getClientID();
            // Counter for number of entities controlled by this client
            int myEntityCounter = 0;

            // For each entity server-side...
            for (int i = 0; i < numEntities; i++) {

                // Get the entity
                Entity *entity = updatedEntities.at(i);

                // If the entity is controlled by this client, update its information on the client-side based on the results from the server-side
                if (entity->attachedClientID == clientID) {

                    Entity *myEntity = myEntities.at(myEntityCounter);
                    myEntity->setFields(entity);

                    myEntityCounter++;

                }

                // If a new entity was added since the client's last communication with the server, just add the new entity
                if ((int)(E.size()) < numEntities) {

                    E.push_back(entity);

                }
                // Otherwise, just update the entity on the client-side
                else {

                    Entity *currentEntity = E.at(i);
                    currentEntity->setFields(entity);

                }

            }

        }
        // If this connection is the server...
        else {

            // For each entity server-side...
            for (int i = 0; i < numEntities; i++) {

                // Get the entity
                Entity *entity = updatedEntities.at(i);

                // If a new entity was added to the server's ServerClient component, add it here as well
                if ((int)(E.size()) < numEntities) {

                    E.push_back(entity);

                }
                // Otherwise, update the already-existing entity
                else {

                    Entity *currentEntity = E.at(i);
                    currentEntity->setFields(entity);

                }

            }

        }

        // Update subsystems. This applies to both server and clients since both have their own entities
        physicsSubsystem->doPhysics(E);
        collisionSubsystem->doCollision(E);
        animationSubsystem->doAnimation(E);

        // If this connection is a client, render the scene
        if (connection.isClient()) {

            renderer->presentScene(E);

        }

    }

    // If this connection is a client, clean up the scene
    if (connection.isClient()) {

        renderer->cleanup();

    }

    return 0;
}