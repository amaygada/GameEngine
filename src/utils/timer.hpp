#pragma once
#include <chrono>
#include <mutex>
#include <vector>

class Timeline {
    private:
        std::mutex mtx;
        int64_t start_time;                  // the time of the *anchor when created
        int64_t elapsed_paused_time; 
        int64_t last_paused_time;
        int64_t tic;                         // units of anchor timeline per stop
        bool paused;
        bool parent_paused;
        Timeline *anchor;                    // the timeline to which this timeline is anchored
        std::vector<Timeline*> children;          // the timelines anchored to this timeline
    public:
        Timeline(Timeline *anchor, int64_t tic);
        Timeline();
        int64_t getTime();
        void pause();
        void resume();
        void changeTic(int64_t tic);
        bool isPaused();
        bool isParentPaused();
};