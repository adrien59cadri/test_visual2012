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

##Library features
This library should provide a very simple way to :

 - list audio devices
 - check format supported by this device
 - register a callback function for a specified device
 - create a callback (that will be called by the device)

Other ideas that I will not implement now are :

- a Push API to play a buffer on a device on a call decided by the application (the reason for this is that I would like to focus on professional audio applications needing low latency. Without closing that door this feature is not planned.
- the choice of exclusive mode or shared mode for the device connection (this could be implemented as a policy in the future, but for now I focus on a defaut policy : shared as the exlusive mode may fail more often and the big questions is what to do next, perhaps policies could be : *try_exclusive* - if fails try shared - , *exclusive_only*, *shared*)
- other stuffs I don't know yet


##Library interfaces

- ``audio_device_format`` : is a struct describing a format of audio stream for a device, this is used to check if a format is a suported by the device, or get the format supported by the device.

- ``audio_device_collection`` : this is a simple container of ``audio_device`` objects. The static function ``audio_system::get_all_devices`` returns an ``audio_device_collection`` of all audio devices in the system matching the requirements *(requirements are passed by argument)*

- ``audio_device`` : This is the main class of the library, it represents the interface of an audio device, like ``std::thread`` an instance does not represents directly an real audio device. For that you have to check that you have a correct id (you can retrive it by calling ``get_id()`` and check if the device is usable by calling ``is_valid()``) This class can be used to register a callback. This callback will be called once the device is valid an active ( call ``is_playing``to check if the device currenty active, you can ``start()`` and ``stop()`` the device). As a device does not always accept every format the function ``is_format_supported`` is used to check if the format is supported. The ``initialize`` funtion is used to init a device with a given format, it returns true is the format was accepted, false if not. The function ``current_format`` returns the format currently set for the device after the initialization.
- An `audio_callback` is a typedef of `std::function< void(audio_buffer&)>` 
- `audio_buffer` if a struct holding an `audio_format` object, a pointer to the data and the lenght of the buffer (frames number).

###A classic output device initialization could be  :
	
	void my_callback(audio_buffer &){
	//do some awesome processing
	}
	
	{
	auto devices = audio_system::get_all_devices(audio_direction::ouput);
	audio_format format;
	format.sample_rate = "44100."Hz;
	format.buffer_length = "10"ms;
	format.sample_format = audio_sample_type::float_32;
	format.bit_order = bit_order::little;
	for(device : devices){
		if(!device.initialize(format))
			format = device.gurrent_format();
		device.register_callback(my_callback);
		device.start();
	}