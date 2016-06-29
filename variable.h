#pragma once


namespace Bare
{
	template <typename DType>
	class Variable
	{
	public:
		virtual ~Variable()
		{}

		virtual DType operator()() = 0;
		virtual void flag() = 0;
	};
}
