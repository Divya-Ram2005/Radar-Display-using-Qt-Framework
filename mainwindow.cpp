#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QDateTime>
#include <QtMath>
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), sweepAngle(0.0),ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // ---------------- Timers ----------------
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateRadar);

    clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, [this]() {
        dateTimeLabel->setText(QDateTime::currentDateTime().toString("dd-MM-yyyy  hh:mm:ss"));
        update();
    });
    clockTimer->start(1000);

    // ---------------- Central ----------------
    central = new QWidget(this);
    setCentralWidget(central);

    // ---------------- Widgets ----------------
    startButton   = new QPushButton("Start",   this);
    stopButton    = new QPushButton("Stop",    this);
    analyzeButton = new QPushButton("Analyze", this);

    statusLabel = new QLabel("Status: Ready", this);
    statusLabel->setStyleSheet("font-size: 14px; color: black; font-weight: bold;");

    dateTimeLabel = new QLabel(this);
    dateTimeLabel->setAlignment(Qt::AlignCenter);
    dateTimeLabel->setStyleSheet("font-size: 18px; color: black;");

    // =========================================================
    //   LAYOUT:  [ radar + buttons ]   |   [  control panel  ]
    // =========================================================
    auto *mainLayout  = new QHBoxLayout(central);

    // ----- Left: radar controls, status, clock -----
    auto *radarLayout = new QVBoxLayout();
    radarLayout->addWidget(startButton);
    radarLayout->addWidget(stopButton);
    radarLayout->addWidget(analyzeButton);
    radarLayout->addWidget(statusLabel);
    radarLayout->addStretch();
    radarLayout->addWidget(dateTimeLabel, 0, Qt::AlignRight);

    // ----- Right: control panel (interactive) -----
    controlPanel = new QGroupBox("Control Panel", this);
    auto *cpLayout = new QFormLayout(controlPanel);

    prfSpin = new QSpinBox(this);
    prfSpin->setRange(10, 5000);
    prfSpin->setValue(prf);

    rotationSpin = new QDoubleSpinBox(this);
    rotationSpin->setRange(0.1, 60.0);
    rotationSpin->setDecimals(1);
    rotationSpin->setValue(rotationRpm);

    sweepSpeedSpin = new QDoubleSpinBox(this);
    sweepSpeedSpin->setRange(0.01, 50.0);
    sweepSpeedSpin->setDecimals(3);
    sweepSpeedSpin->setSingleStep(0.01);
    sweepSpeedSpin->setValue(sweepSpeedDeg);
    // we derive sweepSpeedDeg from PRF & RPM → make it read-only
    sweepSpeedSpin->setReadOnly(true);
    sweepSpeedSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);

    trackCountLabel = new QLabel("0", this);

    cpLayout->addRow("PRF (Hz):",             prfSpin);
    cpLayout->addRow("Antenna rate (rpm):",   rotationSpin);
    cpLayout->addRow("Sweep speed (°/tick):", sweepSpeedSpin);
    cpLayout->addRow("No. of tracks:",        trackCountLabel);

    // style the panel to look like a radar console
    controlPanel->setStyleSheet(
        "QGroupBox { "
        "   background-color: rgba(0, 50, 0, 180); "
        "   border: 2px solid lime; "
        "   border-radius: 10px; "
        "   color: white; "
        "   font-weight: bold; "
        "   padding: 10px;"
        "} "
        "QLabel { color: white; font-size: 14px; } "
        "QSpinBox, QDoubleSpinBox { color: black; background: #e0ffe0; }"
        );
    // === TARGET PANEL A (Styled like Control Panel) ===
    QGroupBox *groupBox = new QGroupBox("Target Panel A", this);
    QVBoxLayout *groupLayout = new QVBoxLayout();
    groupBox->setLayout(groupLayout);

    targetTableWidget = new QTableWidget(groupBox);
    targetTableWidget->setColumnCount(5);  // ID, Range, Azimuth, Strength, Speed
    targetTableWidget->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Range" << "Azimuth" << "Strength" << "Speed");
    targetTableWidget->horizontalHeader()->setStretchLastSection(true);
    targetTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    targetTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    targetTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    targetTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    groupLayout->addWidget(targetTableWidget);  // ✅ add only once

    // Match styling with control panel
    targetTableWidget->setStyleSheet(
        "QGroupBox { "
        "   background-color: rgba(0, 50, 0, 180); "
        "   border: 2px solid lime; "
        "   border-radius: 10px; "
        "   color: white; "
        "   font-weight: bold; "
        "   padding: 10px;"
        "} "
        "QTableWidget { background-color: black; color: white; }"
        );

    groupLayout->addWidget(targetTableWidget);

    connect(targetTableWidget, &QTableWidget::cellClicked, this, [this](int row, int){
        if (row >= 0 && row < targets.size()) {
            const Target &t = targets[row];
            targetIdLabel->setText(QString::number(row + 1));
            timeDetectedLabel->setText(t.detectedTime.toString("hh:mm:ss"));
            lastUpdateLabel->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
            confidenceLabel->setText(QString::number(t.confidence) + "%");
            targetTypeLabel->setText(t.targetType);
            coordinatesLabel->setText(QString("(%1, %2)").arg(t.range, 0, 'f', 1).arg(t.azimuth, 0, 'f', 1));
            trackingMethodLabel->setText(t.trackingMethod);
            alertLabel->setText(t.alertStatus);

            if (t.alertStatus == "High Threat") {
                alertLabel->setStyleSheet("color: red; font-weight: bold;");
            } else {
                alertLabel->setStyleSheet("color: lime;");
            }
        }
    });


    // === TARGET PANEL B ===
    targetDetailPanel = new QGroupBox("Target Panel B – Selected Target Details", this);
    auto *bLayout = new QFormLayout(targetDetailPanel);

    targetIdLabel = new QLabel("-");
    timeDetectedLabel = new QLabel("-");
    lastUpdateLabel = new QLabel("-");
    confidenceLabel = new QLabel("-");
    targetTypeLabel = new QLabel("-");
    coordinatesLabel = new QLabel("-");
    trackingMethodLabel = new QLabel("-");
    alertLabel = new QLabel("-");

    bLayout->addRow("Target ID:", targetIdLabel);
    bLayout->addRow("Time Detected:", timeDetectedLabel);
    bLayout->addRow("Last Update Time:", lastUpdateLabel);
    bLayout->addRow("Confidence Level (%):", confidenceLabel);
    bLayout->addRow("Target Type:", targetTypeLabel);
    bLayout->addRow("Coordinates (X, Y):", coordinatesLabel);
    bLayout->addRow("Tracking Method:", trackingMethodLabel);
    bLayout->addRow("Alerts:", alertLabel);

    targetDetailPanel->setStyleSheet(
        "QGroupBox { "
        "   background-color: rgba(0, 30, 60, 200); "
        "   border: 2px solid cyan; "
        "   border-radius: 10px; "
        "   color: white; "
        "   font-weight: bold; "
        "   padding: 10px;"
        "} "
        "QLabel { color: white; font-size: 14px; } "
        );

    // === RIGHT PANEL LAYOUT ===
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();
    rightPanelLayout->addWidget(controlPanel);
    rightPanelLayout->addWidget(groupBox);
    rightPanelLayout->addStretch(); // optional: push everything upwards

    rightPanelLayout->addWidget(targetDetailPanel);



    mainLayout->addLayout(radarLayout, 2);   // radar area gets more width
    QWidget *rightPanelWidget = new QWidget;
    rightPanelWidget->setLayout(rightPanelLayout);
    mainLayout->addWidget(rightPanelWidget, 1);
    // panel on the right

    // ---------------- Signals ----------------
    connect(startButton,   &QPushButton::clicked, this, &MainWindow::startRadar);
    connect(stopButton,    &QPushButton::clicked, this, &MainWindow::stopRadar);
    connect(analyzeButton, &QPushButton::clicked, this, &MainWindow::analyzeTargets);

    // PRF & RPM drive sweep speed (deg/tick)
    connect(prfSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int v){
        prf = v;
        applyPrf();                      // restart timer: 1000 / prf
        recalcSweepSpeedFromRpmAndPrf(); // recompute deg/tick & reflect in UI
    });
    connect(rotationSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double v){
        rotationRpm = v;
        recalcSweepSpeedFromRpmAndPrf();
    });

    // initial setup
    applyPrf();
    recalcSweepSpeedFromRpmAndPrf();

    resize(900, 650);
    generateTargets();
}


MainWindow::~MainWindow() {}

void MainWindow::applyPrf()
{
    // Hz -> period ms, clamp to >=1ms
    int periodMs = qMax(1, 1000 / prf);
    timer->start(periodMs);
}

void MainWindow::startRadar() {
    applyPrf(); // start with current PRF
}

void MainWindow::stopRadar() {
    timer->stop();
}

void MainWindow::analyzeTargets() {
    statusLabel->setText("Status: Analyzing...");
}

void MainWindow::generateTargets() {
    targets.clear();
    QStringList strengths = {"Weak", "Moderate", "Strong"};

    for (int i = 0; i < 10; ++i) {
        Target t;
        t.azimuth = QRandomGenerator::global()->generateDouble() * 360.0;
        t.range   = QRandomGenerator::global()->generateDouble() * 200.0 + 50.0;
        int idx   = QRandomGenerator::global()->bounded(strengths.size());
        t.signalStrength = strengths.at(idx);

        t.updateAttributes();

        targets.append(t);
    }

    updateTargetTable(targets.toVector());  // 👈 call it here
}


void MainWindow::updateRadar() {
    // rotate with the live sweep speed
    sweepAngle += sweepSpeedDeg;
    if (sweepAngle >= 360.0)
        sweepAngle -= 360.0;

    int tracksNow = updateTargetInfoNearSweep();
    trackCountLabel->setText(QString::number(tracksNow));
    sweepAngle += sweepSpeedDeg;  // now it’s auto-derived from rpm & prf


    update();
}

void Target::updateAttributes () {
    // Fast and far - likely an aircraft or missile
    if (speed > 800 && range > 200) {
        trackingMethod = "Radar";
        confidence = 95;
        alertStatus = "Possible Missile Detected";
        targetType = "Missile";
    }
    // Fast but close - urgent attention
    else if (speed > 800 && range <= 200) {
        trackingMethod = "High-Freq Radar";
        confidence = 98;
        alertStatus = "High Threat - Fast Close Target";
        targetType = "Fast Intruder";
    }
    // Slow and near - likely a drone or hovering object
    else if (speed < 200 && range < 100) {
        trackingMethod = "Infrared";
        confidence = 85;
        alertStatus = "Low Altitude Object Detected";
        targetType = "Drone";
    }
    // Medium speed, stable direction - commercial aircraft
    else if (speed > 200 && speed <= 800 && direction == "Straight") {
        trackingMethod = "Kalman Filter";
        confidence = 88;
        alertStatus = "Stable Target - Likely Aircraft";
        targetType = "Commercial Aircraft";
    }
    // Target rapidly changing angle - turning or evading
    else if (angle > 30 && angle < 180) {
        trackingMethod = "Optical + Radar Fusion";
        confidence = 70;
        alertStatus = "Evasive Maneuver Detected";
        targetType = "Evasive Aircraft";
    }
    // Very slow or stationary
    else if (speed < 50) {
        trackingMethod = "Thermal Imaging";
        confidence = 60;
        alertStatus = "Suspicious Stationary Object";
        targetType = "Stationary Object";
    }
    // Unknown behavior
    else {
        trackingMethod = "AI Classifier";
        confidence = 50;
        alertStatus = "Unidentified Pattern";
        targetType = "Unknown";
    }
}



int MainWindow::updateTargetInfoNearSweep() {
    const double threshold = 2.0;
    int count = 0;

    for (const Target &t : targets) {
        double diff = std::fmod(std::fabs(t.azimuth - sweepAngle), 360.0);
        if (diff > 180.0) diff = 360.0 - diff;
        if (diff <= threshold) {
            ++count;
        }
    }

    statusLabel->setText(count ? "Status: Target detected!" : "Status: No target detected.");
    return count;
}

void MainWindow::recalcSweepSpeedFromRpmAndPrf()
{
    // protect against div-by-zero
    if (prf <= 0) return;
    sweepSpeedDeg = (rotationRpm * 360.0) / (60.0 * prf);
    // reflect it on the spinbox if you still show it:
    if (sweepSpeedSpin)
        sweepSpeedSpin->setValue(sweepSpeedDeg);
}

void MainWindow::updateTargetTable(const QVector<Target> &targets) {
    targetTableWidget->setRowCount(targets.size());

    for (int row = 0; row < targets.size(); ++row) {
        const Target &t = targets[row];

        targetTableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1))); // ID
        targetTableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(t.range, 'f', 1) + " m"));
        targetTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(t.azimuth, 'f', 1) + "°"));
        targetTableWidget->setItem(row, 3, new QTableWidgetItem(t.signalStrength)); // you called this direction earlier
        targetTableWidget->setItem(row, 4, new QTableWidgetItem("N/A")); // speed not defined
    }
}


void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPoint center(width()*0.4, height() / 2);
    int length = 250;

    // Background
    painter.fillRect(rect(), QColor(135, 206, 250));

    // ----- Range rings + labels -----
    painter.setPen(QPen(QColor(0, 0, 255), 2));
    QFont f = painter.font();
    painter.setFont(f);
    QFontMetrics fm(f);
    for (int r = 50; r <= 250; r += 50) {
        painter.drawEllipse(center, r, r);
        int yOffset = fm.ascent() / 2;
        painter.drawText(center.x() + r + 5, center.y() + yOffset, QString("%1 m").arg(r));
    }

    // ----- Radial degree grid lines -----
    const QVector<int> marks{360, 90, 180, 270};
    painter.setPen(QPen(QColor(0, 0, 255, 120), 1, Qt::DashLine));
    for (int deg : marks) {
        double rad = qDegreesToRadians(static_cast<double>(deg));
        QPoint to(center.x() + length * std::cos(rad),
                  center.y() - length * std::sin(rad));
        painter.drawLine(center, to);
    }

    // ----- Degree labels -----
    painter.setPen(Qt::black);
    f.setBold(true);
    painter.setFont(f);
    const int degreeLabelRadius = length + 30;
    for (int deg : marks) {
        double rad = qDegreesToRadians(static_cast<double>(deg));
        QPoint textPos(center.x() + degreeLabelRadius * std::cos(rad),
                       center.y() - degreeLabelRadius * std::sin(rad));
        painter.drawText(textPos, QString::number(deg) + QChar(0x00B0));
    }

    // ----- Sweep line -----
    painter.setPen(QPen(QColor(0, 128, 0), 4));
    double radians = qDegreesToRadians(sweepAngle);
    QPoint end(center.x() + length * std::cos(radians),
               center.y() - length * std::sin(radians));
    painter.drawLine(center, end);

    // ----- Targets -----
    painter.setBrush(Qt::red);
    painter.setPen(Qt::NoPen);
    for (const Target &t : targets) {
        double angle = qDegreesToRadians(t.azimuth);
        double dist  = t.range;
        QPoint pos(center.x() + dist * std::cos(angle),
                   center.y() - dist * std::sin(angle));
        painter.drawEllipse(pos, 5, 5);
    }
}
