#include "ledmode.h"

LEDMode::LEDMode(Mode mode, uint8_t brightness, uint8_t speed)
    : mode(mode), brightness(brightness), speed(speed), direction(0) {
    colors.fill(QColor(255, 255, 255)); // Default to white
}


void LEDMode::setColor(int index, const QColor &color) {
    if (index >= 0 && index < 4) {
        this -> colors[index] = color;
    }
}
QColor LEDMode::getColor(int index) const {
    if (index >= 0 && index < 4) {
        return colors[index];  // Return the stored QColor
    }
    return QColor(Qt::white);  // Default if out of range
}

void LEDMode::setDirection(bool dir) {
    this -> direction = dir;
}

bool LEDMode::getDirection() {
    return direction;
}

void LEDMode::setSpeed(uint8_t speed) {
    this -> speed = speed;
}

void LEDMode::setBrightness(uint8_t brightness) {
    this -> brightness = brightness;
}
void LEDMode::setMode(Mode newMode) {
    this -> mode = newMode;
}

LEDMode::Mode LEDMode::getMode() {
    return mode;
}

std::map<LEDMode::Mode, QString> LEDMode::getModeNames() {
    return {
        {LEDMode::Static, "Static"},
        {LEDMode::Runway, "Runway"},
        {LEDMode::Rainbow, "Rainbow"},
        {LEDMode::Morph, "Morph"},
        {LEDMode::Breathing, "Breathing"},
        {LEDMode::Meteor, "Meteor"},
        {LEDMode::TickerTape, "Ticker Tape"},
        {LEDMode::ColorfulStarryNight, "Colorful Starry Night"},
        {LEDMode::Fluctuation, "Fluctuation"}

    };
}
std::vector<uint8_t> LEDMode::generatePacket() const {
    std::vector<uint8_t> packet = {
        static_cast<uint8_t>(mode),  // Mode
        static_cast<uint8_t>(brightness),  // Brightness
        static_cast<uint8_t>(speed)  // Speed
    };

    // 🔹 Insert all **4 colors** (Each color is 3 bytes: RGB)
    for (int i = 0; i < 4; i++) {
        packet.push_back(static_cast<uint8_t>(colors[i].red()));
        packet.push_back(static_cast<uint8_t>(colors[i].green()));
        packet.push_back(static_cast<uint8_t>(colors[i].blue()));
    }

    // 🔹 Append fixed values
    packet.push_back(direction);  // Direction (0 = Left, 1 = Right)
    packet.push_back(DISABLED);   // Disabled (0 = Active)
    packet.push_back(SOURCE);     // Source (0 = External, 1 = MCU)
    packet.push_back(SYNC_TO_PUMP); // SyncToPump (0 = No, 1 = Yes)
    packet.push_back(LED_COUNT);  // LED Count (24 LEDs)

    return packet;
}




