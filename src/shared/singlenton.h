//=================================================================================================
//===
//=== singlenton.h
//===
//=== Copyright (c) 2020 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//===
//=================================================================================================

#pragma once

#define SINGLETON(singletonclass) \
	public: \
		virtual ~singletonclass(); \
		static singletonclass& instance() { static singletonclass Singleton; return Singleton; } \
	private: \
		singletonclass(); \
		singletonclass(const singletonclass&); \
		singletonclass& operator=(singletonclass&); \
