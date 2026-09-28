import schemdraw
import schemdraw.elements as elm

with schemdraw.Drawing() as d:
    d += elm.Resistor().label('100K')
    d += elm.Capacitor().down().label('0.1uF')
    d.save('test.svg')
