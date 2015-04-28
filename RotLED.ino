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

volatile uint32_t *R_PMC_PCDR0 = (volatile uint32_t *)0x400E0614; //Pin Clock Disable register 0
volatile uint32_t *R_PMC_PCER0 = (volatile uint32_t *)0x400E0610; //Pin Clock Enable register 0

volatile uint32_t *R_NVIC_ICER = (volatile uint32_t *)0xE000E180; //Interrupt Clear Enable register
volatile uint32_t *R_NVIC_ICPR = (volatile uint32_t *)0xE000E280; //Interrupt Priority register
volatile uint32_t *R_NVIC_ISER = (volatile uint32_t *)0xE000E100; //Interrupt Set Enable register

volatile uint32_t *R_PIOB_IDR = (volatile uint32_t *)0x400E1044; //Interrupt Disable register
volatile uint32_t *R_PIOB_PUDR = (volatile uint32_t *)0x400E1060; //Pull up Disable register
volatile uint32_t *R_PIOB_MDDR = (volatile uint32_t *)0x400E1054; //Multi-driver Disable register
volatile uint32_t *R_PIOB_OER = (volatile uint32_t *)0x400E1010; //Output Enable register
volatile uint32_t *R_PIOB_PER = (volatile uint32_t *)0x400E1000; //Pin Enable register
volatile uint32_t *R_PIOB_SODR = (volatile uint32_t *)0x400E1030; //Set Output Data register
volatile uint32_t *R_PIOB_CODR = (volatile uint32_t *)0x400E1034; //Clear Output Data register

volatile uint32_t *R_PIOD_IDR = (volatile uint32_t *)0x400E1444; //Interrupt Disable register
volatile uint32_t *R_PIOD_PUDR = (volatile uint32_t *)0x400E1460; //Pull up Disable register
volatile uint32_t *R_PIOD_IFER = (volatile uint32_t *)0x400E1420; //Interrupt Filter Enable register
volatile uint32_t *R_PIOD_SCDR = (volatile uint32_t *)0x400E148C; //Slow Clock Devider Register
volatile uint32_t *R_PIOD_DIFSR = (volatile uint32_t *)0x400E1484; //Debouncing Input Filter Select register
volatile uint32_t *R_PIOD_ODR = (volatile uint32_t *)0x400E1414; //Output Disable register
volatile uint32_t *R_PIOD_PER = (volatile uint32_t *)0x400E1400; //Pin Enable register
volatile uint32_t *R_PIOD_AIMDR = (volatile uint32_t *)0x400E14B4; //Additional Interrupt Modes Disable register
volatile uint32_t *R_PIOD_IER = (volatile uint32_t *)0x400E1440; //Interrupt Enable register
volatile uint32_t *R_PIOD_ISR = (volatile uint32_t *)0x400E144C; //Interrupt Status register
volatile uint32_t *R_PIOD_PDSR = (volatile uint32_t *)0x400E143C; //Pin Data Status register

//difference counting up
volatile int diff_up = 0;
//difference counting down
volatile int diff_down = 0;
//current position
volatile int pos = 0;

//interupt handler
void PIOD_Handler(void) {
  //get interupt; don't check, will only take time and at this point, only one pin has been enabled for this interrupt.
  uint32_t isr = *R_PIOD_ISR;
  //get clock and data pin values
  int clock_data = (*R_PIOD_PDSR & CLOCK_PIN);
  int data = (*R_PIOD_PDSR & DATA_PIN);
  //only count if clock_data is HIGH
  if (clock_data && data)
    diff_up++;
  if (clock_data && !data)
    diff_down++;
}

void setup() {
  //Set DATA_PIN and CLOCK_PIN as input
  //disable interrupts
  *R_PIOD_IDR = DATA_PIN;
  *R_PIOD_IDR = CLOCK_PIN;
  //disable internal pull-up
  *R_PIOD_PUDR = DATA_PIN;
  *R_PIOD_PUDR = CLOCK_PIN;
  //enable filtering
  *R_PIOD_IFER = DATA_PIN;
  *R_PIOD_IFER = CLOCK_PIN;
  //set slow clock devider for debounce
  *R_PIOD_SCDR = 0x04;
  //enable input debouncing
  *R_PIOD_DIFSR = DATA_PIN;
  *R_PIOD_DIFSR = CLOCK_PIN;
  //set pins as input
  *R_PIOD_ODR = DATA_PIN;
  *R_PIOD_ODR = CLOCK_PIN;
  //enable pins
  *R_PIOD_PER = DATA_PIN;
  *R_PIOD_PER = CLOCK_PIN;

  //Set LED_PIN as output
  //disable interrupts
  *R_PIOB_IDR = LED_PIN;
  //disable internal pull-up
  *R_PIOB_PUDR = LED_PIN;
  //disable multi-drive
  *R_PIOB_MDDR = LED_PIN;
  //set pin as output
  *R_PIOB_OER = LED_PIN;
  //enable pin
  *R_PIOB_PER = LED_PIN;

  //enable and set interrupt priority on port D
  *R_PMC_PCDR0 = PIOD_IRQ; //ID_PIOD
  *R_NVIC_ICER = PIOD_IRQ; //DisableIRQ
  *R_NVIC_ICPR = PIOD_IRQ; /* Clear pending interrupt */
  *R_NVIC_ISER = PIOD_IRQ; //DisableIRQ
  *R_PMC_PCER0 = PIOD_IRQ; //PIOD_IRQn

  //set interrupt mode CHANGE
  *R_PIOD_AIMDR = CLOCK_PIN;
  //enable interrupt
  *R_PIOD_IER = CLOCK_PIN;

  //start serial for debugging data
  Serial.begin(115200);
  Serial.println(pos);
}

void loop() {
  //if difference is up, set pin, count position up, send to debug
  if (diff_up)
  {
    *R_PIOB_SODR = LED_PIN;
    pos++;
    diff_up--;
    Serial.println(pos);
  }
  //if difference is down, reset pin, count position down, send to debug
  if (diff_down)
  {
    *R_PIOB_CODR = LED_PIN;
    pos--;
    diff_down--;
    Serial.println(pos);
  }
}

