#include <iostream>
#include <regex>
#include <fstream>
#include <string>

class DBLibrary
{
public:
	DBLibrary() { }

	string new_command(const string& command);

private:

	string file_name{ "database_library.txt" };

	int _find(string& to_find);

	int insert(string& text);

	string building_string(const string& line);

	int _delete(const string& line);

	int print(string&);
};


int DBLibrary::_find(string& to_find)
{
	ifstream file(file_name);
	string buf;
	while (getline(file, buf))
	{
		if (buf.find(to_find) != -1)
		{
			smatch m;
			if (regex_search(buf, m, regex("[0-9]{1,}")))
			{
				to_find = buf;
				file.close();
				return stoi(m.str());
			}
		}
	}
	file.close();
	return -1;
}


string DBLibrary::new_command(const string& command)
{
	string buf = command;
	regex com("[A-Z]{1,}");
	//regex inf("\\([a-zA-Z0-9 \\.,]{1,}\\)");
	//regex field("([a-zA-Z0-9\\.][a-zA-Z0-9 \\.]{1,}[a-zA-Z0-9\\.])|([a-zA-Z0-9\\.]{1,2})");
	smatch m;
	if (!regex_match(buf, regex("[A-Z]{1,} *\\([a-zA-Z0-9 \\.,]{1,}\\)")))
	{
		return "An error has occurred in this line and it cannot be processed.\n";
	}

	if (regex_search(buf, m, com))
	{
		if (m.str() == "INSERT")
		{
			buf = m.suffix();

			if (insert(buf))
			{
				return "An error has occurred in this line and it cannot be processed.\n";
			}

		}
		else if (m.str() == "PRINT")
		{
			buf = m.suffix();
			if (!print(buf))
			{
				return buf;
			}
			else
			{
				return "There is no such name";
			}
		}
		else if (m.str() == "DELETE")
		{
			buf = m.suffix();
			_delete(buf);
		}
		else
		{
			return "An error has occurred in this line and it cannot be processed.\n";
		}
	}
	return "";
}

int DBLibrary::print(string& to_print)
{
	regex kod("[0-9]{1,}");
	regex inf("\\([a-zA-Z0-9 \\.,]{1,}\\)");
	regex field("([a-zA-Z0-9\\.][a-zA-Z0-9 \\.]{1,}[a-zA-Z0-9\\.])|([a-zA-Z0-9\\.]{1,2})");
	smatch m;

	if (regex_search(to_print, m, inf))
	{
		to_print = m.str();
	}

	if (regex_search(to_print, m, field))
	{
		to_print = m.str();
		if (_find(to_print) != -1)
		{
			to_print = building_string(to_print);
			return 0;
		}
		else
		{
			return -1;
		}
	}
	to_print = "An error has occurred in this line and it cannot be processed.\n";
	return -1;
}

int DBLibrary::_delete(const string& line)
{
	regex field("([a-zA-Z0-9\\.][a-zA-Z0-9 \\.]{1,}[a-zA-Z0-9\\.])|([a-zA-Z0-9\\.]{1,2})");
	regex inf("\\([a-zA-Z0-9 \\.,]{1,}\\)");
	string name;
	string buf_line;
	smatch m;
	bool flag_to_return = 0;

	if (regex_search(line, m, inf))
	{
		name = m.str();
	}

	if (regex_search(name, m, field))
	{
		name = m.str();

		fstream file(file_name, ios::in | ios::app);
		ofstream tmp_file("temp_file.txt");


		while (getline(file, buf_line))
		{
			if (buf_line.find(name) != -1)
			{
				flag_to_return = 1;
				continue;
			}
			tmp_file << buf_line << endl;
		}
		tmp_file.close();
		file.close();

		if (remove("database_library.txt"))
		{
			perror(strerror(errno));
			return 1;
		}
		if (rename("temp_file.txt", "database_library.txt"))
		{
			perror(strerror(errno));
			return 1;
		}
	}

	if (flag_to_return)
		return 0;
	else
		return -1;

}

string DBLibrary::building_string(const string& line)
{
	regex r_line_in_f("[0-9]+:[a-zA-Z0-9 .]+:[a-zA-Z0-9 .]+:[0-9]+:");
	string res = "";

	if (regex_match(line, r_line_in_f))
	{
		res += "code:{";
		int i = 0;
		while (line[i] != ':')
			res += line[i++];
		i++;
		res += "} title:{";
		while (line[i] != ':')
			res += line[i++];
		i++;
		res += "} author:{";
		while (line[i] != ':')
			res += line[i++];
		i++;
		res += "} count:{";
		while (line[i] != ':')
			res += line[i++];
		res += "}\n";
		return res;
	}
	else
	{
		return "An error has occurred in this line and it cannot be processed.\n";
	}
}

int DBLibrary::insert(string& text)
{
	regex inf("\\([a-zA-Z0-9 \\.,]{1,}\\)");
	regex field("([a-zA-Z0-9\\.][a-zA-Z0-9 \\.]{1,}[a-zA-Z0-9\\.])|([a-zA-Z0-9\\.]{1,2})");
	smatch m;

	fstream file(file_name, ios::in | ios::app);

	string buf;
	string name;

	if (regex_search(text, m, inf))
	{
		name = m.str();
	}

	if (regex_search(name, m, field))
	{
		name = m.suffix();
	}

	if (regex_search(name, m, field)) // получим имя
	{
		buf = m.suffix();
		name = m.str();
	}

	string buf_line;

	if (_find(name) != -1)
	{
		regex_search(buf, m, field);
		buf = m.suffix();
		regex_search(buf, m, field);
		buf = m.str(); // сейчас в buf count


		ofstream tmp_file("temp_file.txt");

		while (getline(file, buf_line))
		{
			if (buf_line.find(name) != -1)
			{
				string count_str = "";
				buf_line.pop_back();
				int i = buf_line.size() - 1;
				while (buf_line[i] != ':')
				{
					count_str.insert(count_str.begin(), buf_line[i--]);
					buf_line.pop_back();
				}
				i = stoi(buf) + stoi(count_str);
				buf_line += to_string(i) + ":";
			}
			tmp_file << buf_line << endl;
		}
		tmp_file.close();
		file.close();
		//if (rename("database_library.txt", "temp_name.txt"))
		//{
		//	perror(strerror(errno));
		//	return 0;
		//}
		if (remove("database_library.txt"))
		{
			perror(strerror(errno));
			return 0;
		}
		if (rename("temp_file.txt", "database_library.txt"))
		{
			perror(strerror(errno));
			return 0;
		}
	}
	else
	{
		buf = text;
		for (int i = 0; i < 4; i++)
		{
			if (regex_search(buf, m, field))
			{
				file << m.str() << ":";
				buf = m.suffix();
			}
		}
		file << endl;
	}
	file.close();
	return 0;
}