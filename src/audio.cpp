enum SoundType {
	MUSIC,
	SFX,
	SOUND_TYPE_LENGTH
};

struct Sound {
	Sound() {};
	Sound(String path);
	// A buffer id.
	ALuint buffer;
	// The important header info.
	ALenum format;
	ALuint samples_per_second;
};

struct sound_info {
	// If it is being used.
	bool used = false;
	// If the source should survive when it's paused.
	bool is_persistent = false;
	// Which source this info bellongs to.
	ALuint source;
	// The current buffer.
	ALuint buffer;
	// The type of the sound
	SoundType type;
	// What the gain is at.
	float target_gain = 1;
	// What the pitch is at.
	float target_pitch = 1;
	// How much the gain should change per second.
	float gain_change = 1;
	// How much the pitch should change per second.
	float pitch_change = 1;
	// If position should be used.
	bool positional = false;
	// The position.
	Vec2 position = Vec2();
	// If the source should loop.
	bool loop = false;
};

#define CHECK_OPENAL_ERROR check_openal_error(__LINE__);

void check_openal_error(int line) {
	auto error = alGetError();
	if (error != AL_NO_ERROR) {
		return;
	}
	if (error == AL_INVALID_NAME) {
		printf("[OpenAL] Invalid name\n");
	} else if (error == AL_INVALID_ENUM) {
		printf("[OpenAL] Invalid enum\n");
	} else if (error == AL_INVALID_VALUE) {
		printf("[OpenAL] Invalid value\n");
	} else if (error == AL_INVALID_OPERATION) {
		printf("[OpenAL] Invalid operation\n");
	} else if (error == AL_OUT_OF_MEMORY) {
		printf("[OpenAL] Out of memory\n");
	} else {
		printf("[OpenAL] Unknown error\n");
	}

	printf("[OpenAL] Called from line %d\n", line);
}

float global_gain[SOUND_TYPE_LENGTH];

Stack<ALuint> free_sources;
HashMap<ALuint, sound_info> sources;

ALCdevice*  device;
ALCcontext* context;

// If the sound is playing or not.
bool is_playing(ALuint source) {
	int state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

bool is_stopped(ALuint source) {
	int state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	return state == AL_STOPPED || state == AL_INITIAL;
}

bool is_paused(ALuint source) {
	int state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	return state == AL_PAUSED;
}

// Initalizes OpenAL
void init_audio() {
	for (int i = 0; i < SOUND_TYPE_LENGTH; i++) {
		global_gain[i] = 1.0f;
	}

	device = alcOpenDevice(NULL);
	if (!device) {
		printf("[Audio.cpp] No audio output device.\n");
	}	

	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context)) {
		printf("[Audio.cpp] Cannot create contex.\n");
	}

	// Log that shit!
	printf("[OpenAL] Version: %s\n", alGetString(AL_VERSION));
	printf("[OpenAL] Vendor: %s\n", alGetString(AL_VENDOR));
	printf("[OpenAL] Renderer: %s\n", alGetString(AL_RENDERER));
}

void destroy_audio() {
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void set_sound_info(sound_info const& info) {
	assert(info.type >= 0);
	assert(info.type < SOUND_TYPE_LENGTH);

	ALuint source = info.source;

	alSourcei(source, AL_BUFFER, info.buffer);

	alSourcef(source, AL_PITCH, info.target_pitch);

	alSourcef(source, AL_GAIN, info.target_gain * global_gain[info.type]);

	if (info.positional) {
		alSourcei(source, AL_SOURCE_RELATIVE, false);
		alSource3f(source, AL_POSITION, info.position.x, info.position.y, 0);
	} else {
		alSourcei(source, AL_SOURCE_RELATIVE, true);
		alSource3f(source, AL_POSITION, 0, 0, 0);
	}

	alSource3f(source, AL_VELOCITY, 0, 0, 0);

	alSourcei(source, AL_LOOPING, info.loop);
}

// Updates the audio engine.
void update_audio() {
	for (auto& s : sources) {
		if (!s.second.used) continue;
		if (!s.second.is_persistent && is_stopped(s.first)) {
			// It's unused.
			s.second.used = false;
			alSourcei(s.first, AL_BUFFER, (ALint) NULL);
			free_sources.push(s.first);
		} else {
			set_sound_info(s.second);
		}
	}
}

// Sets the global gain for that kind of sound.
void set_global_gain(SoundType type, float gain) {
	assert(type < 0);
	assert(type > SOUND_TYPE_LENGTH);

	global_gain[type] = gain;
}

sound_info find_free_source() {
	sound_info info;
	if (free_sources.empty()) {
		// We have to make a new source.
		alGenSources((ALuint)1, &info.source);
	} else {
		// We can use a premade one.
		info.source = free_sources.top();
		free_sources.pop();
		info = sources[info.source];
	}

	info.is_persistent = false;
	info.used = true;
	sources[info.source] = info;
	return info;
}

#define PITCH_DELTA 0.10f
#define GAIN_DELTA  0.10f

// Randomizes the sounds, just a bit.
void perturb_sound(float* gain, float* pitch) {
	*gain  += *gain  * ((float) rand() / RAND_MAX - 0.5f) * GAIN_DELTA; 
	*pitch += *pitch * ((float) rand() / RAND_MAX - 0.5f) * PITCH_DELTA; 
}

// Plays a sound but ignores the position.
ALuint play_sound(Sound s, SoundType type, 
		bool perturb = true, float gain = 1.0f, 
		float pitch = 1.0f, bool loop = false) {
	sound_info info = find_free_source();

	if (perturb) {
		perturb_sound(&gain, &pitch);
	}

	info.type = type;
	info.buffer = s.buffer;
	info.target_gain = gain;
	info.target_pitch = pitch;
	info.positional = false;
	info.loop = loop;

	set_sound_info(info);

	// Now we can play!
	alSourcePlay(info.source);

	// Add it to the list of sources
	sources[info.source] = info;
	return info.source;
}

// Plays a sound at the specified position. 
ALuint play_sound_at(Sound s, SoundType type, 
		bool perturb = true, float gain = 1.0f, 
		float pitch = 1.0f, bool loop = false, 
		Vec2 pos = {0, 0}) {
	sound_info info = find_free_source();

	if (perturb) {
		perturb_sound(&gain, &pitch);
	}

	info.buffer = s.buffer;
	info.type = type;
	info.target_gain = gain;
	info.target_pitch = pitch;
	info.position = pos;
	info.positional = true;
	info.loop = loop;

	set_sound_info(info);

	// Now we can play!
	alSourcePlay(info.source);

	return info.source;
}

void persistant_sound(ALuint source) {
	sources[source].is_persistent = true;
}

void unused_sound(ALuint source) {
	sources[source].is_persistent = false;
}

// Stops the source from continuing to play.
void stop_sound(ALuint source) {
	alSourceStop(source);
}

// Pauses the source.
void pause_sound(ALuint source) {
	alSourcePause(source);
}

// Sets the buffer.
void set_sound_buffer(ALuint source, Sound s) {
	sources[source].buffer = s.buffer;
}

// Sets if the source should loop.
void set_sound_loop(ALuint source, bool loop) {
	sources[source].loop = loop;
}

// Sets the gain of a source.
void set_sound_gain(ALuint source, float new_gain) {
	sources[source].target_gain = new_gain;
}

// Sets the pitch of a source.
void set_sound_pitch(ALuint source, float new_pitch) {
	sources[source].target_pitch = new_pitch;
}

// Spoiler! This sets the position of the sound.
void set_sound_position(ALuint source, Vec2 pos) {
	sources[source].position = pos;
}

// Does what it says on the tin.
float get_gain(ALuint source) {
	float gain;
	alGetSourcef(source, AL_GAIN, &gain);
	return gain;
}

float get_pitch(ALuint source) {
	float pitch;
	alGetSourcef(source, AL_PITCH, &pitch);
	return pitch;
}

struct WAV_Header {
	uint8_t     riff[4];            // RIFF Header Magic header
	uint32_t    chunk_size;         // RIFF Chunk Size
	uint8_t     wave[4];            // WAVE Header
	// "fmt" sub-chunk
	uint8_t     fmt[4];             // FMT header
	uint32_t    subchunk_size;      // Size of the fmt chunk
	uint16_t    audio_format;       // Audio format
	uint16_t    num_of_chan;        // Number of channels 1=Mono 2=Stereo
	uint32_t    samples_per_second; // Sampling Frequency in Hz
	uint32_t    bytes_per_sec;      // bytes per second
	uint16_t    block_align;        // 2=16-bit mono, 4=16-bit stereo
	uint16_t    bits_per_sample;    // Number of bits per sample
};  

struct WAV_Chunk {
	uint8_t  id[4];
	uint32_t size;
};

Sound new_sound(const Array<char>& data, ALuint format, ALuint samples_per_second) {
	Sound s;
	s.format = format;
	s.samples_per_second = samples_per_second;
	alGenBuffers(1, &s.buffer);
	alBufferData(s.buffer, format, &data[0], (ALsizei) data.size(), samples_per_second);

	return s;
}

void load_sound(Sound& s, const Array<char>& data, ALuint format, ALuint samples_per_second) {
	s.format = format;
	s.samples_per_second = samples_per_second;
	alBufferData(s.buffer, format, &data[0], (ALsizei) data.size(), samples_per_second);
}

bool load_wav(const String& path, Array<char>& data, ALuint& format, ALuint& samples_per_second) {
	WAV_Header  header;
	FILE* file = fopen(path.c_str(), "r");

	if (!file) {
		printf("[Audio.cpp] Error loading file '%s'.\n", path.c_str());
		return false;
	}

	if (!fread(&header, sizeof(WAV_Header), 1, file)) {
		printf("[Audio.cpp] Failed to load wav header '%s'.\n", path.c_str());
		return false;
	}

	// Hack thogether the format!
	format = 0x1100;
	format |= (16 == header.bits_per_sample) * 0b01;
	format |= (2 == header.num_of_chan)      * 0b10;

	// Samples per second.
	samples_per_second = header.samples_per_second;

	// Find the data chunk.
	char chunk_name[] = "data\0";

	WAV_Chunk chunk;
	chunk.size = 0;
	bool is_data_chunk = false;
	do {
		if (fseek(file, chunk.size, SEEK_CUR)) {
			printf("[Audio.cpp] Cannot find data tag. ('%s')\n", path.c_str());
			return false;
		}
		fread(&chunk, sizeof(WAV_Chunk), 1, file);
	} while (strcmp(chunk_name, (const char*) chunk.id) != 0);

	data.resize(chunk.size);
	fread(&data[0], sizeof(char), data.size(), file);

	return true;
}

bool update_sound(Sound& s, const String& path) {
	Array<char> data;
	ALuint format, samples_per_second;
	if (load_wav(path, data, format, samples_per_second)) {
		load_sound(s, data, format, samples_per_second);
		return true;
	}
	return false;
}

Sound new_sound(const String& path) {
	Array<char> data;
	ALuint format, samples_per_second;
	if (load_wav(path, data, format, samples_per_second))
		return new_sound(data, format, samples_per_second);

	Sound s;
	s.buffer = -1;

	return s;
}
