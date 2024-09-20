#include "timer.hpp"

Timeline::Timeline(Timeline *anchor, int64_t tic) {
    this->anchor = anchor;
    this->anchor->children.push_back(this);
    this->tic = tic;
    this->start_time = anchor->getTime();
    this->elapsed_paused_time = 0;
    this->last_paused_time = 0;
    this->paused = false;
    this->parent_paused = false;
}

Timeline::Timeline() {
    this->anchor = nullptr;
    this->tic = 1;
    this->start_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    this->elapsed_paused_time = 0;
    this->last_paused_time = 0;
    this->paused = false;
    this->parent_paused = false;
}

int64_t Timeline::getTime() {
    if (this->anchor == nullptr) {
        int64_t current_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        return (current_time - this->start_time - this->elapsed_paused_time)/this->tic;
    } else {
        int64_t current_time = this->anchor->getTime();
        return ((current_time - this->start_time)/this->tic) - this->elapsed_paused_time;
    }
}

void Timeline::pause() {
    std::lock_guard<std::mutex> lock(this->mtx);
    if (!this->paused) {
        this->last_paused_time = this->getTime();
        this->paused = true;
    }
    if (this->children.size() > 0) {
        for (auto &child : this->children) {
            // if(!child->isPaused()){
                child->pause();
                child->parent_paused = true;
                child->paused = true;
            // }
        }
    }
}

void Timeline::resume() {
    std::lock_guard<std::mutex> lock(this->mtx);
    if (this->paused) {
        this->elapsed_paused_time += this->getTime() - this->last_paused_time;
        this->paused = false;
    }
    if (this->children.size() > 0) {
        for (auto &child : this->children) {
            // if(child->parent_paused){
                child->resume();
                child->parent_paused = false;
                child->paused = false;
            // }
        }
    }
}

void Timeline::changeTic(int64_t tic) {
    std::lock_guard<std::mutex> lock(mtx);
    this->tic = tic;
}

bool Timeline::isPaused() {
    return this->paused;
}

bool Timeline::isParentPaused() {
    return this->parent_paused;
}