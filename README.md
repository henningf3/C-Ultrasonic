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

In diesen 3 Methoden wird die lineare Interpolation durchgeführt. Die Werte für die Interpolation der Lautstärke wurden während des Versuches mit einer Experimentalschleife ermittelt. Dazu wurde die Lautstärke (der Duty-Cycle) sukzessive inkrementiert, bis kein Unterschied mehr zu hören war.  
Dies ist die einzige Stelle, die nicht auf Anhieb funktioniert hat und während des Termins angepasst werden musste.

Die 3 kHz werden durch den `WRAP`-Wert eingestellt: $$\mathrm{WRAP} = \frac{125 \mathrm{MHz}\ (clock)}{3 \mathrm{kHz}} = 41666$$
`distance_mm` wird über die Schallgeschwindigkeit ermittelt, wobei `duration` die Zeit in Mikrosekunden ist, die laut Sensor für den Weg zur Wand **und** zurück benötigt wurde. 
$$\mathrm{distance_mm}=\frac{{\mathrm{duration}}_{\mathrm{\mu s}} \cdot 0.340 \frac{\mathrm{mm}}{\mathrm{\mu s}}}{2}$$