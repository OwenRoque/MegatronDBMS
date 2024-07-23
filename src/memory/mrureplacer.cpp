#include "mrureplacer.h"

Memory::MRUReplacer::MRUReplacer(int num_frames) : numFrames(num_frames) {}

Memory::MRUReplacer::~MRUReplacer() = default;

bool Memory::MRUReplacer::victim(frame_id_t *frame_id)
{
    // if no victim page exists
    if (this->size() == 0) {
        return false;
    }

    // pick the most recently used frame (returns victim frame_id)
    *frame_id = frames.front();
    // remove it from victim list
    frames.pop_front();
    // and from the map
    frameMap.remove(*frame_id);

    return true;
}

void Memory::MRUReplacer::pin(frame_id_t frame_id)
{
    // when the frame is not in the frame victim deque, then
    // there's nothing to do
    if (!frameMap.contains(frame_id)) {
        return;
    }

    // the page is pinned (in use), so it cannot be evicted (chosen as a victim)
    auto it = frameMap[frame_id];
    // remove it from the victim list
    frameMap.remove(frame_id);
    frames.erase(static_cast<QList<frame_id_t>::const_iterator>(it));
}

void Memory::MRUReplacer::unpin(frame_id_t frame_id)
{
    // we can't insert a new page if the pool is full
    // we do not reinsert a victim frame if found also
    if (this->size() == numFrames || frameMap.contains(frame_id)) {
        return;
    }

    // then, add it to victim list
    frames.push_back(frame_id);
    frameMap[frame_id] = frames.end() - 1;
}

size_t Memory::MRUReplacer::size()
{
    return frames.size();
}

