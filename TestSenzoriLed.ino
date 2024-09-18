#include <FastLED.h>

#define NUM_FIELDS 9
#define NUM_LEDS_PER_STRIP 24
#define NUM_STRIPS 9
#define BRIGHTNESS 255
#define LED_TYPE WS2811
#define COLOR_ORDER RGB

int fields[NUM_FIELDS] = { A2, A3, A4, A5, A6, A7, A8, A9, A10 };
int initialValues[NUM_FIELDS];
int gameState[NUM_FIELDS];
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];
int activationCount = 0;
const int winCombinations[8][3] = {
  { 0, 1, 2 },
  { 3, 4, 5 },
  { 6, 7, 8 },
  { 0, 3, 6 },
  { 1, 4, 7 },
  { 2, 5, 8 },
  { 0, 4, 8 },
  { 2, 4, 6 }
};
void resetAllLeds(CRGB color = CRGB(0, 0, 0));

void setup() {
  delay(1500);

  FastLED.addLeds<WS2812B, 2>(leds[0], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 3>(leds[1], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 4>(leds[2], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 5>(leds[3], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 6>(leds[4], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 7>(leds[5], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 8>(leds[6], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 9>(leds[7], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<WS2812B, 10>(leds[8], NUM_LEDS_PER_STRIP);

  // treptanje led traka na početku
  for (int j = 0; j < NUM_STRIPS; j++) {
    setStripColor(j, CRGB(255, 0, 255));
    delay(100);
    setStripColor(j, CRGB(0, 0, 0));
    delay(100);
  }

  // Inicijalizacija input pinova
  for (int i = 0; i < NUM_FIELDS; i++) {
    pinMode(fields[i], INPUT);
    initialValues[i] = analogRead(fields[i]);
    gameState[i] = 0;  // Inicijaliziraj sva polja kao praznih
  }
  Serial.begin(9600);
  delay(500);
}

void loop() {
  int ledIndex = getMaxChangeIndex();

  if (ledIndex != -1) {

    int player = (activationCount % 2) + 1;  // promijeni igrača
    gameState[ledIndex] = player;            //postavi vrijednost u niz stanja igre ovisno o igraču

    CRGB color = (player == 1) ? CRGB(0, 255, 0) : CRGB(0, 0, 255);  // postavi boju ovisno o igraču

    // Osvijetli odabrano polje
    setStripColor(ledIndex, color);

    // Provjeri postoji li pobjeda
    int winIndex = checkWin();
    if (winIndex != -1) {
      // Treptanje pobjedničke kombinacije i resetiranje igre
      blinkWinningCombination(winIndex, color, 3, 500);
      delay(3000);
      resetGame();
    } else if (isBoardFull()) {
        delay(3000);
        resetGame();
    }

    activationCount++;
  }

  delay(2000);
}

int getMaxChangeIndex() {
  int maxChange = 0;
  int secondMaxChange = 0;
  int maxIndex = -1;

  for (int i = 0; i < NUM_FIELDS; i++) {
    if (gameState[i] != 0) continue;  // Preskoči već zauzeta polja

    int currentValue = analogRead(fields[i]);
    int change = currentValue - initialValues[i];

    if (change > maxChange) {
      secondMaxChange = maxChange;
      maxChange = change;
      maxIndex = i;
    }
  }

  if (maxChange > 90 && maxChange > (secondMaxChange + 30)) {
    return maxIndex;
  } else {
    return -1;
  }
}

// Funkcija za provjeru pobjede
int checkWin() {
  for (int i = 0; i < 8; i++) {
    int a = winCombinations[i][0];
    int b = winCombinations[i][1];
    int c = winCombinations[i][2];

    if (gameState[a] != 0 && gameState[a] == gameState[b] && gameState[b] == gameState[c]) {
      return i;  // Vrati indeks dobitne kombinacije
    }
  }
  return -1;
}

void blinkWinningCombination(int winIndex, CRGB color, int blinkCount, int blinkDelay) {
  bool inWinningCombination[NUM_STRIPS] = { false };

  for (int i = 0; i < 3; i++) {
    int ledIndex = winCombinations[winIndex][i];
    inWinningCombination[ledIndex] = true;
  }

  for (int strip = 0; strip < NUM_STRIPS; strip++) {
    if (!inWinningCombination[strip]) {
      setStripColor(strip, CRGB(0, 0, 0));
    }
  }

  for (int blink = 0; blink < blinkCount; blink++) {
    for (int i = 0; i < 3; i++) {
      int ledIndex = winCombinations[winIndex][i];
      setStripColor(ledIndex, color);
    }
    delay(blinkDelay);

    for (int i = 0; i < 3; i++) {
      int ledIndex = winCombinations[winIndex][i];
      setStripColor(ledIndex, CRGB(0, 0, 0));
    }
    delay(blinkDelay);
  }

  for (int i = 0; i < 3; i++) {
    int ledIndex = winCombinations[winIndex][i];
    setStripColor(ledIndex, color);
  }
}

void resetGame() {
  resetAllLeds();
  for (int i = 0; i < NUM_FIELDS; i++) {
    gameState[i] = 0;  // postavi sva polja kao prazna polja
  }
  activationCount = 0;
  delay(500);
}

bool isBoardFull() {
  for (int i = 0; i < NUM_FIELDS; i++) {
    if (gameState[i] == 0) {
      return false;
    }
  }
  return true;  // Ne postoji više slobodnih polja za igru
}

void setStripColor(int stripIndex, CRGB color) {
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[stripIndex][i] = color;
  }
  FastLED.show();
}

void resetAllLeds(CRGB color) {
  for (int strip = 0; strip < NUM_STRIPS; strip++) {
    setStripColor(strip, color);
  }
}
