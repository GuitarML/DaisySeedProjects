# Amp Module

Amp modeler using neural networks and impulse responses to digitally recreate guitar amplifiers. Also includes a wet/dry mix control and tone control (low pass filter).
This is a Mono only effect on the Daisy Seed due to the heavy processing required, the left channel output is copied to the right channel, and right channel input is ignored.
This module is compatible with bkshepherd/DaisySeedProjects 125B pedal with OLED Screen.
<br><br>
The neural models use a GRU (gated recurrent unit) trained on audio from the amp (or pedal), to accurately model the distortion 
or saturation qualities of the circuitry / vacuum tubes. The IR uses direct convolution (time domain convolution) borrowed from the [Neural Amp Modeler Plugin](https://github.com/sdatkinson/NeuralAmpModelerPlugin).
The efficiency of the IR processing could be improved using FFT convolution (frequency domain convolution). The current IR implementation is limited to 
around 400 samples (8.3ms) when also using the neural model. 
<br><br>
This module uses [RTNeural](https://github.com/jatinchowdhury18/RTNeural) for processing the neural networks in real-time, and [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) for IR convolution.


## Controls

| Control | Description | Comment |
| --- | --- | --- |
| Ctrl 1 | Gain | Input gain 0 to 2x |
| Ctrl 2 | Mix | Left is full dry, right is full neural amp model |
| Ctrl 3 | Level | Output level |
| Ctrl 4 | Tone | Low pass filter, 400Hz to 20000Hz |
| Ctrl 5 | Model | Selects the neural amp model |
| Ctrl 6 | IR | Selects the impulse response |
| FS 1 |  |  |
| FS 2 | Bypass/Active | Bypass / effect engaged |
| LED 1 |  |  |
| LED 2 | Bypass/Active Indicator |Illuminated when effect is set to Active |
| Audio In 1 | Audio input | Mono in  |
| Audio Out 1 | Mix Out | Stereo or Mono out |

## Effect Parameters (accesible from the rotary encoder / OLED screen)

| Parameter | Description | Comment |
| --- | --- | --- |
| NeuralModel | Turns the Amp model on or off |  |
| IR On | Turns the IR on or off  |  |

## Build
Important: If you are building this module yourself, ensure the following items are set correctly in the DaisySeedProjects GuitarPedal framework.

1. In ```guitar_pedal.cpp```, set the block size to 48 or higher to allow for more intense processing of this effect module. ```hardware.SetAudioBlockSize(48)```  (normally set to 4)
2. In ```guitar_pedal.cpp```, you may need to remove some or all other effect modules besides ReverbDelayModule(). If you experience a frozen OLED screen, there is probably too much data on the stack in addition to this effect.