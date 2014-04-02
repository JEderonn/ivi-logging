#pragma once

#include "dlt_user.h"
#include "log-common.h"
#include "assert.h"
#include <map>

namespace pelagicore {

class DltContextClass : public DltContext {
public:
	DltContextClass(const char* contextID, const char* description) {
		m_contextID = contextID;
		m_description = description;
	}

	void registerDLTContext();

	~DltContextClass() {
		dlt_unregister_context(this);
	}

	bool isEnabled(LogLevel logLevel);

	static DltLogLevelType getDLTLogLevel(LogLevel level) {
		DltLogLevelType v = DLT_LOG_DEFAULT;
		switch (level) {
		case LogLevel::Debug : v = DLT_LOG_DEBUG; break;
		case LogLevel::Info : v = DLT_LOG_INFO; break;
		case LogLevel::Warning : v = DLT_LOG_WARN; break;
		case LogLevel::Error : v = DLT_LOG_ERROR; break;
		case LogLevel::Verbose : v = DLT_LOG_VERBOSE; break;
		case LogLevel::None : v = DLT_LOG_OFF; break;
			//		default : v = DLT_LOG_DEFAULT; break;
		}
		return v;
	}

	void registerContext();

private:
	const char* m_contextID;
	const char* m_description;
	bool m_isInitialized = false;
};

/**
 * C++ Wrapper for DLT.
 * Usage:
 \code

 void sendLogToDLT() {
 DltContextClass myDLTContext("TEST", "This is my context"); // Create a context object and register it
 DltLogData log(myDLTContext, DLT_LOG_INFO);
 log << "This is a string. Here's an int " << 6 << " and a float: " << 5.8F;	// The log object gets destroyed after this line, which actually sends the log to the DLT daemon
 }

 \endcode

 */
class DltLogData : public DltContextData {
public:
	DltLogData(DltContextClass& aContext, LogLevel logLevel, const char* fileName = NULL, int lineNumber = -1,
		   const char* prettyFunction = NULL) :
		context(aContext), m_fileName(fileName), m_lineNumber(lineNumber), m_prettyFunction(prettyFunction) {
		enabled = dlt_user_log_write_start( &context, this, aContext.getDLTLogLevel(logLevel) );
	}

	~DltLogData() {
		if (enabled) {
			if (m_fileName != NULL) dlt_user_log_write_utf8_string(this, m_fileName);
			if (m_lineNumber != -1) dlt_user_log_write_uint32(this, m_lineNumber);
			if (m_prettyFunction != NULL) dlt_user_log_write_utf8_string(this, m_prettyFunction);

			dlt_user_log_write_finish(this);
		}
	}

	bool isEnabled() {
		return enabled;
	}

	void writeFormatString(const char* v) {
		dlt_user_log_write_utf8_string(this, v);
	}

	template<typename ... Args>
	void writeFormatted(const char* format, Args ... args) {
		if (enabled) {
			char b[65536];

#pragma GCC diagnostic push
			// Make sure GCC does not complain about not being able to check the format string since it is no literal string
#pragma GCC diagnostic ignored "-Wformat-security"
			snprintf(b, sizeof(b), format, args ...);
#pragma GCC diagnostic pop

			dlt_user_log_write_utf8_string(this, b);
		}
	}

private:
	bool enabled;
	DltContextClass& context;

	const char* m_fileName;
	int m_lineNumber;
	const char* m_prettyFunction;

};

//inline void test_sendLogToDLT() {
//	DltContextClass myDLTContext("TEST", "This is my context"); // Create a context object and register it
//	DltLogData log(myDLTContext, LogLevel::Info);
//	log << "This is a string. Here's an int " << 6 << " and a float: " << 5.8F;     // The log object gets destroyed after this line, which actually sends the log to the DLT daemon
//}

inline bool DltContextClass::isEnabled(LogLevel logLevel) {
	// TODO : find a way to access the context's current logLevel without having to call a function
	DltContextData d;
	return dlt_user_log_write_start( this, &d, getDLTLogLevel(logLevel) );
}

inline DltLogData& operator<<(DltLogData& data, bool v) {
	dlt_user_log_write_bool(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, const char* v) {
	dlt_user_log_write_utf8_string(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, float f) {
	// we assume a float is 32 bits
	dlt_user_log_write_float32(&data, f);
	return data;
}


// TODO : strangely, it seems like none of the types defined in "stding.h" is equivalent to "long int" on a 32 bits platform
#if __WORDSIZE == 32
inline DltLogData& operator<<(DltLogData& data, long int v) {
	dlt_user_log_write_int32(&data, v);
	return data;
}
#endif

inline DltLogData& operator<<(DltLogData& data, double f) {
	// we assume a double is 64 bits
	dlt_user_log_write_float64(&data, f);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, uint64_t v) {
	dlt_user_log_write_uint64(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, int64_t v) {
	dlt_user_log_write_int64(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, uint32_t v) {
	dlt_user_log_write_uint32(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, int32_t v) {
	dlt_user_log_write_int32(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, uint16_t v) {
	dlt_user_log_write_uint16(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, int16_t v) {
	dlt_user_log_write_int16(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, uint8_t v) {
	dlt_user_log_write_uint8(&data, v);
	return data;
}

inline DltLogData& operator<<(DltLogData& data, int8_t v) {
	dlt_user_log_write_int8(&data, v);
	return data;
}

}
