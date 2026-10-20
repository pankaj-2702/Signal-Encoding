# Digital Signal Encoding & PCM Visualizer

An interactive GUI application built with **C++17** and **SFML** for visualizing digital line encoding schemes, scrambling techniques, and analog Pulse Code Modulation (PCM).

---

## Features

### 1. Digital Line Coding Mode `[D]`
- **Unipolar / Polar / Bipolar Encoders**:
  - NRZ-L (Non-Return-to-Zero Level)
  - NRZ-I (Non-Return-to-Zero Inverted)
  - Manchester (Biphase)
  - Differential Manchester
  - AMI (Alternate Mark Inversion)
- **Scrambling Techniques (AMI)**:
  - Normal (No scrambling) `[N]`
  - B8ZS (Bipolar with 8-Zero Substitution) `[B]`
  - HDB3 (High-Density Bipolar 3-Zero) `[H]`
- **Data Generator Panel**:
  - Random bit generation
  - Longest palindrome substring identification
  - Custom sequence injection (4-zero / 8-zero sequences)

### 2. Analog Pulse Code Modulation Mode `[A]`
- **Continuous Sine Wave Generation**:
  - Clean, high-resolution continuous sine curve $V(t) = A \cdot \sin(2\pi f t)$
- **Interactive Amplitude Control**:
  - Peak amplitude adjustable between $0.10\,\text{V}$ and $1.00\,\text{V}$
  - On-screen `[-]` / `[+]` stepping buttons and quick presets (`0.50 V`, `1.00 V`)
  - Keyboard shortcuts: `[Up]` / `[+]` to increase, `[Down]` / `[-]` to decrease
- **Quantization Modes**:
  - **4-bit PCM (16 levels)**: Displays 16 discrete reconstruction levels and decision boundary lines across $[-1\,\text{V}, +1\,\text{V}]$.
  - **6-bit PCM (64 levels)**: High-resolution reconstruction without cluttering horizontal level lines.
  - Keyboard shortcuts: `[4]` for 4-bit mode, `[6]` for 6-bit mode.
- **Staircase Reconstruction (ZOH)**:
  - Zero-Order Hold DAC reconstruction overlaid with the continuous wave.
  - Sample instant stems and markers.
  - Binary codeword display for each sample.

---

## Build & Run

### Prerequisites
- CMake $\ge$ 3.20
- C++17 compliant compiler (`g++` or `clang++`)
- SFML 2.5 / 2.6 (`sfml-graphics`, `sfml-window`, `sfml-system`)

### Building the Project
```bash
cmake -B build -S .
cmake --build build
```

### Running the Application
```bash
./build/signal-codec
```

### Running the Unit Tests
```bash
ctest --test-dir build --output-on-failure
```
Or run individual test suites:
```bash
./build/test_pcm
./build/test_generator
./build/test_scramblers
```

---

## Controls Summary
| Key | Action |
|:---:|:---|
| `D` | Switch to Digital Line Coding Mode |
| `A` | Switch to Analog PCM Mode |
| `4` | In Analog Mode: Select 4-bit PCM (16 levels) |
| `6` | In Analog Mode: Select 6-bit PCM (64 levels) |
| `Up` / `+` | In Analog Mode: Increase Sine Amplitude (+0.1V) |
| `Down` / `-` | In Analog Mode: Decrease Sine Amplitude (-0.1V) |
| `N` | In Digital AMI Mode: Normal (No scrambling) |
| `B` | In Digital AMI Mode: B8ZS Scrambling |
| `H` | In Digital AMI Mode: HDB3 Scrambling |
| `F11` | Toggle Fullscreen |
