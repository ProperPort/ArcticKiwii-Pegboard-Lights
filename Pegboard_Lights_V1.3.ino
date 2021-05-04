#include <FastLED.h>
#include <OneButton.h>

// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN           33
#define LED_TYPE           WS2812B
#define COLOR_ORDER        GRB
#define NUM_LEDS           460
#define FRAMES_PER_SECOND  120
#define MASTER_BRIGHTNESS  235  // Set the master brigtness value [should be greater then min_brightness value].
uint8_t min_brightness =   40;

CRGB leds[NUM_LEDS];

int potBriVal;
int potHueVal;
int8_t brightness;
int potBri =             34;
int potHue =             35;
int buttonPin =          32;
int8_t patternRotate =   0;

OneButton btn = OneButton(buttonPin, true, true);

void setup() {
  delay(3000); // 3 second delay for recovery

  pinMode(potBri, INPUT);
  pinMode(potHue, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  btn.attachClick(nextPattern);
  btn.attachDoubleClick(doubleClick);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(50);
  Serial.begin(9600);
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {colorPulse, pulseWithGlitter, confetti, chasingLines, juggle, weave, vegasSign, vegasSign2, lightning};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {

  /*EVERY_N_MILLISECONDS(500) {
    Serial.print("Pot Brightness = ");
    Serial.println(potBriVal);
    Serial.print("Pot Hue = ");
    Serial.println(potHueVal);
    Serial.print("gCurrentPatternNumber = ");
    Serial.println(gCurrentPatternNumber);
  }*/


  checkKnobs();  // Call function to check knob positions.

  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // checks the button every loop
  btn.tick();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // do some periodic updates
  // EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  if (patternRotate == 1) {
    EVERY_N_SECONDS( 10 ) {
      nextPattern();  // change patterns periodically
    }
  }

}

void checkKnobs() {
  potBriVal = analogRead(potBri);  // Read potentiometer A (for brightness).
  //potValA = map(potValA, 1023, 0, 0, 1023);  // Reverse reading if potentiometer is wired backwards.
  brightness = map(potBriVal, 0, 4096, min_brightness, MASTER_BRIGHTNESS);  // map(value, fromLow, fromHigh, toLow, toHigh)

  potHueVal = analogRead(potHue);  // Read potentiometer B (for hue).
  if (potHueVal > 4050) {
    EVERY_N_MILLISECONDS( 20 ) {
      gHue++;
    }
  } else {
    gHue = map(potHueVal, 0, 4050, 0, 255);
  }


  FastLED.setBrightness(brightness);  // Set master brightness based on potentiometer position.

}

// #define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void doubleClick() {     // this function will be called when the button was pressed 2 times in a short timeframe
  patternRotate = (patternRotate + 1) % 2;
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern() {
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns); // add one to the current pattern number, and wrap around at the end
}

void colorPulse() {
  // randomly* pulsing strip with color determined by gHue/hue knob
  uint8_t dim = beatsin8(30, 64, 255, 0, 0);
  uint8_t dim2 = beatsin8(45, 64, 255, 0, 0);
  fill_solid(leds, NUM_LEDS, CHSV(gHue, 255, ((dim + dim2) / 2)));
}

void pulseWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  colorPulse();
  addGlitter(127);
}

void addGlitter( fract8 chanceOfGlitter) {
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() {    // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 5);
  int pos = random16(NUM_LEDS);
  int pos2 = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(51), 200, 255);
  leds[pos2] += CHSV( gHue + random8(51), 200, 255);
}

void chasingLines() {
  
  fadeToBlackBy(leds, NUM_LEDS, 30);    // fades the light tails to black, higher number = faster fade and shorter "tails"

  uint8_t bpm = 8;                      // how many times a dot goes from one end to the other
  uint8_t numDots = 10;                 // number of dots across the strip
  int j = 0;                            // a counter that keeps track of beat16's timebase offset
  
  for (uint8_t i = 0; i < numDots; i++) {                             // a for loop to assign the number of dots (numDots) a color and starting position
    
    uint16_t pos = map(beat16(bpm, j), 0, 65535, 0, NUM_LEDS - 1);    // maps beat16's 0 through 65535 to a number between 0 and NUM_LEDS - 1
    
    if (i % 2 == 0) {                                                 // translation --> if(i == even number) {
      leds[NUM_LEDS - pos] = CHSV(gHue + 127, 220, 255);              // sets a dot to a color "opposite" of gHue
    }
    else {                                                            // if "i" is an odd number,
      (leds[pos] = CHSV(gHue, 255, 255));                             // it becomes the standard gHue color
    }
    j += (60 / bpm * 1000 / numDots);    // j is the timebase offset of beat16, timebase is linked to beats like so (60 seconds/bpm *1000) = milliseconds for one beat 
  }                                      // we then divide the milliseconds of one beat by the number of dots to place dots evenly down the led strip
}

void juggle() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    // leds[i] = CHSV(gHue, 255, beat - gHue + (i * 10));
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void weave() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  for ( int i = 0; i < 7 ; i++) {
    leds[beatsin16( i + 5, 0, NUM_LEDS - 1 )] |= CHSV(gHue, 200, 255);
    gHue += 127;
  }
}

void vegasSign () {

  fadeToBlackBy(leds, NUM_LEDS, 255);

  uint8_t moveSpeed = 4;                                                   // how many times a pixel travels across the LED strip in a minute (higher # means faster)
  uint16_t beat = map(beat16(moveSpeed, 0), 0, 65535, 0, NUM_LEDS - 1);    // maps beat16's 0 through 65535 to a number between 0 and NUM_LEDS
  uint16_t posStart = 0;

  for (uint8_t segments = 0; segments < 10; segments++) {                                                    // this for loop assigns 30 pixels a color 10 times, to total 300

    for (uint8_t pos = 0; pos < 46; pos++) {                                                                  // this for loop assigns pixels 0-29 a color
      //if (pos >= 0 && pos <= 1 || pos >= 6 && pos <= 9 || pos >= 20 && pos <= 23 || pos >= 28 && pos <= 29) { // and is designed with exactly 30 pixels assigned exactly
      //  leds[(posStart + pos + beat) % NUM_LEDS] = CRGB::Black;                                               // 10 times across the strip of 300 pixels to fill the strip
      //}
      if (pos >= 6 && pos <= 11) {
        leds[(posStart + pos + beat) % NUM_LEDS] = CRGB::White;                                               // these numbers will have to be manually adjusted
      }                                                                                                       // to a factor of NUM_LEDS that equals a whole number
      if (pos >= 22 && pos <= 39) {
        leds[(posStart + pos + beat) % NUM_LEDS] = CHSV(gHue, 230, 255);                                      // example: NUM_LEDS = 300, 30 is a factor of 300 b/c 300/30=10
      }                                                                                                       // that's why the for loop above this one goes 10 times
    }
    posStart += 46;   // this moves the pixel assignments up 30 pixels so it doesn't just overwrite the first 30 pixels again
  }
}

void vegasSign2() {

  fadeToBlackBy(leds, NUM_LEDS, 127);

  uint16_t beat = beatsin16(2, 0, NUM_LEDS - 1, 0, 0);
  uint16_t posStart = 0;

  if (beat <= NUM_LEDS * 0.01) {
    beat = NUM_LEDS * 0.01;
  }

  if (beat >= NUM_LEDS * 0.99) {
    beat = NUM_LEDS * 0.99;
  }

  for (uint8_t segments = 0; segments < 10; segments++) {

    for (uint8_t pos = 0; pos < 43; pos++) {
      //if (pos >= 5 && pos <= 8) {                                           // white bars
      //  leds[(posStart + pos + beat) % NUM_LEDS] = CRGB::White;
      //}
      if (pos >= 5 && pos <= 17) {                                         // bar 1 color
        leds[(posStart + pos + beat) % NUM_LEDS] = CHSV(gHue, 255, 255);
      }
      if (pos >= 28 && pos <= 40) {                                         // bar 2 color
        leds[(posStart + pos + beat) % NUM_LEDS] = CHSV(gHue+127, 230, 255);
      }
    }
    posStart += 46;
  }
}

void lightning() {

  // The first "flash" in a bolt of lightning is the "leader." The leader
  // is usually duller and has a longer delay until the next flash. Subsequent
  // flashes, the "strokes," are brighter and happen at shorter intervals.
 
  FastLED.clear();
 
  int ledStart = random16(NUM_LEDS);           // Determine starting location of flash
  int ledLength = random16(NUM_LEDS-ledStart);    // Determine length of flash (not to go beyond NUM_LEDS-1)
 
  const uint8_t frequency = 40; // controls the interval between strikes
  const uint8_t flashes = 12;    // the upper limit of flashes per strike
 
  static uint8_t dimmer = 1;
  static uint8_t flashCount = flashes;
  static uint8_t flashCounter = 0;
 
  static uint32_t delayMillis = 0;
  static uint32_t delayStart = 0;
 
  static bool flashing = true;
 
  // bail, if we haven't waited long enough
  if (millis() < delayMillis + delayStart) {
    return;
  }
  flashing = !flashing;
 
  if (flashCounter >= flashCount) {    // if we've finished the current set of flashes, clear the display and wait a bit
    flashCounter = 0;
    //fill_solid(leds + ledStart, ledLength, CRGB::Black); // clear the display
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    delayMillis = random8(frequency) * 130;
    delayStart = millis();
    return;
  }
 
  if (flashCounter == 0) {
    dimmer = 5;                     // the brightness of the leader is scaled down by a factor of 5
  }
  else dimmer = random8(1, 3);      // return strokes are brighter than the leader
 
  if (flashing) {
    fill_solid(leds + ledStart, ledLength, CHSV(255, 0, 255 / dimmer));
    delayMillis = random8(4, 10);
    delayStart = millis();
  }
  
  else {
    //fill_solid(leds+ledStart, ledLength, CRGB::Black); // clear the display
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    delayMillis = 50 + random8(100);
    if (flashCount == 0) delayMillis += 150; // longer delay until next flash after the leader
  }
 
  flashCounter++;
  
}
