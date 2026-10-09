#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Formats the time to 0:00, etc.
static std::string formatTime(float seconds)
{
    if (seconds < 0.f)
        seconds = 0.f;
    int total = static_cast<int>(seconds);
    return fmt::format("{}:{:02}", total / 60, total % 60);
}

// Modifies PlayLayer (Level)
class $modify(TimeProgressPlayLayer, PlayLayer)
{
    struct Fields
    {
        CCLabelBMFont *timeLabel = nullptr;
        float totalTime = -1.f;
    };

    // Run when level has fully loaded
    void setupHasCompleted()
    {
        PlayLayer::setupHasCompleted();

        // Skip platformer levels
        if (!m_level || m_level->isPlatformer())
            return;

        // Create new label for text
        auto label = CCLabelBMFont::create("0:00 / 0:00", "bigFont.fnt");
        label->setID("time-progress-label"_spr);

        // Get percentage label
        auto percent = typeinfo_cast<CCLabelBMFont *>(
            this->getChildByIDRecursive("percentage-label"));

        CCNode *parent = this->getChildByType<UILayer>(0);
        if (!parent)
            parent = this;

        if (percent && percent->getParent())
        {
            // Add time label underneath percentage label
            label->setScale(percent->getScale() * 0.6f);
            label->setAnchorPoint(percent->getAnchorPoint());
            label->setPosition(percent->getPosition() - ccp(0.f, 14.f));
        }
        else
        {
            // Failsafe for if player's have percentage disabled (I think)
            auto winSize = CCDirector::get()->getWinSize();
            label->setScale(0.35f);
            label->setPosition({winSize.width / 2.f, winSize.height - 22.f});
        }

        parent->addChild(label, 100);

        m_fields->timeLabel = label;
    }

    // Fix for some levels displaying incorrect final time
    float getTotalTime()
    {
        // 2.2+ levels total time in ticks (240/sec)
        if (m_level && m_level->m_timestamp > 0)
        {
            return m_level->m_timestamp / 240.f;
        }
        // 2.1- levels total time in seconds
        if (m_levelLength > 0.f)
        {
            return this->timeForPos(ccp(m_levelLength, 0.f), 0, 0, true, 0);
        }
        return -1.f;
    }

    // Actual functionality of time label
    void updateProgressbar()
    {
        PlayLayer::updateProgressbar();

        auto label = m_fields->timeLabel;
        if (!label || !m_player1)
            return;

        if (m_fields->totalTime <= 0.f)
        {
            m_fields->totalTime = getTotalTime();
        }
        if (m_fields->totalTime <= 0.f)
            return;

        float current = this->timeForPos(m_player1->getPosition(), 0, 0, true, 0);
        current = std::min(current, m_fields->totalTime);

        label->setString(
            fmt::format("{} / {}", formatTime(current), formatTime(m_fields->totalTime)).c_str());
    }
};