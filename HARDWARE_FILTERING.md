# Hardware Filtering Guide for ADC Stability

## Issue Summary
ESP32-C3 ADC readings show noise and instability:
- Jitter (±1 fluctuation)
- Hitting 4095 before full joystick deflection
- Readings not smooth

## Software Filtering (Already Implemented)

**Oversampling**: Read ADC 4 times and average
**Exponential Moving Average**: Smooth readings over time (alpha = 0.3)

## Hardware Filtering (If Software Filtering Not Enough)

### Option 1: Basic Capacitor Filter (RECOMMENDED)

Add **0.1µF (100nF) ceramic capacitor** between each joystick analog pin and GND.

```
Joystick VRx ──────┬────> ESP32 GPIO (ADC)
                   │
                  ═╪═ 0.1µF capacitor
                   │
                  GND
```

**Where to add:**
- Between Joy1 VRx and GND (near ESP32 pin)
- Between Joy1 VRy and GND
- Between Joy2 VRx and GND
- Between Joy2 VRy and GND

**Total needed:** 4x 0.1µF ceramic capacitors

**Effect:** Filters high-frequency noise on ADC inputs

---

### Option 2: RC Low-Pass Filter (If Basic Filter Not Enough)

Add **resistor + capacitor** for stronger filtering:

```
Joystick VRx ───[10kΩ]───┬────> ESP32 GPIO (ADC)
                         │
                        ═╪═ 0.1µF capacitor
                         │
                        GND
```

**Components per analog input:**
- 10kΩ resistor (in series with signal)
- 0.1µF ceramic capacitor (to GND)

**Total needed:**
- 4x 10kΩ resistors
- 4x 0.1µF capacitors

**Cutoff frequency:** ~160Hz (plenty fast for 10Hz joystick update rate)

---

### Option 3: Power Supply Filtering (For Severe Noise)

If joysticks are noisy due to power supply issues:

**Add to joystick 3.3V power line:**
```
ESP32 3.3V ───[100Ω]───┬────> Joystick VCC
                       │
                      ═╪═ 10µF electrolytic capacitor
                       │
                      GND
```

**Components:**
- 100Ω resistor
- 10µF electrolytic capacitor (watch polarity!)

**Effect:** Decouples joystick power supply from ESP32

---

## Testing Procedure

### Step 1: Test Software Filtering First
```bash
make all
```

Watch the numbers on display:
- Should be much smoother
- Less jitter
- More stable at extremes

### Step 2: If Still Noisy, Add Basic Capacitors
- Add 0.1µF caps between ADC pins and GND
- Test again

### Step 3: If Still Noisy, Add RC Filters
- Add 10kΩ resistors in series
- Keep the 0.1µF caps
- Test again

### Step 4: If Power Supply Issue Suspected
- Add power supply filtering
- Use multimeter to check 3.3V is stable (should not fluctuate)

---

## Diagnosing the Issue

### Check 1: Is it ADC noise or mechanical issue?
- Hold joystick perfectly still at center
- If numbers fluctuate > ±5, it's electrical noise
- If numbers are stable but jump when moving, it's mechanical/software

### Check 2: Is 4095 premature clipping still happening?
Move joystick slowly to extremes:
- Should reach ~100-200 at minimum
- Should reach ~3900-4000 at maximum
- Should NOT hit 0 or 4095 unless at physical stop

If hitting 4095 in middle of travel:
1. Check wiring: Joysticks must be powered by 3.3V (NOT 5V)
2. Measure voltage at joystick VRx/VRy outputs with multimeter
3. Voltage should be ~1.65V at center, ~0.1V at min, ~3.2V at max

### Check 3: Connection Quality
- Use short wires (< 10cm if possible)
- Avoid breadboard if possible (solder directly)
- Twist ADC signal wires with GND wire (reduces noise pickup)
- Keep ADC wires away from power wires

---

## Component Shopping List (If Needed)

**Basic filtering (start here):**
- 4x 0.1µF (100nF) ceramic capacitors

**Advanced filtering:**
- 4x 10kΩ resistors (1/4W)
- 4x 0.1µF ceramic capacitors

**Power supply filtering:**
- 1x 100Ω resistor (1/4W)
- 1x 10µF electrolytic capacitor (16V or higher)

**Total cost:** < $2 USD typically

---

## Software Tuning Parameters

If you want to adjust software filtering in `transmitter.ino`:

### Adjust Smoothing Amount
```c
#define SMOOTHING_ALPHA 0.3  // Change this: 0.1-0.5
```
- **Lower (0.1)**: More smoothing, slower response
- **Higher (0.5)**: Less smoothing, faster response
- **Default (0.3)**: Good balance

### Adjust Oversampling
```c
#define ADC_SAMPLES 4  // Change this: 2-10
```
- **Lower (2)**: Faster but more noise
- **Higher (10)**: Slower but very smooth
- **Default (4)**: Good balance

---

## Expected Results

**With software filtering only:**
- Jitter reduced from ±1 to near-zero at rest
- Smooth movement without jumps
- May still have some noise

**With basic capacitor filter:**
- Very stable readings at rest
- Smooth across full range
- Eliminates most electrical noise

**With RC low-pass filter:**
- Rock-solid stable readings
- Professional-grade filtering
- Suitable for precision control

---

## Notes

1. **Start with software filtering** - already implemented, test first
2. **Add capacitors if needed** - cheap and easy
3. **Only add RC filter if still problematic** - requires more components
4. **Don't over-filter** - too much filtering makes joystick feel sluggish

5. **The 4095 clipping issue** is likely:
   - Bad potentiometer in joystick (mechanical wear)
   - Wiring issue (voltage drop)
   - Wrong power voltage (must be 3.3V)

---

*Try software filtering first, then add hardware filtering only if needed.*
