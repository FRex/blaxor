#pragma once

//base class to make a class non-copyable, the copy and assignment are deleted
//ctor is protected to make it only work as a base class, dtor is non-virtual
//but protected to prevent trying to delete a derived class via ptr to this one

class BlaNonCopyable
{
private:
    BlaNonCopyable(const BlaNonCopyable&) = delete;
    BlaNonCopyable& operator=(const BlaNonCopyable&) = delete;

protected:
    BlaNonCopyable() = default;
    ~BlaNonCopyable() = default;

};
