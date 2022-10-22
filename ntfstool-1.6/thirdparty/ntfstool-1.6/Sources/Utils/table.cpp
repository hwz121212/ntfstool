#include "Utils/table.h"
#include "Utils/utils.h"

#include <sstream>
#include <iostream>
#include <iomanip>

utils::ui::Table::Table()
{
	margin_left = 0;
}

void utils::ui::Table::add_header_line(std::string header, utils::ui::TableAlign align)
{
	std::vector<std::string> s;
	s.push_back(header);
	headers.push_back(s);
	column_align.push_back(align);
}

void utils::ui::Table::add_header_multiline(std::initializer_list<std::string> header, utils::ui::TableAlign align)
{
	std::vector<std::string> s;
	for (auto& i : header)
	{
		s.push_back(i);
	}
	headers.push_back(s);
	column_align.push_back(align);
}

void utils::ui::Table::add_item_line(std::string item)
{
	std::vector<std::string> s;
	DWORD value_len = utils::strings::utf8_string_size(item);

	if (value_len < cell_max_size)
	{
		s.push_back(item);
	}
	else
	{
		if (value_len > 2 * cell_max_size)
		{
			cell_max_size = 32;
		}
		for (unsigned int i = 0; i < value_len; i += cell_max_size)
		{
			s.push_back(item.substr(i, cell_max_size));
		}
	}
	current_line.push_back(s);
}

void utils::ui::Table::add_item_multiline(std::initializer_list<std::string> list)
{
	std::vector<std::string> s;
	for (auto& i : list)
	{
		s.push_back(i);
	}
	add_item_multiline(s);
}

void utils::ui::Table::add_item_multiline(std::vector<std::string> list)
{
	std::vector<std::string> s;
	for (auto& i : list)
	{
		auto i_len = utils::strings::utf8_string_size(i);
		if (i_len > cell_max_size) {
			size_t sep = i.find(':');
			if (sep != std::string::npos && sep < i_len - 1 && i[sep + 1] == ' ')
			{
				std::string key = i.substr(0, sep);
				auto key_len = utils::strings::utf8_string_size(key);
				std::string whitespace = "";
				for (unsigned int n = 0; n < key_len; n++) whitespace += " ";
				std::string value = i.substr(sep + 2);
				auto value_len = utils::strings::utf8_string_size(value);
				auto offset = value.length() - value_len;
				if (value_len > cell_max_size)
				{
					auto tmp_max_size = cell_max_size;
					if (value_len > 2 * cell_max_size)
					{
						cell_max_size = 32;
					}
					s.push_back(key + ": " + value.substr(0, cell_max_size + offset));
					value = value.substr(cell_max_size + offset);
					value_len = utils::strings::utf8_string_size(value);
					offset = value.length() - value_len;
					while (value_len > cell_max_size) {
						s.push_back(whitespace + "  " + value.substr(0, cell_max_size + offset));
						value = value.substr(cell_max_size + offset);
						value_len = utils::strings::utf8_string_size(value);
					}
					if (value_len > 0)
					{
						s.push_back(whitespace + "  " + value);
					}
					cell_max_size = tmp_max_size;
				}
				else
				{
					s.push_back(key + ": " + value);
				}
			}
			else
			{
				std::string value = i;

				while (value.length() > cell_max_size) {
					s.push_back(value.substr(0, cell_max_size));
					value = value.substr(cell_max_size);
				}
				if (value.length() > 0) {
					if (value == TABLE_SEPARATOR)
					{
						std::string padded_sep = "";
						for (int i = 0; i < (cell_max_size - strlen(TABLE_SEPARATOR)) / 2; i++)
						{
							padded_sep.append(" ");
						}
						s.push_back(padded_sep + value);
					}
					else
					{
						s.push_back(value);
					}
				}
			}
		}
		else
		{
			s.push_back(i);
		}
	}
	current_line.push_back(s);
}

void utils::ui::Table::set_margin_left(uint32_t margin_left)
{
	this->margin_left = margin_left;
}

void utils::ui::Table::set_cell_max_size(uint32_t max_size)
{
	this->cell_max_size = max_size;
}

void utils::ui::Table::new_line()
{
	data.push_back(current_line);
	current_line.clear();
}

void utils::ui::Table::render(std::ostream& out)
{
	std::ios_base::fmtflags flag_backup(out.flags());
	size_t line_size = 0;
	out << std::unitbuf;

	std::vector<uint32_t> column_size(this->headers.size(), 0);

	unsigned int i;
	for (i = 0; i < this->headers.size(); i++) {
		uint32_t m = 0;
		for (unsigned int j = 0; j < this->data.size(); j++) {
			uint32_t cell_max_width = 0;
			for (unsigned int k = 0; k < this->data[j][i].size(); k++) {
				cell_max_width = max(cell_max_width, utils::strings::utf8_string_size(this->data[j][i][k]));
			}
			m = max(m, cell_max_width);
		}
		for (unsigned int j = 0; j < this->headers[i].size(); j++)
		{
			m = max(m, utils::strings::utf8_string_size(this->headers[i][j]));
		}
		column_size[i] = m;
		line_size += m;
	}
	line_size += 3 * (this->headers.size() - 1);

	out << std::setfill(' ');

	if (border_top)
	{
		for (i = 0; i < margin_left; i++) out << " ";
		if (border_left)
		{
			out << (corner ? "+" : " ") << "-";
		}
		for (i = 0; i < line_size; i++) out << "-";
		if (border_right)
		{
			out << "-" << (corner ? "+" : " ");
		}
		out << std::endl;
	}

	size_t header_lines = 0;
	for (auto& h : headers)
	{
		header_lines = max(header_lines, h.size());
	}
	for (unsigned int header_line_index = 0; header_line_index < header_lines; header_line_index++)
	{
		for (i = 0; i < margin_left; i++) out << " ";
		if (border_left)
		{
			out << "| ";
		}
		for (i = 0; i < headers.size() - 1; i++) {
			out.width(column_size[i]);
			if (header_line_index < headers[i].size()) out << (column_align[i] == TableAlign::LEFT ? std::left : std::right) << headers[i][header_line_index];
			else out << std::left << "";
			out << " | ";
		}
		out.width(column_size[i]);
		if (header_line_index < headers[i].size()) out << (column_align[i] == TableAlign::LEFT ? std::left : std::right) << headers[i][header_line_index];
		else out << std::left << "";
		if (border_right)
		{
			out << " |";
		}
		out << std::endl;
	}

	for (auto& line : data) {
		// Header / Data line		
		if (interline || header_interline)
		{
			for (i = 0; i < margin_left; i++) out << " ";
			header_interline = false;
			if (border_left)
			{
				out << (corner ? "+" : " ") << "-";
			}
			for (i = 0; i < line_size; i++) out << "-";
			if (border_right)
			{
				out << "-" << (corner ? "+" : " ");
			}
			out << std::endl;
		}

		//for (i = 0; i < margin_left; i++) out << " ";

		// Max cell height for the line
		size_t cell_height = 0;
		for (auto& cell : line)
		{
			cell_height = max(cell_height, cell.size());
		}
		// Print cells
		for (unsigned int lines_i = 0; lines_i < cell_height; lines_i++)
		{
			for (i = 0; i < margin_left; i++) out << " ";
			if (border_left)
			{
				out << "| ";
			}
			size_t fix = 0;
			for (i = 0; i < line.size() - 1; i++)
			{
				out.width(column_size[i]);
				if (lines_i < line[i].size())
				{
					fix = (line[i][lines_i].length() - utils::strings::utf8_string_size(line[i][lines_i]));
					out.width(column_size[i]);

					if (column_align[i] == TableAlign::LEFT) out << std::left << line[i][lines_i];
					else  out << std::right << line[i][lines_i];
				}
				else out << std::left << " ";
				out << " | ";
			}

			if (lines_i < line[i].size())
			{
				fix = (line[i][lines_i].length() - utils::strings::utf8_string_size(line[i][lines_i]));
				out.width(column_size[i] + fix);
				if (column_align[i] == TableAlign::LEFT) out << std::left << line[i][lines_i];
				else out << std::right << line[i][lines_i];
			}
			else {
				out.width(column_size[i]);
				out << std::left << " ";
			}
			if (border_right)
			{
				out << " |";
			}

			out << std::endl;
		}
	}

	if (border_bottom)
	{
		for (i = 0; i < margin_left; i++) out << " ";
		if (border_left)
		{
			out << (corner ? "+" : " ") << "-";
		}
		for (i = 0; i < line_size; i++) out << "-";
		if (border_right)
		{
			out << "-" << (corner ? "+" : " ");
		}
		out << std::endl;
	}
	out.flags(flag_backup);
}
