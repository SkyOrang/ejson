#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace ejson
{
	enum vtype
	{
		INT,//整数
		FLOAT,//浮点
		STRING,//字符串
		ARRAY,//数组
		OBJECT,//对象
		PAIR,//键值对
		NUL,//空值
		DEL,//删除
		EARRAY,//空数组
		EOBJECT,//空对象
		NIPAIR//新数字键值对
	};

	struct pair;

	class jsonfile;

	class jsonvalue
	{
	private:
		void ToString(std::string&);

		int ToInt();

		double ToFloat();

		void InsertString(const char* data);

		void InsertJString(const char* data);

		void InsertInt(const int& data);

		void InsertFloat(const double& data);
	
		void InsertNull();

		void DeleteString(char* data);

		void DeleteInt(char* data);

		void DeleteFloat(char* data);

		void DeleteArray(char* data);

		void DeleteNull(char* data);

		void DeleteObject(char* data);

		void CheckDelete();

		void DeleteArrayNode();

		void DeleteObjectNode();

		//插入操作
		void InsertNewArray();

		void InsertNewObject();

		void InsertNewPair(const char* key);

		void InsertNewIPair();

		size_t SeekEnd(char* ptr, char check);

	public:
		jsonvalue() = default;

		jsonvalue(jsonfile* fptr, long offset, vtype type,vtype outer);

		//类型判断
		vtype GetVType();

		//遍历
		bool IPairs(int& key, jsonvalue& value, int step_length = 1);

		bool Pairs(std::string& key, jsonvalue& object);

		//读取操作
		jsonvalue& operator>>(std::string& data);

		jsonvalue& operator>>(int& data);

		jsonvalue& operator>>(double& data);

		// 
		jsonvalue& operator<<(const int& data);

		jsonvalue& operator<<(const char* data);

		jsonvalue& operator<<(const char** data);

		jsonvalue& operator<<(const double& data);

		//删除,赋空值,插入新的array和object
		jsonvalue& operator<<(const vtype& data);

		jsonvalue& operator<<(const pair& data);

		//访问object
		jsonvalue operator[](const char* key);

		//访问array
		jsonvalue operator[](const int& index);

		jsonvalue SeekIPair(const int& nindex, const int& index);

		jsonvalue SeekPair(const char* index);

		void Flush();

	private:
		std::pair<long, vtype> _data;
		jsonfile* _file;
		vtype _outer;
	};

	struct pair
	{
		pair() = default;
		pair(const char* str)
			:k(str)
		{

		}
		std::string k;
		jsonvalue v;
	};

	struct ipair
	{
		int k=0;
		jsonvalue v;
	};

#define NPAIR(key) (const pair(key))//新键值对
#define JSTRING(key) ((const char**)(key))//jstring

	class jsonfile
	{
		friend class jsonvalue;
	private:
		std::pair<long, vtype> CheckValidData(long index, char sign=0);

	public:
		jsonfile();

		jsonfile(const char* path);

		jsonvalue GetRootValue();

		void Flush();

		~jsonfile();
	private:
		FILE* _file;
		std::string _buffer;
		std::string _path;
		long _front;
		long _back;
		long _size;
		bool _Is_Changed;
	};
}
