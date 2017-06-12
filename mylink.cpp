#include "mylink.h"

MyLink::MyLink(size_t from_id, std::string effect_time, size_t to_id, std::string condition, std::string condition_time, bool being_threaten)
{
    from_id_ = from_id;
    effect_time_ = effect_time;
    to_id_ = to_id;
    condition_ = condition;
    condition_time_ = condition_time;
    being_threaten_ = being_threaten;

    if((condition_time != "at end")&&
        (condition_time != "over all")&&
        (condition_time != "at start"))
    {
        std::cerr << "wrong condition time in links\n";
        throw;
    }
}

bool operator== (MyLink &l1, MyLink &l2)
{
    if((l1.from_id_ == l2.from_id_)&&(l1.effect_time_ == l2.effect_time_)&&(l1.to_id_ == l2.to_id_)
       &&(l1.condition_ == l2.condition_) && (l1.condition_time_ == l2.condition_time_) && (l1.being_threaten_ == l2.being_threaten_))
    {
        return true;
    }
    else
        return false;
}

