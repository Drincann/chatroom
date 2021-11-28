#include<iostream>
#include<deque>
class HistoryMessage {
private:
    std::deque<std::string> history;
    std::string historyStr;
    int maxmsg;
    bool isChange = false;
public:
    HistoryMessage(int maxmsg = 500) {
        this->maxmsg = maxmsg;
    }

    void pushMessage(char* str) {
        this->history.push_back(std::string(str));
        if (this->history.size() > this->maxmsg) {
            this->history.pop_front();
        }
        this->isChange = true;
    }

    const char* getHistoryStr() {
        if (this->isChange) {
            this->isChange = false;
            this->historyStr.clear();
            for (std::string& currstr : this->history) {
                this->historyStr += currstr;
            }
        }
        return this -> historyStr.c_str();
    }

};