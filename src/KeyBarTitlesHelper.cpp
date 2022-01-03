#include <cstring>
#include <farplug-wide.h>
#include "KeyBarTitlesHelper.h"

KeyBarTitlesHelper::KeyBarTitlesHelper() :
    m_keyBar { }
{
    std::memset(&m_keyBar, 0, sizeof(m_keyBar));
}

KeyBarTitlesHelper::~KeyBarTitlesHelper()
{
    size_t i;
    for (i = 0; i < ARRAYSIZE(m_keyBar.Titles); i++)
        delete[] m_keyBar.Titles[i];
    for (i = 0; i < ARRAYSIZE(m_keyBar.CtrlTitles); i++)
        delete[] m_keyBar.CtrlTitles[i];
    for (i = 0; i < ARRAYSIZE(m_keyBar.AltTitles); i++)
        delete[] m_keyBar.AltTitles[i];
    for (i = 0; i < ARRAYSIZE(m_keyBar.ShiftTitles); i++)
        delete[] m_keyBar.ShiftTitles[i];
    for (i = 0; i < ARRAYSIZE(m_keyBar.CtrlShiftTitles); i++)
        delete[] m_keyBar.CtrlShiftTitles[i];
    for (i = 0; i < ARRAYSIZE(m_keyBar.AltShiftTitles); i++)
        delete[] m_keyBar.AltShiftTitles[i];
    for (i = 0; i < ARRAYSIZE(m_keyBar.CtrlAltTitles); i++)
        delete[] m_keyBar.CtrlAltTitles[i];
    std::memset(&m_keyBar, 0, sizeof(m_keyBar));
}

KeyBarTitles& KeyBarTitlesHelper::getKeyBar()
{
    return m_keyBar;
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setKey(int index,
                                               KeyBarTitlesHelper::KeyType type,
                                               const std::wstring& hint)
{
    switch(type)
    {
    case KeyType::NORMAL:
        setNormalKey(index, hint);
        break;
    case KeyType::ALT:
        setAltKey(index, hint);
        break;
    case KeyType::ALT_SHIFT:
        setAltShiftKey(index, hint);
        break;
    case KeyType::CTRL:
        setCtrlKey(index, hint);
        break;
    case KeyType::CTRL_ALT:
        setCtrlAltKey(index, hint);
        break;
    case KeyType::CTRL_SHIFT:
        setCtrlShiftKey(index, hint);
        break;
    case KeyType::SHIFT:
        setShiftKey(index, hint);
        break;
    }
    return *this;
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setNormalKey(int index,
                                                     const std::wstring& hint)
{
    return setKeyGeneric(index, m_keyBar.Titles, hint);
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setCtrlKey(int index,
                                                   const std::wstring& hint)
{
    return setKeyGeneric(index, m_keyBar.CtrlTitles, hint);
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setAltKey(int index,
                                                  const std::wstring& hint)
{
    return setKeyGeneric(index, m_keyBar.AltTitles, hint);
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setShiftKey(int index,
                                                    const std::wstring& hint)
{
    return setKeyGeneric(index, m_keyBar.ShiftTitles, hint);
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setCtrlShiftKey(int index,
                                                        const std::wstring& hint)
{
    return setKeyGeneric(index, m_keyBar.CtrlShiftTitles, hint);
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setAltShiftKey(int index,
                                                       const std::wstring& hint)
{
    return setKeyGeneric(index, m_keyBar.AltShiftTitles, hint);
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setCtrlAltKey(int index,
                                                      const std::wstring& hint)
{
    return setKeyGeneric(index, m_keyBar.CtrlAltTitles, hint);
}

KeyBarTitlesHelper& KeyBarTitlesHelper::setKeyGeneric(int index, wchar_t* key[],
                                                      const std::wstring& hint)
{
    // ignore invalid index; all hiht arrays have equal length
    if ((index < 0) || (index >= int(ARRAYSIZE(m_keyBar.Titles))))
        return *this;
    if (key[index]) delete[] key[index];
    key[index] = new wchar_t[hint.size() + 1];
    std::memset(key[index], 0, sizeof(wchar_t) * (hint.size() + 1));
    std::copy(hint.begin(), hint.end(), key[index]);
    return *this;
}
