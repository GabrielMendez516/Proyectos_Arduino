#include <Wire.h> 
#include <LiquidCrystal_I2C.h> // Incluye la librería para el LCD I2C

// Pines HC-SR04 
const int trigPin = 9; 
const int echoPin = 10;

// Parámetros de calibración (AJUSTADOS PARA TANQUE DE 1000 LITROS)
// 1. Altura total del tanque: 1.95 mts = 195.0 cm
const float tanqueAltura_cm = 195.0; 
// 2. Área de la base: Pi * (40 cm)^2 ≈ 5026.55 cm²
const float tanqueAreaBase_cm2 = 5026.55; 

// Variables globales para la medición
float distancia_cm = 0.0; 
float nivel_pct = 0.0;
float volumen_ml = 0.0; // Se calcula en mililitros, luego se convierte a litros

// Inicialización del objeto LCD: Dirección I2C (0x27, 0x3F, etc.), Columnas (16), Filas (2)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- Funciones de Medición del HC-SR04 ---

long readMicrosecondsHC() { 
    // Genera pulso de 10μs en Trig 
    digitalWrite(trigPin, LOW); 
    delayMicroseconds(2); 
    digitalWrite(trigPin, HIGH); 
    delayMicroseconds(10); 
    digitalWrite(trigPin, LOW); 
    return pulseIn(echoPin, HIGH);
}

float readDistance_cm() { 
    long duration = readMicrosecondsHC(); 
    // Velocidad del sonido ~343 m/s = 0.0343 cm/μs 
    float distance = (duration * 0.0343) / 2.0; 
    
    // Filtrado simple: El sensor no debe medir más allá de la altura total del tanque.
    if (distance < 2.0) distance = 2.0; 
    if (distance > tanqueAltura_cm) distance = tanqueAltura_cm; 
    
    return distance;
}

// --- Setup ---

void setup() { 
    Serial.begin(9600); 
    pinMode(trigPin, OUTPUT); 
    pinMode(echoPin, INPUT); 
    
    // Iniciar pantalla I2C 
    lcd.init(); 
    lcd.backlight(); 
    lcd.print("Capacidad: 1000 L");
    delay(2000);
    lcd.clear();
}

// --- Loop ---

void loop() { 
    distancia_cm = readDistance_cm(); 
    
    // 1. Calcular Altura útil (Agua)
    // Altura útil (agua) = altura total del tanque - distancia al agua 
    float altura_util_cm = tanqueAltura_cm - distancia_cm; 
    
    // Asegura que el nivel esté entre 0 y la altura total
    if (altura_util_cm < 0) altura_util_cm = 0;
    if (altura_util_cm > tanqueAltura_cm) altura_util_cm = tanqueAltura_cm; 
    
    // 2. Calcular Nivel Porcentual
    nivel_pct = (altura_util_cm / tanqueAltura_cm) * 100.0;

    // 3. Calcular Volumen en Mililitros (1 cm³ = 1 ml)
    volumen_ml = altura_util_cm * tanqueAreaBase_cm2;
    
    // Convertir volumen a Litros para facilitar la lectura
    float volumen_L = volumen_ml / 1000.0;

    // --- Salida por Serial --- 
    Serial.print("Distancia: ");
    Serial.print(distancia_cm, 1);
    Serial.print(" cm, Nivel: ");
    Serial.print(nivel_pct, 1);
    Serial.print(" %, Volumen: ");
    Serial.print(volumen_L, 1); // Mostramos Litros con un decimal
    Serial.println(" L");
    
    // --- Alertas simples (15% y 95%) --- 
    bool alerta = false;
    if (nivel_pct < 15.0) { 
        Serial.println("ALERTA: Nivel bajo (Menos de 150 L)"); 
        alerta = true;
    } else if (nivel_pct > 95.0) { 
        Serial.println("ALERTA: Nivel alto (Mas de 950 L)"); 
        alerta = true;
    } 
    
    // --- Visualización en LCD I2C ---
    lcd.clear();
    
    // Línea 1: Nivel en Litros (L)
    lcd.setCursor(0, 0);
    lcd.print("VOL: ");
    lcd.print(volumen_L, 1); // Mostramos Litros con un decimal
    lcd.print(" L");
    
    // Línea 2: Nivel en Porcentaje o Alerta
    lcd.setCursor(0, 1);
    if (alerta) {
      lcd.print("  *** ALERTA *** ");
    } else {
      lcd.print("Nivel: ");
      lcd.print((int)nivel_pct); // Muestra el nivel como entero
      lcd.print("% (");
      lcd.print(altura_util_cm, 0); // Altura en cm
      lcd.print("cm)");
    }
    
    delay(3000); // Muestra las lecturas cada 3 segundos
}