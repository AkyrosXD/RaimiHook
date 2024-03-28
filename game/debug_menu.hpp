#pragma once

#include "ngl.hpp"
#include "input_mgr.hpp"
#include "xenon_input_mgr.hpp"
#include "windows_app.hpp"
#include <vector>

#pragma warning (disable: 4996)  /* _CRT_SECURE_NO_WARNINGS */

#define DEBUG_MENU_FONT_SCALE 0.85f
#define DEBUG_MENU_MAX_HEIGHT 480.0f
#define DEBUG_MENU_ENTRY_RIGHT_PADDING 15.0f
#define DEBUG_MENU_ENTRY_LEFT_PADDING 5.0f
#define DEBUG_MENU_ENTRY_BOTTOM_PADDING 5.0f
#define DEBUG_MENU_TITLE_TOP_PADDING 5.0f
#define DEBUG_MENU_ENTRY_MAX_CHAR 256
#define DEBUG_MENU_ENTRY_MAX_DISPLAY_TEXT_LENGTH 128
#define DEBUG_MENU_UP_ARROW " ^ ^ ^"
#define DEBUG_MENU_DOWN_ARROW " v v v"

#define RGBA_TO_INT(r, g, b, a) ((a << 24) | (r << 16) | (g << 8) | b)

/// <summary>
/// The type of an entry
/// </summary>
enum class E_NGLMENU_ENTRY_TYPE
{
	/// <summary>
	/// Button entry
	/// </summary>
	BUTTON,

	/// <summary>
	/// Toggle entry
	/// </summary>
	BOOLEAN,

	/// <summary>
	/// Sub-menu button entry
	/// </summary>
	MENU,

	/// <summary>
	/// Multiple selection entry
	/// </summary>
	SELECT,

	/// <summary>
	/// Selection element of a multiple selection entry
	/// </summary>
	SELECT_OPTION,

	/// <summary>
	/// Text entry or a selection for a multiple selection entry
	/// </summary>
	TEXT
};

/// <summary>
/// A struct for the debug menu callbacks
/// </summary>
struct debug_menu_entry_callback
{
	/// <summary>
	/// The address of the function
	/// </summary>
	void* callback_ptr;

	/// <summary>
	/// A pointer to the argument of the function. Set it to NULL if the function has no arguments
	/// </summary>
	void* callback_arg;
};

/// <summary>
/// Debug Menu Entry class
/// </summary>
class debug_menu_entry
{
public:
	/// <summary>
	/// The debug menu instance of the entry
	/// </summary>
	class debug_menu* parent;

	/// <summary>
	/// The type of the entry
	/// </summary>
	E_NGLMENU_ENTRY_TYPE type;

	/// <summary>
	/// The text of the entry
	/// </summary>
	char text[DEBUG_MENU_ENTRY_MAX_CHAR];

	/// <summary>
	/// If the current entry is a sub-menu, this will be the entries of the sub-menu.
	/// If the current entry is a multiple-selection entry, this will be the available items to select
	/// </summary>
	class debug_menu_entry_list* sublist;

	/// <summary>
	/// The height of the text
	/// </summary>
	int text_height;

	/// <summary>
	/// The width of the text
	/// </summary>
	int text_width;

	/// <summary>
	/// The Y position of the entry
	/// </summary>
	float pos_y;

	/// <summary>
	/// The Y position of where on the screen the entry should be displayed
	/// </summary>
	float display_pos_y;

	/// <summary>
	/// If the entry is visible (used for the scrolling functionality)
	/// </summary>
	bool is_visible;

	union
	{
		/// <summary>
		/// Pointer to the current value
		/// </summary>
		void* value_ptr;

		/// <summary>
		/// Pointer to the callback function
		/// </summary>
		void* handle_ptr;
	};

	union
	{
		/// <summary>
		/// Pointer to the function argument
		/// </summary>
		void* callback_arg;

		/// <summary>
		/// If the entry is a button for a sub-menu, this returns the name of the menu
		/// </summary>
		const char* menu_name;
	};

	/// <summary>
	/// Adds a sub-entry to the the entry. Only if the current entry is of type MENU or SELECT
	/// </summary>
	/// <param name="type">Type of the entry</param>
	/// <param name="text">Text of the entry</param>
	/// <param name="value_or_handle_ptr">A pointer to the corresponding value or the pointer of the callback function</param>
	/// <param name="callback_arg">A pointer to the argument of the callback. Set this to NULL if this parameter is not needed</param>
	/// <returns>A pointer to the added entry</returns>
	debug_menu_entry* add_sub_entry(E_NGLMENU_ENTRY_TYPE type, const char* text, void* value_or_handle_ptr, void* callback_arg);
};

class debug_menu_entry_list
{
public:
	/// <summary>
	/// Current list of entries
	/// </summary>
	std::vector<debug_menu_entry*> entries;

	/// <summary>
	/// Previous list
	/// </summary>
	debug_menu_entry_list* previous;

	/// <summary>
	/// The index in the list of the selected entry
	/// </summary>
	size_t selected_entry_index;

	/// <summary>
	/// The title of the current menu
	/// </summary>
	const char* menu_title;

	/// <summary>
	/// The position of the vertical scroll
	/// </summary>
	float scroll_pos_y;

	/// <summary>
	/// Create a new entry list instance
	/// </summary>
	/// <param name="menu_title">The title of the list</param>
	debug_menu_entry_list(const char* menu_title);

	/// <summary>
	/// Adds an entry to the list
	/// </summary>
	/// <param name="type">Type of the entry</param>
	/// <param name="text">Text of the entry</param>
	/// <param name="value_or_handle_ptr">A pointer to the corresponding value or the pointer of the callback function</param>
	/// <param name="callback_arg">A pointer to the argument of the callback. Set this to NULL if this parameter is not needed</param>
	/// <returns>A pointer to the added entry</returns>
	debug_menu_entry* add_entry(class debug_menu* parent, E_NGLMENU_ENTRY_TYPE type, const char* text, void* value_or_handle_ptr, void* callback_arg);

	/// <summary>
	/// Gets the first entry
	/// </summary>
	/// <returns>A pointer of the first entry</returns>
	debug_menu_entry* first() const;

	/// <summary>
	/// Gets the last entry
	/// </summary>
	/// <returns>A pointer of the last entry</returns>
	debug_menu_entry* last() const;

	/// <summary>
	/// Gets the selected entry
	/// </summary>
	/// <returns>A pointer of the selected entry</returns>
	debug_menu_entry* get_selected() const;

	/// <summary>
	/// Gets the entry at the given index
	/// </summary>
	/// <param name="index">The index of the entry</param>
	/// <returns>A pointer of the entry</returns>
	debug_menu_entry* entry_at(const size_t& index) const;

	/// <summary>
	/// Gets the size of the list
	/// </summary>
	/// <returns>The number of entries in the list</returns>
	size_t size() const;

	/// <summary>
	/// Checks if the list has any entries or not
	/// </summary>
	/// <returns>True if there are no entries in the list. Otherwise false</returns>
	bool empty() const;
};

struct debug_menu_draw_entry_parameters
{
	size_t current_entry_index;
	const debug_menu_entry* last_entry;
	float y_start;
	float current_entry_pos_y;
	float entry_max_pos_y;
	float menu_max_height;
	float current_width;
	char* const current_entry_display_text;
};

class debug_menu
{
private:
	bool m_is_open;
	void* on_show;
	void* on_hide;
	const char* m_name;
	float m_window_pos_x;
	float m_window_pos_y;
	float m_width;
	float m_default_width;
	float m_height;
	float m_default_height;
	int m_down_arrow_width;
	int m_down_arrow_height;
	int m_up_arrow_width;
	int m_up_arrow_height;
	nglWindow m_ngl_box_data;
	debug_menu_entry_list* m_default_entry_list;
	debug_menu_entry_list* m_current_entry_list;
	debug_menu_entry_callback m_current_callback;
	bool get_on_hide();
	bool get_on_show();
	void go_back();
	void change_entry_list(debug_menu_entry_list* list);
	float get_menu_max_height() const;
	float get_max_pos_y_for_entries() const;
	float get_min_pos_y_for_entries() const;
	bool can_scoll_down() const;
	bool can_scoll_up() const;
	float get_down_scroll_indicator_pos_y() const;
	float get_up_scroll_indicator_pos_y() const;
	debug_menu_entry* get_selected_entry() const;
	debug_menu_entry* get_last_entry() const;
	void draw_entry(debug_menu_draw_entry_parameters& parameters);
	void adjust_height();
	void draw_scroll_indicators();

public:
	/// <summary>
	/// Creates a new debug menu instance
	/// </summary>
	/// <param name="title">The main title of the menu</param>
	/// <param name="x">X position on the screen</param>
	/// <param name="y">Y position on the screen</param>
	debug_menu(const char* title, float x, float y);

	/// <summary>
	/// Checks if the menu is open
	/// </summary>
	/// <returns>True if the menu is open. Otherwise false</returns>
	bool is_open() const;

	/// <summary>
	/// Shows/opens the menu
	/// </summary>
	void show();

	/// <summary>
	/// Hides/closes the menu
	/// </summary>
	void hide();

	/// <summary>
	/// Sets the function callback for when the menu is opened
	/// </summary>
	/// <param name="callback">A pointer to the function</param>
	void set_on_show(void* callback);

	/// <summary>
	/// Sets the function callback for when the menu is closed
	/// </summary>
	/// <param name="callback">A pointer to the function</param>
	void set_on_hide(void* callback);

	/// <summary>
	/// Adds an entry to the menu
	/// </summary>
	/// <param name="type">Type of the entry</param>
	/// <param name="text">Text of the entry</param>
	/// <param name="value_or_handle_ptr">A pointer to the corresponding value or the pointer of the callback function</param>
	/// <param name="callback_arg">A pointer to the argument of the callback. Set this to NULL if this parameter is not needed</param>
	/// <returns>A pointer to the added entry</returns>
	debug_menu_entry* add_entry(E_NGLMENU_ENTRY_TYPE type, const char* text, void* value_or_handle_ptr, void* callback_arg);

	/// <summary>
	/// Executes the current callback of the menu
	/// </summary>
	void execute_current_callback();

	/// <summary>
	/// Removes the current callback of the menu
	/// </summary>
	void reset_current_callback();

	/// <summary>
	/// Draws the menu
	/// </summary>
	void draw();

	/// <summary>
	/// Handles the input of the user
	/// </summary>
	void handle_input();
};