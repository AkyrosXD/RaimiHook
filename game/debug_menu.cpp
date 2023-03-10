#include "debug_menu.hpp"

bool debug_menu::get_on_hide()
{
	typedef bool(*hide_t)();
	hide_t phide = (hide_t)this->on_hide;
	if (phide != nullptr)
	{
		return phide();
	}
	return true;
}

bool debug_menu::get_on_show()
{
	typedef bool(*show_t)();
	show_t pshow = (show_t)this->on_show;
	if (pshow != nullptr)
	{
		return pshow();
	}
	return true;
}

void debug_menu::go_back()
{
	if (this->m_current_entry_list->previous != nullptr)
	{
		this->m_current_entry_list = this->m_current_entry_list->previous;
		this->m_width = (float)this->m_default_width;
	}
	else
	{
		this->m_is_open = !this->get_on_hide();
	}
}


void debug_menu::change_entry_list(debug_menu_entry_list* list)
{
	if (this->m_current_entry_list != list)
	{
		debug_menu_entry_list* current_list = this->m_current_entry_list;
		this->m_current_entry_list = list;
		this->m_current_entry_list->previous = current_list;
		int w, h;
		nglGetTextSize(list->menu_title, &w, &h, DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
		this->m_default_width = (float)w + DEBUG_MENU_ENTRY_LEFT_PADDING + DEBUG_MENU_ENTRY_RIGHT_PADDING;
	}
}

float debug_menu::get_menu_max_height() const
{
	return DEBUG_MENU_MAX_HEIGHT - this->m_window_pos_y;
}

float debug_menu::get_max_pos_y_for_entries() const
{
	return this->get_menu_max_height() - DEBUG_MENU_ENTRY_BOTTOM_PADDING - DEBUG_MENU_TITLE_TOP_PADDING - (float)this->m_down_arrow_height - DEBUG_MENU_ENTRY_BOTTOM_PADDING;
}

float debug_menu::get_min_pos_y_for_entries() const
{
	float result = this->get_up_scroll_indicator_pos_y();
	if (this->can_scoll_up())
	{
		result += ((float)this->m_up_arrow_height + DEBUG_MENU_ENTRY_BOTTOM_PADDING);
	}
	return result;
}

bool debug_menu::can_scoll_down() const
{
	return !this->get_last_entry()->is_visible;
}

bool debug_menu::can_scoll_up() const
{
	return this->m_current_entry_list->selected_entry_index != 0 && this->get_last_entry()->pos_y > this->get_max_pos_y_for_entries();
}

float debug_menu::get_down_scroll_indicator_pos_y() const
{
	return this->get_menu_max_height() - (float)this->m_down_arrow_height - DEBUG_MENU_ENTRY_BOTTOM_PADDING;
}

float debug_menu::get_up_scroll_indicator_pos_y() const
{
	return this->m_window_pos_y + (float)this->m_default_height;
}

debug_menu_entry* debug_menu::get_selected_entry() const
{
	return this->m_current_entry_list->get_selected();
}

debug_menu_entry* debug_menu::get_last_entry() const
{
	return this->m_current_entry_list->last();
}

debug_menu_entry* debug_menu::add_entry(E_NGLMENU_ENTRY_TYPE type, const char* text, void* value_or_callback_ptr, void* callback_arg)
{
	return this->m_default_entry_list->add_entry(this, type, text, value_or_callback_ptr, callback_arg);
}

debug_menu::debug_menu(const char* title, float x, float y)
{
	this->m_is_open = false;
	this->m_default_entry_list = new debug_menu_entry_list(title);
	this->m_current_entry_list = this->m_default_entry_list;
	this->on_show = nullptr;
	this->on_hide = nullptr;
	this->m_name = title;
	this->m_window_pos_x = x;
	this->m_window_pos_y = y;
	nglConstructBox(&this->m_ngl_box_data);
	int default_width, default_height;
	nglGetTextSize(title, &default_width, &default_height, DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
	this->m_default_width = (float)default_width + DEBUG_MENU_ENTRY_LEFT_PADDING + DEBUG_MENU_ENTRY_RIGHT_PADDING;
	this->m_default_height = (float)default_height + DEBUG_MENU_TITLE_TOP_PADDING + DEBUG_MENU_ENTRY_BOTTOM_PADDING;
	nglGetTextSize(DEBUG_MENU_DOWN_ARROW, &this->m_down_arrow_width, &this->m_down_arrow_height, DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
	nglGetTextSize(DEBUG_MENU_UP_ARROW, &this->m_up_arrow_width, &this->m_up_arrow_height, DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
	this->m_width = (float)this->m_default_width;
	this->m_height = (float)this->m_default_height;
	nglSetBoxColor(&this->m_ngl_box_data, 0xC0000000);
	this->m_current_callback = { nullptr, nullptr };
	memset(this->m_last_fast_scroll_time, 0, sizeof(m_last_fast_scroll_time));
}

bool debug_menu::is_open() const
{
	return this->m_is_open;
}

void debug_menu::show()
{
	this->m_is_open = true;
}

void debug_menu::hide()
{
	this->m_is_open = false;
}

void debug_menu::set_on_show(void* callback)
{
	this->on_show = callback;
}

void debug_menu::set_on_hide(void* callback)
{
	this->on_hide = callback;
}

void debug_menu::execute_current_callback()
{
	if (this->m_current_callback.callback_ptr != nullptr)
	{
		typedef void(*entrycallback_t)(void*);
		entrycallback_t callback = (entrycallback_t)this->m_current_callback.callback_ptr;
		callback(this->m_current_callback.callback_arg);
	}
}

void debug_menu::reset_current_callback()
{
	this->m_current_callback.callback_ptr = nullptr;
	this->m_current_callback.callback_arg = nullptr;
}

void debug_menu::draw()
{
	if (!this->m_is_open)
		return;

	if (!this->m_current_entry_list->empty())
	{
		nglSetBoxRect(&this->m_ngl_box_data, this->m_window_pos_x, this->m_window_pos_y, this->m_width, this->m_height);
		nglDrawBox(&this->m_ngl_box_data);
		nglDrawText(this->m_current_entry_list->menu_title, RGBA_TO_INT(255, 255, 0, 255), this->m_window_pos_x + DEBUG_MENU_ENTRY_LEFT_PADDING, this->m_window_pos_y + DEBUG_MENU_TITLE_TOP_PADDING, DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);

		const float y_start = this->get_min_pos_y_for_entries();
		const debug_menu_entry* last_entry = this->get_last_entry();
		const float entry_max_pos_y = this->get_max_pos_y_for_entries();
		const float menu_max_height = this->get_menu_max_height();

		float current_entry_pos_y = y_start;
		float current_width = this->m_default_width;
		char current_entry_display_text[128];

		if (this->can_scoll_up())
		{
			current_width = max(current_width, (float)this->m_up_arrow_width + DEBUG_MENU_ENTRY_LEFT_PADDING + DEBUG_MENU_ENTRY_RIGHT_PADDING);
		}
		if (this->can_scoll_down())
		{
			current_width = max(current_width, (float)this->m_down_arrow_width + DEBUG_MENU_ENTRY_LEFT_PADDING + DEBUG_MENU_ENTRY_RIGHT_PADDING);
		}

		for (size_t i = 0; i < this->m_current_entry_list->size(); i++)
		{
			debug_menu_entry* current_entry = this->m_current_entry_list->entry_at(i);
			nglColor_t current_text_color = RGBA_TO_INT(255, 255, 255, 255);
			current_entry->pos_y = current_entry_pos_y;
			const bool is_selected = (i == this->m_current_entry_list->selected_entry_index);
			if (is_selected)
			{
				current_text_color = RGBA_TO_INT(0, 255, 21, 255);
			}
			switch (current_entry->type)
			{
			case E_NGLMENU_ENTRY_TYPE::BOOLEAN:
			{
				bool val = *(bool*)current_entry->value_ptr;
				sprintf(current_entry_display_text, "%s: %s", current_entry->text, val ? "True" : "False");
			}
			break;

			case E_NGLMENU_ENTRY_TYPE::BUTTON:
			{
				strcpy(current_entry_display_text, current_entry->text);
			}
			break;

			case E_NGLMENU_ENTRY_TYPE::MENU:
			{
				sprintf(current_entry_display_text, "%s: ...", current_entry->text);
			}
			break;

			case E_NGLMENU_ENTRY_TYPE::SELECT:
			{
				if (!current_entry->sublist->empty())
				{
					const debug_menu_entry* selection = current_entry->sublist->get_selected();
					if (selection->type == E_NGLMENU_ENTRY_TYPE::TEXT)
					{
						sprintf(current_entry_display_text, "%s: %s", current_entry->text, selection->text);
					}
				}
				else
				{
					sprintf(current_entry_display_text, "%s:", current_entry->text);
				}
			}
			break;

			case E_NGLMENU_ENTRY_TYPE::TEXT:
				strcpy(current_entry_display_text, current_entry->text);
				break;

			default:
				break;
			}
			nglGetTextSize(current_entry_display_text, &current_entry->text_width, &current_entry->text_height, DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
			bool overflow = current_entry->pos_y > entry_max_pos_y;
			if (!overflow)
			{
				this->m_height = current_entry->pos_y + (float)current_entry->text_height + DEBUG_MENU_ENTRY_BOTTOM_PADDING;
			}
			else
			{
				if (current_entry->display_pos_y > entry_max_pos_y)
				{
					this->m_height = this->get_menu_max_height();
				}
				else
				{
					this->m_height = current_entry->display_pos_y + (float)current_entry->text_height + DEBUG_MENU_ENTRY_BOTTOM_PADDING;
				}
			}
			if (is_selected)
			{
				this->m_current_entry_list->scroll_pos_y = current_entry->pos_y;
			}
			if (last_entry->pos_y > entry_max_pos_y)
			{
				current_entry->display_pos_y = current_entry->pos_y - this->m_current_entry_list->scroll_pos_y + y_start;
			}
			else
			{
				current_entry->display_pos_y = current_entry->pos_y;
			}
			current_entry->is_visible = current_entry->display_pos_y >= y_start && current_entry->display_pos_y <= entry_max_pos_y;
			if (current_entry->is_visible)
			{
				float tmpWidth = (float)current_entry->text_width + DEBUG_MENU_ENTRY_LEFT_PADDING + DEBUG_MENU_ENTRY_RIGHT_PADDING;
				current_width = max(current_width, tmpWidth);
				nglDrawText(current_entry_display_text, current_text_color, this->m_window_pos_x + DEBUG_MENU_ENTRY_LEFT_PADDING, current_entry->display_pos_y, DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
			}

			current_entry_pos_y += (float)current_entry->text_height + DEBUG_MENU_ENTRY_BOTTOM_PADDING;
		}
		if (this->can_scoll_down())
		{
			nglDrawText(DEBUG_MENU_DOWN_ARROW, RGBA_TO_INT(217, 0, 255, 255), this->m_window_pos_x + DEBUG_MENU_ENTRY_LEFT_PADDING, this->get_down_scroll_indicator_pos_y(), DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
		}
		if (this->can_scoll_up())
		{
			nglDrawText(DEBUG_MENU_UP_ARROW, RGBA_TO_INT(217, 0, 255, 255), this->m_window_pos_x + DEBUG_MENU_ENTRY_LEFT_PADDING, this->get_up_scroll_indicator_pos_y(), DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
		}
		this->m_width = current_width;
	}
}

void debug_menu::handle_input()
{
	// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

	const int VK_A = 0x41;
	const int VK_D = 0x44;
	const int VK_W = 0x57;
	const int VK_S = 0x53;

	input_mgr::initialize();

	if (!xenon_input_mgr::is_initialized())
	{
		xenon_input_mgr::initialize();
	}
	else
	{
		xenon_input_mgr::update_state();
	}

	if (input_mgr::is_key_pressed_once(VK_INSERT) || (xenon_input_mgr::is_button_pressed_once(XINPUT_GAMEPAD_LEFT_THUMB) && xenon_input_mgr::is_button_pressed_once(XINPUT_GAMEPAD_RIGHT_THUMB)))
	{
		this->m_is_open = (!(this->m_is_open && this->get_on_hide()) && (!this->m_is_open && this->get_on_show()));
	}

	if (!this->m_is_open)
		return;

	const debug_menu_entry* const selected_entry = this->get_selected_entry();
	const size_t last_entry_index = this->m_current_entry_list->size() - 1;

	const bool up = input_mgr::is_key_pressed_once(VK_W)
		|| input_mgr::is_key_pressed_repeated(VK_W)
		|| input_mgr::is_key_pressed_once(VK_UP)
		|| input_mgr::is_key_pressed_repeated(VK_UP)
		|| xenon_input_mgr::left_thumb_up_once()
		|| xenon_input_mgr::left_thumb_up_repeat();

	const bool down = input_mgr::is_key_pressed_once(VK_S)
		|| input_mgr::is_key_pressed_repeated(VK_S)
		|| input_mgr::is_key_pressed_once(VK_DOWN)
		|| input_mgr::is_key_pressed_repeated(VK_DOWN)
		|| xenon_input_mgr::left_thumb_down_once()
		|| xenon_input_mgr::left_thumb_down_repeat();


	const bool right =
		input_mgr::is_key_pressed_once(VK_D)
		|| input_mgr::is_key_pressed_repeated(VK_D)
		|| input_mgr::is_key_pressed_once(VK_RIGHT)
		|| input_mgr::is_key_pressed_repeated(VK_RIGHT)
		|| xenon_input_mgr::left_thumb_right_once()
		|| xenon_input_mgr::left_thumb_right_repeat();


	const bool left = input_mgr::is_key_pressed_once(VK_A)
		|| input_mgr::is_key_pressed_repeated(VK_A)
		|| input_mgr::is_key_pressed_once(VK_LEFT)
		|| input_mgr::is_key_pressed_repeated(VK_LEFT)
		|| xenon_input_mgr::left_thumb_left_once()
		|| xenon_input_mgr::left_thumb_left_repeat();
	
	
	const bool back = input_mgr::is_key_pressed_once(VK_ESCAPE)
		|| xenon_input_mgr::is_button_pressed_once(XINPUT_GAMEPAD_B)
		|| xenon_input_mgr::is_button_pressed_repeat(XINPUT_GAMEPAD_B);
	
	const bool execute = input_mgr::is_key_pressed_once(VK_SPACE)
		|| xenon_input_mgr::is_button_pressed_once(XINPUT_GAMEPAD_A);

	if (up)
	{
		do
		{
			if (this->m_current_entry_list->selected_entry_index == 0)
			{
				this->m_current_entry_list->selected_entry_index = last_entry_index;
			}
			else
			{
				this->m_current_entry_list->selected_entry_index--;
			}
		} while (this->get_selected_entry()->type == E_NGLMENU_ENTRY_TYPE::TEXT);
	}

	if (down)
	{
		do
		{
			if (this->m_current_entry_list->selected_entry_index == last_entry_index)
			{
				this->m_current_entry_list->selected_entry_index = 0;
			}
			else
			{
				this->m_current_entry_list->selected_entry_index++;
			}
		} while (this->get_selected_entry()->type == E_NGLMENU_ENTRY_TYPE::TEXT);
	}

	if (back)
	{
		this->go_back();
	}

	if (execute)
	{
		switch (selected_entry->type)
		{
		case E_NGLMENU_ENTRY_TYPE::BOOLEAN:
		{
			if (selected_entry->value_ptr != nullptr)
			{
				bool* val = (bool*)selected_entry->value_ptr;
				*val = !*val;
			}
		}
		break;

		case E_NGLMENU_ENTRY_TYPE::BUTTON:
		{
			if (selected_entry->handle_ptr != nullptr)
			{
				this->m_current_callback.callback_ptr = selected_entry->handle_ptr;
				this->m_current_callback.callback_arg = selected_entry->callback_arg;
				this->m_is_open = !this->get_on_hide();
			}
		}
		break;

		case E_NGLMENU_ENTRY_TYPE::MENU:
		{
			if (!selected_entry->sublist->empty())
			{
				selected_entry->sublist->menu_title = selected_entry->text;
				this->change_entry_list(selected_entry->sublist);
			}
		}
		break;

		default:
			break;
		}
	}

	if (left)
	{
		switch (selected_entry->type)
		{
		case E_NGLMENU_ENTRY_TYPE::BOOLEAN:
		{
			if (selected_entry->value_ptr != nullptr)
			{
				bool* val = (bool*)selected_entry->value_ptr;
				*val = !*val;
			}
		}
		break;

		case E_NGLMENU_ENTRY_TYPE::SELECT:
		{
			if (!selected_entry->sublist->empty())
			{
				int prevIndex = selected_entry->sublist->selected_entry_index;
				if (prevIndex == 0)
				{
					selected_entry->sublist->selected_entry_index = selected_entry->sublist->size() - 1;
				}
				else
				{
					selected_entry->sublist->selected_entry_index--;
				}
				if (selected_entry->sublist->selected_entry_index != prevIndex)
				{
					const debug_menu_entry* selection = selected_entry->sublist->get_selected();
					this->m_current_callback.callback_ptr = selection->handle_ptr;
					this->m_current_callback.callback_arg = selection->callback_arg;
				}
			}
		}
		break;

		default:
			break;
		}
	}

	if (right)
	{
		switch (selected_entry->type)
		{
		case E_NGLMENU_ENTRY_TYPE::BOOLEAN:
		{
			if (selected_entry->value_ptr != nullptr)
			{
				bool* val = (bool*)selected_entry->value_ptr;
				*val = !*val;
			}
		}
		break;

		case E_NGLMENU_ENTRY_TYPE::SELECT:
		{
			if (!selected_entry->sublist->empty())
			{
				int prevIndex = selected_entry->sublist->selected_entry_index;
				if (prevIndex == selected_entry->sublist->size() - 1)
				{
					selected_entry->sublist->selected_entry_index = 0;
				}
				else
				{
					selected_entry->sublist->selected_entry_index++;
				}
				if (selected_entry->sublist->selected_entry_index != prevIndex)
				{
					debug_menu_entry* const selection = selected_entry->sublist->get_selected();
					this->m_current_callback.callback_ptr = selection->handle_ptr;
					this->m_current_callback.callback_arg = selection->callback_arg;
				}
			}
		}
		break;

		default:
			break;
		}
	}
}

debug_menu_entry* debug_menu_entry::add_sub_entry(E_NGLMENU_ENTRY_TYPE type, const char* text, void* value_or_callback_ptr, void* callback_arg)
{
	return this->sublist->add_entry(this->parent, type, text, value_or_callback_ptr, callback_arg);
}

debug_menu_entry_list::debug_menu_entry_list(const char* menu_title)
{
	this->entries = std::vector<debug_menu_entry*>();
	this->menu_title = menu_title;
	this->previous = nullptr;
	this->scroll_pos_y = 0.0f;
	this->selected_entry_index = 0;
}

debug_menu_entry* debug_menu_entry_list::add_entry(class debug_menu* parent, E_NGLMENU_ENTRY_TYPE type, const char* text, void* value_or_callback_ptr, void* callback_arg)
{
	debug_menu_entry* entry = new debug_menu_entry;
	entry->type = type;
	strncpy(entry->text, text, DEBUG_MENU_ENTRY_MAX_CHAR);
	entry->value_ptr = value_or_callback_ptr;
	entry->callback_arg = callback_arg;
	nglGetTextSize(text, &entry->text_width, &entry->text_height, DEBUG_MENU_FONT_SCALE, DEBUG_MENU_FONT_SCALE);
	entry->parent = parent;
	entry->sublist = new debug_menu_entry_list(nullptr);
	this->entries.push_back(entry);
	return entry;
}

debug_menu_entry* debug_menu_entry_list::first() const
{
	return this->entries[0];
}

debug_menu_entry* debug_menu_entry_list::last() const
{
	return this->entries[this->entries.size() - 1];
}

debug_menu_entry* debug_menu_entry_list::get_selected() const
{
	return this->entries[this->selected_entry_index];
}

debug_menu_entry* debug_menu_entry_list::entry_at(const size_t& index) const
{
	return this->entries[index];
}

size_t debug_menu_entry_list::size() const
{
	return this->entries.size();
}

bool debug_menu_entry_list::empty() const
{
	return this->entries.empty();
}