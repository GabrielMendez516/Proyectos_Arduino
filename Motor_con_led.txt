// Motor DC con L293D: cambio de sentido, parada y 3 LEDs de estado

const int ENA = 9;   // Pin PWM para velocidad (Enable)
const int IN1 = 6;   // Pin de dirección 1
const int IN2 = 7;   // Pin de dirección 2

// Pulsadores (Configuración Pull-Down en hardware)
const int pulsadorAlternar = 2; // Pulsador para alternar Sentido 1 <-> Sentido 2
const int pulsadorParar = 3;    // Pulsador para Parada de Emergencia

// LEDs de Retroalimentación
const int ledGiro1 = 10;   // LED Verde: Sentido 1
const int ledParado = 11;  // LED Rojo: Motor Parado
const int ledGiro2 = 12;   // LED Amarillo: Sentido 2

// Constantes de Estado
const int PARADO = 0;
const int SENTIDO_1 = 1;
const int SENTIDO_2 = 2;

// Constante de Velocidad y debounce
const int velocidad = 200;
const int DEBOUNCE_DELAY = 100; // 100 milisegundos para eliminar rebotes

// --- 2. Variables Globales de Control ---

// Variable que almacena el estado actual del motor (inicialmente PARADO)
int estadoMotor = PARADO; 

// Variables para el control de rebote del pulsador
long ultimoTiempoAlternar = 0;
long ultimoTiempoParar = 0;

// --- 3. SETUP ---

void setup() {
  // Pines de salida
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ledGiro1, OUTPUT);
  pinMode(ledParado, OUTPUT);
  pinMode(ledGiro2, OUTPUT);

  // Pines de entrada
  pinMode(pulsadorAlternar, INPUT);
  pinMode(pulsadorParar, INPUT);

  Serial.begin(9600);
  Serial.println("Control de Motor Listo. Esperando entrada...");

  // Estado inicial: Motor Detenido
  actualizarEstado(PARADO);
}

// --- 4. LOOP (Bucle Principal) ---

void loop() {
  // --- Lógica del Pulsador ALTERNAR (Pin 2) ---
  if (digitalRead(pulsadorAlternar) == HIGH) {
    // Control de rebote (debounce): solo ejecuta si ha pasado suficiente tiempo
    if (millis() - ultimoTiempoAlternar > DEBOUNCE_DELAY) {
      
      if (estadoMotor == PARADO) {
        // Estaba parado -> Inicia en Sentido 1
        actualizarEstado(SENTIDO_1);
      } else if (estadoMotor == SENTIDO_1) {
        // Estaba en Sentido 1 -> Cambia a Sentido 2
        actualizarEstado(SENTIDO_2);
      } else if (estadoMotor == SENTIDO_2) {
        // Estaba en Sentido 2 -> Cambia a Sentido 1
        actualizarEstado(SENTIDO_1);
      }
      
      ultimoTiempoAlternar = millis(); // Actualiza el tiempo de la última pulsación
    }
  }

  // --- Lógica del Pulsador PARAR (Pin 3) ---
  if (digitalRead(pulsadorParar) == HIGH) {
     if (millis() - ultimoTiempoParar > DEBOUNCE_DELAY) {
        // Siempre detiene el motor
        actualizarEstado(PARADO);
        ultimoTiempoParar = millis(); // Actualiza el tiempo de la última pulsación
     }
  }

  // Nota: El motor siempre mantiene su estado hasta que se presiona un pulsador.
}

// --- 5. Función Unificada para Actualizar Estado, LEDs y Motor ---

void actualizarEstado(int nuevoEstado) {
  // Solo actualiza si el estado realmente ha cambiado
  if (nuevoEstado == estadoMotor && nuevoEstado != PARADO) {
    return;
  }
  
  estadoMotor = nuevoEstado; // Almacena el nuevo estado

  // Apagar todos los LEDs primero
  digitalWrite(ledGiro1, LOW);
  digitalWrite(ledGiro2, LOW);
  digitalWrite(ledParado, LOW);

  // Lógica del Motor (IN1, IN2)
  if (nuevoEstado == SENTIDO_1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, velocidad);
    digitalWrite(ledGiro1, HIGH); // Enciende LED Sentido 1
    Serial.println("ESTADO: Sentido 1 (Verde)");
    
  } else if (nuevoEstado == SENTIDO_2) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, velocidad);
    digitalWrite(ledGiro2, HIGH); // Enciende LED Sentido 2
    Serial.println("ESTADO: Sentido 2 (Amarillo)");

  } else { // PARADO (incluye cualquier otro valor por seguridad)
    // Parada: Deshabilitar el driver y poner entradas a LOW (frenado rápido)
    analogWrite(ENA, 0); // Deshabilita el driver
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(ledParado, HIGH); // Enciende LED Parado
    Serial.println("ESTADO: Detenido (Rojo)");

  }
}