# Radar-Display-using-Qt-Framework

Overview

This project is an interactive **Radar Display System** developed using **C++ and the Qt framework**.  
It simulates a rotating radar antenna, detects multiple targets, and visually displays their
parameters such as **range, azimuth, signal strength, confidence level, tracking method, and alert status**.

The system is designed to resemble a real radar console used in defense and air-traffic monitoring
applications, with real-time updates and user interaction.

Components:

Application entry point : main.cpp
  Initializes the Qt application
  Loads system locale and translations
  Creates and displays the main window
Main Window & UI Layout : mainwindow.ui
  The user interface is designed using "Qt Designer" (we have only Qt creator in which the designer code is geerated automatically once build) and consists of :
  Control buttons :
    Start – Starts the radar sweep
    Stop– Stops the radar sweep
    Analyse – Triggers target analysis
  Control Panel :
    PRF (Pulse Repetition Frequency)
    Antenna Rotation Rate (RPM)
    Sweep Speed (auto-calculated)
    Number of Tracks detected
  Target Panel :
    Target ID
    Azimuth
    Speed
    Range
    Signal Strength
  Radar Display Area:
   Circular radar screen
   Rotating sweep line
   Target positions plotted in real time

Radar Sweep Mechanism :
  The radar sweep rotates continuously from **0° to 360°**
  Sweep speed is calculated using:Sweep Speed (deg/tick) = (Rotation RPM × 360) / (60 × PRF)

How to Run the Project:
  1. Open 'radar.pro' in "Qt Creator"
  2. Configure the Desktop kit
  3. Build the project
  4. Run the application
     Or
  1. click the project name in the home page of Qt creator

Depndencies:
  Qt creator with the version higher than 11.0.0 with the complier version higher than 4.0.0.

A detailed academic explanation is available in : Divya_report_isro.pdf

