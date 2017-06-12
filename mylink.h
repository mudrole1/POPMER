#ifndef MYLINK_H
#define MYLINK_H

#include<string>
#include<iostream>

class MyLink
{
private:
    /* Id of step that link goes from. */
    size_t from_id_;
    /* Time of effect satisfying link. */
    std::string effect_time_;
    /* Id of step that link goes to. */
    size_t to_id_;
    /* Condition satisfied by link. */
    std::string condition_;
    /* Time of condition satisfied by link. */
    std::string condition_time_;
    bool being_threaten_;
public:

    MyLink(size_t from_id, std::string effect_time, size_t to_id, std::string condition, std::string condition_time, bool being_threaten);

    friend bool operator== (MyLink &l1, MyLink &l2);

    void setFromId(size_t fi)
    {
        from_id_ = fi;
    }
    size_t getFromId()
    {
        return from_id_;
    }
    void setToId(size_t fi)
    {
        to_id_ = fi;
    }
    size_t getToId()
    {
        return to_id_;
    }
    void setEffectTime(std::string s)
    {
        effect_time_ = s;
    }
    std::string getEffectTime()
    {
        return effect_time_;
    }
    void setCondition(std::string s)
    {
        condition_ = s;
    }
    std::string getCondition()
    {
        return condition_;
    }
    void setConditionTime(std::string s)
    {
        if((s != "at end")&&
            (s != "over all")&&
            (s != "at start"))
        {
            std::cerr << "wrong condition time in links\n";
            throw;
        }
        condition_time_ = s;
    }
    std::string getConditionTime()
    {
        return condition_time_;
    }
    void setBeingThreaten(bool b)
    {
        being_threaten_ = b;

    }
    bool getBeingThreaten()
    {
        return being_threaten_;
    }
};

#endif // MYLINK_H
