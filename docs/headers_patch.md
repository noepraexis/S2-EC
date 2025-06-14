Cabeçalhos incluindo hal.h usando caminho relativo, quebrando a inclusão do "Arduino.h" em main.cpp
```
kernel@m0nst3r:~/projects/github.com/noepraexis/S2-EC$ grep -iRn '#include "\.\./hal.h"' ~/.platformio/packages/
/home/kernel/.platformio/packages/framework-arduinoespressif32/tools/sdk/esp32s2/include/xtensa/esp32s2/include/xtensa/config/core.h:44:#include "../hal.h"
/home/kernel/.platformio/packages/framework-arduinoespressif32/tools/sdk/esp32s3/include/xtensa/esp32s3/include/xtensa/config/core.h:44:#include "../hal.h"
/home/kernel/.platformio/packages/framework-espidf/components/xtensa/esp32s2/include/xtensa/config/core.h:44:#include "../hal.h"
/home/kernel/.platformio/packages/framework-espidf/components/xtensa/esp32s3/include/xtensa/config/core.h:44:#include "../hal.h"
```
Aplicar patch usando a ferramenta: mnemon/utils/headers_patch