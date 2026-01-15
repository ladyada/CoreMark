// CoreMark Benchmark for Arduino compatible boards
//   original CoreMark code: https://github.com/eembc/coremark

#include <stdarg.h>

// A way to call the C-only coremark function from Arduino's C++ environment
extern "C" int coremark_main(void);



// Dual-core synchronization for RP2040/RP2350
volatile bool core1_go = false;
volatile bool core1_done = false;
volatile void *core1_func_arg = NULL;
typedef void* (*iterate_func_t)(void*);
volatile iterate_func_t core1_func = NULL;

// Start work on core 1 (non-blocking)
extern "C" void start_on_core1(void* (*func)(void*), void* arg) {
  core1_func = func;
  core1_func_arg = arg;
  core1_done = false;
  core1_go = true;
}

// Wait for core 1 to complete
extern "C" void wait_for_core1(void) {
  while (!core1_done) {
    // spin
  }
}


void setup()
{
	Serial.begin(9600);
	while (!Serial) ; // wait for Arduino Serial Monitor
	delay(500);

	Serial.println("CoreMark Performance Benchmark");
	Serial.println();
	Serial.println("CoreMark measures how quickly your processor can manage linked");
	Serial.println("lists, compute matrix multiply, and execute state machine code.");
	Serial.println();
	Serial.println("Iterations/Sec is the main benchmark result, higher numbers are better");
	Serial.println("Running.... (usually requires 12 to 20 seconds)");
	Serial.println();


	delay(250);
	coremark_main(); // Run the benchmark  :-)
}

void loop()
{
}

void setup1() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop1() {
  if (core1_go && !core1_done) {
    if (core1_func != NULL) {
      digitalWrite(LED_BUILTIN, HIGH);
      core1_func((void*)core1_func_arg);
      core1_func = NULL;
    }
    core1_done = true;
    core1_go = false;
  }
}

// CoreMark calls this function to print results.
extern "C" int ee_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	for (; *format; format++) {
		if (*format == '%') {
			bool islong = false;
			format++;
			if (*format == '%') { Serial.print(*format); continue; }
			if (*format == '-') format++; // ignore size
			while (*format >= '0' && *format <= '9') format++; // ignore size
			if (*format == 'l') { islong = true; format++; }
			if (*format == '\0') break;
			if (*format == 's') {
				Serial.print((char *)va_arg(args, int));
			} else if (*format == 'f') {
				Serial.print(va_arg(args, double));
			} else if (*format == 'd') {
				if (islong) Serial.print(va_arg(args, long));
				else Serial.print(va_arg(args, int));
			} else if (*format == 'u') {
				if (islong) Serial.print(va_arg(args, unsigned long));
				else Serial.print(va_arg(args, unsigned int));
			} else if (*format == 'x') {
				if (islong) Serial.print(va_arg(args, unsigned long), HEX);
				else Serial.print(va_arg(args, unsigned int), HEX);
			} else if (*format == 'c' ) {
				Serial.print(va_arg(args, int));
			}
		} else {
			if (*format == '\n') Serial.print('\r');
			Serial.print(*format);
		}
	}
	va_end(args);
	return 1;
}

// CoreMark calls this function to measure elapsed time
extern "C" uint32_t Arduino_millis(void)
{
	return millis();
} 
