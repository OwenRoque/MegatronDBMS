#include "lrureplacer.h"

Memory::LRUReplacer::LRUReplacer(int num_frames) : num_frames(num_frames) {}

Memory::LRUReplacer::~LRUReplacer() = default;

bool Memory::LRUReplacer::victim(frame_id_t *frame_id)
{
    // if no victim page exists
    if (this->size() == 0) {
        return false;
    }

    // pick oldest frame (returns victim frame_id)
    *frame_id = deque.back();
    // remove it from victim list
    deque.pop_back();
    // and from the map
    map.remove(*frame_id);

    return true;
}

void Memory::LRUReplacer::pin(frame_id_t frame_id)
{
    // when the frame is not in the frame victim deque, then
    // there's nothing to do
    if (!map.contains(frame_id)) {
        return;
    }

    // the page is pinned (in use), so it cannot be evicted (chosen as a victim)
    auto it = map[frame_id];
    // remove it from the victim list
    map.remove(frame_id);
    deque.erase(static_cast<QList<frame_id_t>::const_iterator>(it));
}

void Memory::LRUReplacer::unpin(frame_id_t frame_id)
{
    // we can't insert a new page if the pool is full
    // we do not reinsert a victim frame if found also
    if (this->size() == num_frames || map.contains(frame_id)) {
        return;
    }

    // then, add it to victim list
    deque.push_front(frame_id);
    map[frame_id] = deque.begin();
}

size_t Memory::LRUReplacer::size()
{
    return deque.size();
}



