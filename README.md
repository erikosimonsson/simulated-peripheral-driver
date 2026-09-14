# simulated-peripheral-driver
C-based project focused on learning machine-adjacent programming.

The project consists of a simulated chip and its driver, such as a sensor chip connected to a processor.

The chip will have:
* Power states, off, starting, ready, and fault.
* A small set of registers for config, status, and measurements.
* Startup delay before accepting commands.
* Fixed-size buffer holding measurement bytes.
* Fault conditions such as an invalid command or a startup timeout.

Driver powers chip on, waits, configures, reads output into memory, handles errors, and shuts the chip down.
