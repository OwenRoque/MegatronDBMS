#include "frame.h"

/// Frame Interface Implementation

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

/// LRU Frame Implementation

Memory::LRUFrame::LRUFrame() : Frame() {}

void Memory::LRUFrame::update(const UpdateParams& params)
{
    this->page = params.page;
    this->pinCount = params.pinCount;
    this->dirtyBit = params.dirtyBit;
}

/// MRU Frame Implementation

Memory::MRUFrame::MRUFrame() : Frame() {}

void Memory::MRUFrame::update(const UpdateParams &params)
{
    this->page = params.page;
    this->pinCount = params.pinCount;
    this->dirtyBit = params.dirtyBit;
}

/// CLOCK Frame Implementation

Memory::ClockFrame::ClockFrame() : Frame(), refBit(false) {}

void Memory::ClockFrame::update(const UpdateParams& params)
{
    this->page = params.page;
    this->pinCount = params.pinCount;
    this->dirtyBit = params.dirtyBit;
    this->refBit = params.refBit;
}

// void Memory::LRUKFrame::update(Core::Page *page, int pinCount, bool dirtyBit, const QList<int> &accessHistory)
// {
//     Frame::update(page, pinCount, dirtyBit);
//     this->accessHistory = accessHistory;
// }
