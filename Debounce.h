#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <functional>
#include <cstdint>
#include <vector>

class Debounce {

public:
    using Handler = std::function<void(std::vector<uint8_t>)>;

private:
    uint32_t detector_cnt {0};
    uint32_t detectorMax {9};
    std::vector<uint8_t> m_cardId_tmp {9,0};
    std::vector<uint8_t> m_cardId {9,0};
    bool lock{false};
    Handler m_handler;

public:

    void setNewCardHandler(Handler&& handler);
    bool setCardId(std::vector<uint8_t> cardId);

};

#endif // DEBOUNCE_H
