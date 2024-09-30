#include "p2pclient.hpp"

P2PClient::P2PClient() : 
    context(1),
    my_pub_socket(context, ZMQ_PUB),
    my_req_rep_socket(context, ZMQ_REQ),
    peer_rep_socket(context, ZMQ_REP)
    {
        my_req_rep_socket.setsockopt(ZMQ_RCVTIMEO, 1000);
        my_req_rep_socket.connect("tcp://localhost:4000");
    }

unordered_map<int, std::vector<Entity *>> P2PClient::get_entity_map(){
    return entityMap;
}

unordered_map<int, zmq::socket_t>& P2PClient::get_socket_map(){
    return sub_socket_map;
}

void P2PClient::update_id(int id){
    this->id = id;
}

int P2PClient::get_id(){
    return id;
}

void P2PClient::listen_for_new_peers(zmq::socket_t &socket){
    // listen for new peers
    zmq::message_t request;
    while(true){
        my_req_rep_socket.recv(request, zmq::recv_flags::none);
        std::string message = std::string(static_cast<char*>(request.data()), request.size());
        handle_master_req_rep(message);
    }
}

void P2PClient::bind_publisher(){
    int port = 5000 + id;
    my_pub_socket.bind("tcp://*:"+std::to_string(port));
}

void P2PClient::expose_port_and_connect(){
    // try connecting to tcp:5000 3 times
    for(int i=0; i<3; i++){
        string message = messageHandler.createMessage(0, "Hello");
        messageHandler.sendMessage(my_req_rep_socket, message);
        string reply = messageHandler.receiveMessage(my_req_rep_socket, zmq::recv_flags::none);
        if(reply==""){
                id = 0;
                my_req_rep_socket.close();
                // my_req_rep_socket = zmq::socket_t(context, ZMQ_REP);
                peer_rep_socket.bind("tcp://*:4000");
                std::thread listen_thread(&P2PClient::listen_for_new_peers, this, std::ref(peer_rep_socket));
                listen_thread.detach();
                break;
        }
        
    }
}

void P2PClient::send_id_request(){
    if(id == 0) return;
    string message = messageHandler.createMessage(1, "Hello");
    messageHandler.sendMessage(my_req_rep_socket, message);
    string reply = messageHandler.receiveMessage(my_req_rep_socket);
    auto msg = messageHandler.parseMessage(reply);
    string peers;
    int peer_id;
    sscanf(msg.second.c_str(), "Id:%d Peers:%255[^n]", &peer_id, &peers);
    update_id(peer_id);
    
    // start listening at 4000+id
    peer_rep_socket.bind("tcp://*:"+std::to_string(4000+id));
    std::thread listen_thread(&P2PClient::listen_for_new_peers, this, std::ref(req_socket_map[id]));

    // connect to peers
    std::istringstream iss(peers);
    std::string peer;
    while (std::getline(iss, peer, ',')) {
        int peer_id = std::stoi(peer);
        if(peer_id == id) continue;
        zmq::socket_t sub_socket(context, ZMQ_SUB);
        sub_socket_map[peer_id] = std::move(sub_socket);
        sub_socket_map[peer_id].connect("tcp://localhost:"+std::to_string(5000+peer_id));
        zmq::socket_t req_socket(context, ZMQ_REQ);
        req_socket_map[peer_id] = std::move(req_socket);
        req_socket_map[peer_id].connect("tcp://localhost:"+std::to_string(4000+peer_id));

        // send handshake request
        if(peer_id == 0) continue;
        string message = messageHandler.createMessage(2, "New peer Id: "+std::to_string(id));
        messageHandler.sendMessage(req_socket_map[peer_id], message);
        string peer_reply = messageHandler.receiveMessage(req_socket_map[peer_id]);
        cout << peer_reply << endl;
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

void P2PClient::handle_master_req_rep(string message){
    auto msg = messageHandler.parseMessage(message);
    int type = msg.first;
    string data = msg.second;

    if (type == 0){
        string reply = messageHandler.createMessage(0, "Hello");
        messageHandler.sendMessage(my_req_rep_socket, reply);
    }

    // HANDSHAKE REQUEST
    if( type == 1 ){
        // get id in a thread safe manner
        mutex.lock();
        int id = id_counter++;
        zmq::socket_t sub_socket(context, ZMQ_SUB);
        sub_socket_map[id] = std::move(sub_socket);
        sub_socket_map[id].connect("tcp://localhost:"+std::to_string(5000+id));
        zmq::socket_t req_socket(context, ZMQ_REQ);
        req_socket_map[id] = std::move(req_socket);
        req_socket_map[id].connect("tcp://localhost:"+std::to_string(4000+id));
        mutex.unlock();
        // send id to client
        string id_str = std::to_string(id);
        string message = "Id:"+id_str+" Peers:"+get_comma_sep_sockets();
        string reply = messageHandler.createMessage(1, message);
        messageHandler.sendMessage(my_req_rep_socket, reply);
    }

    if (type == 2){
        int peer_id;
        sscanf(data.c_str(), "New peer Id: %d", &peer_id);
        // zmq::socket_t sub_socket(context, ZMQ_SUB);
        // sub_socket_map[peer_id] = std::move(sub_socket);
        // sub_socket_map[peer_id].connect("tcp://localhost:"+std::to_string(5000+peer_id));
        // zmq::socket_t req_socket(context, ZMQ_REQ);
        // req_socket_map[peer_id] = std::move(req_socket);
        // req_socket_map[peer_id].connect("tcp://localhost:"+std::to_string(4000+peer_id));
        cout<<message<<endl;
        messageHandler.sendMessage(req_socket_map[peer_id], "Okay");
    }
}