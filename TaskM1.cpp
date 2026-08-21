#include <LiquidCrystal.h>

// =====================================================
// LCD
// =====================================================
LiquidCrystal lcd(6, 7, 11, 12, 13, A0);

// =====================================================
// INPUTS
// =====================================================
const byte PIR_PIN       = 8;
const byte TILT_PIN      = 9;
const byte HEARTBEAT_PIN = 10;
const byte BUTTON_PIN    = A1;

// =====================================================
// OUTPUTS
// =====================================================
const byte GREEN_LED  = 2;
const byte YELLOW_LED = 3;
const byte RED_LED    = 4;
const byte BUZZER     = 5;

// =====================================================
// PCI VARIABLES
// =====================================================
volatile bool pirChanged = false;
volatile bool tiltChanged = false;
volatile bool heartbeatChanged = false;

volatile bool pirState = false;
volatile bool tiltState = false;
volatile bool heartbeatState = false;

// Count heartbeat transitions
volatile unsigned int heartbeatPulses = 0;

// =====================================================
// TIMER VARIABLE
// =====================================================
volatile bool timerTick = false;

// =====================================================
// HEARTBEAT MONITOR
// =====================================================
unsigned long lastHeartbeat = 0;

const unsigned long HEARTBEAT_TIMEOUT = 3000;

// =====================================================
// PIN CHANGE INTERRUPT
// D8  = PIR
// D9  = TILT
// D10 = 555 HEARTBEAT
// =====================================================
ISR(PCINT0_vect)
{
  byte currentState = PINB;

  bool newPir =
    currentState & (1 << PB0);

  bool newTilt =
    currentState & (1 << PB1);

  bool newHeartbeat =
    currentState & (1 << PB2);

  // -----------------------------
  // PIR CHANGE
  // -----------------------------
  if (newPir != pirState)
  {
    pirState = newPir;
    pirChanged = true;
  }

  // -----------------------------
  // TILT CHANGE
  // -----------------------------
  if (newTilt != tiltState)
  {
    tiltState = newTilt;
    tiltChanged = true;
  }

  // -----------------------------
  // HEARTBEAT CHANGE
  // -----------------------------
  if (newHeartbeat != heartbeatState)
  {
    heartbeatState = newHeartbeat;
    heartbeatChanged = true;

    heartbeatPulses++;
  }
}

// =====================================================
// TIMER1 INTERRUPT
// Approximately 1 SECOND
// =====================================================
ISR(TIMER1_COMPA_vect)
{
  timerTick = true;
}

// =====================================================
// SETUP
// =====================================================
void setup()
{
  Serial.begin(9600);

  // Inputs
  pinMode(PIR_PIN, INPUT);
  pinMode(TILT_PIN, INPUT_PULLUP);
  pinMode(HEARTBEAT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Outputs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Initial outputs
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // LCD
  lcd.begin(16, 2);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FRED SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("STARTING...");

  delay(1000);

  // ===================================================
  // INITIAL PORT B STATE
  // ===================================================
  byte initialState = PINB;

  pirState =
    initialState & (1 << PB0);

  tiltState =
    initialState & (1 << PB1);

  heartbeatState =
    initialState & (1 << PB2);

  // ===================================================
  // PIN CHANGE INTERRUPTS
  // ===================================================
  PCICR |= (1 << PCIE0);

  // D8 - PIR
  PCMSK0 |= (1 << PCINT0);

  // D9 - TILT
  PCMSK0 |= (1 << PCINT1);

  // D10 - 555 HEARTBEAT
  PCMSK0 |= (1 << PCINT2);

  // ===================================================
  // TIMER1
  // ===================================================
  cli();

  TCCR1A = 0;
  TCCR1B = 0;

  // CTC mode
  TCCR1B |= (1 << WGM12);

  // Prescaler 1024
  TCCR1B |= (1 << CS12);
  TCCR1B |= (1 << CS10);

  // Approximately 1 second
  OCR1A = 15624;

  // Enable Timer1 Compare A interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();

  lastHeartbeat = millis();

  // ===================================================
  // SERIAL HEADER
  // ===================================================
  Serial.println();
  Serial.println("========================================");
  Serial.println("          FRED SYSTEM MONITOR");
  Serial.println("========================================");
  Serial.println("PCI D8  : PIR");
  Serial.println("PCI D9  : TILT");
  Serial.println("PCI D10 : 555 HEARTBEAT");
  Serial.println("TIMER1  : 1 SECOND");
  Serial.println("A1      : MANUAL HEARTBEAT");
  Serial.println("========================================");
  Serial.println("SYSTEM READY");
  Serial.println("========================================");
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop()
{
  // ===================================================
  // PIR PCI EVENT
  // ===================================================
  if (pirChanged)
  {
    noInterrupts();

    bool state = pirState;
    pirChanged = false;

    interrupts();

    Serial.println();

    if (state)
    {
      Serial.println("[PIR] PCI EVENT");
      Serial.println("[HIGH ALERT] MOTION DETECTED");
      Serial.println("[ACTION] RED LED + BUZZER");
    }
    else
    {
      Serial.println("[PIR] PCI EVENT");
      Serial.println("[HIGH ALERT] MOTION CLEARED");
      Serial.println("[STATUS] Monitoring resumed");
    }
  }

  // ===================================================
  // TILT PCI EVENT
  // ===================================================
  if (tiltChanged)
  {
    noInterrupts();

    bool state = tiltState;
    tiltChanged = false;

    interrupts();

    Serial.println();

    // Tilt sensor is active LOW
    if (!state)
    {
      Serial.println("[TILT] PCI EVENT");
      Serial.println("[WARNING] TAMPER DETECTED");
      Serial.println("[ACTION] YELLOW LED + BUZZER");
    }
    else
    {
      Serial.println("[TILT] PCI EVENT");
      Serial.println("[WARNING] TAMPER CLEARED");
      Serial.println("[STATUS] Monitoring resumed");
    }
  }

  // ===================================================
  // HEARTBEAT PCI EVENT
  // ===================================================
  if (heartbeatChanged)
  {
    noInterrupts();

    heartbeatChanged = false;

    interrupts();

    // Any heartbeat transition means 555 is alive
    lastHeartbeat = millis();
  }

  // ===================================================
  // MANUAL HEARTBEAT BUTTON
  // ===================================================
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    lastHeartbeat = millis();

    Serial.println("[BUTTON] Manual heartbeat received");

    delay(100);
  }

  // ===================================================
  // TIMER1 EVENT
  // ===================================================
  if (timerTick)
  {
    noInterrupts();

    timerTick = false;

    unsigned int pulses = heartbeatPulses;

    heartbeatPulses = 0;

    interrupts();

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("[TIMER] 1-second periodic check");

    // Heartbeat information
    if (pulses > 0)
    {
      Serial.print("[HEARTBEAT] PCI pulses detected: ");
      Serial.println(pulses);
    }
    else
    {
      Serial.println("[HEARTBEAT] PCI pulses detected: 0");
    }

    // Current system states
    bool heartbeatFault =
      (millis() - lastHeartbeat > HEARTBEAT_TIMEOUT);

    bool pir = pirState;

    bool tilt = !tiltState;

    // =================================================
    // STATUS PRIORITY
    // =================================================

    if (pir)
    {
      Serial.println("[STATUS] HIGH ALERT - MOTION");
    }
    else if (tilt)
    {
      Serial.println("[STATUS] WARNING - TAMPER");
    }
    else if (heartbeatFault)
    {
      Serial.println("[STATUS] FAULT - HEARTBEAT LOST");
    }
    else
    {
      Serial.println("[STATUS] SYSTEM HEALTHY");
    }

    Serial.println("----------------------------------------");
  }

  // ===================================================
  // HEARTBEAT FAILURE CHECK
  // ===================================================
  bool heartbeatFault =
    (millis() - lastHeartbeat > HEARTBEAT_TIMEOUT);

  // ===================================================
  // CURRENT SENSOR STATES
  // ===================================================
  bool pir = pirState;

  // Tilt is active LOW
  bool tilt = !tiltState;

  // ===================================================
  // PRIORITY 1 - HIGH ALERT / PIR
  // ===================================================
  if (pir)
  {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("!!! HIGH ALERT");

    lcd.setCursor(0, 1);
    lcd.print("MOTION DETECTED");
  }

  // ===================================================
  // PRIORITY 2 - TILT
  // ===================================================
  else if (tilt)
  {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, HIGH);

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("SYSTEM WARNING");

    lcd.setCursor(0, 1);
    lcd.print("TAMPER DETECTED");
  }

  // ===================================================
  // PRIORITY 3 - HEARTBEAT FAILURE
  // ===================================================
  else if (heartbeatFault)
  {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("SYSTEM FAULT");

    lcd.setCursor(0, 1);
    lcd.print("HEARTBEAT LOST");
  }

  // ===================================================
  // NORMAL OPERATION
  // ===================================================
  else
  {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("FRED SYSTEM");

    lcd.setCursor(0, 1);
    lcd.print("SYSTEM HEALTHY");
  }
}