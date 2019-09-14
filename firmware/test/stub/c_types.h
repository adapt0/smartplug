// override to avoid type conflicts between avr libraries and system

// framework-arduinoespressif8266/tools/sdk/include/c_types.h:36:29: error: conflicting declaration 'typedef long long unsigned int u_int64_t'
// /usr/include/x86_64-linux-gnu/sys/types.h:181:1: note: previous declaration as 'typedef long unsigned int u_int64_t'

