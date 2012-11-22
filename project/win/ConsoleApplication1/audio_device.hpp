#pragma once

#include <string>
#include <functional>
#include <chrono>
//just to write the specification itself.

enum class byte_order{
	little,
	big
};
enum class audio_sample_data_type{
	float_32,
	int_32,
	int_16,
	uint_16,
	uint_32
};

struct audio_format{
	double sample_rate;
	std::chrono::nanoseconds buffer_period;
	audio_sample_data_type data_type;
	byte_order endianness;
};


struct audio_buffer{
	audio_format format;
	size_t size;
	void * data;
};


typedef std::function<void (audio_buffer&)> audio_callback;
class audio_device{
public:
	class id : std::wstring{
	public:
        id():std::wstring(){}
        id(wchar_t*pwstr):std::wstring(pwstr){}
        inline bool operator==(const id& rhs){ return compare(rhs)==0;}
        inline bool operator!=(const id& rhs){ return !operator==(rhs);}
        friend std::wostream& operator<<(std::wostream& out, const audio_device::id& id){
            return out<<id.data();	
        } 
	};
	typedef void * native_handle;//windows IMMDevice mac AudioDevice
	audio_device(native_handle handle = nullptr);
	const id get_id()const;
	bool is_valid() const {return id() != get_id();}
	native_handle native_handle() const;
	bool register_callback(audio_callback & callback);
	bool initialize(const audio_format & requested_format);
	audio_format current_format()const;
	bool is_playing()const;
	bool start();
	bool stop();
    bool is_supported(const audio_format& format);
};
