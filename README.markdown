#Audio device simple lib

##intro
This small project aims to be a very simple way to use audio devices on Mac OSX(using Core Audio), Winows (using  WASAPI) and linux (ALSA).
I don't want to support other APIs like DirectX or OSS because I just want to tale the best of each system for low latency audio. I don't want to be compatible with libraries like ASIO, PortAudio, JACK (...) because this would be oo much work, and obviously those systems are based on operating system's APIs and the latency would not be better.

##motivation
There is currently no easy way to connect to an audio driver in the standard library, and most of other solutions are frameworks doing much more than audio devices (like JUCE or Qt and JACK) or proprietary (ASIO) or not really C++ oriented (PortAudio).

*Disclaimer : There is a C++ binding for portaudio and this is a pretty stable library, but I don't find it is really C++ oriented, it is still a pretty big library and not so easy to use.*

I hope this might be usefull to me or anyone else in the future. And perhaps better coders could improve this or learn something from it. 

##design choices
I will use C++11 and try to keep the design as much object oriented as possible. The design should be very simple (few classes doing few stuff). The choice of the C++11 is mainly because I want to use < atomic >, < thread >, < chrono > libraries.

