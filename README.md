# Data Link Layer Simulation

This project demonstrates basic implementations of error detection techniques in the **Data Link Layer**, namely **CRC (Cyclic Redundancy Check)** and **Checksum**. It aims to simulate how data integrity can be verified in data communication systems using simple C++ logic.

## Authors

- **Melih Tuna İpek**  
  [LinkedIn](https://www.linkedin.com/in/melih-tuna-ipek/)  
  📧 melihtunaipek.mti@gmail.com

- **Author 2 Name**  
  [LinkedIn Profile]  
  📧 your.email@example.com

- **Author 3 Name**  
  [LinkedIn Profile]  
  📧 your.email@example.com

- **Author 4 Name**  
  [LinkedIn Profile]  
  📧 your.email@example.com

## About the Data Link Layer

The **Data Link Layer** is the second layer in the OSI model. It is responsible for **node-to-node delivery** of data and provides mechanisms for **error detection and correction** to ensure reliable transmission over the physical medium.

### CRC (Cyclic Redundancy Check)

CRC is a technique that uses polynomial division to detect errors in transmitted messages. A CRC code (or "remainder") is generated at the sender's side and verified at the receiver’s side.

### Checksum

Checksum is a simpler method where the sum of all data segments is calculated and sent along with the message. The receiver recalculates the sum to check for errors.

## How to Run the Project

### 1. Install MinGW (Windows Users)

You need to install **MinGW** to compile and run the project.

- Download MinGW from:  
  [https://sourceforge.net/projects/mingw/](https://sourceforge.net/projects/mingw/)

- During installation, make sure to check:
  - `mingw32-gcc-g++`
  - `mingw32-base`

- After installation, **add MinGW to your system PATH**:
  - Go to `System Properties > Environment Variables`
  - Under **System variables**, select `Path` > `Edit`
  - Add the path to the `bin` folder of MinGW, typically:  
    `C:\MinGW\bin`

- Open a new Command Prompt to ensure changes are applied.

### 2. Compile and Run

Assuming the main file is `bitset.cpp`:

```bash
g++ bitset.cpp -o bitset.exe
```
Then run the executable:
```bash
bitset.exe
```
