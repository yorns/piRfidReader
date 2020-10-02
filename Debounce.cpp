#include "Debounce.h"

void Debounce::setNewCardHandler(Debounce::Handler &&handler) {
    m_handler = std::move(handler);
}

bool Debounce::setCardId(std::vector<uint8_t> cardId) {
    bool found {false};

    if (cardId != m_cardId_tmp)
        detector_cnt = 0;

    if (detector_cnt < detectorMax) {
        m_cardId_tmp = cardId;
        ++detector_cnt;
        lock = false;
    }
    else {
        m_cardId = m_cardId_tmp;
        if (!lock) {
            m_handler(m_cardId);
            lock = true;
        }
        found = true;
    }

    return found;
}
