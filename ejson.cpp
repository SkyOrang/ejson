#include "ejson.h"

namespace ejson
{
	//用于用户读取操作，将对应字符串缓冲区的数据写入外部string
	void jsonvalue::ToString(std::string& sdata)
	{
		int i = 0;
		char* data = (char*)(_file->_buffer.data() + _data.first);
		while (1)
		{
			if (data[i] == '\"')
			{
				break;
			}
			i++;
		}
		sdata.resize(i);
		memcpy((char*)sdata.data(),data , i);
		/*return (char*)(_file->_buffer.data()+_data.first);*/
	}

	//用于用户读取操作，将字符串缓冲区的数据转为int
	int jsonvalue::ToInt()
	{
		return std::atoi(_file->_buffer.data() + _data.first);
	}

	//用于用户读取操作，将字符串缓冲区的数据转为float
	double jsonvalue::ToFloat()
	{
		return std::atof((char*)(_file->_buffer.data() + _data.first));
	}

	//用于用户写入操作，插入string
	void jsonvalue::InsertString(const char* data)
	{
		size_t size = strlen(data);
		std::string tem;
		tem += '"';
		tem += data;
		tem += '"';
		_file->_buffer.insert(_data.first, tem.c_str());
		_file->_size += size+2;
		_file->_back += size+2;
	}

	//用于用户写入操作，插入整int
	void jsonvalue::InsertInt(const int& data)
	{
		std::string tem(std::to_string(data));
		size_t size = strlen(tem.c_str());
		_file->_buffer.insert(_data.first, tem);
		_file->_size += size;
		_file->_back += size;
	}

	//用于用户写入操作，插入float
	void jsonvalue::InsertFloat(const double& data)
	{
		std::string tem(std::to_string(data));
		size_t size = strlen(tem.c_str());
		_file->_buffer.insert(_data.first, tem);
		_file->_size += size;
		_file->_back += size;
	}

	//用于用户写入操作，插入null
	void jsonvalue::InsertNull()
	{
		InsertString("null");
	}

	//用于用户写入操作，删除string
	void jsonvalue::DeleteString(char* data)
	{
		int i = 0;
		while (data[i] != '"')
		{
			i++;
		}
		_file->_buffer.erase(_data.first -1, i+2);
		_data.first-=1;
		_file->_size -= i+2;
		_file->_back -= i+2;
	}

	//用于用户写入操作，删除int
	void jsonvalue::DeleteInt(char* data)
	{
		int i = 0;
		while (data[i] == 45 || (data[i] >= 48 && (data[i] <= 57)))
		{
			i++;
		}
		_file->_buffer.erase(_data.first,i);
		_file->_size -= i;
		_file->_back -= i;
	}

	//用于用户写入操作，删除float
	void jsonvalue::DeleteFloat(char* data)
	{
		int i = 0;
		while (data[i] == 45 || (data[i] >= 48 && (data[i] <= 57)) || data[i] == '.')
		{
			i++;
		}
		_file->_buffer.erase(_data.first, i);
		_file->_size -= i;
		_file->_back -= i;

	}

	//用于用户写入操作，删除array
	void jsonvalue::DeleteArray(char* data)
	{
		int i = 0;
		int count = 1;
		while (1)
		{
			if (data[i] == '[')
			{
				count++;
			}
			if (data[i] == ']')
			{
				if (count == 1)
				{
					break;
				}
				else
				{
					count--;
				}

			}
			
			i++;
		}
		_file->_buffer.erase(_data.first - 1, i+2);
		_file->_size -= i+2;
		_file->_back -= i+2;
		_data.first -= 1;

	}

	//用于用户写入操作，删除null
	void jsonvalue::DeleteNull(char* data)
	{
		int i = 0;
		while ((data[i] >= 97 && (data[i] <= 122)))
		{
			i++;
		}
		_file->_buffer.erase(_data.first, i);
		_file->_size -= i;
		_file->_back -= i;
	}

	//用于用户写入操作，删除object
	void jsonvalue::DeleteObject(char* data)
	{
		int i = 0;
		int count = 1;
		while (data[i] != '}' || count > 0)
		{
			if (data[i] == '}')
			{
				if (count == 1)
				{
					i++;
					break;
				}
				else
				{
					count--;
				}

			}
			if (data[i] == '{')
			{
				count++;
			}
			i++;
		}
		_file->_buffer.erase(_data.first - 1, i + 1);
		_file->_size -= i + 1;
		_file->_back -= i + 1;
		_data.first -= 1;

	}

	//用于用户写入操作，删除array中的一个数据节点
	void jsonvalue::DeleteArrayNode()
	{
		CheckDelete();
		int i = 0;
		char* data = (char*)(_file->_buffer.data() + _data.first);
		bool is_upexit = false;
		int up = 0;
		while (1)
		{
			if (*(data - up) == ','&&up!=0)
			{
				is_upexit = true;
				break;
			}
			if (*(data - up) == '[')
			{
				break;
			}

			up++;
		}

		bool is_downexit = false;
		int down = 0;
		while (1)
		{
			if (*(data + down) == ',')
			{
				is_downexit = true;
				break;
			}
			if (*(data + down) == ']')
			{
				break;
			}

			down++;
		}

		if (is_upexit == true)
		{
			_file->_buffer.erase(_data.first-up, up);
			_file->_size -= up;
			_file->_back -= up;
			_data.first -= up;
		}
		else if (is_downexit == true)
		{
			_file->_buffer.erase(_data.first, down+1);
			_file->_size -= down+1;
			_file->_back -= down+1;
		}
		
	}

	//用于用户写入操作，删除object中的一个数据节点
	void jsonvalue::DeleteObjectNode()
	{
		int head = 0;
		bool is_check = false;
		int str_count = 0;
		char* data = (char*)(_file->_buffer.data() + _data.first);
		while (1)
		{
			if (*(data - head) == ':')
			{
				is_check = true;
			}

			if (*(data - head) == '"')
			{
				str_count++;
			}

			if (is_check==true)
			{
				if (str_count == 2)
				{
					break;
				}
			}
			head++;
		}
		bool is_upexit = false;
		int up = head;
		while (1)
		{
			if (*(data - up) == ',')
			{
				is_upexit = true;
				break;
			}
			if (*(data - up) == '{')
			{
				break;
			}

			up++;
		}

		bool is_downexit = false;
		int down = 0;
		while (1)
		{
			if (*(data +down) == ',')
			{
				is_downexit = true;
				break;
			}
			if (*(data + down) == '}')
			{
				break;
			}

			down++;
		}
		if (is_upexit == true)
		{
			_file->_buffer.erase(_data.first - up, down + up);
			_file->_size -= down+up;
			_file->_back -= down+up;
			_data.first -= up;
		}
		else if (is_downexit == true)
		{
			_file->_buffer.erase(_data.first-head, head + down );
			_file->_size -= head + down;
			_file->_back -= head + down;
			_data.first -= head;
		}
		else
		{
			_file->_buffer.erase(_data.first - head, head + down );
			_file->_size -= head + down;
			_file->_back -= head + down;
			_data.first -= head;
		}

	}

	//用于用户写入操作，检查自身类型完成数据删除
	void jsonvalue::CheckDelete()
	{
		switch (_data.second)
		{
		case INT:
			DeleteInt((char*)(_file->_buffer.data() + _data.first));
			break;
		case FLOAT:
			DeleteFloat((char*)(_file->_buffer.data() + _data.first));
			break;
		case STRING:
			DeleteString((char*)(_file->_buffer.data() + _data.first));
			break;
		case ARRAY:
			DeleteArray((char*)(_file->_buffer.data() + _data.first));
			break;
		case NUL:
			DeleteNull((char*)(_file->_buffer.data() + _data.first));
			break;
		case OBJECT:
			DeleteObject((char*)(_file->_buffer.data() + _data.first));
			break;

		}
	}

	jsonvalue::jsonvalue(jsonfile* fptr, long offset, vtype type,vtype outer)
		:_data(std::make_pair(offset,type))
		, _file(fptr)
		,_outer(outer)
	{

	}

	//类型判断
	vtype jsonvalue::GetVType()
	{
		return _data.second;
	}

	jsonvalue jsonvalue::SeekIPair(const int& nindex,const int& index)
	{
		int tindex = nindex;
		int i = _data.first;
		int count = 1;
		if (_data.second == OBJECT || _data.second == ARRAY)
		{
			count = 2;
		}

		while (1)
		{
			if (_file->_size - 1 < i)
			{
				return jsonvalue(_file, -1, NUL, ARRAY);
			}



			if (_file->_buffer[i] == '[')
			{
				count++;
			}
			if (_file->_buffer[i] == '{')
			{
				count++;
			}
			if (_file->_buffer[i] == '}')
			{
				count--;
			}
			if (_file->_buffer[i] == ']')
			{
				count--;
				if (count == 0)
				{
					return jsonvalue(_file, -1, NUL, ARRAY);
				}
			}


			if (index == tindex)
			{
				std::pair<long, vtype> ret = _file->CheckValidData(i, '[');
				long re = ret.first;

				if (re == -1)
				{
					std::cout << "It's found that json's format is not correct ，finding array members ." << std::endl;
					exit(1);
				}

				return jsonvalue(_file, re, ret.second, ARRAY);
			}

			if (count == 1)
			{
				if (_file->_buffer[i] == ',')
				{
					tindex++;
				}
			}

			i++;
		}
	}

	jsonvalue jsonvalue::SeekPair(const char*key)
	{
		int i = _data.first;
		int count = 1;
		while (1)
		{
			if (_file->_size - 1 < i)
			{
				return jsonvalue(_file, -1, NUL, OBJECT);
			}



			if (_file->_buffer[i] == '{')
			{
				count++;
			}

			if (_file->_buffer[i] == '}')
			{
				count--;
				if (count == 0)
				{
					return jsonvalue(_file, -1, NUL, OBJECT);
				}
			}


			if (_file->_buffer[i] == '"')
			{
				bool is_match = false;
				long _key_index = i + 1;
				size_t length = SeekEnd(&(_file->_buffer[i + 1]), '"') - (i + 1);
				if (strlen(key) == length)
				{
					if (strncmp(&(_file->_buffer[i + 1]), key, length) == 0)
					{
						is_match = true;
					}
				}
				i += length + 2;
				if (is_match == true)
				{
					std::pair<long, vtype> ret = _file->CheckValidData(i, '{');
					long re = ret.first;

					if (re == -1)
					{
						std::string tem;
						this->ToString(tem);
						std::cout << "It's found that json's format is not correct ，finding " << tem << " ." << std::endl;
						exit(1);
					}

					return jsonvalue(_file, re, ret.second, OBJECT);


				}
			}
			i++;
		}
	}

	//array遍历
	bool jsonvalue::IPairs(int& key, jsonvalue& value, int step_length)
	{
		if (key == 0)
		{
			key += step_length;
			const jsonvalue& tem = (*this)[key];
			if (tem._data.first == -1)
			{
				return false;
			}
			value = (jsonvalue&)tem;
			return true;
		}
		else
		{
			const jsonvalue& tem = SeekIPair(key,key+1);
			key += step_length;
			if (tem._data.first == -1)
			{
				return false;
			}
			value = (jsonvalue&)tem;
			return true;
		}

		

	}

	//object遍历
	bool jsonvalue::Pairs(std::string& key, jsonvalue& value)
	{
		if (key.size() == 0)
		{
			auto re = _file->CheckValidData(_data.first,'{');
			if (re.first == -1)
			{
				return false;
			}
			(jsonvalue(_file, re.first, re.second, OBJECT)).ToString(key);
			value = (*this)[key.c_str()];
			return true;
		}
		else
		{
			const jsonvalue& tem = SeekPair(key.c_str());
			long offset = tem._data.first;
			char* data = (char*)(_file->_buffer.data() + offset);
			int i = 0;
			bool is_find = false;
			int count1 = 1;
			if (tem._data.second==OBJECT|| tem._data.second == ARRAY)
			{
				count1 = 2;
			}
			
			int count2 = 0;
			while (1)
			{
				if (_file->_size - 1 < i+offset)
				{
					return false;
				}
				if (data[i] == '[')
				{
					count1++;
				}
				if (data[i] == ']')
				{
					count1--;
				}
				if (data[i] == '{')
				{
					count1++;
				}
				if (data[i] == '}')
				{
					count1--;
					if (count1 == 0)
					{
						break;
					}
				}
				if (data[i] == ','&&count1==1)
				{
					is_find = true;
					break;
				}
			
				i++;
			}
			if (is_find == false)
			{
				return false;
			}
			auto re=_file->CheckValidData(offset+i,'{');
			(jsonvalue(_file, re.first, re.second, OBJECT)).ToString(key);
			value = (*this)[key.c_str()];
		}
		
		
	}

	//读取操作,读取字符串写入到用户string中
	jsonvalue& jsonvalue::operator>>(std::string& data)
	{
		if (_data.second == STRING)
		{
			ToString(data);
		}
		return*this;
	}

	//读取操作,读取字符串写入到用户int中
	jsonvalue& jsonvalue::operator>>(int& data)
	{
		if (_data.second == INT)
		{
			data = ToInt();
		}
		return*this;
	}

	//读取操作,读取字符串写入到用户double中
	jsonvalue& jsonvalue::operator>>(double& data)
	{
		if (_data.second == FLOAT)
		{
			data = ToFloat();
		}
		return*this;
	}

	//写入操作,将用户int数据写入到字符串缓冲区
	jsonvalue& jsonvalue::operator<<(const int& data)
	{
		CheckDelete();
		_data.second = INT;
		InsertInt(data);
		_file->_Is_Changed = true;
		return *this;
	}

	//写入操作,将用户string数据写入到字符串缓冲区
	jsonvalue& jsonvalue::operator<<(const char* data)
	{
		CheckDelete();
		_data.second = STRING;
		InsertString(data);
		_data.first += 1;
		_file->_Is_Changed = true;
		return *this;
	}

	//写入操作,将用户double数据写入到字符串缓冲区
	jsonvalue& jsonvalue::operator<<(const double& data)
	{
		CheckDelete();
		_data.second = FLOAT;
		InsertFloat(data);
		_file->_Is_Changed = true;
		return *this;
	}

	//写入操作,支持用户写入null,删除对应字段，写入空array，空object已经空数字键值对(array成员)
	jsonvalue& jsonvalue::operator<<(const vtype& data)
	{
		switch (data)
		{
		case NUL:
			CheckDelete();
			_data.second = NUL;
			InsertNull();
			break;
		case DELETE:
			{
				if (_outer == ARRAY)
				{
					DeleteArrayNode();
				}
				else if (_outer == OBJECT)
				{
					DeleteObjectNode();
				}
				break;
			}
		case EARRAY:
			CheckDelete();
			_data.second = ARRAY;
			InsertNewArray();
			_data.first += 1;
			break;
		case EOBJECT:
			CheckDelete();
			_data.second = OBJECT;
			InsertNewObject();
			_data.first += 1;
			break;
		case NIPAIR:
			if (_data.second != ARRAY)
			{
				std::cout << "Only the array can insert ipairs." << std::endl;
				exit(1);
			}
			/*_data.second =NUL ;*/
			InsertNewIPair();
			break;
		}
		_file->_Is_Changed = true;
		return *this;
	}

	//写入操作,支持用户写入键值对（object成员），配合宏NPAIR
	jsonvalue& jsonvalue::operator<<(const pair& data)
	{
		if (_data.second != OBJECT)
		{
			std::cout << "Only the object can insert pairs." << std::endl;
			exit(1);
		}
		/*_data.second = NUL;*/
		InsertNewPair(data.k.c_str());
		_file->_Is_Changed = true;
		return *this;
	}

	//插入操作，往字符串缓冲区写入空array
	void jsonvalue::InsertNewArray()
	{
		//插入新数组
	
		_file->_buffer.insert(_data.first, "[ ]");
		_file->_size += 3;
		_file->_back += 3;

	}

	//插入操作，往字符串缓冲区写入空object
	void jsonvalue::InsertNewObject()
	{
		//插入新对象
	
		_file->_buffer.insert(_data.first, "{ }");
		_file->_size += 3;
		_file->_back += 3;
		
	}

	//插入操作，往字符串缓冲区写入新键值对
	void jsonvalue::InsertNewPair(const char* key)
	{
		//插入新键值对
		auto re = _file->CheckValidData((char*)(_file->_buffer.data() + _data.first) - (char*)_file->_buffer.data(),'{');
		if (re.first == -1)
		{
			std::string tem;
			tem += '"';
			tem += key;
			tem += '"';
			tem += ": null";
			_file->_buffer.insert(_data.first, tem);
			_file->_size += strlen((tem).c_str());
			_file->_back += strlen((tem).c_str());
		}
		else
		{
			std::string tem;
			tem += '"';
			tem +=key;
			tem += '"';
			tem += ": null,";
			_file->_buffer.insert(_data.first, tem);
			_file->_size += strlen((tem).c_str());
			_file->_back += strlen((tem).c_str());
		}
	
	}

	//插入操作，往字符串缓冲区写入数字键值对
	void jsonvalue::InsertNewIPair()
	{
		//插入新数字键值对
		auto re = _file->CheckValidData((char*)(_file->_buffer.data() + _data.first) - (char*)_file->_buffer.data(), '[');
		if (re.first == -1)
		{
			std::string tem;
			tem += "null";
			_file->_buffer.insert(_data.first, tem);
			_file->_size += strlen((tem).c_str());
			_file->_back += strlen((tem).c_str());
		}
		else
		{
			auto ret = SeekEnd((char*)(_file->_buffer.data() + _data.first), '[');
			std::string tem;
			tem += ",null";
			_file->_buffer.insert(ret, tem);
			_file->_size += strlen((tem).c_str());
			_file->_back += strlen((tem).c_str());
		}

	}

	size_t jsonvalue::SeekEnd(char* ptr, char check)
	{
		int i = 0;
		int count = 1;
		int mode = 0;
		if (check == '{')
		{
			mode = 1;
		}
		else if (check == '[')
		{
			mode = 2;
		}

		while (1)
		{
			if (mode == 1)
			{
				if (ptr[i] == '{')
				{
					count++;
				}
				if (ptr[i] == '}')
				{
					count--;
					if (count == 0)
					{
						return (ptr-(char*)(_file->_buffer.data()))+i;
					}
				}
			}
			else if (mode == 2)
			{
				if (ptr[i] == '[')
				{
					count++;
				}
				if (ptr[i] == ']')
				{
					count--;
					if (count == 0)
					{
						return (ptr - (char*)(_file->_buffer.data())) + i;
					}
				}
			}
			else
			{
				if (ptr[i] == check)
				{
					return (ptr - (char*)(_file->_buffer.data())) + i;
				}
			}
		
		
			i++;
		}
	}

	//访问object
	jsonvalue jsonvalue::operator[](const char* key)
	{
		int i = _data.first;
		int count = 1;
		while(1)
		{
			if (_file->_size - 1 < i)
			{
				return jsonvalue(_file, -1, NUL, OBJECT);
			}



			if (_file->_buffer[i] == '{')
			{
				count++;
			}

			if (_file->_buffer[i] == '}')
			{
				count--;
				if (count == 0)
				{
					return jsonvalue(_file, -1, NUL, OBJECT);
				}
			}

			
			if (_file->_buffer[i] == '"')
			{
				bool is_match = false;
				long _key_index = i + 1;
				size_t length = SeekEnd(&(_file->_buffer[i + 1]),'"')-(i+1);
				if (strlen(key) == length)
				{
					if (strncmp(&(_file->_buffer[i + 1]), key, length) == 0)
					{
						is_match = true;
					}
				}
				i += length + 2;
				if (is_match == true)
				{
					std::pair<long, vtype> ret = _file->CheckValidData(i,'{');
					long re = ret.first;

					if (re == -1)
					{
						std::string tem;
						this->ToString(tem);
						std::cout << "It's found that json's format is not correct ，finding " << tem << " ." << std::endl;
						exit(1);
					}

					return jsonvalue(_file, re, ret.second,OBJECT);


				}
			}
			i++;
		}
	}

	//访问array
	jsonvalue jsonvalue::operator[](const int& index)
	{
		int tindex = 1;
		int i = _data.first;
		int count = 1;
		while (1)
		{
			if (_file->_size - 1 < i)
			{
				return jsonvalue(_file, -1, NUL, ARRAY);
			}



			if (_file->_buffer[i] == '[')
			{
				count++;
			}
			if (_file->_buffer[i] == '{')
			{
				count++;
			}
			if (_file->_buffer[i] == '}')
			{
				count--;
			}
			if (_file->_buffer[i] == ']')
			{
				count--;
				if (count == 0)
				{
					return jsonvalue(_file, -1, NUL, ARRAY);
				}
			}
			

			if (index == tindex)
			{
				std::pair<long, vtype> ret = _file->CheckValidData(i,'[');
				long re = ret.first;

				if (re == -1)
				{
					std::cout << "It's found that json's format is not correct ，finding array members ." << std::endl;
					exit(1);
				}

				return jsonvalue(_file, re, ret.second,ARRAY);
			}
	
			if (count==1)
			{
				if (_file->_buffer[i] == ',')
				{
					tindex++;
				}
			}

			i++;
		}
	}

	//将字符串数据刷新到文件中
	void jsonvalue::Flush()
	{
		if (_file->_Is_Changed == true)
		{
			_file->Flush();
			_file->_Is_Changed = false;
		}
	
	}

	//检测后面的有效数据情况
	std::pair<long, vtype> jsonfile::CheckValidData(long index,char sign)
	{
		int count = 1;
		bool is_open = false;
		int value = 0;
		if (sign != 0)
		{
			is_open = true;
			if (sign == '[')
			{
				value = 0;
			}
			else if (sign == '{')
			{
				value = 1;
			}

		}
		for (int i = index; i < _back; i++)
		{
			if (is_open == true)
			{
				if (value == 0)
				{
					if (_buffer[i] == '[')
					{
						count++;
					}
					if (_buffer[i] == ']')
					{
						count--;
						if (count == 0)
						{
							break;
						}
					}
				
				}
				else if(value==1)
				{
					if (_buffer[i] == '{')
					{
						count++;
					}
					if (_buffer[i] == '}')
					{
						count--;
						if (count == 0)
						{
							break;
						}
					}
				}
			}
			//数组
			if (_buffer[i] == 91)
			{
				return std::make_pair(i+1, ARRAY);
			}

			//对象
			if (_buffer[i] == 123)
			{
				return std::make_pair(i+1, OBJECT);
			}

			//字符串
			if (_buffer[i] == '\"')
			{
				return std::make_pair(i+1, STRING);
			}

			//数字
			if (_buffer[i] == 45 || (_buffer[i] >= 48 && (_buffer[i] <= 57)))
			{
				bool is_float = false;
				int j = i;
				while (_buffer[j] == 45 || (_buffer[j] >= 48 && _buffer[j] <= 57) || _buffer[j] == '.')
				{
					if (_buffer[j] == '.')
					{
						is_float = true;
						break;
					}
					j++;
				}
				if (is_float == true)
				{
					return std::make_pair(i, FLOAT);
				}
				else
				{
					return std::make_pair(i, INT);
				}

			}

			//空值
			if (_buffer[i] >= 97 && _buffer[i] <= 122)
			{

				return std::make_pair(i, NUL);
			}
		}

		return std::make_pair(-1, INT);
	}

	jsonfile::jsonfile()
		:_file(NULL)
		,_Is_Changed(false)
	{
		_size = 4;
		_buffer += "null";
		_front = 0;
		_back = 4;
		_Is_Changed = true;
	}

	//构造函数，当对应path没有文件自动创建并初始化为null
	jsonfile::jsonfile(const char* path)
		:_Is_Changed(false)
	{
		_path = path;
		FILE* file = fopen(path, "rb+");
		if (file != NULL)
		{
			_file = file;
			fseek(_file, 0, SEEK_END); // 跳到文件末尾
			long size = ftell(_file);  // 获取当前位置（也就是总字节数）
			fseek(_file, 0, SEEK_SET); // 跳回文件开头
			if (size == 0)
			{
				std::cout << path << " is empty." << std::endl;
				exit(1);
			}
			_buffer.resize(size);
			size_t re = fread((char*)(_buffer.data()), sizeof(char), size, _file);
			if (re != size)
			{
				ferror(_file);
				exit(1);
			}
			_size = size;
			_front = 0;
			_back = size;
		}
		else
		{
			_file = fopen(path, "wb+");
			//char* str = (char*)"null";
			//fwrite(str, sizeof(char), strlen(str), _file);
			_size = 4;
			_buffer += "null";
			_front = 0;
			_back = 4;
			_Is_Changed = true;

		}
	}

	//获取根节点的json数据
	jsonvalue jsonfile::GetRootValue()
	{
		std::pair<long, vtype> ret = CheckValidData(0);
		long re = ret.first;

		if (re == -1)
		{
			std::cout << "json's format is not correct finding root value ." << std::endl;
			exit(1);
		}

		return jsonvalue(this, re, ret.second,NUL);
	}

	//将缓冲区数据刷新到文件中
	void jsonfile::Flush()
	{
		if (_file != NULL && _Is_Changed == true)
		{
			fclose(_file);
			fopen(_path.c_str(), "wb+");
			fwrite(_buffer.data(), sizeof(char), _size, _file);
			fclose(_file);
			_Is_Changed = false;
		}
	}

	//RAII，当对象被释放时自动将数据刷新到文件中(如果数据被更改了)
	jsonfile::~jsonfile()
	{
		if (_file != NULL&&_Is_Changed==true)
		{
			fclose(_file);
			fopen(_path.c_str(), "wb+");
			fwrite(_buffer.data(), sizeof(char), _size, _file);
			fclose(_file);
			_Is_Changed = false;
		}
	}
	
	

}