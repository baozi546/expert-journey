#ifndef AVSYNC_H
#define AVSYNC_H

#include <chrono>
#include <ctime>
#include <mutex>
#include <QDebug>

using namespace std::chrono;
class AVSync
{
public:
    AVSync(){};

    void initClock(){
        my_pts=0;
        my_lastnow=GetMicroseconds();
    }
    time_t setClock(long long pts){
        std::lock_guard<std::mutex>lk(my_mutex);
        time_t ret = pts - my_pts;
        my_pts=pts;
        return ret;
    }
    time_t setTime(){
        time_t now=GetMicroseconds();
        time_t ret = now - my_lastnow;
        my_lastnow = now;
        return ret;
    }
    time_t GetBack()
    {
        std::lock_guard<std::mutex>lk(my_mutex);
        return my_pts;
    }

    time_t  GetMicroseconds(){
        system_clock::time_point time_point_new = system_clock::now();
        system_clock::duration duration = time_point_new.time_since_epoch();

        time_t us=duration_cast<microseconds>(duration).count();
        return us;
    }
private:
    time_t my_pts;
    time_t my_lastnow;
    std::mutex my_mutex;
};

#endif // AVSYNC_H
