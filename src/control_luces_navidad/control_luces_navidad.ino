/*
 * ============================================================
 *  CONTROL DE LUCES DE NAVIDAD - 3 COLORES / 3 MODOS
 * ============================================================
 * Proyecto: arduino-luces-navidad-3-modos
 * Autor: Seykarim
 *
 * Controla 3 LEDs de colores en 3 modos de animación distintos,
 * seleccionables con un solo pulsador:
 *
 *   MODO 0 - ALEATORIO: enciende un color al azar cada cierto
 *            intervalo de tiempo.
 *   MODO 1 - SECUENCIA CONTINUA: enciende los colores en orden
 *            (1 -> 2 -> 3 -> 1 -> 2 -> 3...).
 *   MODO 2 - IDA Y VUELTA: recorre los colores hacia adelante y
 *            hacia atrás (1 -> 2 -> 3 -> 2 -> 1 -> 2 -> 3...).
 *
 * La velocidad de todas las animaciones se controla en vivo con
 * un potenciómetro. El programa NO usa delay() para las
 * animaciones, para que el botón de cambio de modo responda
 * siempre al instante, sin quedar "atrapado" esperando un delay.
 * ============================================================
 */

// ---------- CONFIGURACIÓN DE PINES ----------
const int PIN_LED_ROJO   = 9;
const int PIN_LED_VERDE  = 10;
const int PIN_LED_AZUL   = 11;
const int PIN_BOTON      = 2;   // Pin con interrupción (2 o 3 en Arduino Uno)
const int PIN_POTENCIOMETRO = A0;

// ---------- VARIABLES DE MODO ----------
// volatile porque esta variable se modifica dentro de una interrupción
volatile int modoActual = 0;   // 0 = aleatorio, 1 = secuencia, 2 = ida y vuelta
const int TOTAL_MODOS = 3;

// ---------- ANTI-REBOTE (DEBOUNCE) DEL BOTÓN ----------
volatile unsigned long ultimoCambioModo = 0;
const unsigned long TIEMPO_DEBOUNCE = 250; // milisegundos mínimos entre pulsaciones válidas

// ---------- VARIABLES PARA LAS ANIMACIONES ----------
unsigned long ultimoPaso = 0;   // Momento (ms) del último cambio de LED
int pasoActual = 0;             // Índice del LED activo en la secuencia
int direccion = 1;               // 1 = avanzando, -1 = retrocediendo (para modo ida y vuelta)

// Arreglo con los pines de los 3 colores, para recorrerlos fácilmente en bucle
const int pinesColores[3] = {PIN_LED_ROJO, PIN_LED_VERDE, PIN_LED_AZUL};

void setup() {
  Serial.begin(9600);

  // Configuramos los 3 LEDs como salidas
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AZUL, OUTPUT);

  // El botón usa la resistencia pull-up interna del Arduino:
  // así no necesitamos una resistencia física para el pulsador.
  // En reposo el pin lee HIGH; al presionar, lee LOW.
  pinMode(PIN_BOTON, INPUT_PULLUP);

  // Configuramos una interrupción: cada vez que el pin del botón
  // pasa de HIGH a LOW (FALLING = flanco de bajada), se ejecuta
  // automáticamente la función cambiarModo(), sin importar qué
  // esté haciendo el resto del programa en ese momento.
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), cambiarModo, FALLING);

  apagarTodosLosLeds();

  Serial.println("Sistema de luces de navidad iniciado.");
  Serial.println("Modo actual: 0 (Aleatorio)");
}

void loop() {
  // Leemos el potenciómetro (0-1023) y lo convertimos a un rango de
  // velocidad útil para la animación (entre 50 ms y 800 ms por paso).
  // map() hace esa conversión de escala automáticamente.
  int valorPotenciometro = analogRead(PIN_POTENCIOMETRO);
  int velocidad = map(valorPotenciometro, 0, 1023, 800, 50);

  unsigned long tiempoActual = millis();

  // Solo avanzamos la animación si ya pasó el tiempo de "velocidad"
  // desde el último paso. Esto reemplaza el uso de delay(), que
  // bloquearía el programa y haría que el botón no respondiera.
  if (tiempoActual - ultimoPaso >= (unsigned long)velocidad) {
    ultimoPaso = tiempoActual;

    switch (modoActual) {
      case 0:
        animacionAleatoria();
        break;
      case 1:
        animacionSecuenciaContinua();
        break;
      case 2:
        animacionIdaYVuelta();
        break;
    }
  }
}

// ---------- FUNCIÓN DE INTERRUPCIÓN: CAMBIO DE MODO ----------
void cambiarModo() {
  unsigned long ahora = millis();

  // Solo aceptamos el cambio de modo si ha pasado suficiente tiempo
  // desde el último cambio. Esto filtra el "rebote" mecánico del
  // botón (pequeñas vibraciones eléctricas al presionar/soltar que
  // el Arduino podría interpretar como varias pulsaciones seguidas).
  if (ahora - ultimoCambioModo > TIEMPO_DEBOUNCE) {
    modoActual = (modoActual + 1) % TOTAL_MODOS; // Avanza al siguiente modo y vuelve a 0 tras el último
    ultimoCambioModo = ahora;

    // Reiniciamos las variables de animación para que el nuevo modo
    // siempre empiece limpio, sin arrastrar el estado del anterior.
    pasoActual = 0;
    direccion = 1;
    apagarTodosLosLeds();
  }
}

// ---------- MODO 0: ALEATORIO ----------
void animacionAleatoria() {
  apagarTodosLosLeds();
  int colorAlAzar = random(0, 3); // Elige un número entre 0 y 2 (los 3 colores)
  digitalWrite(pinesColores[colorAlAzar], HIGH);
}

// ---------- MODO 1: SECUENCIA CONTINUA (1 -> 2 -> 3 -> 1...) ----------
void animacionSecuenciaContinua() {
  apagarTodosLosLeds();
  digitalWrite(pinesColores[pasoActual], HIGH);

  pasoActual++;
  if (pasoActual >= 3) {
    pasoActual = 0; // Al llegar al final, reiniciamos al primer color
  }
}

// ---------- MODO 2: IDA Y VUELTA (1 -> 2 -> 3 -> 2 -> 1...) ----------
void animacionIdaYVuelta() {
  apagarTodosLosLeds();
  digitalWrite(pinesColores[pasoActual], HIGH);

  pasoActual += direccion;

  // Si llegamos al último o al primer color, invertimos la dirección
  // en vez de reiniciar, generando el efecto de "rebote".
  if (pasoActual >= 2 || pasoActual <= 0) {
    direccion *= -1;
  }
}

// ---------- FUNCIÓN AUXILIAR ----------
void apagarTodosLosLeds() {
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_AZUL, LOW);
}
