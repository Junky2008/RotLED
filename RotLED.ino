/*
 * Author: Floris Venhuizen
 * ES31 - Vervangende opdracht - RotLED
 *
 * Sources:
 *  http://www.arduino.cc/
 *  http://forum.arduino.cc/index.php?topic=285336.5;wap2
*/
//define pins
#define CLOCK_PIN (0x01 << 5) // PORTD P5, PIN D15
#define DATA_PIN (0x01 << 4) // PORTD P4, PIN D14
#define LED_PIN (0x01 << 25) // PORTB P25, PIN D2

#define PIOD_IRQ (0x01 << 14) // PORTD Interupt

volatile uint32_t *REG_ICER = (volatile uint32_t *)0xE000E180;
volatile uint32_t *REG_ICPR = (volatile uint32_t *)0xE000E280;
volatile uint32_t *REG_ISER = (volatile uint32_t *)0xE000E100;

//difference counting up
volatile int diff_up = 0;
//difference counting down
volatile int diff_down = 0;
//current position
volatile int pos = 0;

//interupt handler
void PIOD_Handler(void) {
  //get interupt; don't check, will only take time and at this point, only one pin has been enabled for this interrupt.
  uint32_t isr = PIOD->PIO_ISR;
  //get clock and data pin values
  int clock_data = (PIOD->PIO_PDSR & CLOCK_PIN);
  int data = (PIOD->PIO_PDSR & DATA_PIN);
  //only count if clock_data is HIGH
  if (clock_data && data)
    diff_up++;
  if (clock_data && !data)
    diff_down++;
}

void setup() {
  //Set DATA_PIN and CLOCK_PIN as input
  //disable interrupts
  REG_PIOD_IDR = DATA_PIN;
  REG_PIOD_IDR = CLOCK_PIN;
  //disable internal pull-up
  REG_PIOD_PUDR = DATA_PIN;
  REG_PIOD_PUDR = CLOCK_PIN;
  //enable filtering
  REG_PIOD_IFER = DATA_PIN;
  REG_PIOD_IFER = CLOCK_PIN;
  //set slow clock devider for debounce
  REG_PIOD_SCDR = 0x04;
  //enable input debouncing
  REG_PIOD_DIFSR = DATA_PIN;
  REG_PIOD_DIFSR = CLOCK_PIN;
  //set pins as input
  REG_PIOD_ODR = DATA_PIN;
  REG_PIOD_ODR = CLOCK_PIN;
  //enable pins
  REG_PIOD_PER = DATA_PIN;
  REG_PIOD_PER = CLOCK_PIN;

  //Set LED_PIN as output
  //disable interrupts
  REG_PIOB_IDR = LED_PIN;
  //disable internal pull-up
  REG_PIOB_PUDR = LED_PIN;
  //disable multi-drive
  REG_PIOB_MDDR = LED_PIN;
  //set pin as output
  REG_PIOB_OER = LED_PIN;
  //enable pin
  REG_PIOB_PER = LED_PIN;

  //enable and set interrupt priority on port D
  REG_PMC_PCDR0 = PIOD_IRQ; //ID_PIOD
  *REG_ICER = PIOD_IRQ; //DisableIRQ
  *REG_ICPR = PIOD_IRQ; /* Clear pending interrupt */
  *REG_ISER = PIOD_IRQ; //DisableIRQ
  REG_PMC_PCER0 = PIOD_IRQ; //PIOD_IRQn

  //set interrupt mode CHANGE
  PIOD->PIO_AIMDR = CLOCK_PIN;
  //enable interrupt
  PIOD->PIO_IER = CLOCK_PIN;

  //start serial for debugging data
  Serial.begin(115200);
  Serial.println(pos);
}

void loop() {
  //if difference is up, set pin, count position up, send to debug
  if (diff_up)
  {
    PIOB->PIO_SODR = LED_PIN;
    pos++;
    diff_up--;
    Serial.println(pos);
  }
  //if difference is down, reset pin, count position down, send to debug
  if (diff_down)
  {
    PIOB->PIO_CODR = LED_PIN;
    pos--;
    diff_down--;
    Serial.println(pos);
  }
}

