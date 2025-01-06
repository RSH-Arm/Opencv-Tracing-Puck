#ifndef _MULTI_BUFFERS_
#define _MULTI_BUFFERS_

#include <mutex>
#include <vector>
#include <thread>
#include <unordered_map>
#include <iostream>

namespace multi
{

	template<typename T, typename M>
	class MultiPipeline_RW
	{
	private:

		struct Data
		{
			T					data;
			bool record		=	true;
			std::vector<M>		result;
			std::vector<bool>	readers;

			Data(size_t listeners_) : result(listeners_, M()), readers(listeners_, false) {};
		};

	public:

		using MultiData	= typename std::vector<Data>;
		using iterator	= typename MultiData::iterator;

	private:

		MultiData _Buffer;

		iterator _Pointer;
		iterator _PointerRes;
		iterator _PointerEnd;

		std::size_t _Listeners;

		std::vector<iterator> _it;

	public:

		void start(size_t size_, size_t listeners_)
		{
			_Listeners = listeners_;
			_Buffer.resize(size_, Data(listeners_));

			_Pointer	= _Buffer.begin();
			_PointerRes	= _Buffer.begin();
			_PointerEnd	= _Buffer.end() - 1;

			_it.resize(listeners_, _Buffer.begin());
		}



		///********************************************************************************************************************///
		bool getRaw(T& data_, size_t id_thread)
		{
			if (_it[id_thread]->readers[id_thread] == true && _it[id_thread]->record == false) // ���� ����� ����� ���������
			{
				data_ = _it[id_thread]->data; // ��������� ������
				return true;
			}

			return false;
		}

		bool setResult(M& result_, size_t id_thread)
		{
			_it[id_thread]->result[id_thread]	= result_;
			_it[id_thread]->readers[id_thread]	= false;

			moveIt(_it[id_thread]);
			return true;
		}
		///********************************************************************************************************************///



		///********************************************************************************************************************///
		bool canRecord()
		{
			if (_Pointer->record == true)	// ����� ���������� ����� ������
				return true;
			else return false;				// �� ����� ���������� ����� ������
		}

		void setRaw(T&& t)
		{
			try {
				_Pointer->data		= t;		// ���������� ������
				_Pointer->record	= false;	// ����  - ������ � ����

				for (int i = 0; i != _Listeners; i++) _Pointer->readers[i] = true;	// ����� - ������ � ������

				moveIt(_Pointer);
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}
		///********************************************************************************************************************///



		///********************************************************************************************************************///
		bool getResult(T& data, std::vector<M>& result)
		{
			if (_PointerRes->record == false) // ������ �������� ����� �������
			{
				for (int i = 0; i != _Listeners; i++)
					if (_PointerRes->readers[i] == true) return false; // ���� ���� ���� ����� �� �������� ������

				data	= _PointerRes->data;
				result	= _PointerRes->result;

				return true;
			}
			return false;
		}

		void resultRelease()
		{
			_PointerRes->record = true;

			moveIt(_PointerRes);
		}
		///********************************************************************************************************************///



	private:

		void moveIt(iterator& it)
		{
			if (it == _PointerEnd)	it = _Buffer.begin();	// moved the iterator to begin
			else					it++;					// moved the iterator
		}


	};
}

#endif // _MULTI_BUFFERS_