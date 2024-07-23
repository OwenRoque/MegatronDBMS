#include "frame.h"

/// Frame Implementation

Memory::Frame::Frame() : page(nullptr), pinCount(0), dirtyBit(false) {}

QSharedPointer<Core::Page> Memory::Frame::getPage() const
{
    return page;
}

void Memory::Frame::setPage(QSharedPointer<Core::Page> page)
{
    this->page = page;
}

bool Memory::Frame::isDirty() const
{
    return dirtyBit;
}

int Memory::Frame::getPinCount() const
{
    return pinCount;
}

void Memory::Frame::setDirty(bool value)
{
    this->dirtyBit = value;
}

void Memory::Frame::increasePinCount()
{
    this->pinCount++;
}

void Memory::Frame::decreasePinCount()
{
    this->pinCount--;
}

void Memory::Frame::update(const UpdateParams& params)
{
    this->page = params.page;
    this->pinCount = params.pinCount;
    this->dirtyBit = params.dirtyBit;
}
