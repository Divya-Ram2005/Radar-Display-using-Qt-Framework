#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qdatetime.h"
#include <QMainWindow>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QList>
#include <QTableWidget>
#include <QElapsedTimer>
#include <QVector>

struct Target {
    double azimuth;         // degrees
    double range;           // pixels for now (50..250)
    QString signalStrength;
    double prevAzimuth = 0.0;
    double prevRange = 0.0;
    double speed = 0.0;
    double angle;             // ✅ Add this
    QString direction;
    QDateTime detectedTime = QDateTime::currentDateTime();
    QDateTime lastUpdateTime = QDateTime::currentDateTime();
    QString targetType ;         // e.g., "Drone"
    QString trackingMethod ; // default
    int confidence;               // %
    QString alertStatus ;
    void updateAttributes();
    double elevation;
};


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void updateTargetTable(const QVector<Target> &targets);
    void updateAttributes(double speed, double range, double angle, const QString& direction);

    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void startRadar();
    void stopRadar();
    void analyzeTargets();
    void updateRadar();
    //void onTargetSelected(const Target &target);


private:
    // timers
    QTimer *timer = nullptr;
    QTimer *clockTimer = nullptr;

    // widgets
    QWidget *central = nullptr;
    QPushButton *startButton = nullptr;
    QPushButton *stopButton = nullptr;
    QPushButton *analyzeButton = nullptr;

    QLabel *statusLabel = nullptr;
    QLabel *dateTimeLabel = nullptr;

    // --- Control panel (interactive) ---
    QGroupBox *controlPanel = nullptr;
    QSpinBox *prfSpin = nullptr;              // Hz
    QDoubleSpinBox *rotationSpin = nullptr;   // rpm (display only for now)
    QDoubleSpinBox *sweepSpeedSpin = nullptr; // deg/tick
    QLabel *trackCountLabel = nullptr;        // dynamic

    // runtime params
    int    prf = 100;           // Hz
    double rotationRpm = 10.0;  // rpm
    double sweepSpeedDeg = 1.0; // deg per tick
    double sweepAngle = 0.0;

    QList<Target> targets;

    // helpers
    void generateTargets();
    int  updateTargetInfoNearSweep(); // return # under sweep
    void applyPrf();                  // restart timer with PRF
    void recalcSweepSpeedFromRpmAndPrf();

     Ui::MainWindow *ui;
     QTableWidget *targetTableWidget;
     QElapsedTimer elapsedTimer;
     //QVector<Target> targets;
     // Target Panel B
     QGroupBox *targetDetailPanel = nullptr;
     QLabel *targetIdLabel = nullptr;
     QLabel *timeDetectedLabel = nullptr;
     QLabel *lastUpdateLabel = nullptr;
     QLabel *confidenceLabel = nullptr;
     QLabel *targetTypeLabel = nullptr;
     QLabel *coordinatesLabel = nullptr;
     QLabel *trackingMethodLabel = nullptr;
     QLabel *alertLabel = nullptr;
     double calculateAngle(double azimuth, double elevation);  // or just elevation if needed
     QString getDirectionFromAzimuth(double azimuth);


};

#endif // MAINWINDOW_H
