# include "p2pclient.hpp"

P2PClient::P2PClient() : 
    context(1),
    my_pub_socket(context, ZMQ_PUB),
    my_master_socket(context, ZMQ_REQ),
    my_rep_socket(context, ZMQ_REP)
    {
        my_master_socket.setsockopt(ZMQ_RCVTIMEO, 1000);
        my_master_socket.connect("tcp://localhost:4000");
        negotiateTimeline = new Timeline(globalTimeline, 1e10);
    }

void P2PClient::listen_on_detached_thread(zmq::socket_t &socket){
    // listen for new peers
    cout<<"Binding to "<<4000+id<<endl;
    zmq::message_t request;
    while(true){
        socket.recv(request, zmq::recv_flags::none);
        std::string message = std::string(static_cast<char*>(request.data()), request.size());
        handle_req_rep(message);
    }
}

int P2PClient::get_id(){return id;}

unordered_map<int, std::vector<Entity *>> P2PClient::get_entity_map(){
    return entityMap;
}

void P2PClient::expose_port_and_connect(){
    // try connecting to tcp:5000 3 times
    for(int i=0; i<3; i++){
        string message = messageHandler.createMessage(0, "Hello");
        messageHandler.sendMessage(my_master_socket, message);
        string reply = messageHandler.receiveMessage(my_master_socket, zmq::recv_flags::none);
        if(reply==""){
                id = 0;
                my_master_socket.close();
                my_master_socket = zmq::socket_t(context, ZMQ_REP);
                my_master_socket.bind("tcp://*:4000");
                std::thread listen_thread(&P2PClient::listen_on_detached_thread, this, std::ref(my_master_socket));
                listen_thread.detach();

                bind_publisher();
                break;
        }
        
    }
}

void P2PClient::update_id(int id){
    this->id = id;
}

void P2PClient::bind_publisher(){
    if(id==-1) exit(0);
    int port = 5000 + id;
    my_pub_socket.bind("tcp://*:"+std::to_string(port));
}

void P2PClient::bind_rep_socket(){
    if(id==-1) exit(0);
    if(id==0) return;
    int port = 4000 + id;
    my_rep_socket.bind("tcp://*:"+std::to_string(port));
    std::thread listen_thread(&P2PClient::listen_on_detached_thread, this, std::ref(my_rep_socket));
    listen_thread.detach();
}

void P2PClient::subscribe_to_peer(int peer_id){
    if(peer_id == id) return;
    zmq::socket_t sub_socket(context, ZMQ_SUB);
    sub_socket_map[peer_id] = std::move(sub_socket);
    sub_socket_map[peer_id].connect("tcp://localhost:"+std::to_string(5000+peer_id));
    sub_socket_map[peer_id].set(zmq::sockopt::subscribe, ""); // Subscribe to all messages
}

void P2PClient::connect_to_peer(int peer_id){
    if(peer_id == id) return;
    if(peer_id == 0) return;
    zmq::socket_t req_socket(context, ZMQ_REQ);
    req_socket_map[peer_id] = std::move(req_socket);
    req_socket_map[peer_id].connect("tcp://localhost:"+std::to_string(4000+peer_id));
}

void P2PClient::request_id(){
    if(id == 0) return;
    string message = messageHandler.createMessage(1, "Hello");
    messageHandler.sendMessage(my_master_socket, message);
    string reply = messageHandler.receiveMessage(my_master_socket);
    auto msg = messageHandler.parseMessage(reply);
    char peers[255];
    int new_id;
    sscanf(msg.second.c_str(), "Id:%d Peers:%255[^n]", &new_id, &peers);
    update_id(new_id);

    bind_publisher();
    bind_rep_socket();

    // connect to peers
    std::istringstream iss(peers);
    std::string peer;
    while (std::getline(iss, peer, ',')) {
        int peer_id = std::stoi(peer);
        subscribe_to_peer(peer_id);
        connect_to_peer(peer_id);
    }

}

string P2PClient::get_comma_sep_sockets(){
    string sockets = "";
    for(auto it=sub_socket_map.begin(); it!=sub_socket_map.end(); it++){
        sockets += std::to_string(it->first) + ",";
    }
    sockets += std::to_string(id);

    return sockets;
}

void P2PClient::announce_new_peer(){
    string message = messageHandler.createMessage(2, "New peer Id: "+std::to_string(id));
    for(auto it=req_socket_map.begin(); it!=req_socket_map.end(); it++){
        if(it->first == id) continue;
        if(it->first == 0) continue;
        messageHandler.sendMessage(it->second, message);
        string reply = messageHandler.receiveMessage(it->second);
    }
}

void P2PClient::publish_entity_positions(){
    // send message over pub socket
    // serialize entity
    string my_entity_str = serializer.serializeEntity(entityMap[id][0]);
    string platform_entity_str = serializer.serializeEntity(entityMap[-1][0]);
    string message = messageHandler.createMessage(3, "ClientID:"+std::to_string(id)+" Entity:"+my_entity_str+" PlatformEntity:"+platform_entity_str);
    messageHandler.sendMessage(my_pub_socket, message);
}

void P2PClient::receive_entity_positions(){
    // for all sub sockets
    vector<Entity*> platform_entities;
    for(auto it=sub_socket_map.begin(); it!=sub_socket_map.end(); it++){
        string broadcast_data;
        broadcast_data = messageHandler.receiveMessageLoop(it->second, zmq::recv_flags::dontwait);
        if(broadcast_data == "") continue;
        auto msg = messageHandler.parseMessage(broadcast_data);
        int client_id;
        char entity_data[256];
        char platform_data[256];
        sscanf(msg.second.c_str(), "ClientID:%d Entity:%255[^P] PlatformEntity:%255[^\n]", &client_id, &entity_data, &platform_data);
        Entity *entity = serializer.deserializeEntity(entity_data);
        vector<Entity*> temp;
        temp.push_back(entity);
        entityMap[client_id] = temp;
        Entity *platform_entity = serializer.deserializeEntity(platform_data);
        platform_entities.push_back(platform_entity);
    }

    negotiate_platform_position(platform_entities);
}

void P2PClient::negotiate_platform_position(vector<Entity*> pe){
    if(this->negotiate_start_time == -1){
        this->negotiate_start_time = negotiateTimeline->getTime();
    }
    int64_t current_time = negotiateTimeline->getTime();
    if(current_time - this->negotiate_start_time < 1) return;
    this->negotiate_start_time = current_time;

    if(pe.size() == 0) return;
    map<int,int> x;
    map<int,int> y;

    for(auto i : pe){
        x[i->x] += 1;
        y[i->y] += 1;
    }
    
    int final_x, final_y;
    // find the key for which the value is maximum
    int max_x = -1;
    int max_y = -1;
    for(auto it=x.begin(); it!=x.end(); it++){
        if(it->second > max_x){
            max_x = it->second;
            final_x = it->first;
        }
    }
    for(auto it=y.begin(); it!=y.end(); it++){
        if(it->second > max_y){
            max_y = it->second;
            final_y = it->first;
        }
    }

    Entity *entity = pe[0];
    entity->x = final_x;
    entity->y = final_y;
    vector<Entity*> temp;

    std::vector<SDL_Rect> shape3Path = {};
    shape3Path.push_back({SCREEN_WIDTH-100,10, 1, 1});
    shape3Path.push_back({10, 10, 1, 1});
    entity->patternHandler = new DefaultPatternHandler(shape3Path);
    temp.push_back(entity);
    entityMap[-1] = temp;
}

void P2PClient::handle_req_rep(string message){
    auto msg = messageHandler.parseMessage(message);
    int type = msg.first;
    string data = msg.second;

    if (type == 0){
        string reply = messageHandler.createMessage(0, "Hello");
        messageHandler.sendMessage(my_master_socket, reply);
    }

    // HANDSHAKE REQUEST
    if( type == 1 ){
        // get id in a thread safe manner
        mutex.lock();
        int id = id_counter++;
        // subscribe to 5000+id
        zmq::socket_t sub_socket(context, ZMQ_SUB);
        sub_socket_map[id] = std::move(sub_socket);
        sub_socket_map[id].connect("tcp://localhost:"+std::to_string(5000+id));
        sub_socket_map[id].set(zmq::sockopt::subscribe, ""); // Subscribe to all messages
        // connect to 4000+id
        zmq::socket_t req_socket(context, ZMQ_REQ);
        req_socket_map[id] = std::move(req_socket);
        req_socket_map[id].connect("tcp://localhost:"+std::to_string(4000+id));
        mutex.unlock();

        string reply = messageHandler.createMessage(1, "Id:"+std::to_string(id)+" Peers:"+get_comma_sep_sockets());
        messageHandler.sendMessage(my_master_socket, reply);
    }

    if (type == 2){
        int peer_id;
        sscanf(data.c_str(), "New peer Id: %d", &peer_id);
        subscribe_to_peer(peer_id);
        connect_to_peer(peer_id);
        string reply = messageHandler.createMessage(2, "Okay");
        messageHandler.sendMessage(my_rep_socket, reply);
    }
}