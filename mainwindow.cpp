#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <hidapi/hidapi.h>
#include <vector>
#include <QMessageBox>
#include <QSet>

#define VENDOR_ID 0x0416
#define PRODUCT_ID 0x7399

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), ledMode(LEDMode::Static)
{
    ui->setupUi(this);
    loadProfileList();

    QPushButton* colorButtons[] = {ui->ledColor0, ui->ledColor1, ui->ledColor2, ui->ledColor3};
    connect(ui->applyLEDSettings, &QPushButton::clicked, this, &MainWindow::applyLEDSettings);
    connect(ui->ledSpeed, &QSlider::valueChanged, this, &MainWindow::snapToTick);
    connect(ui->ledBrightness, &QSlider::valueChanged, this, &MainWindow::snapToTick);
    connect(ui->ledDirection, &QCheckBox::checkStateChanged, this, &MainWindow::setDirection);
    connect(ui->saveProfileButton, &QPushButton::clicked, this, &MainWindow::saveProfile);
    connect(ui->loadProfileButton, &QPushButton::clicked, this, &MainWindow::loadProfile);
    connect(ui->deleteProfileButton, &QPushButton::clicked, this, &MainWindow::deleteProfile);
    connect(ui->newProfileButton, &QPushButton::clicked, this, &MainWindow::newProfile);
    connect(ui->profileList, &QListWidget::itemClicked, this, &MainWindow::updateProfileName);

    for (int i = 0; i < 4; i++) {
        connect(colorButtons[i], &QPushButton::clicked, this, [this, i]() { openColorPicker(i); });
    }

    for (const auto& [mode, name] : LEDMode::getModeNames()) {
        ui->ledModes->addItem(name, static_cast<int>(mode));
    }

    connect(ui->ledModes, &QComboBox::currentIndexChanged, this, &MainWindow::updateSelectedMode);
    updateSelectedMode();
}

void MainWindow::snapToTick() {
    QSlider *slider = qobject_cast<QSlider*>(sender());
    if (!slider) return;

    int snappedValue = std::clamp(std::round(slider->value()), 0.0, 4.0);
    slider->setValue(snappedValue);

    if (slider == ui->ledBrightness) {
        ledMode.setBrightness(snappedValue);
        qDebug() << "Updated Brightness:" << snappedValue;
    } else if (slider == ui->ledSpeed) {
        ledMode.setSpeed(snappedValue);
        qDebug() << "Updated Speed:" << snappedValue;
    }
}

void MainWindow::setDirection() {
    ledMode.setDirection(ui->ledDirection->isChecked());
    qDebug() << "Direction:" << ui->ledDirection->isChecked();
}

void MainWindow::applyLEDSettings() {
    hid_init();
    hid_device *device = hid_open(VENDOR_ID, PRODUCT_ID, nullptr);
    if (!device) {
        qDebug() << "Failed to open HID device!";
        return;
    }

    std::vector<uint8_t> led_packet = ledMode.generatePacket();
    std::vector<uint8_t> full_packet(64, 0x00);
    full_packet[0] = 0x01;
    full_packet[1] = 0x85;
    full_packet[5] = led_packet.size();
    std::copy(led_packet.begin(), led_packet.end(), full_packet.begin() + 6);

    int res = hid_write(device, full_packet.data(), full_packet.size());
    if (res < 0) {
        qDebug() << "❌ Failed to send HID packet!";
    } else {
        qDebug() << "✅ LED settings applied successfully!";
    }

    hid_close(device);
    hid_exit();
}

void MainWindow::updateSelectedMode() {
    LEDMode::Mode selectedMode = static_cast<LEDMode::Mode>(ui->ledModes->currentData().toInt());
    ledMode.setMode(selectedMode);
    qDebug() << "Selected Mode:" << ui->ledModes->currentText();
}

void MainWindow::openColorPicker(int index) {
    QColor color = QColorDialog::getColor(Qt::white, this, "Select a Color");
    if (color.isValid()) {
        ledMode.setColor(index, color);

        QPushButton* colorButtons[] = {ui->ledColor0, ui->ledColor1, ui->ledColor2, ui->ledColor3};
        colorButtons[index]->setStyleSheet(QString("background-color: %1; border-radius: 5px;").arg(color.name()));

        qDebug() << "Selected Color (" << index << "):" << color.name();
    }
}

void MainWindow::saveProfile() {
    QString profileName = ui->profileNameInput->text().trimmed();
    if (profileName.isEmpty()) {
        qDebug() << "Profile name is empty. Cannot save.";
        return;
    }

    if (settings.childGroups().contains(profileName)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Overwrite Profile", "A profile with this name already exists. Overwrite?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;
    }

    settings.beginGroup(profileName);
    settings.setValue("mode", static_cast<int>(ledMode.getMode()));
    settings.setValue("brightness", ledMode.getBrightness());
    settings.setValue("speed", ledMode.getSpeed());
    settings.setValue("direction", ledMode.getDirection());

    for (int i = 0; i < 4; i++) {
        settings.setValue(QString("color%1").arg(i), ledMode.getColor(i).name());
    }

    settings.endGroup();
    settings.sync();
    qDebug() << "Profile saved:" << profileName;

    loadProfileList();
    ui->profileNameInput->clear();
}

void MainWindow::updateProfileName(QListWidgetItem *item) {
    if (item) ui->profileNameInput->setText(item->text());
}

void MainWindow::loadProfile() {
    QListWidgetItem *selectedItem = ui->profileList->currentItem();
    if (!selectedItem) {
        qDebug() << "No profile selected. Cannot load.";
        return;
    }

    QString profileName = selectedItem->text();
    ui->profileNameInput->setText(profileName);

    settings.beginGroup(profileName);
    ledMode.setMode(static_cast<LEDMode::Mode>(settings.value("mode").toInt()));
    ledMode.setBrightness(settings.value("brightness").toUInt());
    ledMode.setSpeed(settings.value("speed").toUInt());
    ledMode.setDirection(settings.value("direction").toBool());

    QColor colors[4];
    for (int i = 0; i < 4; i++) {
        colors[i] = QColor(settings.value(QString("color%1").arg(i)).toString());
        ledMode.setColor(i, colors[i]);
    }
    settings.endGroup();

    ui->ledModes->setCurrentIndex(ui->ledModes->findData(static_cast<int>(ledMode.getMode())));
    ui->ledBrightness->setValue(ledMode.getBrightness());
    ui->ledSpeed->setValue(ledMode.getSpeed());
    ui->ledDirection->setChecked(ledMode.getDirection());

    QPushButton* colorButtons[] = {ui->ledColor0, ui->ledColor1, ui->ledColor2, ui->ledColor3};
    for (int i = 0; i < 4; i++) {
        colorButtons[i]->setStyleSheet(QString("background-color: %1; border-radius: 5px;").arg(colors[i].name()));
    }

    qDebug() << "Profile loaded:" << profileName;
}

void MainWindow::deleteProfile() {
    QListWidgetItem *selectedItem = ui->profileList->currentItem();
    if (!selectedItem) {
        qDebug() << "No profile selected. Cannot delete.";
        return;
    }

    settings.remove(selectedItem->text());
    qDebug() << "Profile deleted:" << selectedItem->text();

    loadProfileList();
}

void MainWindow::newProfile() {
    ui->profileNameInput->clear();
    ledMode = LEDMode(LEDMode::Static);
    qDebug() << "New profile initialized.";
}

void MainWindow::loadProfileList() {
    ui->profileList->clear();
    ui->profileList->addItems(settings.childGroups());
}

MainWindow::~MainWindow() {
    delete ui;
}
