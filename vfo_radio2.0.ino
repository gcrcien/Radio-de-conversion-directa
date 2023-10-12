#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <si5351.h>
#include <Encoder.h>
#include <EEPROM.h>
#define ENCODER_PIN_1 3
#define ENCODER_PIN_2 5
#define BUTTON_PIN 4
#include <avr/io.h>
#include <SPI.h>
unsigned int hz;
unsigned int khz;
unsigned int mhz;
String shz;
String skhz;
String smhz;
String fstep = "1khz";
String frequency_string;
int pos = 70;
int adc_value;    //Variable used to store the value read from the ADC converter
Encoder encoder(ENCODER_PIN_1, ENCODER_PIN_2);
Si5351 si5351;
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x38

int correction = 0; // correccion de frecuencia arreglar
String signo;
unsigned long currentFrequency; // Start at 1 MHz
unsigned long minFrequency = 50000; // 1 MHz
unsigned long maxFrequency = 5000000; // 30 MHz
unsigned long stepSize = 1000; // Step size in Hz
unsigned long long pll_freq = 70500000000ULL;

String banda;
int sensorValue;
int change;
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}
template <class T> int EEPROM_readAnything(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}



void setup() {
  /////////////////////Setting up LCD ///////////////////////
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Jugo Mental");
  lcd.display();
  lcd.setCursor(12, 1);
  lcd.print(":D");
  delay(500);
  /////////////////////////////////////

  ///////////////////////////////////////
  si5351.set_freq_manual(currentFrequency, pll_freq, SI5351_CLK0);
  si5351.set_freq_manual(currentFrequency, pll_freq, SI5351_CLK1);


  EEPROM_readAnything(5, currentFrequency);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_6MA);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_6MA);

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);

  si5351.update_status();
  actualizar();
}

void actualizar() {
  fstring();
  banda = getband(currentFrequency);

  ////////////////////////////////////
  si5351.set_freq(((currentFrequency * 2 ) * SI5351_FREQ_MULT) ,  SI5351_CLK0);
  //si5351.set_freq(((currentFrequency ) * SI5351_FREQ_MULT) ,  SI5351_CLK1);

  ////////////////////////////////////
  lcd.setCursor(0, 0);
  lcd.print(("VFO") + frequency_string );
  lcd.setCursor(12, 0);
  lcd.print(" MHz");
  lcd.setCursor(0, 1);
  lcd.print(fstep);
  lcd.setCursor(6, 1);
  lcd.print("Banda");
  lcd.setCursor(12, 1);
  lcd.print(banda);
  lcd.display();
  EEPROM_writeAnything(5, currentFrequency);
}


void loop() {
  change = encoder.read();
  if (change != 0) {
    cambioEncoder();
    actualizar();
  }
  if (digitalRead(BUTTON_PIN) == LOW) {
    lcd.clear();
    stepSize = getNextStepSize(stepSize);
    if (stepSize == 10) {
      fstep = "10Hz";
    }    if (stepSize == 100) {
      fstep = "100Hz";

    }    if (stepSize == 1000) {
      fstep = "1KHz";

    } if (stepSize == 5000) {
      fstep = "5KHz";

    } if (stepSize == 10000) {
      fstep = "10KHz";

    }    if (stepSize == 100000) {
      fstep = "100KHz";

    }    if (stepSize == 1000000) {
      fstep = "1MHz";

    }
    actualizar();
    delay(300); // Debounce
  }
  encoder.write(0);
}

void cambioEncoder() {
  currentFrequency += change * stepSize;
  if (currentFrequency < minFrequency) {
    currentFrequency = minFrequency;
  } else if (currentFrequency > maxFrequency) {
    currentFrequency = maxFrequency;
  }
  if (change < 0) {
    signo = "-";
    change = 1;
  }
  if (change > 0) {
    signo = "+";
    change = -1;
  }
  if (change == 0) {
    signo = "";
  }

}

unsigned long getNextStepSize(unsigned long currentStepSize) {
  unsigned long nextStepSize;
  if (currentStepSize == 10) {
    nextStepSize = 100;
  } else if (currentStepSize == 100) {
    nextStepSize = 1000;
  } else if (currentStepSize == 1000) {
    nextStepSize = 10000;
  } else if (currentStepSize == 10000) {
    nextStepSize = 100000;
  } else if (currentStepSize == 100000) {
    nextStepSize = 1000000;
  } else {
    nextStepSize = 10;
  }
  return nextStepSize;
}
void fstring() {

  mhz = currentFrequency / 1000000;
  khz = (currentFrequency % 1000000) / 1000;
  hz = currentFrequency - ((mhz * 1000000) + (khz * 1000));
  // skhz = String(khz);
  smhz = String(mhz);
  if (mhz < 10 && mhz >= 1) {
    smhz = " " + String(mhz);
  }
  //##############  HZ
  if (hz >= 100) {
    shz = "," + String(hz);
  }
  if (hz < 100 && hz > 10) {
    shz = ",0" + String(hz);
  }
  if (hz < 10 && hz >= 1) {
    shz = ",00" + String(hz);
  }
  if (hz == 0) {
    shz = ",000//";
  }
  //#################### KHZ

  if (khz >= 100) {
    skhz = "," + String(khz);
  }
  if (khz < 100 && khz > 10) {
    skhz = ",0" + String(khz);
  }
  if (khz < 10 && khz >= 1) {
    skhz = ",00" + String(khz);
  }
  if (khz == 0) {
    skhz = ",000";
  }

  frequency_string = smhz + skhz + shz;

}

String getband(unsigned long currentFrequency1) {
  if (currentFrequency1 >= 1800000 && currentFrequency1 <= 2000000) {
    return "160m ";
  } else if (currentFrequency1 >= 3500000 && currentFrequency1 <= 4000000) {
    return "80m ";
  } else if (currentFrequency1 >= 7000000 && currentFrequency1 <= 7300000) {
    return "40m ";
  } else if (currentFrequency1 >= 14000000 && currentFrequency1 <= 14350000) {
    return "20m ";
  } else if (currentFrequency1 >= 21000000 && currentFrequency1 <= 21450000) {
    return "15m ";
  } else if (currentFrequency1 >= 26900000 && currentFrequency1 <= 27500000) {
    return "Pedro  ";
  } else if (currentFrequency1 >= 28000000 && currentFrequency1 <= 29700000) {
    return "10m ";
  } else {
    return "N/A ";
  }
}
