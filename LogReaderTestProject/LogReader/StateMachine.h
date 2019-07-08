/**
–еализаци€ простейшего конечного автомата
*/

#pragma once

#include "helpers.h"

//////////////////////////////////////////////////////////////////////////
//CStateMachine
//  ласс конечного автомата дл€ поиска подстроки в строке по заданному шаблону.
// —пец. символы:
// '*' - последовательность любых символов неограниченной длины
// '? - один любой символ
class CStateMachine
{
public:
	// конструктор  ј. «адает шаблон сравнени€ pPattern и флаг учета регистра bCaseSensitive
	CStateMachine(const char *pPattern, bool bCaseSensitive)
	{
		if (strlen(pPattern))
		{
			CState* pCurState = AddInitialState();
			CState* pReverseState = NULL;
			const char* pPatternChar = pPattern;
			while (*pPatternChar) {
				if (!pCurState->IsAsterisk() || *pPatternChar != '*')  // исключаем избыточные *, пренебрегаем случа€ми типа "**"
				{
					CState* pNewState = AddState(*pPatternChar, bCaseSensitive);
					pCurState->SetForwardState(pNewState);
					if (pReverseState)
						pNewState->SetReverseState(pReverseState);
					if (pNewState->IsAsterisk())
						pReverseState = pNewState;

					pCurState = pNewState;
				}
				++pPatternChar;
			}

			assert(pCurState);
			if (pCurState)
				pCurState->SetForwardState(AddFinalState());
		}
	}
	~CStateMachine()
	{
		for (UINT n = 0; n < m_statesArray.GetSize(); ++n)
			delete m_statesArray[n];

		m_statesArray.Clear();
	}

	/// поиск совпадени€ шаблона в строке pString
	bool Match(const char *pString)
	{
		const size_t stringLen = strlen(pString);
		if (stringLen == 0)
			return false;

		if (m_statesArray.GetSize() < 3) // минимально возможный размер - 3 состо€ни€ (init, operational, final)
			return false;

		CState* pCurState = GetInitialState();
		const char* pCurChar = pString;
		const char* pReverseChar = NULL;

		while (*pCurChar)
		{
			CState* pNextState = GetNextState(*pCurChar, pCurState);
			if (pNextState == NULL) {
				if(pReverseChar && pCurState->GetReverseState()) {
					// возвращаемс€ к последнему состо€нию '*' (не используем рекурсию)
					pCurState = pCurState->GetReverseState();
					pCurChar = pReverseChar;
				}
				else {
					pCurState = GetInitialState();
					break;
				}
			}
			else
				pCurState = pNextState;

			++pCurChar;

			if (pCurState->GetForwardState()->IsFinal() && 
				(*pCurChar == '\0' || pCurState->IsAsterisk()) ) 
			{
				// соответствие шаблону, переходим в финальное состо€ние
				pCurState = pCurState->GetForwardState();
				break;
			}

			if(pCurState->IsAsterisk())
				pReverseChar = pCurChar; // позици€ дл€ возврата
		}

		assert(pCurState);

		return pCurState->IsFinal();
	}

protected:	
	///  ласс, оперирующий состо€нием  ј
	class CState
	{
	public:
		// типы состо€ний
		enum StateType {
			stInitial,		// начальное
			stFinal,		// конечное
			stOperational	// рабочее
		};

		CState(StateType st) : m_stateType(st)
		{
			Init();
			m_stateType = st;
		}

		CState(const char chPattern, bool bCaseSensitive)
		{
			Init();
			m_chPattern = chPattern;
			m_bCaseSensitive = bCaseSensitive;
		}

		void SetForwardState(CState* pState)
		{
			assert(pState);
			assert(!m_pForwardState);
			m_pForwardState = pState;
		}
		CState* GetForwardState() const { return m_pForwardState; }

		CState* GetReverseState() const { return m_pReverseState; }

		void SetReverseState(CState* pState)
		{
			assert(pState);
			assert(!m_pReverseState);
			m_pReverseState = pState;
		}

		bool IsFinal() const { return m_stateType == stFinal; }
		bool IsInitial() const { return m_stateType == stInitial; }
		bool IsAsterisk() const { return m_chPattern == '*'; }

		// совпадение с указанным символом
		bool IsMatch(const char ch) const
		{
			if (!IsFinal()) {
				switch (m_chPattern)
				{
				case '*':
				case '?':
					return true;

				default:
					{
						return m_bCaseSensitive
							? m_chPattern == ch
							: toupper(m_chPattern) == toupper(ch);
					}
				}
			}
			return false;
		}

	private:
		StateType m_stateType;
		char m_chPattern;
		CState* m_pForwardState;
		CState* m_pReverseState;
		bool m_bCaseSensitive;

		void Init()
		{
			m_stateType = stOperational;
			m_chPattern = 0;
			m_pForwardState = NULL;
			m_pReverseState = NULL;
			m_bCaseSensitive = false;
		}
	}; // end of class CState

protected:
	CSimpleDynArray<CState*> m_statesArray;

	CState* AddState(const char ch, bool bCaseSensitive)
	{
		CState* pState = new CState(ch, bCaseSensitive);
		m_statesArray.Add(pState);
		return pState;
	}

	CState* AddInitialState()
	{
		CState* pState = new CState(CState::stInitial);
		m_statesArray.Add(pState);
		return pState;
	}

	CState* AddFinalState()
	{
		CState* pState = new CState(CState::stFinal);
		m_statesArray.Add(pState);
		return pState;
	}

	CState* GetInitialState() 
	{
		assert(m_statesArray.GetSize());
		if (m_statesArray.GetSize() > 0)
			return m_statesArray[0];
		return NULL;
	}

	CState* GetFirstState() { return GetInitialState()->GetForwardState(); }

	CState* GetNextState(const char ch, CState* pState)
	{
		if (!pState->IsFinal() && ch!='\0')
		{
			CState* pForwardState = pState->GetForwardState();
			assert(pForwardState);
			if (pForwardState->IsMatch(ch))
				return pForwardState; // move to next
			else if (pState->IsAsterisk())
				return pState; // stay in current
		}
		return NULL;
	}
};
//CStateMachine
//////////////////////////////////////////////////////////////////////////
