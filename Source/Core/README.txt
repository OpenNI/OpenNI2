// Processes
* Initialization
- Find available devices
	Go over all shared object files in the single repository directory for devices (environment variable in Win, fixed in others)
	Try to load the shared objects
	If successful - create an internal Device object. The context will hold a list of pointers to the device objects. These will be the DeviceHandle, available outside to the C API.

* Interesting streams
- out of the available devices, find the streams we want

* Mainloop
- event driven - register to specific streams - get callback with framebuffer (separate for each stream)
- polling - WaitAny - get the streamHandle, check its type (wait returns with a single stream, or a list of them?)

* Depth format
- 16bit in mm - up to 65m
- 16bit in mm/1000 - up to 6.5m
-- scale

* Device API

* Platform types
	XnUInt32 and friends. Needed in the API of OpenNI, and also used in PSCommon. Where do they get defined, if PSCommon is _not_ API of OpenNI.
	CALLBACK_TYPE, PLATFORM

* PSCommon
- all kinds of data structures, and OS-specific infrastructures (threads, strings, memory, ...). Not API of OpenNI! Used in its implementation, though.

* Convenience Layer
- Shortcuts for reasonable usage. Novice users are expected to use this layer. Expert users will use complex APIs in OpenNI

* Wrappers
- C++ for sure
- Java, C#, others - may leave for community?

* Platforms
- Windows - 32/64
- Linux - 32/64 (Ubuntu 12.4?)
- MAC?
- ARM?