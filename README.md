# Dokumentation MRT Versuch 2
Ziel dieses Versuches war es, eine Abstandskontrolle mithilfe eines RCWL-1601 Sensors
zu realisieren. In Abhängigkeit der Entfernung sollten mit einem LK-Buzzer linear interpolierte 
Signale (die lineare Interpolation bezieht sich auf Lautstärke und Tonfolge) ausgegeben werden.
## Umsetzung
Dafür wird zunächst `init()` aufgerufen, in der die GPIO-Pins (für die Kommunikation mit dem Sensor) sowie der PWM-Pin initialisiert werden.
`slice_num` und `chan` (aus der `main`) werden hier gesetzt. Diese Werte sind zwar durch die
`PWM_PIN`-Definition bereits eindeutig festgelegt, aber für die Wiederverwendbarkeit (mit anderen Pins) des Codes werden sie hier nochmal explizit abgefragt.
Anschließend wird die Hauptschleife gestartet. Hier wird ständig der Sensor abgefragt (`distance_mm = readSensor()`).
In Abhängigkeit von `distance_mm` wird die Ausgabe angepasst:
- Mit `setVolume(distance_mm, &pwm_compare)` wird ein neuer Duty-Cycle (`pwm_compare`) und damit die Lautstärke des Buzzers gesetzt. 
- Durch `determine_high_time(distance_mm)` wird die Länge des Tons bestimmt.
- Äquivalent dazu wird die Pause zwischen den Tönen errechnet (`determine_low_time(distance_mm)`).

In diesen 3 Funktionen wird die lineare Interpolation durchgeführt. 
Die 3 kHz werden durch den `WRAP`-Wert eingestellt: $$\mathrm{WRAP} = \frac{125 \mathrm{MHz}\ (clock)}{3 \mathrm{kHz}} = 41666$$
`distance_mm` wird über die Schallgeschwindigkeit ermittelt, wobei `duration` die Zeit in Mikrosekunden ist, die laut Sensor für den Weg zur Wand **und** zurück benötigt wurde. 
$$\mathrm{distance_mm}=\frac{{\mathrm{duration}}_{\mathrm{\mu s}} \cdot 0.340 \frac{\mathrm{mm}}{\mathrm{\mu s}}}{2}$$

## Probleme
Die Werte für die Interpolation der Lautstärke wurden während des Versuches mit einer Experimentalschleife (siehe auskommentierter Experimentalteil) ermittelt. Dazu wurde die Lautstärke (der Duty-Cycle) sukzessiv inkrementiert, bis kein Unterschied mehr zu hören war.  
Allerdings war die vorbereitete Funktion falsch herum (wenn man laut hören sollte, hörte man leise und umgekehrt), weshalb diese noch angepasst werden musste.

Ein anderes Problem war, dass das Programm nicht schnell genug reagierte. In der ursprünglichen Implementierung wurde die aktuelle Entfernung nach jeder Pause determiniert. Die Lösung erfolgte dadurch, dass die Entfernung nun nach jedem Ton und jeder Pause ermittelt wird.
