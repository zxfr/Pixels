Pixels
======
Graphics library for TTF LCD devices.

Pixels is an open source TFT LCD graphics library for Arduino (and easily portable to other platforms). 
In addition to usual graphics features, it implements anti-aliased graphics primitives, supports converted 
TTF fonts, and provides an access to hardware scrolling features.

The library structure separates hardware specific layer (LCD controller driver, communication protocol) and 
general graphics layer. The separation simplifies a porting of the library for new devices.

The library is natively supported by Pixelmeister GUI modeling tool. Pixelmeister emulates the library API on 
desktop computers, converts TTF fonts and images to Pixels-friendly format and exports developed programs 
prototypes (sketches) to be compiled for a target device. 

For more info see http://pd4ml.com/pixelmeister/pixels.htm


