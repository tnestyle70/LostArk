#include "DataJson.h"

#include <bit>
#include <cstdint>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

using Client::CDataJson;
using Client::DATA_JSON_TYPE;
using Client::DATA_JSON_VALUE;

namespace
{
	unsigned g_Checks = 0;

	void Require(const bool result, const char* message)
	{
		++g_Checks;
		if (!result)
			throw std::runtime_error(message);
	}

	bool Equal(const DATA_JSON_VALUE& left, const DATA_JSON_VALUE& right)
	{
		if (left.Get_Type() != right.Get_Type())
			return false;
		switch (left.Get_Type())
		{
		case DATA_JSON_TYPE::BOOLEAN:
			return left.Get_Boolean() == right.Get_Boolean();
		case DATA_JSON_TYPE::NUMBER:
			return std::bit_cast<uint64_t>(left.Get_Number()) ==
				std::bit_cast<uint64_t>(right.Get_Number()) &&
				left.Was_FloatingPointToken() == right.Was_FloatingPointToken();
		case DATA_JSON_TYPE::STRING:
			return left.Get_String() == right.Get_String();
		case DATA_JSON_TYPE::ARRAY:
		{
			const auto& a = left.Get_Array();
			const auto& b = right.Get_Array();
			if (a.size() != b.size())
				return false;
			for (size_t index = 0; index < a.size(); ++index)
				if (!Equal(a[index], b[index]))
					return false;
			return true;
		}
		case DATA_JSON_TYPE::OBJECT:
		{
			const auto& a = left.Get_Object();
			const auto& b = right.Get_Object();
			if (a.size() != b.size() || left.Get_ObjectInsertionOrder() !=
				right.Get_ObjectInsertionOrder())
				return false;
			auto other = b.begin();
			for (const auto& [key, value] : a)
			{
				if (key != other->first || !Equal(value, other->second))
					return false;
				++other;
			}
			return true;
		}
		default:
			return true;
		}
	}

	DATA_JSON_VALUE Parse(const char* text)
	{
		DATA_JSON_VALUE value;
		std::string error;
		Require(CDataJson::Parse(text, value, error), "fixture parse");
		return value;
	}

	void CheckValueContracts()
	{
		DATA_JSON_VALUE::OBJECT fields;
		fields.emplace("z", DATA_JSON_VALUE::Boolean(true));
		fields.emplace("a", DATA_JSON_VALUE::Number(-0.0, true));
		auto sorted = DATA_JSON_VALUE::Object(fields);
		Require(sorted.Get_ObjectInsertionOrder() ==
			std::vector<std::string>({ "a", "z" }), "factory fallback order");
		auto ordered = DATA_JSON_VALUE::Object(fields, { "z", "a" });
		Require(ordered.Get_ObjectInsertionOrder() ==
			std::vector<std::string>({ "z", "a" }), "factory explicit order");
		auto repaired = DATA_JSON_VALUE::Object(fields, { "z" });
		Require(Equal(repaired, sorted), "factory count mismatch order repair");
		Require(ordered.Find("missing") == nullptr, "missing object member");
		Require(ordered.Find("z")->Get_Boolean(), "factory object child");

		DATA_JSON_VALUE::ARRAY elements;
		elements.push_back(DATA_JSON_VALUE::Number(1));
		elements.push_back(DATA_JSON_VALUE::String(std::string("x\0y", 3)));
		const DATA_JSON_VALUE values[] = {
			DATA_JSON_VALUE::Null(), DATA_JSON_VALUE::Boolean(true),
			DATA_JSON_VALUE::Number(-0.0, true),
			DATA_JSON_VALUE::String(std::string("x\0y", 3)),
			DATA_JSON_VALUE::Array(std::move(elements)), ordered
		};
		for (const auto& value : values)
		{
			if (!value.Is_String())
				Require(value.Get_String().empty(), "wrong string getter");
			if (!value.Is_Array())
				Require(value.Get_Array().empty(), "wrong array getter");
			if (!value.Is_Object())
			{
				Require(value.Get_Object().empty(), "wrong object getter");
				Require(value.Get_ObjectInsertionOrder().empty(), "wrong order getter");
				Require(value.Find("z") == nullptr, "wrong type Find");
			}

			for (const auto& previous : values)
			{
				auto assigned = previous;
				assigned = value;
				Require(Equal(assigned, value), "copy assignment type transition");
				assigned = assigned;
				Require(Equal(assigned, value), "self copy");
				auto input = value;
				assigned = std::move(input);
				Require(Equal(assigned, value), "move assignment type transition");
				input = DATA_JSON_VALUE::Boolean(false);
				Require(input.Is_Boolean() && !input.Get_Boolean(), "moved source reuse");
			}
			auto selfMoved = value;
			selfMoved = std::move(selfMoved);
			// Self-move need not preserve content; the value must remain reusable.
			(void)selfMoved.Get_String();
			(void)selfMoved.Get_Array();
			(void)selfMoved.Get_Object();
			(void)selfMoved.Get_ObjectInsertionOrder();
			(void)selfMoved.Find("missing");
			selfMoved = value;
			Require(Equal(selfMoved, value), "self moved value reuse");
		}

		auto source = Parse(R"({"b":{"inner":[1,2,{"text":"long-owned-string-0123456789"}]},"a":true})");
		auto snapshot = source;
		auto destination = std::move(source);
		Require(Equal(destination, snapshot), "object move destination");
		Require(source.Is_Object(), "moved object type retained");
		Require(source.Get_Object().empty(), "moved object map empty");
		Require(source.Get_ObjectInsertionOrder().empty(), "moved object order empty");
		Require(source.Find("b") == nullptr, "moved object Find");
		auto emptyCopy = source;
		Require(emptyCopy.Is_Object() && emptyCopy.Get_Object().empty(), "moved object copy");
		auto assignedEmpty = DATA_JSON_VALUE::String("replace");
		assignedEmpty = source;
		Require(assignedEmpty.Is_Object() && assignedEmpty.Get_Object().empty(), "moved object assignment");
		source = snapshot;
		Require(Equal(source, snapshot), "moved object deep copy reuse");
		Require(source.Find("b") != snapshot.Find("b"), "deep copy node independence");
		Require(&source.Find("b")->Get_Object() != &snapshot.Find("b")->Get_Object(),
			"deep copy nested object independence");
		snapshot = DATA_JSON_VALUE::Null();
		Require(source.Find("b")->Find("inner")->Get_Array().size() == 3,
			"copy survives original replacement");
	}

	void CheckStrongCopyAlias()
	{
		auto value = Parse(R"({"nested":{"x":[1,{"y":"owned-text"}]},"other":false})");
		auto expected = *value.Find("nested");
		value = *value.Find("nested");
		Require(Equal(value, expected), "copy assignment from owned descendant");
	}

#ifdef _DEBUG
	long g_AllocationsUntilFailure = -1;
	bool g_DeniedAllocation = false;

	int AllocationFailureHook(int kind, void*, size_t, int blockType,
		long, const unsigned char*, int)
	{
		if (kind == _HOOK_ALLOC && blockType != _CRT_BLOCK &&
			g_AllocationsUntilFailure >= 0)
		{
			if (g_AllocationsUntilFailure-- == 0)
			{
				g_DeniedAllocation = true;
				return FALSE;
			}
		}
		return TRUE;
	}

	class ScopedAllocationFailure final
	{
	public:
		explicit ScopedAllocationFailure(const long index)
		{
			g_AllocationsUntilFailure = index;
			g_DeniedAllocation = false;
			m_Previous = _CrtSetAllocHook(AllocationFailureHook);
		}
		~ScopedAllocationFailure()
		{
			_CrtSetAllocHook(m_Previous);
			g_AllocationsUntilFailure = -1;
		}
	private:
		_CRT_ALLOC_HOOK m_Previous = nullptr;
	};

	unsigned CheckCopyFailureRollback()
	{
		const auto source = Parse(R"({"z":[1,2,{"long":"abcdefghijklmnopqrstuvwxyz0123456789"}],"a":{"nested":true}})");
		const auto original = Parse(R"({"keep":{"saved":[false,99.0]},"name":"previous-value"})");
		unsigned deniedCases = 0;
		for (long index = 0; index < 256; ++index)
		{
			auto target = original;
			bool threw = false;
			bool denied = false;
			{
				ScopedAllocationFailure failure(index);
				try
				{
					target = source;
				}
				catch (const std::bad_alloc&)
				{
					threw = true;
				}
				denied = g_DeniedAllocation;
			}
			if (!denied)
			{
				Require(!threw && Equal(target, source), "copy succeeds after failure sweep");
				Require(deniedCases != 0, "allocation fault injection exercised");
				return deniedCases;
			}
			Require(threw, "denied allocation throws bad_alloc");
			Require(Equal(target, original), "failed copy assignment preserves destination");
			++deniedCases;
		}
		throw std::runtime_error("allocation failure sweep did not reach success");
	}
#endif
}

int main(int argc, char** argv)
{
	try
	{
		const bool requireStrongCopy = argc == 2 &&
			std::string(argv[1]) == "--require-strong-copy";
		CheckValueContracts();
		unsigned allocationFailures = 0;
		if (requireStrongCopy)
		{
			CheckStrongCopyAlias();
#ifdef _DEBUG
			allocationFailures = CheckCopyFailureRollback();
#endif
		}
		std::cout << "checks=" << g_Checks
			<< " allocationFailures=" << allocationFailures
			<< " iteratorDebugLevel=" << _ITERATOR_DEBUG_LEVEL
			<< " strongCopy=" << requireStrongCopy << " failures=0\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "checks=" << g_Checks << " error=" << error.what() << '\n';
		return 1;
	}
}
