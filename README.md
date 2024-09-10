

# Introduction
> **프로젝트 요약**:  
프로그래머스 데브코스 리눅스 커널 과정에서 진행한 토이 프로젝트입니다.  
This is a toy project carried out in the Programmers devcourse - Linux kernel track(korean bootcamp).

라즈베리파이 4에서 **인터럽트**, **디바이스 드라이버**, **멀티쓰레드**, **타이머**, **GPIO**, **IPC**(메시지 큐, 시그널, 공유 메모리), **sysfs**, **BMP280 센서**, **LED** 등을 활용했습니다.  
This project uses interrupts, device drivers, multi-threads, timers, GPIO, IPC (message queue, signals, shared memory), BMP280 sensors, and LEDs on Raspberry Pi 4.

---

# Environment
- **OS**: Ubuntu 22.04 (or later)
- **Tools**: Buildroot, Raspberry Pi 4 (target), Makefile
- **Kernel**: Minimal kernel for Raspberry Pi 4 (buildroot `raspberrypi_64_defconfig`)

---

# Usage - please refer to the GIF below

### refer to bin/ :
buildroot의 커널 컴파일 필요없이 라즈베리파이4에 바로 사용할 수 있게 프로젝트의 실행을 위한 컴파일 결과 파일들이 bin폴더에 있습니다(scp로 해당 파일들을 라즈베리파이4에 전송).

Compile result files for the execution of the project are in the bin folder so that they can be used immediately for Raspberry Pi 4 without kernel compile using buildroot. (send them to Raspberry Pi 4 using scp)  

### How to Execute:
1. Compile the project and transfer the necessary files to the Raspberry Pi 4 using `scp`.  
2. Load the required drivers into the kernel and execute the program:  
   - `insmod sensor.ko`  
   - `insmod engine.ko`  
3. Check the sensor values and motor speed changes through the web GUI in real time.  
4. The logs will display sensor data received via the message queue and the messages from the GUI.


### Execute Example:
- **로그 출력**: 오른쪽 터미널에 메시지 큐로 수신된 센서 값 및 GUI에서 받은 메시지가 출력됩니다.  
  Logs are displayed in the right terminal, showing sensor values received via the message queue and messages from the GUI.

- **BMP280 센서**: 실시간으로 읽은 온도 데이터가 웹 GUI에 표시됩니다.  
  The real-time temperature data read by the BMP280 sensor is displayed on the web GUI.

- **모터 속도 조절**: GUI에서 입력한 모터 속도가 로그에 표시되며, LED로 구현된 모터 속도가 변경됩니다.  
  The motor speed entered on the GUI is displayed in the log, and the speed of the virtual motor (LED) changes accordingly.
![d2560](https://github.com/user-attachments/assets/e8818c9c-d1a9-40e9-9139-90a20a29b548)
![rpi](https://github.com/user-attachments/assets/f05bea6e-ebfd-4982-a3f5-808c9f3bbcd6)
### 처음에는 좌측/우측 led가 같은 속도로 점멸하다가 웹gui에 모터 스피드 값으로 30을 입력하여 왼쪽 led가 느려지고 다음에는 90을 주어 빨라진 모습입니다.
### First, the right/left LED blinks at the same speed, then the left LED slows down by entering 30 as the motor speed value in the webgui, and then it is given 90 to speed it up.


---

# Structure

![Untitled](https://github.com/user-attachments/assets/f719118d-e4fc-4374-a5d6-56e1f9c982ab)

이 그림은 프로젝트의 **프로세스**, **쓰레드**, **드라이버** 간의 관계를 보여줍니다.  
This diagram shows the relationship between the processes, threads, and drivers in the project.

- **화살표**: 웹 기반 GUI의 결과물 과정을 나타냅니다.  
  The arrows indicate the process flow as displayed on the web-based GUI.
  
- 웹 GUI에 표시되지 않는 여러 기능이 백그라운드에서 동작합니다.  
  Several functions operate in the background, even if not displayed in the web GUI.

---

# Driver

### **Engine**:  
- `insmod engine.ko`  
- led로 만든 가상의 모터이며 각각 좌측/우측 led의 점멸 속도로 모터 속도를 나타냅니다.
- web-based-ui의 모터 속도 조절에 반응하며 또한 ui/input.c의 toy_set_motor_1_speed, toy_set_motor_2_speed 함수를 통해 사용자의 m1, m2 값 입력으로도 속도 조절이 가능합니다.
- It is a virtual motor made of led and represents the speed of the motor with the flashing speed of the left/right led, respectively.
It responds to the speed control of the web-based-ui motor and also
The toy_set_motor_1_speed, toy_set_motor_2_speed function of ui/input.c also allows speed control with the user's input of "m1 [value]" and "m2 [value]".
  **Virtual motor** using LEDs, controlled by web UI and functions in `input.c`.

### **Simple I/O**:  
- `insmod simple_io.ko`  
- **한국어**:
  - `n` 입력 시 상태 변경:
    - 1st state: 좌측 4개 LED on/off  
    - 2nd state: 우측 4개 LED on/off  
    - 3rd state: 좌우 번갈아 on/off  
    - 4th state: 주기적 교차 on/off  
    - 5th state: 스위치(GPIO 16)로 제어  
    - 6th state: 스위치 해제 시 상태 변경  
- **English**:  
  - When `n` is entered, the state changes:
    - 1st state: Left 4 LEDs on/off  
    - 2nd state: Right 4 LEDs on/off  
    - 3rd state: Left and right LEDs alternate on/off  
    - 4th state: Periodic alternating on/off  
    - 5th state: Controlled by the switch (GPIO 16)  
    - 6th state: LED state changes when the switch is released  

### **State Machine**:  
- `insmod state_machine.ko`  
- 해당 프로젝트와 무관하게 상태 머신 디자인 패턴을 사용하여 구현한 LED on/off 드라이버입니다.  
  This is a standalone LED on/off driver implemented with the state machine design pattern.

### **v4l2 Driver (Video for Linux 2)**:  
- 아직 적용되지 않았지만, 추후에 추가하여 HAL 계층과 통신하며 프로그램의 카메라 드라이버로 사용될 예정입니다.  
  This driver will be added in the future to communicate with the HAL layer and function as the program's camera driver.

### **BMP280 Sensor Driver**:  
- `insmod sensor.ko`  
- 라즈베리파이 4 GPIO에 연결된 BMP280 센서 드라이버로, 온도 센서 값을 불러오는 데 사용됩니다.  
  The driver for the BMP280 sensor connected to the Raspberry Pi 4 GPIO, used to retrieve temperature sensor values.

---

# Process Threads

### **Input Process** (`input.c`)
- **Command Thread**: 사용자 명령을 처리, 다양한 함수로 전달  
  Processes user commands and passes them to various functions.

- **Sensor Thread**: 센서 데이터 읽기 및 메시지 큐로 전달  
  Reads sensor data and sends it via the message queue.

### **System Server Process** (`system_server.c`)
- **Engine Thread(*Real-time Thread)**: 좌우 모터 제어  
  Controls the left and right motors.

- **Monitor Thread**: 센서 데이터를 수집하고, 다른 모듈로 전송  
  Collects sensor data and sends it to other modules.

- **Disk Service Thread**:  
  라즈베리파이 4 환경으로 이전하기 전에 **PC 실행 환경에서 inotify**를 사용하여 파일 시스템의 변화를 감지하고, 디렉토리 크기를 확인하며 파일 시스템 상태를 모니터링하는 역할을 합니다.  
  This thread monitored the status of the file system using **inotify** in the PC environment to detect file system changes and check directory size before moving to the Raspberry Pi 4 environment.

- **Camera Service Thread**:  
  이 쓰레드는 **카메라 HAL 모듈**과 상호작용합니다. 아직 v4lv2 드라이버가 추가되기 전이라서 현재는 **카메라 공유 라이브러리**와 통신하며 입력에 따른 동작 및 문자열을 출력하는 기능을 수행합니다.  
  This thread interacts with the **camera HAL module**. Since the v4lv2 driver hasn’t been added yet, it communicates with the **camera shared library**, executing actions and outputting strings based on the input.

### Web-based gui부분은 해당 프로젝트에 중요한 부분이 아니라 레퍼런스 코드를 사용했고 깃허브의 사용언어 통계에 빠지도록 설정해놨습니다.
### The Web-based gui part is not an important part of the project, so I used the reference code without modification and set it to be excluded from the statistics of the used language in Github.

---

# Installation & Development Setup

### **Buildroot**:
```sh
git clone https://github.com/raspberrypi/buildroot
cd buildroot
git reset 39a2ff16f92a61a3e8593c394d211c18d01de8d4 –hard
make raspberrypi4_64_defconfig
make linux-menuconfig -> kernel hacking -> compile-time checks and compiler options -> compile the kernel with debug info (o)
make menuconfig -> Target options -> Filesystem images -> cpio the root filesystem (o)
```

### **Makefile**:
프로젝트를 다시 빌드하려면 빌드루트의 커널 컴파일 이후 프로젝트에 존재하는 makefile의 내용을 아래처럼 buildroot에 대한 경로를 바꿔줘야합니다.

You need to modify Makefile. Set the paths for **`BUILDROOT_DIR`** and **`TOOLCHAIN_DIR`** as follows:

```Makefile
# Modify the top-level Makefile
BUILDROOT_DIR = /path/to/buildroot   # Set the buildroot path
TOOLCHAIN_DIR = $(BUILDROOT_DIR)/output/host/bin   # Set the toolchain path
```

### **Host (PC)**:
```sh
make toy-fe
make toy-be
make
scp libcamera.so bin/toy_system toy-be/toy-be drivers/engine/engine.ko drivers/sensor/sensor.ko root@#your-rpi4-ip
```

### **Target (Raspberry Pi 4)**:
```sh
modprobe i2c-bcm2835
modprobe i2c-dev
insmod sensor.ko
insmod engine.ko
./toy_system
```

