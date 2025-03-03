#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColorDialog>
#include <QPushButton>
#include <QSettings>
#include <QListWidget>
#include <QLineEdit>
#include "ledmode.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void saveProfile();
    void loadProfile();
    void deleteProfile();
    void newProfile();
private:
    void applyLEDSettings();
    void openColorPicker(int index);
    void setDirection();
    void updateSelectedMode();
    void updateProfileName(QListWidgetItem *item);
    Ui::MainWindow *ui;
    void snapToTick();
    LEDMode ledMode;  // 🔹 Instantiate LEDMode object as a member
    QSettings settings;  // Persistent storage
    void loadProfileList();  // Helper function to update the list widget
};
#endif // MAINWINDOW_H
