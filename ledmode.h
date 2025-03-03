#ifndef LEDMODE_H
#define LEDMODE_H

#include <array>
#include <QColor>
#include <vector>
#include <cstdint>
#include <map>
class LEDMode {

public:
    // Fixed values that never change
    static constexpr uint8_t DISABLED = 0x00;
    static constexpr uint8_t SOURCE = 0x00;
    static constexpr uint8_t SYNC_TO_PUMP = 0x00;
    static constexpr uint8_t LED_COUNT = 0x18;

    // Available lighting modes including a couple extra found during reverse engineering
    enum Mode {
        Static = 0x03,
        Runway = 0x05,
        Rainbow = 0x01,
        Morph = 0x02,
        Breathing = 0x04,
        Meteor = 0x06,
        TickerTape = 0x7,
        ColorfulStarryNight = 0x0a,
        Fluctuation = 0x08
    };

    // Set Default values
    LEDMode(Mode mode = Static, uint8_t brightness = 2, uint8_t speed = 2);

    // Sets an array of 4 colors
    void setColor(int index, const QColor &color);
    QColor getColor(int index) const;

    // Sets the flag to reverse the lighting direction
    void setDirection(bool dir);
    bool getDirection();

    // Sets the speed of lighting movements 0-4
    void setSpeed(uint8_t speed);
    uint8_t getSpeed() const { return speed; }

    // Sets the LED brightness 0-4
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const { return brightness; }

    // Sets the lighting mode from the enum above
    void setMode(Mode newMode);
    Mode getMode();

    // Generates the HIDUSB packet because that's what you do
    std::vector<uint8_t> generatePacket() const;
    static std::map<Mode, QString> getModeNames();

private:
    Mode mode;
    uint8_t brightness=2;
    uint8_t speed=2;
    std::array<QColor, 4> colors; // Always store 4 colors (firmware ignores extras)
    bool direction = 0;
};

#endif // LEDMODE_H
