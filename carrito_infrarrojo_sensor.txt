// Control de carrito con control infrarrojo y sensor de obstaculos más maniobra
#include <IRremote.hpp>

// Receptor IR
const int IR_PIN = 11;

// Pines L293D
const int ENA = 9;   // PWM izquierda
const int ENB = 10;  // PWM derecha
const int IN1 = 2;
const int IN2 = 3;
const int IN3 = 4;
const int IN4 = 5;

int speedVal = 200; // Velocidad inicial

// Sensor ultrasónico HC-SR04
const int TRIG_PIN = 6;
const int ECHO_PIN = 7;

// Códigos IR (reemplaza con los tuyos)
const unsigned long CODE_FORWARD      = 0xEE11BF00;
const unsigned long CODE_BACKWARD     = 0xEA15BF00;
const unsigned long CODE_LEFT_FORWARD = 0xEF10BF00;
const unsigned long CODE_RIGHT_FORWARD= 0xED12BF00;
const unsigned long CODE_LEFT_BACK    = 0xEB14BF00;
const unsigned long CODE_RIGHT_BACK   = 0xE916BF00;
const unsigned long CODE_STOP         = 0xF30CBF00;

// Máquina de estados para evitar obstáculos
enum ObstacleState {IDLE, STOPPED, BACKWARD, RIGHTBACKWARD, LEFTFORWARD, FORWARD, FINALSTOP};
ObstacleState obsState = IDLE;
unsigned long lastActionTime = 0;

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(9600);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
  stopMotors();
}

void loop() {
  // --- Control manual por IR ---
  if (IrReceiver.decode()) {
    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("Código: 0x"); Serial.println(code, HEX);

    if (code == CODE_FORWARD) forward();
    else if (code == CODE_BACKWARD) backward();
    else if (code == CODE_LEFT_FORWARD) leftForward();
    else if (code == CODE_RIGHT_FORWARD) rightForward();
    else if (code == CODE_LEFT_BACK) leftBackward();
    else if (code == CODE_RIGHT_BACK) rightBackward();
    else if (code == CODE_STOP) stopMotors();

    IrReceiver.resume();
  }

  // --- Control automático por sensor ---
  long distance = getDistance();
  if (distance > 0 && distance <= 15 && obsState == IDLE) {
    // Detecta obstáculo y arranca secuencia
    stopMotors();
    obsState = STOPPED;
    lastActionTime = millis();
    Serial.println("Obstáculo detectado! Iniciando maniobra...");
  }

  // Máquina de estados con temporización
  switch (obsState) {
    case STOPPED:
      if (millis() - lastActionTime >= 2000) {
        backward();
        obsState = BACKWARD;
        lastActionTime = millis();
      }
      break;

    case BACKWARD:
      if (millis() - lastActionTime >= 2000) {
        rightBackward();
        obsState = RIGHTBACKWARD;
        lastActionTime = millis();
      }
      break;

    case RIGHTBACKWARD:
      if (millis() - lastActionTime >= 2000) {
        leftForward();
        obsState = LEFTFORWARD;
        lastActionTime = millis();
      }
      break;

    case LEFTFORWARD:
      if (millis() - lastActionTime >= 2000) {
        forward();
        obsState = FORWARD;
        lastActionTime = millis();
      }
      break;

    case FORWARD:
      if (millis() - lastActionTime >= 2000) {
        stopMotors();
        obsState = FINALSTOP;
        Serial.println("Maniobra completada, listo para continuar.");
      }
      break;

    case FINALSTOP:
      // Espera hasta que se libere el obstáculo
      if (distance > 15) {
        obsState = IDLE;
      }
      break;

    case IDLE:
    default:
      // Nada especial
      break;
  }
}

// --- Función para medir distancia ---
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 20000); // timeout 20ms
  long distance = duration * 0.034 / 2; // cm
  return distance;
}

// --- Funciones de movimiento ---
void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  applySpeed();
  Serial.println("Adelante");
}

void backward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  applySpeed();
  Serial.println("Atras");
}

void leftForward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, speedVal);
  Serial.println("Adelante/Izquierda");
}

void rightForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, speedVal);
  analogWrite(ENB, 0);
  Serial.println("Adelante/Derecha");
}

void leftBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, 0);
  analogWrite(ENB, speedVal);
  Serial.println("Atras/Izquierda");
}

void rightBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, speedVal);
  analogWrite(ENB, 0);
  Serial.println("Atras/Derecha");
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  Serial.println("Parar");
}

void applySpeed() {
  analogWrite(ENA, speedVal);
  analogWrite(ENB, speedVal);
}
