#include <cstring>
#include "KeyBarTitlesHelper.h"

#include "plugin.hpp"

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
}

KeyBarTitles& KeyBarTitlesHelper::getKeyBar()
{
    return m_keyBar;
}

void KeyBarTitlesHelper::setKey(int index, KeyBarTitlesHelper::KeyType type, const std::wstring& name)
{
    switch(type)
    {
    case KeyType::NORMAL:
        setNormalKey(index, name);
        break;
    case KeyType::ALT:
        setAltKey(index, name);
        break;
    case KeyType::ALT_SHIFT:
        setAltShiftKey(index, name);
        break;
    case KeyType::CTRL:
        setCtrlKey(index, name);
        break;
    case KeyType::CTRL_ALT:
        setCtrlAltKey(index, name);
        break;
    case KeyType::CTRL_SHIFT:
        setCtrlShiftKey(index, name);
        break;
    case KeyType::SHIFT:
        setShiftKey(index, name);
        break;
    }
}

void KeyBarTitlesHelper::setNormalKey(int index, const std::wstring& name)
{
    setKeyGeneric(index, m_keyBar.Titles, name);
}

void KeyBarTitlesHelper::setAltKey(int index, const std::wstring& name)
{
    setKeyGeneric(index, m_keyBar.AltTitles, name);
}

void KeyBarTitlesHelper::setShiftKey(int index, const std::wstring& name)
{
    setKeyGeneric(index, m_keyBar.ShiftTitles, name);
}

void KeyBarTitlesHelper::setCtrlShiftKey(int index, const std::wstring& name)
{
    setKeyGeneric(index, m_keyBar.CtrlShiftTitles, name);
}

void KeyBarTitlesHelper::setAltShiftKey(int index, const std::wstring& name)
{
    setKeyGeneric(index, m_keyBar.AltShiftTitles, name);
}

void KeyBarTitlesHelper::setCtrlAltKey(int index, const std::wstring& name)
{
    setKeyGeneric(index, m_keyBar.CtrlAltTitles, name);
}

void KeyBarTitlesHelper::setCtrlKey(int index, const std::wstring& name)
{
    setKeyGeneric(index, m_keyBar.CtrlTitles, name);
}

void KeyBarTitlesHelper::setKeyGeneric(int index, wchar_t* key[], const std::wstring& name)
{
    if (key[index]) delete[] key[index];
    key[index] = new wchar_t[name.size() + 1];
    std::memset(key[index], 0, sizeof(wchar_t) * (name.size() + 1));
    std::copy(name.begin(), name.end(), key[index]);
}
