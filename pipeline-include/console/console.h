#pragma once
#include <memory>

#include <string>
#include <string_view>

enum class icon_type : unsigned short {
	CON_OK = 10,
	CON_STAGE = 6,
	CON_WARN = 14,
	CON_ERROR = 12,
	CON_DEBUG = 9,
	CON_TEXT = 7,
	CON_TEXT_DIM = 8,
	CON_UNKNOWN = 5
};

class c_console {
public:
	void               print(std::string_view input, ...) noexcept;
	void               spawn(std::string_view window) noexcept;
	void               kill() noexcept;
	[[nodiscard]] bool get_visible() const noexcept;

private:
	bool               is_open = false;
	std::string        title{};
	static std::string icon(icon_type icon) noexcept;
	void               set_color(icon_type col);
};


inline c_console console = {};