// Pines
int led = 12;
int buzzer = 13;
int pot = A0;

// Estados del ciclo
int state = 0;
unsigned long previousMillis = 0;

// Proporciones fisiológicas del ciclo
float p_lub = 0.15;      
float p_pause = 0.10;    
float p_dub = 0.12;      
float p_diastole = 0.63; 

// Variables
unsigned long dur_lub, dur_pause, dur_dub, dur_diastole;

int lastPot = 0;   // para medir cambio brusco del potenciómetro
unsigned long lastStressMillis = 0;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1) LEER POTENCIÓMETRO Y CALCULAR BPM ---
  int lectura = analogRead(pot);
  int bpm = map(lectura, 0, 1023, 40, 150);

  int delta = abs(lectura - lastPot);  
  lastPot = lectura;

  // Un cambio brusco del potenciómetro simula estrés
  bool stressEvent = (delta > 80);  // lo ajustamos según tu potenciómetro


  // --- 2) DURACIÓN DEL CICLO ---
  unsigned long ciclo = 60000UL / bpm;

  dur_lub      = ciclo * p_lub;
  dur_pause    = ciclo * p_pause;
  dur_dub      = ciclo * p_dub;
  dur_diastole = ciclo * p_diastole;


  // ------------------------------
  // 3) GENERAR ECG FICTICIO + PICOS DE ESTRÉS
  // ------------------------------

  float t = millis() / 1000.0;
  float freq = bpm / 60.0;

  int base = 450;

  // onda cardiaca base
  float ecg = sin(2 * 3.14159 * freq * t) * 80;

  // pico QRS artificial
  if (fmod(t * freq, 1.0) < 0.05) {
    ecg += 160;
  }

  // --------------------------------
  //  PICOS DE ESTRÉS (simpatoadrenal)
  // --------------------------------
  int stressSpike = 0;

  // 1) si el potenciómetro cambia brusco, pico grande
  if (stressEvent) {
    stressSpike = random(120, 220);
    lastStressMillis = millis();
  }

  // 2) vibración simpática (ruido leve)
  if (millis() - lastStressMillis < 500) {
    stressSpike += random(-30, 30);  // temblor simpático leve
  }

  int FakeECG = base + ecg + stressSpike;


  // Línea de BPM (amplificada)
  int BPM_line = bpm * 3;


  // ------------------------------
  // 4) ENVIAR AL SERIAL PLOTTER
  // ------------------------------
  Serial.print("FakeECG:");
  Serial.print(FakeECG);

  Serial.print("  BPM_line:");
  Serial.println(BPM_line);


  // ------------------------------
  // 5) MÁQUINA DE ESTADOS CARDÍACOS
  // ------------------------------
  switch (state) {

    case 0:
      digitalWrite(led, HIGH);
      tone(buzzer, 500);
      if (currentMillis - previousMillis >= dur_lub) {
        previousMillis = currentMillis;
        state = 1;
      }
      break;

    case 1:
      digitalWrite(led, LOW);
      noTone(buzzer);
      if (currentMillis - previousMillis >= dur_pause) {
        previousMillis = currentMillis;
        state = 2;
      }
      break;

    case 2:
      digitalWrite(led, HIGH);
      tone(buzzer, 700);
      if (currentMillis - previousMillis >= dur_dub) {
        previousMillis = currentMillis;
        state = 3;
      }
      break;

    case 3:
      digitalWrite(led, LOW);
      noTone(buzzer);
      if (currentMillis - previousMillis >= dur_diastole) {
        previousMillis = currentMillis;
        state = 0;
      }
      break;
  }
}
