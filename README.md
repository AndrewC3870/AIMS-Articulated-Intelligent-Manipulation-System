# AIMS: Articulated Intelligent Manipulation System

<p align="center">
  <img src="images/full_robot.png" width="50%" />
  <img src="images/AIMS.jpg" width="40%" />
</p>

> **A low-cost, six-degree-of-freedom (6-DoF) robotic manipulator with fully integrated Cartesian control.**

---

## Key Features

*   **Standalone Operation:** The system runs entirely on an **ESP32-S3** microcontroller. It handles inverse kinematics, motion planning, calibration, and teach-and-repeat functionality without requiring an external computer.
*   **Wireless Web Interface:** Enables intuitive, wireless control of the robot directly from any web browser.
*   **Computer Vision Integration:** A separate vision module powered by OpenCV and YOLO performs real-time object detection and workspace mapping.
*   **High Resolution, Low Cost:** Built using stepper motors, belt transmissions, and planetary gear reductions to maximize precision while keeping hardware costs down.
*   **Advanced Kinematics:** Utilizes a real-angle joint model to simplify kinematics, paired with gravity-induced deformation compensation to significantly improve accuracy under load.

## Performance & Capabilities

*   **Repeatability:** Sub-millimeter accuracy.
*   **Workspace Range:** Approximately 750 mm.
*   **Applications:** Highly efficient vision-guided manipulation and automated tasks.
